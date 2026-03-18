#include "dd_actuator_analog/dd_actuator_analog.h"
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

static uint8_t s_ena_pin = 10;
static uint8_t s_in1_pin = 8;
static uint8_t s_in2_pin = 7;
static int s_requested_pwm = 0;
static int s_applied_pwm = 0;
static SemaphoreHandle_t s_mutex = NULL;

static int clamp_pwm(int value) {
    if (value < DD_ACTUATOR_ANALOG_PWM_MIN) return DD_ACTUATOR_ANALOG_PWM_MIN;
    if (value > DD_ACTUATOR_ANALOG_PWM_MAX) return DD_ACTUATOR_ANALOG_PWM_MAX;
    return value;
}

void dd_actuator_analog_setup(uint8_t ena_pin, uint8_t in1_pin, uint8_t in2_pin) {
    s_ena_pin = ena_pin;
    s_in1_pin = in1_pin;
    s_in2_pin = in2_pin;

    pinMode(s_ena_pin, OUTPUT);
    pinMode(s_in1_pin, OUTPUT);
    pinMode(s_in2_pin, OUTPUT);

    digitalWrite(s_in1_pin, LOW);
    digitalWrite(s_in2_pin, LOW);
    analogWrite(s_ena_pin, 0);

    s_requested_pwm = 0;
    s_applied_pwm = 0;
    s_mutex = xSemaphoreCreateMutex();
}

void dd_actuator_analog_set_requested_pwm(int pwm) {
    if (xSemaphoreTake(s_mutex, portMAX_DELAY) == pdTRUE) {
        s_requested_pwm = clamp_pwm(pwm);
        xSemaphoreGive(s_mutex);
    }
}

void dd_actuator_analog_apply() {
    if (xSemaphoreTake(s_mutex, portMAX_DELAY) == pdTRUE) {
        s_applied_pwm = clamp_pwm(s_requested_pwm);

        if (s_applied_pwm > 0) {
            // L298 forward drive on channel A: IN1=HIGH, IN2=LOW, ENA=PWM.
            digitalWrite(s_in1_pin, HIGH);
            digitalWrite(s_in2_pin, LOW);
        } else {
            // Coast/stop when requested PWM is zero.
            digitalWrite(s_in1_pin, LOW);
            digitalWrite(s_in2_pin, LOW);
        }

        analogWrite(s_ena_pin, s_applied_pwm);
        xSemaphoreGive(s_mutex);
    }
}

int dd_actuator_analog_get_requested_pwm() {
    int value = 0;
    if (xSemaphoreTake(s_mutex, portMAX_DELAY) == pdTRUE) {
        value = s_requested_pwm;
        xSemaphoreGive(s_mutex);
    }
    return value;
}

int dd_actuator_analog_get_applied_pwm() {
    int value = 0;
    if (xSemaphoreTake(s_mutex, portMAX_DELAY) == pdTRUE) {
        value = s_applied_pwm;
        xSemaphoreGive(s_mutex);
    }
    return value;
}
