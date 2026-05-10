#ifndef APP_LAB_7_2_TASK_CONFIG_H
#define APP_LAB_7_2_TASK_CONFIG_H

#include <Arduino_FreeRTOS.h>
#include <semphr.h>

// ── Task timing ───────────────────────────────────────────────────────────────
#define ACQ_PERIOD_MS            20     // T1: input sampling period
#define FSM_PERIOD_MS            50     // T2: FSM evaluation period
#define DISPLAY_PERIOD_MS       500     // T3: semaphore timeout / LCD refresh
#define SERIAL_HEARTBEAT_MS   10000     // T3: unconditional serial reprint

// ── Traffic light phase durations ────────────────────────────────────────────
#define EW_YELLOW_MS           3000     // EW yellow hold before NS green
#define NS_GREEN_MS            5000     // NS green hold duration
#define NS_YELLOW_MS           3000     // NS yellow hold before return to EW
#define EMERGENCY_MS          10000     // both-red emergency hold duration

// Steps = duration / FSM_PERIOD_MS
#define EW_YELLOW_TICKS   (EW_YELLOW_MS  / FSM_PERIOD_MS)   // 60
#define NS_GREEN_TICKS    (NS_GREEN_MS   / FSM_PERIOD_MS)   // 100
#define NS_YELLOW_TICKS   (NS_YELLOW_MS  / FSM_PERIOD_MS)   // 60
#define EMERGENCY_TICKS   (EMERGENCY_MS  / FSM_PERIOD_MS)   // 200

// ── Debounce ──────────────────────────────────────────────────────────────────
#define BTN_PERSISTENCE_SAMPLES  3      // consecutive samples to confirm press

// ── Hardware pins — EW direction (reusing dd_led pins) ───────────────────────
#define PIN_EW_RED       9              // dd_led  LED_PIN
#define PIN_EW_YELLOW   11              // dd_led  LED_2_PIN
#define PIN_EW_GREEN    12              // dd_led  LED_1_PIN

// ── Hardware pins — NS direction (new LEDs) ───────────────────────────────────
#define PIN_NS_RED       6
#define PIN_NS_YELLOW    7
#define PIN_NS_GREEN     8

// ── Hardware pins — inputs ────────────────────────────────────────────────────
#define PIN_BTN_NS        2             // dd_button BUTTON_PIN   - NS request
#define PIN_BTN_EMERGENCY 3             // dd_button BUTTON_1_PIN - emergency

// ── Global FSM phases ─────────────────────────────────────────────────────────
// Single shared FSM for the intersection — directions are interlocked.
//
//  Phase          EW light    NS light    Trigger / Duration
//  -------------- ----------- ----------- -------------------------
//  EW_GREEN       GREEN       RED         default; waits for NS request
//  EW_YELLOW      YELLOW      RED         timed: EW_YELLOW_TICKS steps
//  NS_GREEN       RED         GREEN       timed: NS_GREEN_TICKS steps
//  NS_YELLOW      RED         YELLOW      timed: NS_YELLOW_TICKS steps
//  EMERGENCY      RED         RED         timed: EMERGENCY_TICKS steps
//                                         from any phase via pin 3
typedef enum {
    PHASE_EW_GREEN   = 0,
    PHASE_EW_YELLOW  = 1,
    PHASE_NS_GREEN   = 2,
    PHASE_NS_YELLOW  = 3,
    PHASE_EMERGENCY  = 4
} TrafficPhase_t;

// ── Shared snapshot (T2 -> T3) ────────────────────────────────────────────────
typedef struct {
    TrafficPhase_t phase;           // current intersection phase
    TrafficPhase_t pre_emergency;   // phase to resume after emergency
    int            timer_ticks;     // ticks remaining in current timed phase
    bool           ns_request;      // pending NS crossing request
    bool           emergency;       // emergency currently active
    uint16_t       cycle_count;     // completed EW_GREEN->...->EW_GREEN cycles
    uint32_t       uptime_ms;       // ms since scheduler start
} App72Snapshot_t;

extern App72Snapshot_t   g_snap72;
extern SemaphoreHandle_t g_snap72_mutex;
extern SemaphoreHandle_t g_fsm_event_72;   // T2 signals T3 on phase change

#endif // APP_LAB_7_2_TASK_CONFIG_H