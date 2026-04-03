#include "app_lab_6_1_task3.h"
#include "task_config.h"
#include "srv_stdio_lcd/srv_stdio_lcd.h"
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>

// ── Mod output serial ─────────────────────────────────────────────────────
// Comenteaza una din cele doua linii de mai jos:
#define SERIAL_MODE_MONITOR   // raport text detaliat  → Serial Monitor
// #define SERIAL_MODE_PLOTTER   // SP,Temp,Out          → Serial Plotter

// ── Change detection ──────────────────────────────────────────────────────

static bool snapshot_changed(const App61Snapshot_t *a,
                              const App61Snapshot_t *b) {
    return a->temperature_raw  != b->temperature_raw  ||
           a->humidity         != b->humidity         ||
           a->set_point        != b->set_point        ||
           a->hyst_band        != b->hyst_band        ||
           a->relay_requested  != b->relay_requested  ||
           a->relay_pending    != b->relay_pending    ||
           a->relay_state      != b->relay_state      ||
           a->deviation_alert  != b->deviation_alert;
}

// ── Task body ─────────────────────────────────────────────────────────────

void app_lab_6_1_task3(void *pvParameters) {
    (void)pvParameters;

#ifdef SERIAL_MODE_MONITOR
    App61Snapshot_t last     = {};
    bool            has_last = false;
    TickType_t      last_tick = 0;
    const TickType_t hb = pdMS_TO_TICKS(HEARTBEAT_MS);
#endif

    for (;;) {
        // 1. Read shared snapshot
        App61Snapshot_t snap = {};
        if (xSemaphoreTake(g_snap61_mutex, portMAX_DELAY) == pdTRUE) {
            snap = g_snap61;
            xSemaphoreGive(g_snap61_mutex);
        }

        // ── LCD (mereu activ) ─────────────────────────────────────────────
        const char *relay_str   = snap.relay_state    ? "ON " : "OFF";
        const char *pending_str = snap.relay_pending  ? "*"   : " ";
        const char *alert_str   = snap.deviation_alert ? "!ALT" : "    ";

        int t_int  = snap.temperature_raw / 10;
        int t_dec  = snap.temperature_raw < 0
                     ? -(snap.temperature_raw % 10)
                     :   snap.temperature_raw % 10;
        int sp_int = snap.set_point / 10;
        int sp_dec = snap.set_point % 10;

        char r0[17] = {0}, r1[17] = {0};
        snprintf(r0, 17, "T:%3d.%1dC SP:%2d.%1d",
                 t_int, t_dec, sp_int, sp_dec);
        snprintf(r1, 17, "R:%3s%1s H:%2d%%%4s",
                 relay_str, pending_str, snap.humidity, alert_str);

        lcd.clear();
        lcd.setCursor(0, 0); lcd.print(r0);
        lcd.setCursor(0, 1); lcd.print(r1);

        // ── Serial Monitor ────────────────────────────────────────────────
#ifdef SERIAL_MODE_MONITOR
        TickType_t now     = xTaskGetTickCount();
        bool changed       = !has_last || snapshot_changed(&snap, &last);
        bool heartbeat_due = has_last  && (now - last_tick >= hb);

        if (changed || heartbeat_due) {
            printf("==============================\n");
            printf(" Lab 6.1 - ON-OFF Hysteresis Control\n");
            printf("------------------------------\n");
            printf(" [Sensor]\n");
            printf("  Temperature:  %d.%d C\n", t_int, t_dec < 0 ? -t_dec : t_dec);
            printf("  Humidity:     %d %%RH\n", snap.humidity);
            printf(" [Control]\n");
            printf("  SetPoint:     %d.%d C\n", sp_int, sp_dec);
            printf("  Hyst band:  +/-%d.%d C  (total: %d.%d C)\n",
                   snap.hyst_band / 10, snap.hyst_band % 10,
                   (snap.hyst_band * 2) / 10, (snap.hyst_band * 2) % 10);
            printf("  Threshold ON:  >= %d.%d C\n",
                   snap.threshold_on / 10, snap.threshold_on % 10);
            printf("  Threshold OFF: <= %d.%d C\n",
                   snap.threshold_off / 10, snap.threshold_off % 10);
            printf("  Error:        %+d.%d C\n",
                   snap.error / 10,
                   snap.error < 0 ? -(snap.error % 10) : snap.error % 10);
            printf(" [Relay]\n");
            printf("  Requested:    %s\n", snap.relay_requested ? "ON"  : "OFF");
            printf("  State:        %s\n", snap.relay_state     ? "ON"  : "OFF");
            printf("  Debounce:     %s\n", snap.relay_pending   ? "PENDING" : "STABLE");
            printf(" [Alerts]\n");
            if (snap.deviation_alert) {
                printf("  !! DEVIATION ALERT: |error| > %d.%d C !!\n",
                       ALERT_DEVIATION / 10, ALERT_DEVIATION % 10);
            } else {
                printf("  Deviation:    OK\n");
            }
            printf("==============================\n\n");

            last      = snap;
            has_last  = true;
            last_tick = now;
        }
#endif

        // ── Serial Plotter ────────────────────────────────────────────────
#ifdef SERIAL_MODE_PLOTTER
        printf("SP:%d,Temp:%d,Out:%d\n",
               snap.set_point,
               snap.temperature_raw,
               snap.relay_state ? 100 : 0);
#endif

        vTaskDelay(pdMS_TO_TICKS(REPORT_PERIOD_MS));
    }
}