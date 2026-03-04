#include <Arduino.h>
#include "app_lab_1_1/app_lab_1_1.h"
#include "app_lab_1_2/app_lab_1_2.h"
#include "app_lab_2_1/app_lab_2_1.h"
#include "app_lab_3_1/app_lab_3_1.h"
#include "app_lab_3_2/app_lab_3_2.h"
#include "app_lab_4_1/app_lab_4_1.h"
#define USE_APP_LAB_1_1 11
#define USE_APP_LAB_1_2 12
#define USE_APP_LAB_2_1 21
#define USE_APP_LAB_3_1 31
#define USE_APP_LAB_3_2 32
#define USE_APP_LAB_4_1 41

#define APP_USED USE_APP_LAB_4_1

void setup() {
  #if APP_USED == USE_APP_LAB_1_1
    app_lab_1_1_setup();
  #elif APP_USED == USE_APP_LAB_1_2
    app_lab_1_2_setup();
  #elif APP_USED == USE_APP_LAB_2_1
    app_lab_2_1_setup();
  #elif APP_USED == USE_APP_LAB_3_1
    app_lab_3_1_setup();
  #elif APP_USED == USE_APP_LAB_3_2
    app_lab_3_2_setup();
  #elif APP_USED == USE_APP_LAB_4_1
    app_lab_4_1_setup();
  #endif
}

void loop() {
  #if APP_USED == USE_APP_LAB_1_1
    app_lab_1_1_loop();
  #elif APP_USED == USE_APP_LAB_1_2
    app_lab_1_2_loop();
  #elif APP_USED == USE_APP_LAB_2_1
    app_lab_2_1_loop();
  #elif APP_USED == USE_APP_LAB_3_1
    app_lab_3_1_loop();
  #elif APP_USED == USE_APP_LAB_3_2
    app_lab_3_2_loop();
  #elif APP_USED == USE_APP_LAB_4_1
    app_lab_4_1_loop();
  #endif
}