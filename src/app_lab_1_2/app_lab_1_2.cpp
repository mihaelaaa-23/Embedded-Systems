#include "app_lab_1_2.h"
#include "srv_stdio_lcd/srv_stdio_lcd.h"
#include "srv_stdio_keypad/srv_stdio_keypad.h"
#include <stdio.h>

void app_lab_1_2_setup() {
    srv_stdio_lcd_setup();
    srv_stdio_keypad_setup();
    printf("Hello, World!\n");
}

char code[4] = {'1', '2', '3', '4'};
char input[4];

void app_lab_1_2_loop() {
    scanf("%c%c%c%c", &input[0], &input[1], &input[2], &input[3]);

    printf("\x1b");

    if (input[0] == code[0] && input[1] == code[1] && input[2] == code[2] && input[3] == code[3]) {
        printf("Correct code!\n");
    } else {
        printf("Wrong code!\n");
    }
}