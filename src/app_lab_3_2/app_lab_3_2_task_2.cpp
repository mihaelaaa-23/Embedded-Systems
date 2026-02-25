#include "app_lab_3_2_task_2.h"
#include "app_lab_3_2_task_1.h"
#include "dd_led/dd_led.h"
#include <stdio.h>

int           g_3_2_total_presses = 0;
int           g_3_2_short_presses = 0;
int           g_3_2_long_presses  = 0;
unsigned long g_3_2_sum_short_ms  = 0;
unsigned long g_3_2_sum_long_ms   = 0;

void app_lab_3_2_task_2(void *pvParameters)
{
    while (1)
    {
        if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE)
        {
            xSemaphoreTake(xStatsMutex, portMAX_DELAY);
            int duration  = g_3_2_last_duration_ms;
            int was_short = g_3_2_last_was_short;
            g_3_2_total_presses++;
            if (was_short)
            {
                g_3_2_short_presses++;
                g_3_2_sum_short_ms += duration;
            }
            else
            {
                g_3_2_long_presses++;
                g_3_2_sum_long_ms += duration;
            }
            xSemaphoreGive(xStatsMutex);

            printf("TASK 2: total=%d short=%d long=%d\n",
                   g_3_2_total_presses, g_3_2_short_presses, g_3_2_long_presses);

            int blinks = was_short ? 5 : 10;

            for (int i = 0; i < blinks; i++)
            {
                dd_led_2_set_target(1);
                dd_led_apply();
                vTaskDelay(100 / portTICK_PERIOD_MS);
                dd_led_2_set_target(0);
                dd_led_apply();
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }

            dd_led_set_target(0);
            dd_led_1_set_target(0);
            dd_led_2_set_target(0);
            dd_led_apply();
        }
    }
}