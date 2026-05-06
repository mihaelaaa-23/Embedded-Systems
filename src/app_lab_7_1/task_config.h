#ifndef APP_LAB_7_1_TASK_CONFIG_H
#define APP_LAB_7_1_TASK_CONFIG_H

#include <Arduino_FreeRTOS.h>
#include <semphr.h>

// ── Task timing ───────────────────────────────────────────────────────────────
#define ACQ_PERIOD_MS 20          // T1: button raw sampling
#define FSM_PERIOD_MS 50          // T2: debounce + FSM step
#define DISPLAY_PERIOD_MS 500     // T3: max wait between LCD refreshes
#define SERIAL_HEARTBEAT_MS 10000 // T3: unconditional serial reprint

// ── Debounce (3-sample counter implemented directly in T2) ───────────────────
#define BTN_PERSISTENCE_SAMPLES 3 // consecutive T1 samples to confirm press

// ── Blink intermediate state ──────────────────────────────────────────────────
#define BLINK_COUNT 5            // full on/off cycles before settling
#define BLINK_HALF_PERIOD_MS 100 // half-period of each blink (ms)
// Steps per half-period: BLINK_HALF_PERIOD_MS / FSM_PERIOD_MS = 2 steps/phase
// Total blink duration: 5 x 2 x 2 x 50ms = 1000ms  (spans > 2 x DISPLAY_PERIOD_MS)
#define BLINK_HALF_TICKS (BLINK_HALF_PERIOD_MS / FSM_PERIOD_MS)

// ── Auto-off timeout ──────────────────────────────────────────────────────────
// If LED stays ON for AUTO_OFF_MS with no press, FSM transitions to OFF.
// Timeout ticks = AUTO_OFF_MS / FSM_PERIOD_MS steps.
#define AUTO_OFF_MS 10000 // 10 s auto-off when LED is ON idle
#define AUTO_OFF_TICKS (AUTO_OFF_MS / FSM_PERIOD_MS)

// ── Auto-off warning window ───────────────────────────────────────────────────
// When remaining ticks drop below this threshold, green LED blinks as warning.
// Half-period = GREEN_WARN_BLINK_TICKS x FSM_PERIOD_MS = 200ms
#define AUTO_OFF_WARN_MS 3000 // warn when <= 3s remaining
#define AUTO_OFF_WARN_TICKS (AUTO_OFF_WARN_MS / FSM_PERIOD_MS)
#define GREEN_WARN_BLINK_TICKS 4 // 4 steps x 50ms = 200ms half-period

// ── Stuck-button detection ────────────────────────────────────────────────────
// If the raw button reads PRESSED continuously for STUCK_MS, raise alert.
#define STUCK_MS 2000 // 2 s of continuous press = stuck
#define STUCK_TICKS (STUCK_MS / FSM_PERIOD_MS)

// ── Hardware pins ─────────────────────────────────────────────────────────────
#define PIN_BUTTON 2      // BUTTON_PIN, INPUT_PULLUP
#define PIN_LED_CTRL 9    // LED_PIN  – FSM-controlled LED (red)
#define PIN_LED_OK 12     // LED_1_PIN – green, system OK
#define PIN_LED_MIRROR 11 // LED_2_PIN – yellow, mirrors FSM state

// ── FSM states ────────────────────────────────────────────────────────────────
typedef enum
{
    FSM_STATE_OFF = 0,
    FSM_STATE_BLINKING = 1, // intermediate: blinks BLINK_COUNT times
    FSM_STATE_ON = 2
} LedFsmState_t;

// ── Shared snapshot (T2 → T3) ─────────────────────────────────────────────────
typedef struct
{
    LedFsmState_t fsm_state;    // current FSM state
    bool led_physical;          // actual pin output (reflects blink phase)
    bool btn_raw;               // last raw button reading from T1
    bool btn_stable;            // debounced button state
    int blink_cycle;            // blink cycles completed so far (0..BLINK_COUNT)
    LedFsmState_t target_state; // where FSM will settle after blinking
    uint16_t press_count;       // total confirmed presses since boot
    uint32_t uptime_ms;         // ms since scheduler start (from T2 tick)
    int timeout_ticks;          // ticks remaining until auto-off (0 = inactive)
    bool stuck_alert;           // true if button held > STUCK_MS
} App71Snapshot_t;

extern App71Snapshot_t g_snap71;
extern SemaphoreHandle_t g_snap71_mutex;

// ── FSM event signal (T2 → T3) ───────────────────────────────────────────────
// Binary semaphore: T2 gives it on every FSM state change.
// T3 blocks on it with DISPLAY_PERIOD_MS timeout — wakes immediately on
// FSM events, falls back to periodic LCD refresh + heartbeat when idle.
extern SemaphoreHandle_t g_fsm_event;

#endif // APP_LAB_7_1_TASK_CONFIG_H