#include "app_lab_2_1_task_1.h"
#include "dd_led/dd_led.h"
#include "dd_button/dd_button.h"

void app_lab_2_1_task_1_setup(){

}

void app_lab_2_1_task_1_loop(){
    //asteptam apasarea butonului
    if(dd_button_is_pressed()){ //no spinlock
        printf("TASK 1: Button Pressed Detected\n");
        //verificam starea ledului
        if (dd_led_is_on()){
            // daca e aprins il stingem
            dd_led_turn_off();
    } else {
            // daca e stins il aprindem
            dd_led_turn_on();
        }
        // evit debounce-ul
        delay(300);
    }
}
