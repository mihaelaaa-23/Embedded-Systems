#include "app_lab_3_2.h"
#include "Arduino_FreeRTOS.h"
#include "semphr.h"
#include "srv_serial_stdio/srv_serial_stdio.h"
#include "dd_button/dd_button.h"
#include "dd_led/dd_led.h"

void app_lab_3_2_task_1(void *pvParameters);
void app_lab_3_2_task_2(void *pvParameters);
void app_lab_3_2_task_3(void *pvParameters);

SemaphoreHandle_t xSemaphore;
SemaphoreHandle_t xStatsMutex;

void app_lab_3_2_setup()
{
    dd_led_setup();
    dd_button_setup();
    srv_serial_stdio_setup();

    printf("App Lab 3.2: Started\n");

    xSemaphore  = xSemaphoreCreateBinary();
    xStatsMutex = xSemaphoreCreateMutex();

    xTaskCreate(app_lab_3_2_task_1, "Task 1", 256, NULL, 1, NULL);
    xTaskCreate(app_lab_3_2_task_2, "Task 2", 256, NULL, 1, NULL);
    xTaskCreate(app_lab_3_2_task_3, "Task 3", 256, NULL, 1, NULL);

    vTaskStartScheduler();
}

void app_lab_3_2_loop()
{
}