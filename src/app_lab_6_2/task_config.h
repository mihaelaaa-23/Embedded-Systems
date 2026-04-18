#ifndef APP_LAB_6_2_TASK_CONFIG_H
#define APP_LAB_6_2_TASK_CONFIG_H

#include <Arduino_FreeRTOS.h>
#include <semphr.h>

// ── Timing ──────────────────────────────────────────────────────────────────
#define CMD_PERIOD_MS        100
#define ACQ_PERIOD_MS        200
#define REPORT_PERIOD_MS     500
#define HEARTBEAT_MS       10000

// ── SetPoint ─────────────────────────────────────────────────────────────────
#define SET_POINT_DEFAULT    250    // 25.0 °C (0.1 °C units)
#define SET_POINT_MIN        100
#define SET_POINT_MAX        400
#define SET_POINT_STEP         5    // 0.5 °C per UP/DOWN

// ── PID parameters (scaled ×100) ─────────────────────────────────────────────
#define PID_KP_DEFAULT       200    // Kp = 2.00
#define PID_KI_DEFAULT        50    // Ki = 0.50
#define PID_KD_DEFAULT        10    // Kd = 0.10
#define PID_OUT_MIN            0
#define PID_OUT_MAX          100
#define PID_INTEGRAL_MAX    5000

// ── Time-proportional relay window ───────────────────────────────────────────
// Window = RELAY_WINDOW_STEPS × ACQ_PERIOD_MS = 10 × 200ms = 2000ms
// PID output 0-100 % → relay ON for 0-10 steps per window
#define RELAY_WINDOW_STEPS    10

// ── Deviation alert ──────────────────────────────────────────────────────────
#define ALERT_DEVIATION       50    // |error| > 5.0 °C

// ── Hardware pins ─────────────────────────────────────────────────────────────
#define PIN_RELAY              6
#define PIN_LED_RED            9
#define PIN_LED_GREEN         12
#define PIN_LED_YELLOW        11

// ── Shared structures ────────────────────────────────────────────────────────
typedef struct {
    int  set_point;
    int  kp;
    int  ki;
    int  kd;
    bool changed;
} App62Cmd_t;

typedef struct {
    int  temperature_raw;
    int  humidity;
    int  set_point;
    int  error;
    int  pid_output;
    int  kp;
    int  ki;
    int  kd;
    bool relay_state;
    bool deviation_alert;
} App62Snapshot_t;

extern App62Snapshot_t    g_snap62;
extern SemaphoreHandle_t  g_snap62_mutex;

#endif
