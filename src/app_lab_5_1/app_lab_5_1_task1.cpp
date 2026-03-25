#include "app_lab_5_1_task1.h"
#include "task_config.h"
#include <Arduino.h>
#include <ctype.h>
#include <stdlib.h>
#include "srv_serial_stdio/srv_serial_stdio.h"
#include <string.h>

static App5UserCmd_t s_cmd = {false, ANALOG_MODE_AUTO, 0};
static SemaphoreHandle_t s_cmd_mutex = NULL;

static bool parse_pwm_value(const char *input, int *out) {
    char *endptr = NULL;
    long value = strtol(input, &endptr, 10);
    if (endptr == input || *endptr != '\0') {
        return false;
    }
    if (value < ANALOG_PWM_MIN || value > ANALOG_PWM_MAX) {
        return false;
    }
    *out = (int)value;
    return true;
}

static void str_to_upper(char *text) {
    for (int i = 0; text[i] != '\0'; i++) {
        text[i] = (char)toupper((unsigned char)text[i]);
    }
}

static char *trim_whitespace(char *text) {
    while (*text != '\0' && isspace((unsigned char)*text)) {
        text++;
    }

    if (*text == '\0') {
        return text;
    }

    char *end = text + strlen(text) - 1;
    while (end > text && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }

    return text;
}

static bool is_command_char_set_valid(const char *text) {
    for (int i = 0; text[i] != '\0'; i++) {
        char ch = text[i];
        bool is_upper = (ch >= 'A' && ch <= 'Z');
        bool is_digit = (ch >= '0' && ch <= '9');
        bool is_space = (ch == ' ');
        if (!is_upper && !is_digit && !is_space) {
            return false;
        }
    }
    return true;
}

static void apply_command(const char *line) {
    char cmd[40] = {0};
    strncpy(cmd, line, sizeof(cmd) - 1);
    char *clean = trim_whitespace(cmd);
    if (*clean == '\0') {
        return;
    }
    str_to_upper(clean);

    // Ignore random serial noise lines without spamming errors.
    if (!is_command_char_set_valid(clean)) {
        return;
    }

    if (strcmp(clean, "ON") == 0) {
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            s_cmd.bin_requested = true;
            xSemaphoreGive(s_cmd_mutex);
        }
        printf("CMD OK: BIN=ON\n");
        return;
    }

    if (strcmp(clean, "OFF") == 0) {
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            s_cmd.bin_requested = false;
            xSemaphoreGive(s_cmd_mutex);
        }
        printf("CMD OK: BIN=OFF\n");
        return;
    }

    if (strcmp(clean, "AUTO") == 0) {
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            s_cmd.analog_mode = ANALOG_MODE_AUTO;
            xSemaphoreGive(s_cmd_mutex);
        }
        printf("CMD OK: ANALOG=AUTO\n");
        return;
    }

    if (strncmp(clean, "PWM ", 4) == 0) {
        int pwm = 0;
        if (!parse_pwm_value(clean + 4, &pwm)) {
            printf("CMD ERR: PWM must be 0..255\n");
            return;
        }

        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            s_cmd.manual_pwm = pwm;
            s_cmd.analog_mode = ANALOG_MODE_MANUAL;
            xSemaphoreGive(s_cmd_mutex);
        }

        printf("CMD OK: ANALOG=MANUAL PWM=%d\n", pwm);
        return;
    }

    if (strcmp(clean, "HELP") == 0) {
        printf("Commands: ON | OFF | AUTO | PWM <0..255>\n");
        return;
    }

    printf("CMD ERR: Unknown command. Use HELP\n");
}

void app_lab_5_1_task1_init() {
    s_cmd_mutex = xSemaphoreCreateMutex();
}

App5UserCmd_t app_lab_5_1_task1_get_latest() {
    App5UserCmd_t snapshot = {false, ANALOG_MODE_AUTO, 0};

    if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
        snapshot = s_cmd;
        xSemaphoreGive(s_cmd_mutex);
    }

    return snapshot;
}

void app_lab_5_1_task1(void *pvParameters) {
    (void)pvParameters;

    char line_buf[40] = {0};

    printf("Lab 5.1 ready. Use: ON | OFF | AUTO | PWM <0..255>\n");

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(10));
        
        if (Serial.available() > 0) {
            scanf(" %39[^\r\n]", line_buf);
            apply_command(line_buf);
        }
    }
}
