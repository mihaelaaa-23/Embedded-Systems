#include "srv_stdio_keypad.h"

//1. Includere STDIO
#include <stdio.h>
//2. Includere Keypad
#include <Keypad.h>

// Configurare Keypad
#define SRV_KEYPAD_REPEAT_DELAY 100

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {3, 2, 1, 0}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {7, 6, 5, 4}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

//3. Functie pentru citirea unei taste de la Keypad
int srv_stdio_keypad_get_key(FILE *stream) {
    char customKey;
    do {
        customKey = customKeypad.getKey();
    } while (customKey == NO_KEY); // Așteptăm până când o tastă este apăsată

    // delay(SRV_KEYPAD_REPEAT_DELAY); // read frequency 1/SRV_KEYPAD_REPEAT_DELAY

    return (int)customKey;   
}

void srv_stdio_keypad_setup() {
  //4. initializare periferii
  // nu necesita, e initializat prin configuratia de definitie, vezi mai sus

  //5. Definire stream
  FILE *srv_stdio_keypad_stream = fdevopen(NULL, srv_stdio_keypad_get_key);

  // link stream to keypad
   if (srv_stdio_keypad_stream != NULL) {
        stdin = srv_stdio_keypad_stream;
    }
}