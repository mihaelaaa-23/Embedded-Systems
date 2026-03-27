#include "app_lab_5_2_task3.h"
#include "task_config.h"
#include "srv_stdio_lcd/srv_stdio_lcd.h"
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>

static bool snapshot_changed(const App52Snapshot_t *a,
                              const App52Snapshot_t *b) {
    return a->bin_requested != b->bin_requested ||
           a->bin_pending  != b->bin_pending  ||
           a->bin_state    != b->bin_state    ||
           a->analog_mode  != b->analog_mode  ||
           a->potentiometer_raw != b->potentiometer_raw ||
           a->raw          != b->raw          ||
           a->saturated    != b->saturated    ||
           a->median       != b->median       ||
           a->wma          != b->wma          ||
           a->ramped       != b->ramped       ||
           a->alert        != b->alert        ||
           a->at_limit     != b->at_limit;
}

void app_lab_5_2_task3(void *pvParameters) {
    (void)pvParameters;

    App52Snapshot_t last     = {0};
    bool            has_last = false;
    TickType_t      last_tick = 0;
    const TickType_t hb = pdMS_TO_TICKS(HEARTBEAT_MS);

    for (;;) {
        App52Snapshot_t snap = {0};
        if (xSemaphoreTake(g_snap52_mutex, portMAX_DELAY) == pdTRUE) {
            snap = g_snap52;
            xSemaphoreGive(g_snap52_mutex);
        }

        // LCD: row 0 = binary state + motor PWM, row 1 = motor status
        const char *bin_state = snap.bin_state ? "ON" : "OF";
        const char *bin_mark  = snap.bin_pending ? "P" : " ";
        const char *mode = (snap.analog_mode == ANALOG_MODE_AUTO) ? "AU" : "MN";
        
        // Show MAX!/MIN! when at limits, otherwise show alert/limit/ok status
        const char *motor_status;
        if (snap.raw == PWM_MAX) {
            motor_status = "MAX!";
        } else if (snap.raw == PWM_MIN) {
            motor_status = "MIN!";
        } else {
            motor_status = snap.alert    ? "AL" :
                          snap.at_limit ? "LM" : "OK";
        }
        
        char r0[17] = {0}, r1[17] = {0};
        snprintf(r0, 17, "B:%2s%1s M:%3d", bin_state, bin_mark, snap.ramped);
        snprintf(r1, 17, "%2s:%3d %2s", mode, snap.potentiometer_raw, motor_status);
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print(r0);
        lcd.setCursor(0, 1); lcd.print(r1);

        // Serial: print on change or heartbeat
        TickType_t now      = xTaskGetTickCount();
        bool changed        = !has_last || snapshot_changed(&snap, &last);
        bool heartbeat_due  = has_last  && (now - last_tick >= hb);

        if (changed || heartbeat_due) {
            printf("==============================\n");
            printf(" Lab 5.2 - Two Actuators\n");
            printf("------------------------------\n");
            printf(" [Binary Actuator]\n");
            printf("  Command:     %s\n", snap.bin_requested ? "ON" : "OFF");
            printf("  State:       %s\n", snap.bin_state ? "ON" : "OFF");
            printf("  Debounce:    %s\n", snap.bin_pending ? "PENDING" : "STABLE");
            printf(" [Analog Actuator (Motor)]\n");
            printf("  Mode:        %s\n",
                   snap.analog_mode == ANALOG_MODE_AUTO ? "AUTO (potentiometer)" : "MANUAL (serial PWM)");
            printf("  Potentiometer: %d / 255\n", snap.potentiometer_raw);
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
            
            // Advisory messages for limit conditions
            if (snap.raw == PWM_MAX) {
                printf("  ⚠️  WARNING: Motor at MAXIMUM power (255/255)!\n");
            } else if (snap.raw == PWM_MIN) {
                printf("  ℹ️  INFO: Motor at MINIMUM power (0/255) - Stopped\n");
            }
            if (snap.alert) {
                printf("  ⚠️  ALERT: Power consumption near threshold!\n");
            }
            
            printf("==============================\n");
            last      = snap;
            has_last  = true;
            last_tick = now;
        }

        vTaskDelay(pdMS_TO_TICKS(REPORT_PERIOD_MS));
    }
}