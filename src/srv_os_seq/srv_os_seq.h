#ifndef SRV_OS_SEQ_H
#define SRV_OS_SEQ_H

#define SRV_OS_SYS_TICK 1
#define TIME_SEC        1000

// Recurenta task-urilor (in tick-uri de timer @ 1KHz)
#define APP_LAB_2_1_TASK_1_REC    (100 / SRV_OS_SYS_TICK)   // 100ms
#define APP_LAB_2_1_TASK_2_REC    (500 / SRV_OS_SYS_TICK)   // 500ms
#define APP_LAB_2_1_TASK_3_REC    (100 / SRV_OS_SYS_TICK)   // 100ms

// Offset-uri pentru activarea in ordinea cuvenita a task-urilor
#define APP_LAB_2_1_TASK_1_OFFSET (0   / SRV_OS_SYS_TICK)   // start imediat
#define APP_LAB_2_1_TASK_2_OFFSET (200 / SRV_OS_SYS_TICK)   // porneste dupa 200ms
#define APP_LAB_2_1_TASK_3_OFFSET (400 / SRV_OS_SYS_TICK)   // porneste dupa 400ms

// Contoare individuale pentru fiecare task (definite in srv_os_seq.cpp)
extern int app_lab_2_1_task_1_cnt;
extern int app_lab_2_1_task_2_cnt;
extern int app_lab_2_1_task_3_cnt;

void svr_os_seq_setup();

#endif // SRV_OS_SEQ_H