#ifndef SRV_OS_SEQ_H
#define SRV_OS_SEQ_H

#define SRV_OS_SYS_TICK 1
#define TIME_SEC 1000

// Recurenta task-urilor
#define APP_LAB_2_1_TASK_1_REC    (TIME_SEC / 10) / SRV_OS_SYS_TICK   // 100ms
#define APP_LAB_2_1_TASK_2_REC    (TIME_SEC / 2)  / SRV_OS_SYS_TICK   // 500ms
#define APP_LAB_2_1_TASK_3_REC    (TIME_SEC / 10) / SRV_OS_SYS_TICK   // 100ms

// Offset-uri
#define APP_LAB_2_1_TASK_1_OFFSET 0
#define APP_LAB_2_1_TASK_2_OFFSET (200 / SRV_OS_SYS_TICK)
#define APP_LAB_2_1_TASK_3_OFFSET (400 / SRV_OS_SYS_TICK)

extern int app_lab_2_1_task_1_cnt;
extern int app_lab_2_1_task_2_cnt;
extern int app_lab_2_1_task_3_cnt;

void svr_os_seq_setup();

#endif // SRV_OS_SEQ_H