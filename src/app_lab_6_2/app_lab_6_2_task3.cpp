#include "app_lab_6_2_task3.h"
#include "task_config.h"
#include "../srv_stdio_lcd/srv_stdio_lcd.h"
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>

// Comment one, uncomment the other before upload:
#define SERIAL_MODE_MONITOR
// #define SERIAL_MODE_PLOTTER

static bool snap_changed(const App62Snapshot_t *a, const App62Snapshot_t *b) {
    return a->temperature_raw != b->temperature_raw ||
           a->pid_output      != b->pid_output      ||
           a->relay_state     != b->relay_state     ||
           a->set_point       != b->set_point       ||
           a->deviation_alert != b->deviation_alert;
}

void app_lab_6_2_task3(void *pvParameters) {
    (void)pvParameters;
#ifdef SERIAL_MODE_MONITOR
    App62Snapshot_t last = {};
    bool has_last = false;
    TickType_t last_tick = 0;
    const TickType_t hb = pdMS_TO_TICKS(HEARTBEAT_MS);
#endif

    for (;;) {
        App62Snapshot_t snap = {};
        if (xSemaphoreTake(g_snap62_mutex, portMAX_DELAY) == pdTRUE) {
            snap = g_snap62;
            xSemaphoreGive(g_snap62_mutex);
        }

        // LCD
        int t_int = snap.temperature_raw / 10;
        int t_dec = snap.temperature_raw < 0 ? -(snap.temperature_raw % 10) : snap.temperature_raw % 10;
        int sp_int = snap.set_point / 10;
        int sp_dec = snap.set_point % 10;
        const char *alert_str = snap.deviation_alert ? "!ALT" : "    ";
        char r0[17] = {0}, r1[17] = {0};
        snprintf(r0, 17, "T:%3d.%1dC SP:%2d.%1d", t_int, t_dec, sp_int, sp_dec);
        snprintf(r1, 17, "R:%3s O:%3d%%%4s", snap.relay_state ? "ON " : "OFF", snap.pid_output, alert_str);
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print(r0);
        lcd.setCursor(0, 1); lcd.print(r1);

#ifdef SERIAL_MODE_MONITOR
        TickType_t now = xTaskGetTickCount();
        bool changed  = !has_last || snap_changed(&snap, &last);
        bool hb_due   =  has_last && (now - last_tick >= hb);
        if (changed || hb_due) {
            printf("==============================\n");
            printf(" Lab 6.2 - PID Temperature Control\n");
            printf("------------------------------\n");
            printf(" [Sensor]\n");
            printf("  Temperature:  %d.%d C\n", t_int, t_dec < 0 ? -t_dec : t_dec);
            printf("  Humidity:     %d %%RH\n", snap.humidity);
            printf(" [Control]\n");
            printf("  SetPoint:     %d.%d C\n", sp_int, sp_dec);
            printf("  Error:        %+d.%d C\n",
                   snap.error/10, snap.error < 0 ? -(snap.error%10) : snap.error%10);
            printf("  PID Output:   %d %%\n", snap.pid_output);
            printf("  Kp=%d.%02d  Ki=%d.%02d  Kd=%d.%02d\n",
                   snap.kp/100, snap.kp%100,
                   snap.ki/100, snap.ki%100,
                   snap.kd/100, snap.kd%100);
            printf(" [Relay]\n");
            printf("  State:        %s\n", snap.relay_state ? "ON" : "OFF");
            printf(" [Alerts]\n");
            if (snap.deviation_alert)
                printf("  !! DEVIATION ALERT: |error| > %d.%d C !!\n", ALERT_DEVIATION/10, ALERT_DEVIATION%10);
            else
                printf("  Deviation:    OK\n");
            printf("==============================\n\n");
            last = snap; has_last = true; last_tick = now;
        }
#endif

#ifdef SERIAL_MODE_PLOTTER
        printf("SP:%d,Temp:%d,Out:%d\n", snap.set_point, snap.temperature_raw, snap.pid_output);
#endif

        vTaskDelay(pdMS_TO_TICKS(REPORT_PERIOD_MS));
    }
}
