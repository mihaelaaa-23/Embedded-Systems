#include "app_lab_7_1_task1.h"
#include "task_config.h"
#include "../dd_button/dd_button.h"
#include <Arduino_FreeRTOS.h>

// ── Internal state ────────────────────────────────────────────────────────────
// A single bool is one byte — atomically read/written on AVR without a mutex.
static volatile bool s_raw_pressed = false;

// ── Public getter (called from T2) ────────────────────────────────────────────
bool app_lab_7_1_task1_get_raw() {
    return s_raw_pressed;
}

// ── Task body ─────────────────────────────────────────────────────────────────
void app_lab_7_1_task1(void *pvParameters) {
    (void)pvParameters;

    printf("==============================\n");
    printf(" Lab 7.1 - FSM Button-LED\n");
    printf("------------------------------\n");
    printf(" Button : pin %d (INPUT_PULLUP)\n", PIN_BUTTON);
    printf(" LED    : pin %d (FSM output)\n",   PIN_LED_CTRL);
    printf(" States : OFF -> BLINK(%dx) -> ON\n", BLINK_COUNT);
    printf(" Press button to toggle LED.\n");
    printf("==============================\n");

    TickType_t last_wake = xTaskGetTickCount();

    for (;;) {
        s_raw_pressed = (bool)dd_button_is_pressed();
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(ACQ_PERIOD_MS));
    }
}