#ifndef APP_LAB_5_2_TASK_CONFIG_H
#define APP_LAB_5_2_TASK_CONFIG_H

#include <Arduino_FreeRTOS.h>
#include <semphr.h>

// Timing 
#define CMD_PERIOD_MS         20    // Task 1: command input polling
#define COND_PERIOD_MS        50    // Task 2: conditioning + actuator control
#define REPORT_PERIOD_MS      500   // Task 3: display and reporting
#define HEARTBEAT_MS          10000 // forced serial print interval

// Signal conditioning parameters 
#define MEDIAN_WINDOW         5     // must be odd
#define WMA_ALPHA             30    // IIR weight for newest sample (0..100%)
#define RAMP_STEP             10    // max PWM change per COND cycle (ramp rate)

// PWM range
#define PWM_MIN               0
#define PWM_MAX               255

// Alert thresholds (hysteresis)
#define ALERT_HIGH            220   // alert ON  above this
#define ALERT_LOW             200   // alert OFF below this

// Binary actuator
#define BIN_PERSISTENCE_SAMPLES 3

// Analog control modes
enum AnalogMode52 {
    ANALOG_MODE_AUTO = 0,
    ANALOG_MODE_MANUAL = 1
};

// Hardware pins
#define PIN_MOTOR_ENA         10
#define PIN_MOTOR_IN1         8
#define PIN_MOTOR_IN2         7
#define PIN_LED_RED           9    // binary actuator ON / overload alert
#define PIN_LED_GREEN         12   // system OK
#define PIN_LED_YELLOW        11   // limit reached
#define PIN_BIN_ACT           6    // binary actuator (relay/solenoid)

// Command structure: both binary and analog control
typedef struct {
    bool bin_requested;     // binary actuator ON/OFF request
    int  analog_mode;       // AUTO (potentiometer) or MANUAL (serial PWM)
    int  target_pwm;        // analog PWM target (0..255) - used in MANUAL mode
} App52Cmd_t;

// Shared snapshot written by Task 2, read by Task 3 
typedef struct {
    // Binary actuator
    bool bin_requested;     // what user requested
    bool bin_pending;       // debouncing in progress
    bool bin_state;         // actual current state
    
    // Analog actuator (motor)
    int  analog_mode;       // AUTO or MANUAL
    int  potentiometer_raw; // raw potentiometer value (for AUTO mode)
    int  raw;               // raw PWM command (0..255)
    int  saturated;         // after saturation clamping
    int  median;            // after median filter
    int  wma;               // after weighted moving average
    int  ramped;            // after ramp limiter (applied to hardware)
    bool alert;             // hysteresis alert flag
    bool at_limit;          // PWM is at maximum (255)
} App52Snapshot_t;

extern App52Snapshot_t     g_snap52;
extern SemaphoreHandle_t   g_snap52_mutex;

#endif // APP_LAB_5_2_TASK_CONFIG_H