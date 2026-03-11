#include "app_lab_4_2_task_3.h"
#include "task_config.h"
#include "dd_sns_dht/dd_sns_dht.h"
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>

// Task 3 – Display & Reporting  (500 ms, priority 1)
//
// stdout is tee'd: LCD (2×16) rows receive the first two lines; Serial
// receives the full structured report that follows.
//
// LCD row 0:  S1 raw + WMA + alert status
// LCD row 1:  S2 raw + WMA + alert status
// Serial:     full pipeline report for both sensors
void task_rep_4_2(void *pvParameters) {
    (void)pvParameters;

    for (;;) {
        // ── Snapshot S1 ──────────────────────────────────────────────────────
        SensorData_t s1 = {0, 0, 0, 0, false};
        if (xSemaphoreTake(g_mutex_s1, portMAX_DELAY) == pdTRUE) {
            s1 = g_data_s1;
            xSemaphoreGive(g_mutex_s1);
        }

        // ── Snapshot S2 ──────────────────────────────────────────────────────
        SensorData_t s2 = {0, 0, 0, 0, false};
        if (xSemaphoreTake(g_mutex_s2, portMAX_DELAY) == pdTRUE) {
            s2 = g_data_s2;
            xSemaphoreGive(g_mutex_s2);
        }

        // ── Snapshot S3 ──────────────────────────────────────────────────────
        SensorData_t s3 = {0, 0, 0, 0, false};
        if (xSemaphoreTake(g_mutex_s3, portMAX_DELAY) == pdTRUE) {
            s3 = g_data_s3;
            xSemaphoreGive(g_mutex_s3);
        }

        int humidity = dd_sns_dht_get_humidity();

        // 2-char status for LCD
        const char *lcd_st1 = s1.alert ? "!!" : "OK";
        const char *lcd_st3 = s3.alert ? "!!" : "OK";

        // ── LCD rows (exactly 16 chars each) + Serial via tee'd stdout ───────
        // Format: "S1 R:NNN W:NNN??" → 5 + 3 + 4 + 3 + 2 = 16 chars max
        printf("\x1b");                                               // clear LCD
        printf("S1 R:%3d W:%3d%2s\n", s1.raw, s1.weighted, lcd_st1);  // LCD row 0
        printf("S3 R:%3d W:%3d%2s\n", s3.raw, s3.weighted, lcd_st3);  // LCD row 1

        // ── Serial-only structured report ────────────────────────────────────
        printf("\r******************************\n");
        printf("\r Lab 4.2  Signal Acq.\n");
        printf("\r******************************\n");
        printf("\r [S1 - ANALOG (Potentiometer)]\n");
        printf("\r  Raw:        %4d C\n",   s1.raw);
        printf("\r  Saturated:  %4d C  (range %d..%d)\n",
               s1.saturated, SAT_MIN_S1, SAT_MAX_S1);
        printf("\r  Median:     %4d C  (window %d)\n",
               s1.median, MEDIAN_WINDOW);
        printf("\r  WMA:        %4d C  (alpha %d%%)\n",
               s1.weighted, WMA_ALPHA);
        printf("\r  Thresholds: HI=%d  LO=%d C\n",
               ALERT_HIGH_S1, ALERT_LOW_S1);
        printf("\r  STATUS:     %s\n", s1.alert ? "!! ALERT !!" : "OK");
        printf("\r------------------------------\n");
        printf("\r [S2 - DIGITAL (DHT22)]\n");
        printf("\r  Raw:        %4d C\n",   s2.raw);
        printf("\r  Saturated:  %4d C  (range %d..%d)\n",
               s2.saturated, SAT_MIN_S2, SAT_MAX_S2);
        printf("\r  Median:     %4d C  (window %d)\n",
               s2.median, MEDIAN_WINDOW);
        printf("\r  WMA:        %4d C  (alpha %d%%)\n",
               s2.weighted, WMA_ALPHA);
        printf("\r  Humidity:   %4d %%\n",  humidity);
        printf("\r  Thresholds: HI=%d  LO=%d C\n",
               ALERT_HIGH_S2, ALERT_LOW_S2);
        printf("\r  STATUS:     %s\n", s2.alert ? "!! ALERT !!" : "OK");
        printf("\r------------------------------\n");
        printf("\r [S3 - ANALOG (MQ-2 Gas Sensor)]\n");
        printf("\r  Raw:        %4d ppm\n",  s3.raw);
        printf("\r  Saturated:  %4d ppm  (range %d..%d)\n",
               s3.saturated, SAT_MIN_S3, SAT_MAX_S3);
        printf("\r  Median:     %4d ppm  (window %d)\n",
               s3.median, MEDIAN_WINDOW);
        printf("\r  WMA:        %4d ppm  (alpha %d%%)\n",
               s3.weighted, WMA_ALPHA);
        printf("\r  Thresholds: HI=%d  LO=%d ppm\n",
               ALERT_HIGH_S3, ALERT_LOW_S3);
        printf("\r  STATUS:     %s\n", s3.alert ? "!! ALERT !!" : "OK");
        printf("\r******************************\n");

        vTaskDelay(pdMS_TO_TICKS(REPORT_PERIOD_MS));
    }
}