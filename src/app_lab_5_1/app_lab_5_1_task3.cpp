#include "app_lab_5_1_task3.h"
#include "task_config.h"
#include "srv_stdio_lcd/srv_stdio_lcd.h"
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>

static bool snapshot_changed(const App5Snapshot_t *a, const App5Snapshot_t *b) {
    return a->bin_requested != b->bin_requested ||
           a->bin_pending != b->bin_pending ||
           a->bin_state != b->bin_state ||
           a->analog_mode != b->analog_mode ||
           a->angle_deg != b->angle_deg ||
           a->analog_requested_pwm != b->analog_requested_pwm ||
           a->analog_applied_pwm != b->analog_applied_pwm ||
           a->analog_alert != b->analog_alert;
}

void app_lab_5_1_task3(void *pvParameters) {
    (void)pvParameters;

    App5Snapshot_t last_serial = {false, false, false, ANALOG_MODE_AUTO, 0, 0, 0, false};
    bool has_last_serial = false;
    TickType_t last_serial_tick = 0;
    const TickType_t heartbeat_ticks = pdMS_TO_TICKS(ACTUATOR_SERIAL_HEARTBEAT_MS);

    for (;;) {
        App5Snapshot_t snap = {false, false, false, ANALOG_MODE_AUTO, 0, 0, 0, false};

        if (xSemaphoreTake(g_app5_snapshot_mutex, portMAX_DELAY) == pdTRUE) {
            snap = g_app5_snapshot;
            xSemaphoreGive(g_app5_snapshot_mutex);
        }

        const char *bin_state = snap.bin_state ? "ON" : "OFF";
        const char *bin_req = snap.bin_requested ? "ON" : "OFF";
        const char *bin_mark = snap.bin_pending ? "PD" : "OK";
        const char *mode = (snap.analog_mode == ANALOG_MODE_AUTO) ? "AU" : "MN";
        const char *alert = snap.analog_alert ? "AL" : "OK";
        char lcd_row0[17] = {0};
        char lcd_row1[17] = {0};

        snprintf(lcd_row0, sizeof(lcd_row0), "B:%3s C:%3s %2s", bin_state, bin_req, bin_mark);
        snprintf(lcd_row1, sizeof(lcd_row1), "A:%3d %2s %2s", snap.analog_applied_pwm, mode, alert);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(lcd_row0);
        lcd.setCursor(0, 1);
        lcd.print(lcd_row1);

         TickType_t now = xTaskGetTickCount();
         bool changed = !has_last_serial || snapshot_changed(&snap, &last_serial);
         bool heartbeat_due = has_last_serial && (now - last_serial_tick >= heartbeat_ticks);

         if (changed || heartbeat_due) {
             printf("==============================\n");
             printf(" Lab 5.1 - Actuator Control\n");
             printf("------------------------------\n");
             printf(" [Binary Actuator]\n");
             printf("  Command:     %s\n", bin_req);
             printf("  State:       %s\n", bin_state);
             printf("  Debounce:    %s\n", snap.bin_pending ? "PENDING" : "STABLE");
             printf(" [DC Motor (L298)]\n");
             printf("  Mode:        %s\n",
                 snap.analog_mode == ANALOG_MODE_AUTO ? "AUTO (angle sensor)" : "MANUAL (serial PWM)");
             printf("  Angle:       %d deg\n", snap.angle_deg);
             printf("  Requested:   %d / 255\n", snap.analog_requested_pwm);
             printf("  Applied:     %d / 255\n", snap.analog_applied_pwm);
             printf("  Alert:       %s (HI=%d LO=%d)\n",
                 snap.analog_alert ? "!! ALERT !!" : "OK",
                 ANALOG_ALERT_HIGH,
                 ANALOG_ALERT_LOW);
             printf("==============================\n");

             last_serial = snap;
             has_last_serial = true;
             last_serial_tick = now;
         }

        vTaskDelay(pdMS_TO_TICKS(ACTUATOR_REPORT_PERIOD_MS));
    }
}
