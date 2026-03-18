#include "app_lab_5_1_task2.h"
#include "app_lab_5_1_task1.h"
#include "task_config.h"
#include "dd_actuator_bin/dd_actuator_bin.h"
#include "dd_actuator_analog/dd_actuator_analog.h"
#include "dd_sns_angle/dd_sns_angle.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

App5Snapshot_t g_app5_snapshot = {
    false,
    false,
    false,
    ANALOG_MODE_AUTO,
    0,
    0,
    0,
    false
};

SemaphoreHandle_t g_app5_snapshot_mutex = NULL;

static int map_angle_to_pwm(int angle_deg) {
    int pwm = map(angle_deg, -135, 135, ANALOG_PWM_MIN, ANALOG_PWM_MAX);
    if (pwm < ANALOG_PWM_MIN) return ANALOG_PWM_MIN;
    if (pwm > ANALOG_PWM_MAX) return ANALOG_PWM_MAX;
    return pwm;
}

void app_lab_5_1_task2_init() {
    g_app5_snapshot_mutex = xSemaphoreCreateMutex();
}

void app_lab_5_1_task2(void *pvParameters) {
    (void)pvParameters;

    TickType_t last_wake = xTaskGetTickCount();

    for (;;) {
        dd_sns_angle_loop();
        int angle_deg = dd_sns_angle_get_value();

        App5UserCmd_t cmd = app_lab_5_1_task1_get_latest();

        dd_actuator_bin_set_requested(cmd.bin_requested);
        dd_actuator_bin_step();

        int requested_pwm = 0;
        if (cmd.analog_mode == ANALOG_MODE_AUTO) {
            requested_pwm = map_angle_to_pwm(angle_deg);
        } else {
            requested_pwm = cmd.manual_pwm;
        }

        dd_actuator_analog_set_requested_pwm(requested_pwm);
        dd_actuator_analog_apply();

        int applied_pwm = dd_actuator_analog_get_applied_pwm();

        bool analog_alert = false;
        bool previous_alert = false;

        if (xSemaphoreTake(g_app5_snapshot_mutex, portMAX_DELAY) == pdTRUE) {
            previous_alert = g_app5_snapshot.analog_alert;
            xSemaphoreGive(g_app5_snapshot_mutex);
        }

        if (!previous_alert && applied_pwm > ANALOG_ALERT_HIGH) {
            analog_alert = true;
        } else if (previous_alert && applied_pwm >= ANALOG_ALERT_LOW) {
            analog_alert = true;
        }

        if (xSemaphoreTake(g_app5_snapshot_mutex, portMAX_DELAY) == pdTRUE) {
            g_app5_snapshot.bin_requested = dd_actuator_bin_get_requested();
            g_app5_snapshot.bin_pending = dd_actuator_bin_get_pending();
            g_app5_snapshot.bin_state = dd_actuator_bin_get_state();
            g_app5_snapshot.analog_mode = cmd.analog_mode;
            g_app5_snapshot.angle_deg = angle_deg;
            g_app5_snapshot.analog_requested_pwm = requested_pwm;
            g_app5_snapshot.analog_applied_pwm = applied_pwm;
            g_app5_snapshot.analog_alert = analog_alert;
            xSemaphoreGive(g_app5_snapshot_mutex);
        }

        // Binary actuator state LED: D9 red ON when binary actuator is active
        digitalWrite(PIN_LED_BIN_ON, dd_actuator_bin_get_state() ? HIGH : LOW);

        // Alert LEDs: D12=OK (green), D11=ALERT (yellow)
        if (analog_alert) {
            digitalWrite(PIN_LED_OK, LOW);
            digitalWrite(PIN_LED_ALERT, HIGH);
        } else {
            digitalWrite(PIN_LED_OK, HIGH);
            digitalWrite(PIN_LED_ALERT, LOW);
        }

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(ACTUATOR_COND_PERIOD_MS));
    }
}
