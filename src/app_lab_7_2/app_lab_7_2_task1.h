#ifndef APP_LAB_7_2_TASK1_H
#define APP_LAB_7_2_TASK1_H

#include <Arduino_FreeRTOS.h>

void app_lab_7_2_task1(void *pvParameters);

// Raw button readings — volatile bool, atomically read on AVR, no mutex needed
bool app_lab_7_2_task1_get_ns_raw();
bool app_lab_7_2_task1_get_emrg_raw();

#endif // APP_LAB_7_2_TASK1_H