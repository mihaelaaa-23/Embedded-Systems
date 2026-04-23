#include "app_lab_6_2_task1.h"
#include "task_config.h"
#include "../srv_serial_stdio/srv_serial_stdio.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static App62Cmd_t        s_cmd = {
    SET_POINT_DEFAULT, PID_KP_DEFAULT, PID_KI_DEFAULT, PID_KD_DEFAULT,
    CTRL_MODE_AUTO, 0, false
};
static SemaphoreHandle_t s_cmd_mutex = NULL;

static int clamp_sp(int v) {
    return v < SET_POINT_MIN ? SET_POINT_MIN : (v > SET_POINT_MAX ? SET_POINT_MAX : v);
}

static void apply_command(const char *line) {
    char buf[48] = {0};
    strncpy(buf, line, sizeof(buf) - 1);
    char *p = buf;
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p == '\0') return;
    for (int i = 0; p[i]; i++) p[i] = (char)toupper((unsigned char)p[i]);

    // UP / DOWN
    if (strcmp(p, "UP") == 0) {
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            s_cmd.set_point = clamp_sp(s_cmd.set_point + SET_POINT_STEP);
            s_cmd.changed = true;
            printf("CMD OK: SP=%d.%d C\n", s_cmd.set_point/10, s_cmd.set_point%10);
            xSemaphoreGive(s_cmd_mutex);
        }
        return;
    }
    if (strcmp(p, "DOWN") == 0) {
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            s_cmd.set_point = clamp_sp(s_cmd.set_point - SET_POINT_STEP);
            s_cmd.changed = true;
            printf("CMD OK: SP=%d.%d C\n", s_cmd.set_point/10, s_cmd.set_point%10);
            xSemaphoreGive(s_cmd_mutex);
        }
        return;
    }

    // SP <val>
    if (strncmp(p, "SP ", 3) == 0) {
        char *e = NULL; long val = strtol(p+3, &e, 10);
        if (e==p+3 || *e!='\0' || val<SET_POINT_MIN || val>SET_POINT_MAX) {
            printf("CMD ERR: SP must be %d..%d (0.1C units)\n", SET_POINT_MIN, SET_POINT_MAX);
            return;
        }
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            s_cmd.set_point = (int)val; s_cmd.changed = true;
            printf("CMD OK: SP=%d.%d C\n", s_cmd.set_point/10, s_cmd.set_point%10);
            xSemaphoreGive(s_cmd_mutex);
        }
        return;
    }

    // KP / KI / KD <val>
    if (strncmp(p, "KP ", 3) == 0 || strncmp(p, "KI ", 3) == 0 || strncmp(p, "KD ", 3) == 0) {
        char *e = NULL; long val = strtol(p+3, &e, 10);
        if (e==p+3 || *e!='\0' || val<0 || val>10000) {
            printf("CMD ERR: gain must be 0..10000 (x100)\n"); return;
        }
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            if (p[1]=='P') { s_cmd.kp=(int)val; printf("CMD OK: Kp=%d.%02d\n", s_cmd.kp/100, s_cmd.kp%100); }
            if (p[1]=='I') { s_cmd.ki=(int)val; printf("CMD OK: Ki=%d.%02d\n", s_cmd.ki/100, s_cmd.ki%100); }
            if (p[1]=='D') { s_cmd.kd=(int)val; printf("CMD OK: Kd=%d.%02d\n", s_cmd.kd/100, s_cmd.kd%100); }
            s_cmd.changed = true;
            xSemaphoreGive(s_cmd_mutex);
        }
        return;
    }

    // MODE AUTO / MODE MANUAL
    if (strcmp(p, "MODE AUTO") == 0) {
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            s_cmd.mode = CTRL_MODE_AUTO;
            s_cmd.changed = true;
            printf("CMD OK: Mode = AUTO (PID active)\n");
            xSemaphoreGive(s_cmd_mutex);
        }
        return;
    }
    if (strcmp(p, "MODE MANUAL") == 0) {
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            s_cmd.mode = CTRL_MODE_MANUAL;
            s_cmd.changed = true;
            printf("CMD OK: Mode = MANUAL (use OUT <0-100> to set output)\n");
            xSemaphoreGive(s_cmd_mutex);
        }
        return;
    }

    // OUT <val>  — manual output 0-100 %
    if (strncmp(p, "OUT ", 4) == 0) {
        char *e = NULL; long val = strtol(p+4, &e, 10);
        if (e==p+4 || *e!='\0' || val<0 || val>100) {
            printf("CMD ERR: OUT must be 0..100 (%%)\n"); return;
        }
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            if (s_cmd.mode != CTRL_MODE_MANUAL) {
                printf("CMD ERR: Switch to MANUAL mode first (MODE MANUAL)\n");
            } else {
                s_cmd.manual_output = (int)val;
                s_cmd.changed = true;
                printf("CMD OK: Manual output = %d %%\n", s_cmd.manual_output);
            }
            xSemaphoreGive(s_cmd_mutex);
        }
        return;
    }

    // STATUS
    if (strcmp(p, "STATUS") == 0) {
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            printf("STATUS: SP=%d.%d C  Kp=%d.%02d  Ki=%d.%02d  Kd=%d.%02d  Mode=%s",
                   s_cmd.set_point/10, s_cmd.set_point%10,
                   s_cmd.kp/100, s_cmd.kp%100,
                   s_cmd.ki/100, s_cmd.ki%100,
                   s_cmd.kd/100, s_cmd.kd%100,
                   s_cmd.mode == CTRL_MODE_AUTO ? "AUTO" : "MANUAL");
            if (s_cmd.mode == CTRL_MODE_MANUAL)
                printf("  Out=%d %%", s_cmd.manual_output);
            printf("\n");
            xSemaphoreGive(s_cmd_mutex);
        }
        return;
    }

    // HELP
    if (strcmp(p, "HELP") == 0) {
        printf("Commands:\n");
        printf("  UP / DOWN           raise/lower SP by 0.5 C\n");
        printf("  SP <val>            set SP (0.1C units)\n");
        printf("  KP/KI/KD <val>      set PID gain x100\n");
        printf("  MODE AUTO           enable PID automatic control\n");
        printf("  MODE MANUAL         enable manual output control\n");
        printf("  OUT <0-100>         set manual output %% (MANUAL mode only)\n");
        printf("  STATUS              print current parameters\n");
        printf("  HELP                show this help\n");
        return;
    }

    printf("CMD ERR: Unknown command. Use HELP\n");
}

void app_lab_6_2_task1_init() { s_cmd_mutex = xSemaphoreCreateMutex(); }

App62Cmd_t app_lab_6_2_task1_get_latest() {
    App62Cmd_t snap = { SET_POINT_DEFAULT, PID_KP_DEFAULT, PID_KI_DEFAULT,
                        PID_KD_DEFAULT, CTRL_MODE_AUTO, 0, false };
    if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
        snap = s_cmd; s_cmd.changed = false;
        xSemaphoreGive(s_cmd_mutex);
    }
    return snap;
}

void app_lab_6_2_task1(void *pvParameters) {
    (void)pvParameters;
    char line_buf[48] = {0};
    printf("Lab 6.2 PID Temperature Control ready.\n");
    printf("SP=%d.%d C  Kp=%d.%02d  Ki=%d.%02d  Kd=%d.%02d  Mode=AUTO\n",
           SET_POINT_DEFAULT/10, SET_POINT_DEFAULT%10,
           PID_KP_DEFAULT/100, PID_KP_DEFAULT%100,
           PID_KI_DEFAULT/100, PID_KI_DEFAULT%100,
           PID_KD_DEFAULT/100, PID_KD_DEFAULT%100);
    printf("Commands: UP|DOWN|SP|KP|KI|KD|MODE AUTO|MODE MANUAL|OUT|STATUS|HELP\n");
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(10));
        if (Serial.available() > 0) {
            scanf(" %47[^\r\n]", line_buf);
            apply_command(line_buf);
        }
    }
}
