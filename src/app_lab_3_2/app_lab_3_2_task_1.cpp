#include "app_lab_3_2_task_1.h"
#include "dd_button/dd_button.h"
#include "dd_led/dd_led.h"
#include <stdio.h>

int g_3_2_last_duration_ms = 0;
int g_3_2_last_was_short   = 0;

void app_lab_3_2_task_1(void *pvParameters)
{
    vTaskDelay(TASK_1_OFFSET / portTICK_PERIOD_MS);

    int btn_stable = dd_button_is_pressed();
    int stable_cnt = 0;
    TickType_t press_start = 0;

    while (1)
    {
        int btn_now = dd_button_is_pressed();

        if (btn_now == btn_stable)
        {
            stable_cnt = 0;
        }
        else
        {
            stable_cnt++;
            if (stable_cnt >= DEBOUNCE_TICKS)
            {
                btn_stable = btn_now;
                stable_cnt = 0;

                if (btn_stable == 1)
                {
                    press_start = xTaskGetTickCount();
                }
                else
                {
                    TickType_t now = xTaskGetTickCount();
                    int duration_ms = (int)((now - press_start) * portTICK_PERIOD_MS);

                    if (duration_ms < MIN_PRESS_MS || duration_ms > MAX_PRESS_MS)
                    {
                        vTaskDelay(TASK_1_REC / portTICK_PERIOD_MS);
                        continue;
                    }

                    xSemaphoreTake(xStatsMutex, portMAX_DELAY);
                    g_3_2_last_duration_ms = duration_ms;
                    g_3_2_last_was_short   = (duration_ms < PRESS_THRESHOLD_MS) ? 1 : 0;
                    xSemaphoreGive(xStatsMutex);

                    if (g_3_2_last_was_short)
                    {
                        dd_led_set_target(0);
                        dd_led_1_set_target(1);
                        dd_led_apply();
                        printf("TASK 1: SHORT press %dms - GREEN LED ON\n", duration_ms);
                    }
                    else
                    {
                        dd_led_1_set_target(0);
                        dd_led_set_target(1);
                        dd_led_apply();
                        printf("TASK 1: LONG press %dms - RED LED ON\n", duration_ms);
                    }

                    xSemaphoreGive(xSemaphore);
                }
            }
        }

        vTaskDelay(TASK_1_REC / portTICK_PERIOD_MS);
    }
}