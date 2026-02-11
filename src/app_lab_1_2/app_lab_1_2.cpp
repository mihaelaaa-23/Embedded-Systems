#include "app_lab_1_2.h"
#include "srv_stdio_lcd/srv_stdio_lcd.h"
#include "srv_stdio_keypad/srv_stdio_keypad.h"
#include "dd_led/dd_led.h"
#include <stdio.h>

void app_lab_1_2_setup() {
    srv_stdio_lcd_setup();
    srv_stdio_keypad_setup();
    dd_led_setup();
    printf("\x1bSystem is activated!\n");
    delay(1000);
}

char code[4] = {'1', '2', '3', '4'};
char input[4];

void app_lab_1_2_loop() {
    printf("Enter Code:"); // Curățăm ecranul înainte de fiecare încercare    
    scanf("%c%c%c%c", &input[0], &input[1], &input[2], &input[3]);

    printf("\x1b[2J\n"); // Curățăm ecranul pentru a afișa rezultatul

    if (input[0] == code[0] && input[1] == code[1] && input[2] == code[2] && input[3] == code[3]) {
        printf("Correct code!\n");
        dd_led_turn_on(LED_GREEN);
        dd_led_turn_off(LED_RED);
    } else {
        printf("Wrong code!\n");
        dd_led_turn_on(LED_RED);
        dd_led_turn_off(LED_GREEN);
    }

    delay(2000); // Așteptăm să vadă utilizatorul mesajul și LED-ul
    dd_led_turn_off(LED_GREEN);
    dd_led_turn_off(LED_RED);
}