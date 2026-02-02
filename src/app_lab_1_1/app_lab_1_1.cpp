#include "app_lab_1_1.h"

#include "Arduino.h"

#include "srv_serial_stdio/srv_serial_stdio.h"
#include "dd_led/dd_led.h"

void app_lab_1_1_setup() {
    // Serial.println("app_lab_1_1: Started");
    srv_serial_stdio_setup();
    dd_led_setup();
    
    // Utilizare STDIO
    printf("app_lab_1_1: Started\n");
}

char cmd;

void app_lab_1_1_loop() {
    printf("\r\nEnter command: Led ON/OFF: ");
    scanf(" %c", &cmd);  
    
    printf("\r\nCommand: %c\n", cmd);

    if (cmd == '1'){
        printf("\rLED ON\n");
        dd_led_turn_on();
    }
    else if (cmd == '0'){
        printf("\rLED OFF\n");
        dd_led_turn_off();
    }

    delay(1000);
}