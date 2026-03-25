#include "app_lab_5_2_task3.h"
#include "task_config.h"
#include "srv_stdio_lcd/srv_stdio_lcd.h"
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>

static bool snapshot_changed(const App52Snapshot_t *a,
                              const App52Snapshot_t *b) {
    return a->raw       != b->raw       ||
           a->saturated != b->saturated ||
           a->median    != b->median    ||
           a->wma       != b->wma       ||
           a->ramped    != b->ramped    ||
           a->alert     != b->alert     ||
           a->at_limit  != b->at_limit;
}

void app_lab_5_2_task3(void *pvParameters) {
    (void)pvParameters;

    App52Snapshot_t last     = {0, 0, 0, 0, 0, false, false};
    bool            has_last = false;
    TickType_t      last_tick = 0;
    const TickType_t hb = pdMS_TO_TICKS(HEARTBEAT_MS);

    for (;;) {
        App52Snapshot_t snap = {0, 0, 0, 0, 0, false, false};
        if (xSemaphoreTake(g_snap52_mutex, portMAX_DELAY) == pdTRUE) {
            snap = g_snap52;
            xSemaphoreGive(g_snap52_mutex);
        }

        // LCD: row 0 = raw + applied, row 1 = WMA + status
        const char *status = snap.alert    ? "AL" :
                             snap.at_limit ? "LM" : "OK";
        char r0[17] = {0}, r1[17] = {0};
        snprintf(r0, 17, "RAW:%3d APL:%3d", snap.raw, snap.ramped);
        snprintf(r1, 17, "WMA:%3d MED:%3d", snap.wma, snap.median);
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print(r0);
        lcd.setCursor(0, 1); lcd.print(r1);

        // Serial: print on change or heartbeat
        TickType_t now      = xTaskGetTickCount();
        bool changed        = !has_last || snapshot_changed(&snap, &last);
        bool heartbeat_due  = has_last  && (now - last_tick >= hb);

        if (changed || heartbeat_due) {
            printf("==============================\n");
            printf(" Lab 5.2 - Analog Actuator\n");
            printf("------------------------------\n");
            printf("  Raw command:  %d / 255\n", snap.raw);
            printf("  Saturated:    %d / 255\n", snap.saturated);
            printf("  Median:       %d / 255  (w=%d)\n",
                   snap.median, MEDIAN_WINDOW);
            printf("  WMA:          %d / 255  (a=%d%%)\n",
                   snap.wma, WMA_ALPHA);
            printf("  Applied(ramp):%d / 255  (step=%d)\n",
                   snap.ramped, RAMP_STEP);
            printf("  Alert:        %s (HI=%d LO=%d)\n",
                   snap.alert ? "!! ALERT !!" : "OK",
                   ALERT_HIGH, ALERT_LOW);
            printf("  At limit:     %s\n",
                   snap.at_limit ? "YES" : "NO");
            printf("==============================\n");
            last      = snap;
            has_last  = true;
            last_tick = now;
        }

        vTaskDelay(pdMS_TO_TICKS(REPORT_PERIOD_MS));
    }
}