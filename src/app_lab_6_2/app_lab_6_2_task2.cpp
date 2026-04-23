#include "app_lab_6_2_task2.h"
#include "app_lab_6_2_task1.h"
#include "task_config.h"
#include "../dd_actuator_bin/dd_actuator_bin.h"
#include "../dd_led/dd_led.h"
#include "../dd_sns_dht/dd_sns_dht.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

App62Snapshot_t   g_snap62       = {};
SemaphoreHandle_t g_snap62_mutex = NULL;

// ── PID state ────────────────────────────────────────────────────────────────
static long s_integral  = 0;
static int  s_prev_err  = 0;
static bool s_first_run = true;
static int  s_window_step = 0;

// ── PID compute ──────────────────────────────────────────────────────────────
// error = SP - temp  (positive = too cold = need more heat)
// returns output 0-100 %
static int pid_compute(int temp_raw, int sp, int kp, int ki, int kd) {
    int error = sp - temp_raw;

    long P = (long)error * kp / 100L;

    if (!s_first_run) {
        s_integral += (long)error * ACQ_PERIOD_MS;
        if (s_integral >  (long)PID_INTEGRAL_MAX * 1000L) s_integral =  (long)PID_INTEGRAL_MAX * 1000L;
        if (s_integral < -(long)PID_INTEGRAL_MAX * 1000L) s_integral = -(long)PID_INTEGRAL_MAX * 1000L;
    }
    long I = s_integral / 1000L * ki / 100L;

    long D = 0;
    if (!s_first_run)
        D = (long)(error - s_prev_err) * 1000L / ACQ_PERIOD_MS * kd / 100L;

    s_prev_err  = error;
    s_first_run = false;

    long output = P + I + D;
    if (output < PID_OUT_MIN) output = PID_OUT_MIN;
    if (output > PID_OUT_MAX) output = PID_OUT_MAX;
    return (int)output;
}

void app_lab_6_2_task2_init() { g_snap62_mutex = xSemaphoreCreateMutex(); }

void app_lab_6_2_task2(void *pvParameters) {
    (void)pvParameters;
    TickType_t last_wake = xTaskGetTickCount();

    for (;;) {
        App62Cmd_t cmd = app_lab_6_2_task1_get_latest();

        // Reset PID state on any parameter change
        if (cmd.changed) {
            s_integral  = 0;
            s_prev_err  = 0;
            s_first_run = true;
        }

        // Acquire sensor
        dd_sns_dht_loop();
        int temp_raw = dd_sns_dht_get_raw();
        int humidity = dd_sns_dht_get_humidity();
        int sp = cmd.set_point;

        // ── Determine output based on mode ───────────────────────────────────
        int pid_out;
        if (cmd.mode == CTRL_MODE_MANUAL) {
            // MANUAL: operator-defined output, PID state frozen
            pid_out = cmd.manual_output;
        } else {
            // AUTO: PID computes output
            pid_out = pid_compute(temp_raw, sp, cmd.kp, cmd.ki, cmd.kd);
        }

        // ── Time-proportional relay ──────────────────────────────────────────
        int on_steps = (pid_out * RELAY_WINDOW_STEPS) / 100;
        bool relay_on = (s_window_step < on_steps);
        s_window_step = (s_window_step + 1) % RELAY_WINDOW_STEPS;

        dd_actuator_bin_set_requested(relay_on);
        dd_actuator_bin_step();
        bool relay_state = dd_actuator_bin_get_state();

        // ── Deviation alert (active in both modes) ───────────────────────────
        int error = temp_raw - sp;
        bool deviation_alert = (error > ALERT_DEVIATION || error < -ALERT_DEVIATION);

        // ── Publish snapshot ─────────────────────────────────────────────────
        if (xSemaphoreTake(g_snap62_mutex, portMAX_DELAY) == pdTRUE) {
            g_snap62.temperature_raw = temp_raw;
            g_snap62.humidity        = humidity;
            g_snap62.set_point       = sp;
            g_snap62.error           = error;
            g_snap62.pid_output      = pid_out;
            g_snap62.kp              = cmd.kp;
            g_snap62.ki              = cmd.ki;
            g_snap62.kd              = cmd.kd;
            g_snap62.mode            = cmd.mode;
            g_snap62.relay_state     = relay_state;
            g_snap62.deviation_alert = deviation_alert;
            xSemaphoreGive(g_snap62_mutex);
        }

        // ── LEDs ─────────────────────────────────────────────────────────────
        if (relay_state || deviation_alert) dd_led_turn_on(); else dd_led_turn_off();
        if (!relay_state && !deviation_alert) dd_led_1_turn_on(); else dd_led_1_turn_off();
        if (deviation_alert) dd_led_2_turn_on(); else dd_led_2_turn_off();
        dd_led_apply();

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(ACQ_PERIOD_MS));
    }
}
