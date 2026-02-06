#include "srv_serial_stdio.h"
#include "Arduino.h"

//1. Includere Librarie STDIO
#include <stdio.h>

//2. Definire functie scriere caracter (Output) - printf
int srv_serial_put_char(char ch, FILE *f) {
    // Send character via Serial port
    if (ch == '\n') {
        Serial.write('\r');
    }
    return Serial.write(ch);
}

//3. Definire functie citire caracter (Input) - scanf
int srv_serial_get_char(FILE *f) {
    // Wait until data is available "Blocking Mode"
    while (!Serial.available());
    char ch = Serial.read();
    if (ch == '\r') {
        // Convert carriage return to newline
        Serial.write('\r'); // Echo newline back to terminal
        return '\n';
    }
    Serial.write(ch);
    // Read character from Serial port
    return ch;
}

//C. Setup function to initialize Serial and link stdio functions
void srv_serial_stdio_setup() {
    //4. Initializare Serial Peripheral
    Serial.begin(9600);

    //5. Definire STREAM folosind fdevopen
    FILE *srv_serial_stream = fdevopen(srv_serial_put_char, srv_serial_get_char);

    //6. Inlocuire intrare/iesire standard
    stdin = srv_serial_stream;
    stdout = srv_serial_stream;

}