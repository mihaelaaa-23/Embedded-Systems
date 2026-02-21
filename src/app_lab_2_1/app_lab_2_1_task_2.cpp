#include "app_lab_2_1_task_2.h"
#include "app_lab_2_1_task_3.h"
#include "dd_led/dd_led.h"
#include "dd_button/dd_button.h"

// Task 2 – LED Intermitent: Controlul unui LED intermitent, activ doar atunci cand
// LED-ul din Task 1 este stins.
// Consumer: citeste g_task3_blink_count (produs de Task 3) pentru a stabili
// durata starii ON/OFF a LED-ului intermitent (in multipli de 100ms).

void app_lab_2_1_task_2_setup()
{
    // Nu este necesara nicio initializare specifica pentru acest task,
    // deoarece va monitoriza starea LED-ului din Task 1.
}

void app_lab_2_1_task_2_loop()
{
    // Consumer: citeste variabila de stare produsa de Task 3
    int blink_delay = g_task3_blink_count * 100; // blink_count * 100ms

    // Verificam daca LED-ul din Task 1 este stins
    if (!dd_led_is_on())
    {
        if (!dd_led_1_is_on())
        {
            // Daca LED-ul 1 este stins, aprindem LED-ul 2 pentru a indica activarea Task-ului 2
            dd_led_1_turn_on();
            delay(blink_delay); // LED-ul 2 ramane aprins conform variabilei din Task 3
            dd_led_1_turn_off();
            delay(blink_delay); // LED-ul 2 ramane stins conform variabilei din Task 3
        }
        else
        {
            // Daca LED-ul 1 este aprins, ne asiguram ca LED-ul 2 este stins
            dd_led_1_turn_off();
        }
    }
}