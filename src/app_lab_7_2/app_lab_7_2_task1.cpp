#include "app_lab_7_2_task1.h"
#include "task_config.h"
#include "../dd_button/dd_button.h"
#include <Arduino_FreeRTOS.h>

// Single-byte volatile bools — atomically read/written on AVR, no mutex needed
static volatile bool s_ns_raw   = false;
static volatile bool s_emrg_raw = false;

bool app_lab_7_2_task1_get_ns_raw()   { return s_ns_raw;   }
bool app_lab_7_2_task1_get_emrg_raw() { return s_emrg_raw; }

void app_lab_7_2_task1(void *pvParameters) {
    (void)pvParameters;

    printf("==============================\n");
    printf(" Lab 7.2 - Smart Traffic Light\n");
    printf("------------------------------\n");
    printf(" EW : R=D%d  Y=D%d  G=D%d\n",
           PIN_EW_RED, PIN_EW_YELLOW, PIN_EW_GREEN);
    printf(" NS : R=D%d  Y=D%d  G=D%d\n",
           PIN_NS_RED, PIN_NS_YELLOW, PIN_NS_GREEN);
    printf(" NS request  : button D%d\n", PIN_BTN_NS);
    printf(" Emergency   : button D%d\n", PIN_BTN_EMERGENCY);
    printf(" Default     : EW GREEN\n");
    printf("==============================\n");

    TickType_t last_wake = xTaskGetTickCount();

    for (;;) {
        s_ns_raw   = (bool)dd_button_is_pressed();
        s_emrg_raw = (bool)dd_button_1_is_pressed();
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(ACQ_PERIOD_MS));
    }
}