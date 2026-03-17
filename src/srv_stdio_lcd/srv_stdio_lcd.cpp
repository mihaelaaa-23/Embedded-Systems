#include "srv_stdio_lcd.h"
#include <Arduino.h>
#include <stdio.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <new>

#define CLEAR_KEY 0x1b

int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

static int s_lcd_row = 0;  // tracks current LCD cursor row
static bool s_lcd_available = false;

static bool i2c_device_present(uint8_t address) {
    Wire.beginTransmission(address);
    return Wire.endTransmission() == 0;
}

int srv_stdio_lcd_put_char(char c, FILE *stream) {
    if (c == CLEAR_KEY) {
        if (s_lcd_available) {
            lcd.clear();
            lcd.setCursor(0, 0);
        }
        s_lcd_row = 0;
    } else if (c == '\n') {
        s_lcd_row++;
        if (s_lcd_available && s_lcd_row < lcdRows) {
            lcd.setCursor(0, s_lcd_row);
        }
        Serial.write('\r');
        Serial.write('\n');
    } else if (c == '\r') {
        Serial.write('\r');          // \r: Serial only, skip LCD
    } else {
        if (s_lcd_available && s_lcd_row < lcdRows) {   // rows 0-1 visible on LCD
            lcd.print(c);
        }
        Serial.write(c);             // all chars tee'd to Serial
    }
    return 0;
}

void srv_stdio_lcd_setup() {
    FILE *srv_stdio_lcd_stream = fdevopen(srv_stdio_lcd_put_char, NULL);
    stdout = srv_stdio_lcd_stream;

    Wire.begin();

    uint8_t lcd_address = 0;
    if (i2c_device_present(0x27)) {
        lcd_address = 0x27;
    } else if (i2c_device_present(0x3F)) {
        lcd_address = 0x3F;
    }

    if (lcd_address != 0) {
        // Reconstruct object with detected address while keeping the same global symbol.
        new (&lcd) LiquidCrystal_I2C(lcd_address, lcdColumns, lcdRows);
        lcd.init();
        lcd.backlight();
        lcd.clear();
        lcd.home();
        s_lcd_available = false;
        printf("LCD detected at 0x%02X\n", lcd_address);
        s_lcd_available = true;
    } else {
        s_lcd_available = false;
        printf("LCD not detected at 0x27/0x3F. Check SDA/SCL wiring and power.\n");
    }
}