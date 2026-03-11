#ifndef APP_LAB_4_2_TASK_CONFIG_H
#define APP_LAB_4_2_TASK_CONFIG_H

#include <Arduino_FreeRTOS.h>
#include <semphr.h>

// ── Timing ──────────────────────────────────────────────────────────────────
#define ACQ_PERIOD_MS        50     // ms  – task_1 & task_2 period
#define REPORT_PERIOD_MS    500     // ms  – task_3 period

// ── Signal conditioning parameters ──────────────────────────────────────────
#define MEDIAN_WINDOW         5     // median-filter history (must be odd)
#define WMA_ALPHA            30     // IIR weight for newest sample  (0–100 %)
                                    // new_wma = (ALPHA*sample + (100-ALPHA)*prev) / 100

// ── S1 – Potentiometer (analog temperature sim): 0..100 °C ──────────────────
#define SAT_MIN_S1            0
#define SAT_MAX_S1          100
#define ALERT_HIGH_S1        50     // °C – alert ON  edge
#define ALERT_LOW_S1         45     // °C – alert OFF edge

// ── S2 – DHT22 (digital temperature): −40..80 °C ────────────────────────────
#define SAT_MIN_S2          -40
#define SAT_MAX_S2           80
#define ALERT_HIGH_S2        30     // °C – alert ON  edge
#define ALERT_LOW_S2         25     // °C – alert OFF edge

// ── S3 – MQ-2 Gas Sensor (analog): 0..1000 ppm ──────────────────────────────
#define SAT_MIN_S3            0
#define SAT_MAX_S3         1000
#define ALERT_HIGH_S3       500     // ppm – alert ON  edge
#define ALERT_LOW_S3        450     // ppm – alert OFF edge

// ── Processed sensor snapshot ────────────────────────────────────────────────
// Written by task_cond_4_2, read by task_rep_4_2.
typedef struct {
    int  raw;        // °C  – directly from sensor driver
    int  saturated;  // °C  – after saturation (clamping)
    int  median;     // °C  – after MEDIAN_WINDOW-point median filter
    int  weighted;   // °C  – after IIR weighted moving average
    bool alert;      // true when threshold exceeded (hysteresis applied)
} SensorData_t;

// Shared variables (defined in app_lab_4_2_task_2.cpp)
extern SensorData_t       g_data_s1;
extern SemaphoreHandle_t  g_mutex_s1;

extern SensorData_t       g_data_s2;
extern SemaphoreHandle_t  g_mutex_s2;

extern SensorData_t       g_data_s3;
extern SemaphoreHandle_t  g_mutex_s3;

#endif // APP_LAB_4_2_TASK_CONFIG_H
