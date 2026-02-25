#include "app_lab_3_2_task_3.h"
#include "app_lab_3_2_task_2.h"
#include <stdio.h>

#define REPORT_INTERVAL_MS 10000

void app_lab_3_2_task_3(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, REPORT_INTERVAL_MS / portTICK_PERIOD_MS);

        xSemaphoreTake(xStatsMutex, portMAX_DELAY);
        int           total    = g_3_2_total_presses;
        int           shorts   = g_3_2_short_presses;
        int           longs    = g_3_2_long_presses;
        unsigned long total_ms = g_3_2_sum_short_ms + g_3_2_sum_long_ms;
        int           avg_ms   = (total > 0) ? (int)(total_ms / total) : 0;
        g_3_2_total_presses = 0;
        g_3_2_short_presses = 0;
        g_3_2_long_presses  = 0;
        g_3_2_sum_short_ms  = 0;
        g_3_2_sum_long_ms   = 0;
        xSemaphoreGive(xStatsMutex);

        printf("=== TASK 3 REPORT (10s) ===\n");
        printf("  Total presses : %d\n",    total);
        printf("  Short (<500ms): %d\n",    shorts);
        printf("  Long (>=500ms): %d\n",    longs);
        printf("  Avg duration  : %d ms\n", avg_ms);
        printf("===========================\n");
    }
}