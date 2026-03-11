#include "app_lab_4_2_task_2.h"
#include "task_config.h"
#include "dd_sns_temperature/dd_sns_temperature.h"
#include "dd_sns_dht/dd_sns_dht.h"
#include "dd_sns_gas/dd_sns_gas.h"
#include "dd_led/dd_led.h"
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

// ── Shared state definitions (read by task_rep_4_2 via task_config.h) ────────
SensorData_t       g_data_s1 = {0, 0, 0, 0, false};
SemaphoreHandle_t  g_mutex_s1 = NULL;

SensorData_t       g_data_s2 = {0, 0, 0, 0, false};
SemaphoreHandle_t  g_mutex_s2 = NULL;

SensorData_t       g_data_s3 = {0, 0, 0, 0, false};
SemaphoreHandle_t  g_mutex_s3 = NULL;

// ── Median filter ─────────────────────────────────────────────────────────────
// Circular buffer that keeps the last MEDIAN_WINDOW saturated samples.
typedef struct {
    int  buf[MEDIAN_WINDOW];
    int  head;    // index of the next slot to write
    int  count;   // number of valid samples (0 to MEDIAN_WINDOW)
} MedianBuf_t;

static void median_push(MedianBuf_t *m, int val) {
    m->buf[m->head] = val;
    m->head = (m->head + 1) % MEDIAN_WINDOW;
    if (m->count < MEDIAN_WINDOW) m->count++;
}

// Returns the median of the current contents (insertion sort on a local copy).
static int median_get(const MedianBuf_t *m) {
    int tmp[MEDIAN_WINDOW];
    int n = m->count;
    for (int i = 0; i < n; i++) tmp[i] = m->buf[i];
    // insertion sort
    for (int i = 1; i < n; i++) {
        int key = tmp[i], j = i - 1;
        while (j >= 0 && tmp[j] > key) { tmp[j + 1] = tmp[j]; j--; }
        tmp[j + 1] = key;
    }
    return tmp[n / 2];
}

// ── Helpers ───────────────────────────────────────────────────────────────────
static inline int clamp_val(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

// ── Init ──────────────────────────────────────────────────────────────────────
void task_4_2_init() {
    g_mutex_s1 = xSemaphoreCreateMutex();
    g_mutex_s2 = xSemaphoreCreateMutex();
    g_mutex_s3 = xSemaphoreCreateMutex();
}

// ── Task 2 – Signal Conditioning (50 ms, priority 2) ─────────────────────────
//
// Per sensor per cycle:
//   1. Read raw °C from driver (thread-safe getter)
//   2. Saturate  – clamp to valid physical range
//   3. Median    – 5-point sliding-window median (salt-and-pepper suppression)
//   4. WMA       – IIR weighted moving average (smooth trend)
//   5. Alert     – hysteresis threshold on WMA value
//   6. Publish   – write snapshot under mutex for task_rep_4_2
void task_cond_4_2(void *pvParameters) {
    (void)pvParameters;

    MedianBuf_t mbuf_s1 = {};    // zero-init (count = 0)
    MedianBuf_t mbuf_s2 = {};
    MedianBuf_t mbuf_s3 = {};

    int  wma_s1 = 0;
    bool wma_s1_ready = false;   // true after first sample seeds the IIR
    int  wma_s2 = 0;
    bool wma_s2_ready = false;
    long wma_s3 = 0;
    bool wma_s3_ready = false;

    // 10 ms offset – task_acq_4_2 has already refreshed the sensors
    vTaskDelay(pdMS_TO_TICKS(10));
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        // ── S1 – Potentiometer (analog) ──────────────────────────────────────
        int raw1 = dd_sns_temperature_get_celsius();
        int sat1 = clamp_val(raw1, SAT_MIN_S1, SAT_MAX_S1);

        median_push(&mbuf_s1, sat1);
        int med1 = median_get(&mbuf_s1);

        if (!wma_s1_ready) { wma_s1 = sat1; wma_s1_ready = true; }
        else wma_s1 = (WMA_ALPHA * med1 + (100 - WMA_ALPHA) * wma_s1) / 100;

        if (xSemaphoreTake(g_mutex_s1, portMAX_DELAY) == pdTRUE) {
            // Hysteresis on WMA: turn ON above HIGH, turn OFF below LOW
            bool prev_alert = g_data_s1.alert;
            bool new_alert  = prev_alert ? (wma_s1 >= ALERT_LOW_S1)
                                         : (wma_s1 > ALERT_HIGH_S1);
            g_data_s1.raw       = raw1;
            g_data_s1.saturated = sat1;
            g_data_s1.median    = med1;
            g_data_s1.weighted  = wma_s1;
            g_data_s1.alert     = new_alert;
            xSemaphoreGive(g_mutex_s1);
        }

        // ── S2 – DHT22 (digital) ─────────────────────────────────────────────
        int raw2 = dd_sns_dht_get_celsius();
        int sat2 = clamp_val(raw2, SAT_MIN_S2, SAT_MAX_S2);

        median_push(&mbuf_s2, sat2);
        int med2 = median_get(&mbuf_s2);

        if (!wma_s2_ready) { wma_s2 = sat2; wma_s2_ready = true; }
        else wma_s2 = (WMA_ALPHA * med2 + (100 - WMA_ALPHA) * wma_s2) / 100;

        if (xSemaphoreTake(g_mutex_s2, portMAX_DELAY) == pdTRUE) {
            bool prev_alert = g_data_s2.alert;
            bool new_alert  = prev_alert ? (wma_s2 >= ALERT_LOW_S2)
                                         : (wma_s2 > ALERT_HIGH_S2);
            g_data_s2.raw       = raw2;
            g_data_s2.saturated = sat2;
            g_data_s2.median    = med2;
            g_data_s2.weighted  = wma_s2;
            g_data_s2.alert     = new_alert;
            xSemaphoreGive(g_mutex_s2);
        }

        // ── S3 – MQ-2 Gas Sensor (analog) ─────────────────────────────────────
        int raw3 = dd_sns_gas_get_ppm();
        int sat3 = clamp_val(raw3, SAT_MIN_S3, SAT_MAX_S3);

        median_push(&mbuf_s3, sat3);
        int med3 = median_get(&mbuf_s3);

        if (!wma_s3_ready) { wma_s3 = sat3; wma_s3_ready = true; }
        else wma_s3 = ((long)WMA_ALPHA * med3 + (long)(100 - WMA_ALPHA) * wma_s3) / 100;

        if (xSemaphoreTake(g_mutex_s3, portMAX_DELAY) == pdTRUE) {
            bool prev_alert = g_data_s3.alert;
            bool new_alert  = prev_alert ? (wma_s3 >= ALERT_LOW_S3)
                                         : (wma_s3 > ALERT_HIGH_S3);
            g_data_s3.raw       = raw3;
            g_data_s3.saturated = sat3;
            g_data_s3.median    = med3;
            g_data_s3.weighted  = (int)wma_s3;
            g_data_s3.alert     = new_alert;
            xSemaphoreGive(g_mutex_s3);
        }

        // ── LED indicator ────────────────────────────────────────────────────
        // RED = any alert | YELLOW = gas alert | GREEN = all OK
        bool any_alert = g_data_s1.alert || g_data_s2.alert || g_data_s3.alert;
        if (any_alert) {
            dd_led_turn_on();    // RED
            dd_led_1_turn_off(); // GREEN
        } else {
            dd_led_turn_off();   // RED
            dd_led_1_turn_on();  // GREEN
        }
        if (g_data_s3.alert) dd_led_2_turn_on();  // YELLOW = gas alert
        else                  dd_led_2_turn_off();
        dd_led_apply();

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(ACQ_PERIOD_MS));
    }
}