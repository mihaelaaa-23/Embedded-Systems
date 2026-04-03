#ifndef APP_LAB_6_1_TASK_CONFIG_H
#define APP_LAB_6_1_TASK_CONFIG_H

#include <Arduino_FreeRTOS.h>
#include <semphr.h>

// ── Timing ────────────────────────────────────────────────────────────────
#define CMD_PERIOD_MS         100   // Task 1: command input polling
#define ACQ_PERIOD_MS         200   // Task 2: sensor acquisition + control
#define REPORT_PERIOD_MS      500   // Task 3: display and serial reporting
#define HEARTBEAT_MS          10000 // forced serial print interval (no change)

// ── ON-OFF control with hysteresis ───────────────────────────────────────
// Relay turns ON  when temp >= SET_POINT + HYST_BAND / 2
// Relay turns OFF when temp <= SET_POINT - HYST_BAND / 2
#define SET_POINT_DEFAULT     250   // default SetPoint in 0.1 °C units (25.0 °C)
#define SET_POINT_MIN         100   // 10.0 °C
#define SET_POINT_MAX         400   // 40.0 °C
#define SET_POINT_STEP        5     // 0.5 °C per UP/DOWN press
#define HYST_BAND_DEFAULT     20    // ±1.0 °C  (band = 2 °C total)
#define HYST_BAND_MIN         5     // 0.5 °C
#define HYST_BAND_MAX         100   // 10.0 °C

// ── Additional behaviour: deviation alert (10% bonus) ─────────────────────
// Alert when |temp - setpoint| > ALERT_DEVIATION
#define ALERT_DEVIATION       50    // 5.0 °C

// ── Binary actuator (relay) ───────────────────────────────────────────────
#define BIN_PERSISTENCE_SAMPLES 3   // debounce filter depth

// ── Hardware pins ─────────────────────────────────────────────────────────
// Relay connected to digital pin 6 (same as lab 5.2 binary actuator)
#define PIN_RELAY             6

// LEDs reuse lab 5.2 pins
#define PIN_LED_RED           9    // relay ON / deviation alert
#define PIN_LED_GREEN         12   // system OK (relay OFF, no alert)
#define PIN_LED_YELLOW        11   // deviation alert

// ── Shared data structures ────────────────────────────────────────────────
typedef struct {
    int  set_point;       // SetPoint in 0.1 °C  (e.g. 250 = 25.0 °C)
    int  hyst_band;       // hysteresis half-band in 0.1 °C
    bool sp_changed;      // flag: Task 1 wrote a new SP
} App61Cmd_t;

typedef struct {
    // Sensor readings
    int  temperature_raw; // 0.1 °C  (e.g. 235 = 23.5 °C)
    int  temperature_c;   // integer °C
    int  humidity;        // integer %RH

    // Control
    int  set_point;       // active SetPoint in 0.1 °C
    int  hyst_band;       // active hysteresis band in 0.1 °C
    int  error;           // temperature_raw - set_point  (signed)

    // Thresholds used this cycle
    int  threshold_on;    // set_point + hyst_band
    int  threshold_off;   // set_point - hyst_band

    // Relay state (via dd_actuator_bin)
    bool relay_requested; // what control logic requested
    bool relay_pending;   // debouncing in progress
    bool relay_state;     // actual output state

    // Alerts
    bool deviation_alert; // |error| > ALERT_DEVIATION
} App61Snapshot_t;

extern App61Snapshot_t    g_snap61;
extern SemaphoreHandle_t  g_snap61_mutex;

#endif // APP_LAB_6_1_TASK_CONFIG_H