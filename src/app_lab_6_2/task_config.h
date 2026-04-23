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
#define SET_POINT_DEFAULT    250    // 25.0 C (0.1 C units)
#define SET_POINT_MIN        100
#define SET_POINT_MAX        400
#define SET_POINT_STEP         5    // 0.5 C per UP/DOWN

// ── PID parameters (scaled x100) ─────────────────────────────────────────────
#define PID_KP_DEFAULT       200    // Kp = 2.00
#define PID_KI_DEFAULT        50    // Ki = 0.50
#define PID_KD_DEFAULT        10    // Kd = 0.10
#define PID_OUT_MIN            0
#define PID_OUT_MAX          100
#define PID_INTEGRAL_MAX    5000

// ── Time-proportional relay window ───────────────────────────────────────────
#define RELAY_WINDOW_STEPS    10    // 10 x 200ms = 2s window

// ── Deviation alert ──────────────────────────────────────────────────────────
#define ALERT_DEVIATION       50    // |error| > 5.0 C

// ── Hardware pins ─────────────────────────────────────────────────────────────
#define PIN_RELAY              6
#define PIN_LED_RED            9
#define PIN_LED_GREEN         12
#define PIN_LED_YELLOW        11

// ── Control mode ─────────────────────────────────────────────────────────────
typedef enum {
    CTRL_MODE_AUTO   = 0,   // PID computes output automatically
    CTRL_MODE_MANUAL = 1    // operator sets output directly via OUT command
} CtrlMode_t;

// ── Shared command structure (Task 1 → Task 2) ───────────────────────────────
typedef struct {
    int        set_point;
    int        kp;
    int        ki;
    int        kd;
    CtrlMode_t mode;          // AUTO or MANUAL
    int        manual_output; // 0-100 %, used only in MANUAL mode
    bool       changed;
} App62Cmd_t;

// ── Shared snapshot structure (Task 2 → Task 3) ──────────────────────────────
typedef struct {
    int        temperature_raw;
    int        humidity;
    int        set_point;
    int        error;
    int        pid_output;
    int        kp;
    int        ki;
    int        kd;
    CtrlMode_t mode;
    bool       relay_state;
    bool       deviation_alert;
} App62Snapshot_t;

extern App62Snapshot_t    g_snap62;
extern SemaphoreHandle_t  g_snap62_mutex;

#endif
