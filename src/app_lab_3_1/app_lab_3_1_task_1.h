#ifndef APP_LAB_3_1_TASK_1_H
#define APP_LAB_3_1_TASK_1_H

#define PRESS_THRESHOLD_MS 500

extern int g_last_press_duration_ms;
extern int g_new_press_event;

void app_lab_3_1_task_1_setup();
void app_lab_3_1_task_1_loop();

#endif // APP_LAB_3_1_TASK_1_H