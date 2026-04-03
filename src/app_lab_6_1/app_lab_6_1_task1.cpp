#include "app_lab_6_1_task1.h"
#include "task_config.h"
#include "srv_serial_stdio/srv_serial_stdio.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static App61Cmd_t        s_cmd       = { SET_POINT_DEFAULT, HYST_BAND_DEFAULT, false };
static SemaphoreHandle_t s_cmd_mutex = NULL;

// ── Internal helpers ──────────────────────────────────────────────────────

static int clamp_sp(int v) {
    return v < SET_POINT_MIN ? SET_POINT_MIN : (v > SET_POINT_MAX ? SET_POINT_MAX : v);
}

static void apply_command(const char *line) {
    char buf[40] = {0};
    strncpy(buf, line, sizeof(buf) - 1);

    char *p = buf;
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p == '\0') return;

    for (int i = 0; p[i]; i++)
        p[i] = (char)toupper((unsigned char)p[i]);

    if (strcmp(p, "UP") == 0) {
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            s_cmd.set_point = clamp_sp(s_cmd.set_point + SET_POINT_STEP);
            s_cmd.sp_changed = true;
            printf("CMD OK: SP=+%d.%d -> SP=%d.%d C\n",
                   SET_POINT_STEP / 10, SET_POINT_STEP % 10,
                   s_cmd.set_point / 10, s_cmd.set_point % 10);
            xSemaphoreGive(s_cmd_mutex);
        }
        return;
    }

    if (strcmp(p, "DOWN") == 0) {
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            s_cmd.set_point = clamp_sp(s_cmd.set_point - SET_POINT_STEP);
            s_cmd.sp_changed = true;
            printf("CMD OK: SP=-%d.%d -> SP=%d.%d C\n",
                   SET_POINT_STEP / 10, SET_POINT_STEP % 10,
                   s_cmd.set_point / 10, s_cmd.set_point % 10);
            xSemaphoreGive(s_cmd_mutex);
        }
        return;
    }

    if (strncmp(p, "SP ", 3) == 0) {
        char *endptr = NULL;
        long val = strtol(p + 3, &endptr, 10);
        if (endptr == p + 3 || *endptr != '\0' ||
            val < SET_POINT_MIN || val > SET_POINT_MAX) {
            printf("CMD ERR: SP must be %d..%d (in 0.1C units)\n",
                   SET_POINT_MIN, SET_POINT_MAX);
            return;
        }
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            s_cmd.set_point  = (int)val;
            s_cmd.sp_changed = true;
            printf("CMD OK: SP=%d.%d C\n",
                   s_cmd.set_point / 10, s_cmd.set_point % 10);
            xSemaphoreGive(s_cmd_mutex);
        }
        return;
    }

    if (strncmp(p, "HYST ", 5) == 0) {
        char *endptr = NULL;
        long val = strtol(p + 5, &endptr, 10);
        if (endptr == p + 5 || *endptr != '\0' ||
            val < HYST_BAND_MIN || val > HYST_BAND_MAX) {
            printf("CMD ERR: HYST must be %d..%d (in 0.1C units)\n",
                   HYST_BAND_MIN, HYST_BAND_MAX);
            return;
        }
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            s_cmd.hyst_band = (int)val;
            printf("CMD OK: HYST=%d.%d C (total band = %d.%d C)\n",
                   s_cmd.hyst_band / 10, s_cmd.hyst_band % 10,
                   (s_cmd.hyst_band * 2) / 10, (s_cmd.hyst_band * 2) % 10);
            xSemaphoreGive(s_cmd_mutex);
        }
        return;
    }

    if (strcmp(p, "STATUS") == 0) {
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            int sp   = s_cmd.set_point;
            int hyst = s_cmd.hyst_band;
            xSemaphoreGive(s_cmd_mutex);
            printf("STATUS: SP=%d.%d C  HYST=%d.%d C  ON>=%d.%d C  OFF<=%d.%d C\n",
                   sp / 10, sp % 10,
                   hyst / 10, hyst % 10,
                   (sp + hyst) / 10, (sp + hyst) % 10,
                   (sp - hyst) / 10, (sp - hyst) % 10);
        }
        return;
    }

    if (strcmp(p, "HELP") == 0) {
        printf("Commands:\n");
        printf("  UP             raise SetPoint by 0.5 C\n");
        printf("  DOWN           lower SetPoint by 0.5 C\n");
        printf("  SP <val>       set SetPoint (0.1C units, e.g. SP 250 = 25.0C)\n");
        printf("  HYST <val>     set hysteresis half-band (0.1C units)\n");
        printf("  STATUS         print current SP and HYST values\n");
        printf("  HELP           show this help\n");
        return;
    }

    printf("CMD ERR: Unknown command. Use HELP\n");
}

// ── Public API ────────────────────────────────────────────────────────────

void app_lab_6_1_task1_init() {
    s_cmd_mutex = xSemaphoreCreateMutex();
}

App61Cmd_t app_lab_6_1_task1_get_latest() {
    App61Cmd_t snap = { SET_POINT_DEFAULT, HYST_BAND_DEFAULT, false };
    if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
        snap = s_cmd;
        s_cmd.sp_changed = false;
        xSemaphoreGive(s_cmd_mutex);
    }
    return snap;
}

void app_lab_6_1_task1(void *pvParameters) {
    (void)pvParameters;
    char line_buf[40] = {0};
    printf("Lab 6.1 ON-OFF Hysteresis Control ready.\n");
    printf("Default SP=%d.%d C  HYST=%d.%d C\n",
           SET_POINT_DEFAULT / 10, SET_POINT_DEFAULT % 10,
           HYST_BAND_DEFAULT / 10, HYST_BAND_DEFAULT % 10);
    printf("Commands: UP | DOWN | SP <val> | HYST <val> | STATUS | HELP\n");

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(10));
        if (Serial.available() > 0) {
            scanf(" %39[^\r\n]", line_buf);
            apply_command(line_buf);
        }
    }
}