#include "dd_actuator_bin/dd_actuator_bin.h"
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

static uint8_t s_pin = 13;
static bool s_requested = false;
static bool s_pending_state = false;
static int s_pending_count = 0;
static bool s_filtered_state = false;
static uint8_t s_persistence_samples = 3;
static SemaphoreHandle_t s_mutex = NULL;

void dd_actuator_bin_setup(uint8_t pin, uint8_t persistence_samples) {
    s_pin = pin;
    pinMode(s_pin, OUTPUT);
    digitalWrite(s_pin, LOW);

    if (persistence_samples == 0) persistence_samples = 1;
    s_persistence_samples = persistence_samples;

    s_requested = false;
    s_pending_state = false;
    s_pending_count = 0;
    s_filtered_state = false;
    s_mutex = xSemaphoreCreateMutex();
}

void dd_actuator_bin_set_requested(bool on) {
    if (xSemaphoreTake(s_mutex, portMAX_DELAY) == pdTRUE) {
        s_requested = on;
        xSemaphoreGive(s_mutex);
    }
}

void dd_actuator_bin_step() {
    if (xSemaphoreTake(s_mutex, portMAX_DELAY) == pdTRUE) {
        if (s_requested == s_pending_state) {
            if (s_pending_count < s_persistence_samples) {
                s_pending_count++;
            }
        } else {
            s_pending_state = s_requested;
            s_pending_count = 1;
        }

        if (s_pending_count >= s_persistence_samples) {
            s_filtered_state = s_pending_state;
            digitalWrite(s_pin, s_filtered_state ? HIGH : LOW);
            s_pending_count = s_persistence_samples;
        }

        xSemaphoreGive(s_mutex);
    }
}

bool dd_actuator_bin_get_requested() {
    bool value = false;
    if (xSemaphoreTake(s_mutex, portMAX_DELAY) == pdTRUE) {
        value = s_requested;
        xSemaphoreGive(s_mutex);
    }
    return value;
}

bool dd_actuator_bin_get_pending() {
    bool value = false;
    if (xSemaphoreTake(s_mutex, portMAX_DELAY) == pdTRUE) {
        value = (s_requested != s_filtered_state);
        xSemaphoreGive(s_mutex);
    }
    return value;
}

bool dd_actuator_bin_get_state() {
    bool value = false;
    if (xSemaphoreTake(s_mutex, portMAX_DELAY) == pdTRUE) {
        value = s_filtered_state;
        xSemaphoreGive(s_mutex);
    }
    return value;
}
