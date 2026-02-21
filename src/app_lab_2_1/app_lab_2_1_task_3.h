#ifndef APP_LAB_2_1_TASK_3_H
#define APP_LAB_2_1_TASK_3_H

#define TASK_3_VAR_MIN 1
#define TASK_3_VAR_MAX 10
#define TASK_3_VAR_DEFAULT 5

// Provider signal: number of blink recurrences for Task 2 LED
// Consumer: Task 2 reads this to know how many times to blink
extern volatile int g_task3_blink_count;

void app_lab_2_1_task_3_setup();
void app_lab_2_1_task_3_loop();

#endif // APP_LAB_2_1_TASK_3_H