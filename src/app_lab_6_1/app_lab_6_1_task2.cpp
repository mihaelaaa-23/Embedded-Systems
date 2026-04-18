#include "app_lab_6_1_task2.h"
#include "app_lab_6_1_task1.h"
#include "task_config.h"
#include "dd_actuator_bin/dd_actuator_bin.h"
#include "dd_led/dd_led.h"
#include "dd_sns_dht/dd_sns_dht.h"
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

// ── Shared snapshot (written here, read by Task 3) ─────────────────────────
App61Snapshot_t    g_snap61       = {};
SemaphoreHandle_t  g_snap61_mutex = NULL;

// ── ON-OFF hysteresis controller (HEATING logic) ──────────────────────────
// Relay ON  when temp <= SP - HYST  (too cold → heater ON)
// Relay OFF when temp >= SP + HYST  (warm enough → heater OFF)
// Dead band: SP-HYST < temp < SP+HYST → hold current state
static bool onoff_hysteresis(int temp_raw, int sp, int hyst, bool current_relay_state) {
    int threshold_heat_on  = sp - hyst;   // below this → turn ON
    int threshold_heat_off = sp + hyst;   // above this → turn OFF

    if (!current_relay_state) {
        // Relay OFF: turn ON only if temp drops below lower threshold
        return (temp_raw <= threshold_heat_on);
    } else {
        // Relay ON: turn OFF only if temp rises above upper threshold
        return (temp_raw < threshold_heat_off);
    }
}

void app_lab_6_1_task2_init() {
    g_snap61_mutex = xSemaphoreCreateMutex();
}

void app_lab_6_1_task2(void *pvParameters) {
    (void)pvParameters;

    bool prev_relay_state = false;

    TickType_t last_wake = xTaskGetTickCount();

    for (;;) {
        // 1. Read SetPoint / Hysteresis from Task 1
        App61Cmd_t cmd = app_lab_6_1_task1_get_latest();

        // 2. Acquire sensor data
        dd_sns_dht_loop();
        int temp_raw = dd_sns_dht_get_raw();
        int temp_c   = dd_sns_dht_get_celsius();
        int humidity = dd_sns_dht_get_humidity();

        int sp   = cmd.set_point;
        int hyst = cmd.hyst_band;

        // 3. ON-OFF control with hysteresis
        bool relay_cmd = onoff_hysteresis(temp_raw, sp, hyst, prev_relay_state);
        prev_relay_state = relay_cmd;

        // 4. Apply through binary actuator driver (adds persistence debounce)
        dd_actuator_bin_set_requested(relay_cmd);
        dd_actuator_bin_step();

        bool relay_requested = dd_actuator_bin_get_requested();
        bool relay_pending   = dd_actuator_bin_get_pending();
        bool relay_state     = dd_actuator_bin_get_state();

        // 5. Compute control error and deviation alert
        int error = temp_raw - sp;
        bool deviation_alert = (error > ALERT_DEVIATION || error < -ALERT_DEVIATION);

        // Thresholds for display (heating: ON below, OFF above)
        int threshold_on  = sp - hyst;   // heater turns ON below this
        int threshold_off = sp + hyst;   // heater turns OFF above this

        // 6. Publish snapshot for Task 3
        if (xSemaphoreTake(g_snap61_mutex, portMAX_DELAY) == pdTRUE) {
            g_snap61.temperature_raw  = temp_raw;
            g_snap61.temperature_c    = temp_c;
            g_snap61.humidity         = humidity;
            g_snap61.set_point        = sp;
            g_snap61.hyst_band        = hyst;
            g_snap61.error            = error;
            g_snap61.threshold_on     = threshold_on;
            g_snap61.threshold_off    = threshold_off;
            g_snap61.relay_requested  = relay_requested;
            g_snap61.relay_pending    = relay_pending;
            g_snap61.relay_state      = relay_state;
            g_snap61.deviation_alert  = deviation_alert;
            xSemaphoreGive(g_snap61_mutex);
        }

        // 7. Update LEDs
        if (relay_state || deviation_alert) {
            dd_led_turn_on();
        } else {
            dd_led_turn_off();
        }

        if (!relay_state && !deviation_alert) {
            dd_led_1_turn_on();
        } else {
            dd_led_1_turn_off();
        }

        if (deviation_alert) {
            dd_led_2_turn_on();
        } else {
            dd_led_2_turn_off();
        }

        dd_led_apply();

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(ACQ_PERIOD_MS));
    }
}