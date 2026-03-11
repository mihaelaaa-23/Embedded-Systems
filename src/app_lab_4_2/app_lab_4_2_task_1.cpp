#include "app_lab_4_2_task_1.h"
#include "task_config.h"
#include "dd_sns_temperature/dd_sns_temperature.h"
#include "dd_sns_dht/dd_sns_dht.h"
#include "dd_sns_gas/dd_sns_gas.h"
#include <Arduino_FreeRTOS.h>

// Task 1 – Sensor Acquisition (50 ms, priority 3)
// Refreshes both sensor drivers so dd_sns_*_get_*() return current values.
void task_acq_4_2(void *pvParameters) {
    (void)pvParameters;

    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        dd_sns_temperature_loop();   // S1 – analog potentiometer (ADC)
        dd_sns_dht_loop();           // S2 – digital DHT22 (UART/pin)
        dd_sns_gas_loop();           // S3 – MQ-2 gas sensor (ADC)
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(ACQ_PERIOD_MS));
    }
}