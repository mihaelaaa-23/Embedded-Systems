#include "app_lab_1_1.h"

#include "Arduino.h"
#include <string.h> // Required for strcmp()

#include "srv_serial_stdio/srv_serial_stdio.h"
#include "dd_led/dd_led.h"

void app_lab_1_1_setup() {
    // Initialize Middleware (Serial STDIO) and Driver (LED) layers
    srv_serial_stdio_setup();
    dd_led_setup();
    
    // Print boot message and usage hint via standard output
    printf("app_lab_1_1: Started\n");
    printf("Type 'help' for a list of commands.\n");
}

// Global buffer to store the incoming command string
char cmd[32];

void app_lab_1_1_loop() {
    printf("\r\nEnter command: ");
    
    // Read input: skip leading space, read up to 31 chars, stop at \r or \n
    scanf(" %31[^\r\n]", cmd);  
    
    // Echo the command back to the user for confirmation
    printf("\r\nCommand: %s\n", cmd);

    // --- Command Processing ---
    if (strcmp(cmd, "led on") == 0){
        printf("\rLED ON\n");
        dd_led_turn_on();
    }
    else if (strcmp(cmd, "led off") == 0){
        printf("\rLED OFF\n");
        dd_led_turn_off();
    }
    // --- BONUS: Interactive Help Menu ---
    else if (strcmp(cmd, "help") == 0) {
        printf("\rAvailable Commands:\n");
        printf(" - led on    : Turns the LED permanently ON\n");
        printf(" - led off   : Turns the LED permanently OFF\n");
        printf(" - led blink : Blinks the LED 3 times\n");
        printf(" - help      : Displays this menu\n");
    }
    // --- BONUS: Blink Sequence Control ---
    else if (strcmp(cmd, "led blink") == 0) {
        printf("\rBlinking LED Sequence...\n");
        // Toggle LED 3 times with 200ms delay
        for(int i = 0; i < 3; i++) {
            dd_led_turn_on();
            delay(200); 
            dd_led_turn_off();
            delay(200); 
        }
        printf("Sequence Complete.\n");
    }
    else {
        // Error handling for unknown commands
        printf("\rError: Unknown command '%s'. Type 'help'.\n", cmd);
    }

    // Short delay before next loop iteration
    delay(1000);
}