#ifndef APP_LAB_5_1_TASK_CONFIG_H
#define APP_LAB_5_1_TASK_CONFIG_H

#include <Arduino_FreeRTOS.h>
#include <semphr.h>

#define ACTUATOR_CMD_PERIOD_MS     20
#define ACTUATOR_COND_PERIOD_MS    25
#define ACTUATOR_REPORT_PERIOD_MS  500
#define ACTUATOR_SERIAL_HEARTBEAT_MS 10000

#define BIN_CMD_PERSISTENCE_SAMPLES 3

#define PIN_LED_BIN_ON             9
#define PIN_LED_ALERT              11
#define PIN_LED_OK                 12

#define PIN_MOTOR_ENA              10
#define PIN_MOTOR_IN1              8
#define PIN_MOTOR_IN2              7

#define ANALOG_PWM_MIN               0
#define ANALOG_PWM_MAX             255
#define ANALOG_ALERT_HIGH          220
#define ANALOG_ALERT_LOW           200

enum AnalogControlMode {
    ANALOG_MODE_AUTO = 0,
    ANALOG_MODE_MANUAL = 1
};

typedef struct {
    bool bin_requested;
    bool bin_pending;
    bool bin_state;

    AnalogControlMode analog_mode;
    int angle_deg;
    int analog_requested_pwm;
    int analog_applied_pwm;

    bool analog_alert;
} App5Snapshot_t;

extern App5Snapshot_t g_app5_snapshot;
extern SemaphoreHandle_t g_app5_snapshot_mutex;

#endif // APP_LAB_5_1_TASK_CONFIG_H
