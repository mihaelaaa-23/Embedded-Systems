#include "app_lab_7_1_task2.h"
#include "app_lab_7_1_task1.h"
#include "task_config.h"
#include "../dd_led/dd_led.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

// ── Shared globals (defined here, declared extern in task_config.h) ───────────
App71Snapshot_t   g_snap71       = {};
SemaphoreHandle_t g_snap71_mutex = NULL;
SemaphoreHandle_t g_fsm_event    = NULL;  // binary semaphore: signals T3 on FSM change

// ── Table-driven Moore FSM ────────────────────────────────────────────────────
// Matches the theory structure from Lab 7.1, Listing 7.3 / Table 7.1.
//
// Each row defines one FSM state:
//   out  — LED output for this state (Moore: output depends only on state)
//   next — next state indexed by input value: [0]=no press, [1]=press
//
// The BLINKING state is a self-contained timed state: it stays in BLINKING
// for BLINK_COUNT full cycles, then transitions to its target via fsm_blink_tick().
// Input=1 during BLINKING is mapped to BLINKING itself (press ignored mid-blink).
//
//  Num │ Name      │ Out   │ In=0      │ In=1
//  ────┼───────────┼───────┼───────────┼──────────
//   0  │ LED_OFF   │ false │ LED_OFF   │ BLINKING
//   1  │ BLINKING  │ phase │ BLINKING  │ BLINKING
//   2  │ LED_ON    │ true  │ LED_ON    │ BLINKING

typedef struct {
    bool          out;        // Moore output: LED state for this FSM state
    LedFsmState_t next[2];    // next[0]=no press, next[1]=press confirmed
} FsmState_t;

static const FsmState_t FSM[3] = {
    /* FSM_STATE_OFF      */ { false, { FSM_STATE_OFF,      FSM_STATE_BLINKING } },
    /* FSM_STATE_BLINKING */ { false, { FSM_STATE_BLINKING, FSM_STATE_BLINKING } },
    /* FSM_STATE_ON       */ { true,  { FSM_STATE_ON,       FSM_STATE_BLINKING } },
};

// ── Blink sub-state (internal to BLINKING row) ───────────────────────────────
static LedFsmState_t s_blink_target = FSM_STATE_ON;
static int           s_blink_cycle  = 0;
static int           s_blink_ticks  = 0;
static bool          s_blink_phase  = false;  // false=ON half, true=OFF half

static bool fsm_blink_tick(LedFsmState_t *state) {
    s_blink_ticks++;
    if (s_blink_ticks >= BLINK_HALF_TICKS) {
        s_blink_ticks = 0;
        s_blink_phase = !s_blink_phase;
        if (!s_blink_phase) {       // returned to ON half → full cycle done
            s_blink_cycle++;
        }
        if (s_blink_cycle >= BLINK_COUNT) {
            *state = s_blink_target;
            return (*state == FSM_STATE_ON);
        }
    }
    return !s_blink_phase;
}

static void fsm_blink_enter(LedFsmState_t from) {
    s_blink_target = (from == FSM_STATE_OFF) ? FSM_STATE_ON : FSM_STATE_OFF;
    s_blink_cycle  = 0;
    s_blink_ticks  = 0;
    s_blink_phase  = false;
}

// ── 3-sample debounce ─────────────────────────────────────────────────────────
static bool s_stable     = false;
static bool s_candidate  = false;
static int  s_deb_count  = 0;
static bool s_was_stable = false;

static void debounce_step(bool raw) {
    if (raw == s_candidate) {
        if (s_deb_count < BTN_PERSISTENCE_SAMPLES) s_deb_count++;
    } else {
        s_candidate = raw;
        s_deb_count = 1;
    }
    if (s_deb_count >= BTN_PERSISTENCE_SAMPLES) s_stable = s_candidate;
}

// ── LED output helper ─────────────────────────────────────────────────────────
static void fsm_set_led(bool on) {
    on ? dd_led_turn_on()   : dd_led_turn_off();
    on ? dd_led_2_turn_on() : dd_led_2_turn_off();
}

// ── Task init ─────────────────────────────────────────────────────────────────
void app_lab_7_1_task2_init() {
    g_snap71_mutex = xSemaphoreCreateMutex();
    g_fsm_event    = xSemaphoreCreateBinary();
}

// ── Task body — 4-step Moore evaluation (matches Lab 7.1 Listing 7.5) ────────
void app_lab_7_1_task2(void *pvParameters) {
    (void)pvParameters;

    LedFsmState_t s_fsm_state = FSM_STATE_OFF;
    TickType_t    last_wake   = xTaskGetTickCount();

    for (;;) {
        LedFsmState_t prev_state = s_fsm_state;

        // ── Step 1: Apply output based on current state (Moore: out = f(state))
        bool led_out;
        if (s_fsm_state == FSM_STATE_BLINKING) {
            led_out = fsm_blink_tick(&s_fsm_state); // advances blink, may settle
        } else {
            led_out = FSM[s_fsm_state].out;
        }
        fsm_set_led(led_out);

        // ── Step 2: Wait — handled by vTaskDelayUntil at end of loop

        // ── Step 3: Read input — debounced rising edge from T1
        bool raw = app_lab_7_1_task1_get_raw();
        debounce_step(raw);
        bool rising_edge = (s_stable && !s_was_stable);
        bool input       = rising_edge && (s_fsm_state != FSM_STATE_BLINKING);
        s_was_stable     = s_stable;

        // ── Step 4: Compute next state from table (Moore: next = f(state, input))
        if (input) {
            fsm_blink_enter(s_fsm_state);
        }
        s_fsm_state = FSM[s_fsm_state].next[(int)input];

        // ── Apply all LED targets in one shot (green always ON)
        dd_led_1_turn_on();
        dd_led_apply();

        // ── Publish snapshot for T3
        if (xSemaphoreTake(g_snap71_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            g_snap71.fsm_state    = s_fsm_state;
            g_snap71.led_physical = led_out;
            g_snap71.btn_raw      = raw;
            g_snap71.btn_stable   = s_stable;
            g_snap71.blink_cycle  = s_blink_cycle;
            g_snap71.target_state = s_blink_target;
            xSemaphoreGive(g_snap71_mutex);
        } else {
            printf("[T2] WARN: snapshot mutex timeout\n");
        }

        // ── Signal T3 immediately if FSM state changed
        if (s_fsm_state != prev_state) {
            xSemaphoreGive(g_fsm_event);
        }

        // ── Step 2 (FreeRTOS): fixed-period wait
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(FSM_PERIOD_MS));
    }
}