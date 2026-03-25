#include "app_lab_5_2_task2.h"
#include "app_lab_5_2_task1.h"
#include "task_config.h"
#include "dd_actuator_analog/dd_actuator_analog.h"
#include "dd_led/dd_led.h"
#include "ed_potentiometer/ed_potentiometer.h"
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

// ── Shared snapshot ───────────────────────────────────────────────────────
App52Snapshot_t    g_snap52       = {0, 0, 0, 0, 0, false, false};
SemaphoreHandle_t  g_snap52_mutex = NULL;

// ── Median filter ─────────────────────────────────────────────────────────
typedef struct {
    int buf[MEDIAN_WINDOW];
    int head;
    int count;
} MedianBuf_t;

static void median_push(MedianBuf_t *m, int val) {
    m->buf[m->head] = val;
    m->head = (m->head + 1) % MEDIAN_WINDOW;
    if (m->count < MEDIAN_WINDOW) m->count++;
}

static int median_get(const MedianBuf_t *m) {
    int tmp[MEDIAN_WINDOW];
    int n = m->count;
    for (int i = 0; i < n; i++) tmp[i] = m->buf[i];
    // insertion sort
    for (int i = 1; i < n; i++) {
        int key = tmp[i], j = i - 1;
        while (j >= 0 && tmp[j] > key) { tmp[j+1] = tmp[j]; j--; }
        tmp[j+1] = key;
    }
    return tmp[n / 2];
}

// ── Saturation clamp ──────────────────────────────────────────────────────
static inline int clamp(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

void app_lab_5_2_task2_init() {
    g_snap52_mutex = xSemaphoreCreateMutex();
}

void app_lab_5_2_task2(void *pvParameters) {
    (void)pvParameters;

    MedianBuf_t mbuf    = {};
    int         wma     = 0;
    bool        wma_ready = false;
    int         ramped  = 0;   // current applied PWM (ramp tracks this)
    bool        prev_alert = false;

    TickType_t last_wake = xTaskGetTickCount();

    for (;;) {
        // 1. Read analog input from potentiometer
        ed_potentiometer_loop();
        int raw = map(ed_potentiometer_get_raw(), 0, 1023, 0, 255);

        // 2. Saturation: clamp to valid physical range
        int sat = clamp(raw, PWM_MIN, PWM_MAX);

        // 3. Median filter: suppress impulse noise
        median_push(&mbuf, sat);
        int med = median_get(&mbuf);

        // 4. Weighted moving average: smooth out fluctuations
        if (!wma_ready) { wma = sat; wma_ready = true; }
        else wma = (WMA_ALPHA * med + (100 - WMA_ALPHA) * wma) / 100;

        // 5. Ramp limiter: limit rate of change to protect motor
        if (wma > ramped + RAMP_STEP)       ramped += RAMP_STEP;
        else if (wma < ramped - RAMP_STEP)  ramped -= RAMP_STEP;
        else                                 ramped  = wma;
        ramped = clamp(ramped, PWM_MIN, PWM_MAX);

        // 6. Apply to hardware
        dd_actuator_analog_set_requested_pwm(ramped);
        dd_actuator_analog_apply();
        int applied = dd_actuator_analog_get_applied_pwm();

        // 7. Hysteresis alert
        bool alert = false;
        if (!prev_alert && applied > ALERT_HIGH)  alert = true;
        else if (prev_alert && applied >= ALERT_LOW) alert = true;
        prev_alert = alert;

        bool at_limit = (applied >= PWM_MAX);

        // 8. Publish snapshot
        if (xSemaphoreTake(g_snap52_mutex, portMAX_DELAY) == pdTRUE) {
            g_snap52.raw       = raw;
            g_snap52.saturated = sat;
            g_snap52.median    = med;
            g_snap52.wma       = wma;
            g_snap52.ramped    = applied;
            g_snap52.alert     = alert;
            g_snap52.at_limit  = at_limit;
            xSemaphoreGive(g_snap52_mutex);
        }

        // 9. Update LEDs
        if (alert) {
            dd_led_turn_on();    // red ON  - overload alert
            dd_led_1_turn_off(); // green OFF
            dd_led_2_turn_on();  // yellow ON - limit / alert
        } else if (at_limit) {
            dd_led_turn_off();   // red OFF
            dd_led_1_turn_off(); // green OFF
            dd_led_2_turn_on();  // yellow ON - at limit
        } else {
            dd_led_turn_off();   // red OFF
            dd_led_1_turn_on();  // green ON - OK
            dd_led_2_turn_off(); // yellow OFF
        }
        dd_led_apply();

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(COND_PERIOD_MS));
    }
}