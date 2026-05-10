#include "app_lab_7_2_task2.h"
#include "app_lab_7_2_task1.h"
#include "task_config.h"
#include "../dd_led/dd_led.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

// ── Shared globals ────────────────────────────────────────────────────────────
App72Snapshot_t   g_snap72       = {};
SemaphoreHandle_t g_snap72_mutex = NULL;
SemaphoreHandle_t g_fsm_event_72    = NULL;

// ── LED output helpers ────────────────────────────────────────────────────────
// EW LEDs use dd_led (pins 9, 11, 12).
// NS LEDs use direct digitalWrite (pins 6, 7, 8).
// All six are applied together at the end of each FSM step.

static void set_ew(bool red, bool yellow, bool green) {
    red    ? dd_led_turn_on()   : dd_led_turn_off();    // pin 9
    yellow ? dd_led_2_turn_on() : dd_led_2_turn_off();  // pin 11
    green  ? dd_led_1_turn_on() : dd_led_1_turn_off();  // pin 12
}

static void set_ns(bool red, bool yellow, bool green) {
    digitalWrite(PIN_NS_RED,    red    ? HIGH : LOW);
    digitalWrite(PIN_NS_YELLOW, yellow ? HIGH : LOW);
    digitalWrite(PIN_NS_GREEN,  green  ? HIGH : LOW);
}

static void apply_phase(TrafficPhase_t phase) {
    switch (phase) {
        case PHASE_EW_GREEN:
            set_ew(false, false, true);
            set_ns(true,  false, false);
            break;
        case PHASE_EW_YELLOW:
            set_ew(false, true,  false);
            set_ns(true,  false, false);
            break;
        case PHASE_NS_GREEN:
            set_ew(true,  false, false);
            set_ns(false, false, true);
            break;
        case PHASE_NS_YELLOW:
            set_ew(true,  false, false);
            set_ns(false, true,  false);
            break;
        case PHASE_EMERGENCY:
            set_ew(true,  false, false);
            set_ns(true,  false, false);
            break;
    }
    dd_led_apply();  // commit EW targets; NS already written via digitalWrite
}

// ── FSM table ─────────────────────────────────────────────────────────────────
// Moore model: output = f(phase), encoded in apply_phase() above.
// Next phase = f(phase, input), encoded in the table below.
//
// Timed transitions use a step counter; input-driven transitions are immediate.
// Emergency bypasses the table from any phase.
//
//  Phase         Trigger            Next phase       Timer (ticks)
//  ------------- ------------------ ---------------- -------------
//  EW_GREEN      NS request latched EW_YELLOW        --
//  EW_GREEN      no request         EW_GREEN         (stays)
//  EW_YELLOW     timer expires      NS_GREEN         EW_YELLOW_TICKS
//  NS_GREEN      timer expires      NS_YELLOW        NS_GREEN_TICKS
//  NS_YELLOW     timer expires      EW_GREEN         NS_YELLOW_TICKS
//  EMERGENCY     timer expires      pre_emergency    EMERGENCY_TICKS
//  any           emergency btn      EMERGENCY        --

typedef struct {
    int            duration_ticks;  // 0 = input-driven (no timer)
    TrafficPhase_t next_timed;      // phase after timer expires
} PhaseConfig_t;

static const PhaseConfig_t PHASE_CFG[5] = {
    /* EW_GREEN  */ {  0,               PHASE_EW_GREEN  },  // input-driven
    /* EW_YELLOW */ {  EW_YELLOW_TICKS, PHASE_NS_GREEN  },
    /* NS_GREEN  */ {  NS_GREEN_TICKS,  PHASE_NS_YELLOW },
    /* NS_YELLOW */ {  NS_YELLOW_TICKS, PHASE_EW_GREEN  },
    /* EMERGENCY */ {  EMERGENCY_TICKS, PHASE_EW_GREEN  },  // next set at entry
};

// ── 3-sample debounce (one instance per button) ───────────────────────────────
typedef struct {
    bool raw;
    bool stable;
    bool candidate;
    int  count;
    bool was_stable;
} Debounce_t;

static void debounce_step(Debounce_t *d, bool raw) {
    d->raw = raw;
    if (raw == d->candidate) {
        if (d->count < BTN_PERSISTENCE_SAMPLES) d->count++;
    } else {
        d->candidate = raw;
        d->count     = 1;
    }
    if (d->count >= BTN_PERSISTENCE_SAMPLES) d->stable = d->candidate;
}

static bool rising_edge(Debounce_t *d) {
    bool edge    = d->stable && !d->was_stable;
    d->was_stable = d->stable;
    return edge;
}

// ── Task init ─────────────────────────────────────────────────────────────────
void app_lab_7_2_task2_init() {
    g_snap72_mutex = xSemaphoreCreateMutex();
    g_fsm_event_72    = xSemaphoreCreateBinary();
}

// ── Task body ─────────────────────────────────────────────────────────────────
void app_lab_7_2_task2(void *pvParameters) {
    (void)pvParameters;

    TrafficPhase_t phase         = PHASE_EW_GREEN;
    TrafficPhase_t pre_emergency = PHASE_EW_GREEN;
    int            timer         = 0;
    bool           ns_request    = false;
    uint16_t       cycle_count   = 0;

    Debounce_t deb_ns   = {};
    Debounce_t deb_emrg = {};

    TickType_t last_wake = xTaskGetTickCount();

    for (;;) {
        TrafficPhase_t prev_phase = phase;

        // ── Step 1: Apply output for current phase (Moore: out = f(phase))
        apply_phase(phase);

        // ── Step 3: Read inputs
        debounce_step(&deb_ns,   app_lab_7_2_task1_get_ns_raw());
        debounce_step(&deb_emrg, app_lab_7_2_task1_get_emrg_raw());

        bool ns_press   = rising_edge(&deb_ns);
        bool emrg_press = rising_edge(&deb_emrg);

        // Latch NS request — cleared when NS_GREEN is entered
        if (ns_press) ns_request = true;

        // ── Step 4: Compute next phase ────────────────────────────────────────

        // Emergency preempts any phase immediately
        if (emrg_press && phase != PHASE_EMERGENCY) {
            pre_emergency = phase;
            phase         = PHASE_EMERGENCY;
            timer         = EMERGENCY_TICKS;
            printf("[EMERGENCY] Both directions RED for %ds\n",
                   EMERGENCY_MS / 1000);
        }
        else if (PHASE_CFG[phase].duration_ticks == 0) {
            // Input-driven phase (EW_GREEN): transition on NS request
            if (ns_request) {
                phase     = PHASE_EW_YELLOW;
                timer     = EW_YELLOW_TICKS;
                ns_request = false;
            }
            // else: stay in EW_GREEN
        }
        else {
            // Timed phase: count down, transition when timer expires
            if (timer > 0) {
                timer--;
            }
            if (timer == 0) {
                if (phase == PHASE_EMERGENCY) {
                    // Resume the phase that was interrupted
                    phase = pre_emergency;
                    timer = PHASE_CFG[phase].duration_ticks;
                    // If resuming EW_GREEN (input-driven), timer stays 0
                } else {
                    phase = PHASE_CFG[phase].next_timed;
                    timer = PHASE_CFG[phase].duration_ticks;
                }

                // Count completed full cycles
                if (phase == PHASE_EW_GREEN) cycle_count++;

                // Clear NS request when NS_GREEN is entered
                if (phase == PHASE_NS_GREEN) ns_request = false;
            }
        }

        // ── Signal T3 on phase change
        if (phase != prev_phase) xSemaphoreGive(g_fsm_event_72);

        // ── Publish snapshot
        if (xSemaphoreTake(g_snap72_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            g_snap72.phase         = phase;
            g_snap72.pre_emergency = pre_emergency;
            g_snap72.timer_ticks   = timer;
            g_snap72.ns_request    = ns_request;
            g_snap72.emergency     = (phase == PHASE_EMERGENCY);
            g_snap72.cycle_count   = cycle_count;
            g_snap72.uptime_ms     = (uint32_t)(xTaskGetTickCount()
                                      * portTICK_PERIOD_MS);
            xSemaphoreGive(g_snap72_mutex);
        } else {
            printf("[T2] WARN: snapshot mutex timeout\n");
        }

        // ── Step 2: fixed-period wait
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(FSM_PERIOD_MS));
    }
}