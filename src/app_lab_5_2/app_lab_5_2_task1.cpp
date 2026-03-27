#include "app_lab_5_2_task1.h"
#include "task_config.h"
#include "srv_serial_stdio/srv_serial_stdio.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static App52Cmd_t        s_cmd       = {false, ANALOG_MODE_AUTO, 0};
static SemaphoreHandle_t s_cmd_mutex = NULL;

static void apply_command(const char *line) {
    char buf[40] = {0};
    strncpy(buf, line, sizeof(buf) - 1);

    // trim leading whitespace
    char *p = buf;
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p == '\0') return;

    // uppercase
    for (int i = 0; p[i]; i++)
        p[i] = (char)toupper((unsigned char)p[i]);

    // ON - turn on binary actuator
    if (strcmp(p, "ON") == 0) {
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            s_cmd.bin_requested = true;
            xSemaphoreGive(s_cmd_mutex);
        }
        printf("CMD OK: BIN=ON\n");
        return;
    }

    // OFF - turn off binary actuator
    if (strcmp(p, "OFF") == 0) {
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            s_cmd.bin_requested = false;
            xSemaphoreGive(s_cmd_mutex);
        }
        printf("CMD OK: BIN=OFF\n");
        return;
    }

    // AUTO - switch motor to potentiometer control
    if (strcmp(p, "AUTO") == 0) {
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            s_cmd.analog_mode = ANALOG_MODE_AUTO;
            xSemaphoreGive(s_cmd_mutex);
        }
        printf("CMD OK: MOTOR=AUTO (potentiometer)\n");
        return;
    }

    // PWM <value> - set analog motor PWM (only in MANUAL mode)
    if (strncmp(p, "PWM ", 4) == 0) {
        char *endptr = NULL;
        long val = strtol(p + 4, &endptr, 10);
        if (endptr == p + 4 || *endptr != '\0' ||
            val < PWM_MIN || val > PWM_MAX) {
            printf("CMD ERR: PWM must be %d..%d\n", PWM_MIN, PWM_MAX);
            return;
        }
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            s_cmd.analog_mode = ANALOG_MODE_MANUAL;
            s_cmd.target_pwm = (int)val;
            xSemaphoreGive(s_cmd_mutex);
        }
        printf("CMD OK: MOTOR=MANUAL PWM=%d\n", (int)val);
        return;
    }

    // STOP shorthand
    if (strcmp(p, "STOP") == 0) {
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            s_cmd.analog_mode = ANALOG_MODE_MANUAL;
            s_cmd.target_pwm = 0;
            xSemaphoreGive(s_cmd_mutex);
        }
        printf("CMD OK: MOTOR=MANUAL PWM=0 (STOP)\n");
        return;
    }

    // FULL shorthand
    if (strcmp(p, "FULL") == 0) {
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            s_cmd.analog_mode = ANALOG_MODE_MANUAL;
            s_cmd.target_pwm = PWM_MAX;
            xSemaphoreGive(s_cmd_mutex);
        }
        printf("CMD OK: MOTOR=MANUAL PWM=%d (FULL)\n", PWM_MAX);
        return;
    }

    if (strcmp(p, "HELP") == 0) {
        printf("Commands: ON | OFF | AUTO | PWM <0..255> | STOP | FULL | HELP\n");
        return;
    }

    printf("CMD ERR: Unknown command. Use HELP\n");
}

void app_lab_5_2_task1_init() {
    s_cmd_mutex = xSemaphoreCreateMutex();
}

App52Cmd_t app_lab_5_2_task1_get_latest() {
    App52Cmd_t snap = {false, ANALOG_MODE_AUTO, 0};
    if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
        snap = s_cmd;
        xSemaphoreGive(s_cmd_mutex);
    }
    return snap;
}

void app_lab_5_2_task1(void *pvParameters) {
    (void)pvParameters;
    char line_buf[40] = {0};
    printf("Lab 5.2 ready. Commands: ON | OFF | AUTO | MANUAL | PWM <0..255> | STOP | FULL | HELP\n");
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(10));
        if (Serial.available() > 0) {
            scanf(" %39[^\r\n]", line_buf);
            apply_command(line_buf);
        }
    }
}