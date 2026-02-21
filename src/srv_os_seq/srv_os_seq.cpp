#include "srv_os_seq/srv_os_seq.h"
#include "app_lab_2_1/app_lab_2_1_task_1.h"
#include "app_lab_2_1/app_lab_2_1_task_2.h"
#include <stdio.h>

#include "timer-api.h"

#include "srv_os_seq/srv_os_seq.h"
#include "app_lab_2_1/app_lab_2_1_task_1.h"
#include "app_lab_2_1/app_lab_2_1_task_2.h"
#include "app_lab_2_1/app_lab_2_1_task_3.h"
#include <stdio.h>

#include "timer-api.h"

// Contoare individuale initializate cu offset-ul corespunzator
int app_lab_2_1_task_1_cnt = APP_LAB_2_1_TASK_1_OFFSET + APP_LAB_2_1_TASK_1_REC;
int app_lab_2_1_task_2_cnt = APP_LAB_2_1_TASK_2_OFFSET + APP_LAB_2_1_TASK_2_REC;
int app_lab_2_1_task_3_cnt = APP_LAB_2_1_TASK_3_OFFSET + APP_LAB_2_1_TASK_3_REC;

// system timer
void svr_os_seq_setup()
{

    // freq=1Hz, period=1s
    // частота=1Гц, период=1с
    // timer_init_ISR_1Hz(TIMER_DEFAULT);

    // freq=2Hz, period=500ms
    // частота=2Гц, период=500мс
    // timer_init_ISR_2Hz(TIMER_DEFAULT);

    // freq=5Hz, period=200ms
    // частота=5Гц, период=200мс
    // timer_init_ISR_5Hz(TIMER_DEFAULT);

    // timer_init_ISR_500KHz(TIMER_DEFAULT);
    // timer_init_ISR_200KHz(TIMER_DEFAULT);
    //  timer_init_ISR_100KHz(TIMER_DEFAULT);
    // timer_init_ISR_50KHz(TIMER_DEFAULT);
    // timer_init_ISR_20KHz(TIMER_DEFAULT);
    // timer_init_ISR_10KHz(TIMER_DEFAULT);
    // timer_init_ISR_5KHz(TIMER_DEFAULT);
    // timer_init_ISR_2KHz(TIMER_DEFAULT);
    timer_init_ISR_1KHz(TIMER_DEFAULT);
    // timer_init_ISR_500Hz(TIMER_DEFAULT);
    // timer_init_ISR_200Hz(TIMER_DEFAULT);
    // timer_init_ISR_100Hz(TIMER_DEFAULT);
    // timer_init_ISR_50Hz(TIMER_DEFAULT);
    // timer_init_ISR_20Hz(TIMER_DEFAULT);
    // timer_init_ISR_10Hz(TIMER_DEFAULT);
    // timer_init_ISR_5Hz(TIMER_DEFAULT);
    // timer_init_ISR_2Hz(TIMER_DEFAULT);
    // timer_init_ISR_1Hz(TIMER_DEFAULT);
}

void timer_handle_interrupts(int timer)
{
    // Task 1 – Button LED (recurenta 100ms, offset 0ms)
    if (--app_lab_2_1_task_1_cnt <= 0)
    {
        app_lab_2_1_task_1_cnt = APP_LAB_2_1_TASK_1_REC;
        app_lab_2_1_task_1_loop();
    }

    // Task 2 – LED Intermitent (recurenta 500ms, offset 200ms)
    if (--app_lab_2_1_task_2_cnt <= 0)
    {
        app_lab_2_1_task_2_cnt = APP_LAB_2_1_TASK_2_REC;
        app_lab_2_1_task_2_loop();
    }

    // Task 3 – Variabila de stare (recurenta 100ms, offset 400ms)
    if (--app_lab_2_1_task_3_cnt <= 0)
    {
        app_lab_2_1_task_3_cnt = APP_LAB_2_1_TASK_3_REC;
        app_lab_2_1_task_3_loop();
    }
}