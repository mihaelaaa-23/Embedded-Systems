#ifndef APP_LAB_7_1_TASK1_H
#define APP_LAB_7_1_TASK1_H

#include <Arduino_FreeRTOS.h>

void app_lab_7_1_task1(void *pvParameters);

// Returns the most recent raw button reading (true = pressed).
// Called by T2; protected by internal mutex.
bool app_lab_7_1_task1_get_raw();

#endif // APP_LAB_7_1_TASK1_H
