#ifndef APP_LAB_7_2_TASK_CONFIG_H
#define APP_LAB_7_2_TASK_CONFIG_H

#include <Arduino_FreeRTOS.h>
#include <semphr.h>

// ── Task timing ───────────────────────────────────────────────────────────────
#define ACQ_PERIOD_MS            20     // T1: input sampling
#define FSM_PERIOD_MS            50     // T2: FSM evaluation
#define DISPLAY_PERIOD_MS      1000     // T3: semaphore timeout (1s countdown)
#define SERIAL_HEARTBEAT_MS   10000     // T3: unconditional serial reprint

// ── Traffic light phase durations ────────────────────────────────────────────
#define EW_YELLOW_MS           3000
#define EW_RED_CLEAR_MS        1500     // all-red clearance EW->NS
#define NS_GREEN_MS            5000
#define NS_YELLOW_MS           3000
#define NS_RED_CLEAR_MS        1500     // all-red clearance NS->EW
#define EMERGENCY_MS          10000

#define EW_YELLOW_TICKS    (EW_YELLOW_MS    / FSM_PERIOD_MS)   //  60
#define EW_RED_CLEAR_TICKS (EW_RED_CLEAR_MS / FSM_PERIOD_MS)   //  30
#define NS_GREEN_TICKS     (NS_GREEN_MS     / FSM_PERIOD_MS)   // 100
#define NS_YELLOW_TICKS    (NS_YELLOW_MS    / FSM_PERIOD_MS)   //  60
#define NS_RED_CLEAR_TICKS (NS_RED_CLEAR_MS / FSM_PERIOD_MS)   //  30
#define EMERGENCY_TICKS    (EMERGENCY_MS    / FSM_PERIOD_MS)   // 200

// ── Emergency long-press threshold ───────────────────────────────────────────
// D3 must be held for EMRG_HOLD_MS continuously to activate emergency.
#define EMRG_HOLD_MS           3000
#define EMRG_HOLD_TICKS        (EMRG_HOLD_MS / FSM_PERIOD_MS)   // 60

// ── Debounce ──────────────────────────────────────────────────────────────────
#define BTN_PERSISTENCE_SAMPLES   3

// ── Hardware pins ─────────────────────────────────────────────────────────────
#define PIN_EW_RED       9
#define PIN_EW_YELLOW   11
#define PIN_EW_GREEN    12
#define PIN_NS_RED       6
#define PIN_NS_YELLOW    7
#define PIN_NS_GREEN     8
#define PIN_BTN_NS        2
#define PIN_BTN_EMERGENCY 3

// ── EW FSM states ─────────────────────────────────────────────────────────────
typedef enum {
    EW_GREEN         = 0,   // default: EW green, waits for NS request
    EW_YELLOW        = 1,   // transitioning out: 3s
    EW_RED_CLEAR     = 2,   // all-red buffer before NS green: 1.5s
    EW_RED           = 3,   // NS is active: EW holds red
    EW_YELLOW_RETURN = 4,   // returning: 3s yellow before EW green
} EwState_t;

// ── NS FSM states ─────────────────────────────────────────────────────────────
typedef enum {
    NS_RED           = 0,   // default: NS red, waiting for EW to yield
    NS_GREEN         = 1,   // NS crossing active: 5s
    NS_YELLOW        = 2,   // NS transitioning out: 3s
    NS_RED_CLEAR     = 3,   // all-red buffer before EW returns: 1.5s
} NsState_t;

// ── Shared snapshot (T2 -> T3) ────────────────────────────────────────────────
typedef struct {
    EwState_t  ew_state;        // current EW FSM state
    NsState_t  ns_state;        // current NS FSM state
    int        ew_timer;        // ticks remaining in current EW timed phase
    int        ns_timer;        // ticks remaining in current NS timed phase
    bool       ns_request;      // pending NS crossing request
    bool       emergency;       // emergency currently active
    int        emrg_hold;       // ticks D3 has been held (0..EMRG_HOLD_TICKS)
    uint16_t   cycle_count;     // completed full cycles
    uint32_t   uptime_ms;
} App72Snapshot_t;

extern App72Snapshot_t   g_snap72;
extern SemaphoreHandle_t g_snap72_mutex;
extern SemaphoreHandle_t g_fsm_event_72;

#endif // APP_LAB_7_2_TASK_CONFIG_H