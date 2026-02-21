#include "app_lab_2_1.h"
#include "srv_serial_stdio/srv_serial_stdio.h"
#include "app_lab_2_1_task_1.h"
#include "app_lab_2_1_task_2.h"
#include "app_lab_2_1_task_3.h"
#include "dd_led/dd_led.h"
#include "dd_button/dd_button.h"
#include "srv_os_seq/srv_os_seq.h"


void app_lab_2_1_setup() {
    dd_led_setup();
    dd_button_setup();
    srv_serial_stdio_setup();
    svr_os_seq_setup();
    app_lab_2_1_task_1_setup();
    app_lab_2_1_task_2_setup();
    app_lab_2_1_task_3_setup();
    printf("App Lab 2.1: Started\n");
}

// Idle loop – Consumer: citeste variabilele globale/semnalele produse de taskuri
// si le raporteaza prin STDIO (printf). Rulat in bucla infinita/IDLE (main loop).
void app_lab_2_1_loop() {
    // Consumer: preia starea LED-ului din Task 1 (provider: dd_led)
    int led_state = dd_led_is_on();

    // Consumer: preia starea LED-ului intermitent din Task 2 (provider: dd_led_1)
    int led_blink_state = dd_led_1_is_on();

    // Consumer: preia variabila de stare din Task 3 (provider: g_task3_blink_count)
    int blink_count = g_task3_blink_count;

    printf("App Lab 2.1: Idle | LED1=%s | LED2(blink)=%s | BlinkCount=%d\n",
           led_state      ? "ON"  : "OFF",
           led_blink_state ? "ON"  : "OFF",
           blink_count);

    delay(1000);
}