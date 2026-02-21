#include "app_lab_2_1_task_3.h"
#include "dd_button/dd_button.h"

// Task 3 – Variabila de stare: Incrementarea/decrementarea unei variabile
// prin apasarea a doua butoane, reprezentand numarul de recurente/timp
// in care LED-ul din Task 2 se afla intr-o anumita stare.
// Provider: stocheaza rezultatul in g_task3_blink_count (variabila globala/semnal)

volatile int g_task3_blink_count = TASK_3_VAR_DEFAULT;

void app_lab_2_1_task_3_setup() {
    // Nicio initializare suplimentara - pinii sunt configurati in dd_button_setup()
}

void app_lab_2_1_task_3_loop() {
    // Butonul 1 (BUTTON_1_PIN) - incrementare variabila
    if (dd_button_1_is_pressed()) {
        printf("TASK 3: Button UP Pressed\\n");
        if (g_task3_blink_count < TASK_3_VAR_MAX) {
            g_task3_blink_count++;
        }
        // evit debounce-ul
        delay(300);
    }

    // Butonul 2 (BUTTON_2_PIN) - decrementare variabila
    if (dd_button_2_is_pressed()) {
        printf("TASK 3: Button DOWN Pressed\\n");
        if (g_task3_blink_count > TASK_3_VAR_MIN) {
            g_task3_blink_count--;
        }
        // evit debounce-ul
        delay(300);
    }
}