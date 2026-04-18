#include "app_lab_6_2_task1.h"
#include "task_config.h"
#include "../srv_serial_stdio/srv_serial_stdio.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static App62Cmd_t        s_cmd = { SET_POINT_DEFAULT, PID_KP_DEFAULT, PID_KI_DEFAULT, PID_KD_DEFAULT, false };
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
    if (strncmp(p, "SP ", 3) == 0) {
        char *e = NULL; long val = strtol(p+3, &e, 10);
        if (e==p+3 || *e!='\0' || val<SET_POINT_MIN || val>SET_POINT_MAX) {
            printf("CMD ERR: SP must be %d..%d (0.1C units)\n", SET_POINT_MIN, SET_POINT_MAX); return;
        }
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            s_cmd.set_point = (int)val; s_cmd.changed = true;
            printf("CMD OK: SP=%d.%d C\n", s_cmd.set_point/10, s_cmd.set_point%10);
            xSemaphoreGive(s_cmd_mutex);
        }
        return;
    }
    if (strncmp(p, "KP ", 3) == 0) {
        char *e = NULL; long val = strtol(p+3, &e, 10);
        if (e==p+3 || *e!='\0' || val<0 || val>10000) { printf("CMD ERR: KP must be 0..10000 (Kp x100)\n"); return; }
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            s_cmd.kp = (int)val; s_cmd.changed = true;
            printf("CMD OK: Kp=%d.%02d\n", s_cmd.kp/100, s_cmd.kp%100);
            xSemaphoreGive(s_cmd_mutex);
        }
        return;
    }
    if (strncmp(p, "KI ", 3) == 0) {
        char *e = NULL; long val = strtol(p+3, &e, 10);
        if (e==p+3 || *e!='\0' || val<0 || val>10000) { printf("CMD ERR: KI must be 0..10000 (Ki x100)\n"); return; }
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            s_cmd.ki = (int)val; s_cmd.changed = true;
            printf("CMD OK: Ki=%d.%02d\n", s_cmd.ki/100, s_cmd.ki%100);
            xSemaphoreGive(s_cmd_mutex);
        }
        return;
    }
    if (strncmp(p, "KD ", 3) == 0) {
        char *e = NULL; long val = strtol(p+3, &e, 10);
        if (e==p+3 || *e!='\0' || val<0 || val>10000) { printf("CMD ERR: KD must be 0..10000 (Kd x100)\n"); return; }
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            s_cmd.kd = (int)val; s_cmd.changed = true;
            printf("CMD OK: Kd=%d.%02d\n", s_cmd.kd/100, s_cmd.kd%100);
            xSemaphoreGive(s_cmd_mutex);
        }
        return;
    }
    if (strcmp(p, "STATUS") == 0) {
        if (xSemaphoreTake(s_cmd_mutex, portMAX_DELAY) == pdTRUE) {
            printf("STATUS: SP=%d.%d C  Kp=%d.%02d  Ki=%d.%02d  Kd=%d.%02d\n",
                   s_cmd.set_point/10, s_cmd.set_point%10,
                   s_cmd.kp/100, s_cmd.kp%100,
                   s_cmd.ki/100, s_cmd.ki%100,
                   s_cmd.kd/100, s_cmd.kd%100);
            xSemaphoreGive(s_cmd_mutex);
        }
        return;
    }
    if (strcmp(p, "HELP") == 0) {
        printf("Commands:\n");
        printf("  UP / DOWN         raise/lower SP by 0.5 C\n");
        printf("  SP <val>          set SP (0.1C units, e.g. SP 270 = 27.0C)\n");
        printf("  KP <val>          set Kp x100 (e.g. KP 200 = Kp=2.00)\n");
        printf("  KI <val>          set Ki x100 (e.g. KI 50  = Ki=0.50)\n");
        printf("  KD <val>          set Kd x100 (e.g. KD 10  = Kd=0.10)\n");
        printf("  STATUS            print current parameters\n");
        printf("  HELP              show this help\n");
        return;
    }
    printf("CMD ERR: Unknown command. Use HELP\n");
}

void app_lab_6_2_task1_init() { s_cmd_mutex = xSemaphoreCreateMutex(); }

App62Cmd_t app_lab_6_2_task1_get_latest() {
    App62Cmd_t snap = { SET_POINT_DEFAULT, PID_KP_DEFAULT, PID_KI_DEFAULT, PID_KD_DEFAULT, false };
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
    printf("SP=%d.%d C  Kp=%d.%02d  Ki=%d.%02d  Kd=%d.%02d\n",
           SET_POINT_DEFAULT/10, SET_POINT_DEFAULT%10,
           PID_KP_DEFAULT/100, PID_KP_DEFAULT%100,
           PID_KI_DEFAULT/100, PID_KI_DEFAULT%100,
           PID_KD_DEFAULT/100, PID_KD_DEFAULT%100);
    printf("Commands: UP | DOWN | SP | KP | KI | KD | STATUS | HELP\n");
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(10));
        if (Serial.available() > 0) {
            scanf(" %47[^\r\n]", line_buf);
            apply_command(line_buf);
        }
    }
}
