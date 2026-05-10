#include "app_lab_7_2_task2.h"
#include "app_lab_7_2_task1.h"
#include "task_config.h"
#include "../dd_led/dd_led.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

// ── Shared globals ─────────────────────────────────────────────────────────────
App72Snapshot_t   g_snap72       = {};
SemaphoreHandle_t g_snap72_mutex = NULL;
SemaphoreHandle_t g_fsm_event_72 = NULL;

// ── LED output helpers ─────────────────────────────────────────────────────────
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

// Moore output functions — output = f(state) for each FSM
static void apply_ew(EwState_t s) {
    switch (s) {
        case EW_GREEN:         set_ew(false, false, true);  break;
        case EW_YELLOW:        set_ew(false, true,  false); break;
        case EW_RED_CLEAR:     set_ew(true,  false, false); break;
        case EW_RED:           set_ew(true,  false, false); break;
        case EW_YELLOW_RETURN: set_ew(false, true,  false); break;
    }
}

static void apply_ns(NsState_t s) {
    switch (s) {
        case NS_RED:       set_ns(true,  false, false); break;
        case NS_GREEN:     set_ns(false, false, true);  break;
        case NS_YELLOW:    set_ns(false, true,  false); break;
        case NS_RED_CLEAR: set_ns(true,  false, false); break;
    }
}

// ── 3-sample debounce ─────────────────────────────────────────────────────────
typedef struct {
    bool raw, stable, candidate, was_stable;
    int  count;
} Debounce_t;

static void debounce_step(Debounce_t *d, bool raw) {
    d->raw = raw;
    if (raw == d->candidate) {
        if (d->count < BTN_PERSISTENCE_SAMPLES) d->count++;
    } else { d->candidate = raw; d->count = 1; }
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
    g_fsm_event_72 = xSemaphoreCreateBinary();
}

// ── Task body ─────────────────────────────────────────────────────────────────
void app_lab_7_2_task2(void *pvParameters) {
    (void)pvParameters;

    // Two independent Moore FSMs — one per direction
    EwState_t  ew_state    = EW_GREEN;
    NsState_t  ns_state    = NS_RED;
    int        ew_timer    = 0;
    int        ns_timer    = 0;
    bool       ns_request  = false;
    bool       emergency   = false;
    int        emrg_hold   = 0;   // consecutive ticks D3 held pressed
    uint16_t   cycle_count = 0;

    Debounce_t deb_ns   = {};
    Debounce_t deb_emrg = {};

    TickType_t last_wake = xTaskGetTickCount();

    for (;;) {
        EwState_t prev_ew = ew_state;
        NsState_t prev_ns = ns_state;

        // ── Step 1: Apply outputs (Moore: out = f(state)) ────────────────────
        if (emergency) {
            set_ew(true, false, false);
            set_ns(true, false, false);
        } else {
            apply_ew(ew_state);
            apply_ns(ns_state);
        }
        dd_led_apply();   // commit EW dd_led targets

        // ── Step 3: Read inputs ───────────────────────────────────────────────
        debounce_step(&deb_ns,   app_lab_7_2_task1_get_ns_raw());
        debounce_step(&deb_emrg, app_lab_7_2_task1_get_emrg_raw());

        bool ns_press = rising_edge(&deb_ns);
        if (ns_press) ns_request = true;

        // Emergency long-press detection:
        // D3 must be held for EMRG_HOLD_TICKS consecutive FSM steps.
        // Counter increments while held, resets immediately on release.
        bool emrg_fired = false;
        if (deb_emrg.stable) {
            if (emrg_hold < EMRG_HOLD_TICKS) emrg_hold++;
            if (emrg_hold == EMRG_HOLD_TICKS && !emergency) {
                emergency  = true;
                emrg_fired = true;
                ew_timer   = EMERGENCY_TICKS;
                printf("[EMERGENCY] Activated — both RED for %ds\n",
                       EMERGENCY_MS / 1000);
            }
        } else {
            emrg_hold = 0;
            // Emergency clears only via timer, not on button release
        }

        // ── Step 4: Compute next states ───────────────────────────────────────

        if (emergency) {
            // ── Emergency mode: both FSMs frozen, single shared timer
            if (!emrg_fired) {   // don't decrement on the cycle it just fired
                if (ew_timer > 0) ew_timer--;
            }
            if (ew_timer == 0) {
                emergency = false;
                emrg_hold = 0;
                // Return both FSMs to default safe state
                ew_state  = EW_GREEN;
                ns_state  = NS_RED;
                ns_timer  = 0;
                ew_timer  = 0;
                cycle_count++;
                printf("[EMERGENCY] Cleared — resuming EW GREEN\n");
            }
        } else {
            // ── EW FSM step ──────────────────────────────────────────────────
            switch (ew_state) {

                case EW_GREEN:
                    // Input-driven: yield to NS request
                    if (ns_request) {
                        ew_state   = EW_YELLOW;
                        ew_timer   = EW_YELLOW_TICKS;
                        ns_request = false;
                    }
                    break;

                case EW_YELLOW:
                    if (ew_timer > 0) ew_timer--;
                    if (ew_timer == 0) {
                        ew_state = EW_RED_CLEAR;
                        ew_timer = EW_RED_CLEAR_TICKS;
                    }
                    break;

                case EW_RED_CLEAR:
                    // All-red buffer: EW red, NS still red
                    // When this expires, signal NS FSM to go green
                    if (ew_timer > 0) ew_timer--;
                    if (ew_timer == 0) {
                        ew_state = EW_RED;
                        // Kick NS FSM
                        ns_state = NS_GREEN;
                        ns_timer = NS_GREEN_TICKS;
                    }
                    break;

                case EW_RED:
                    // Wait for NS FSM to finish (NS_RED_CLEAR expires)
                    // NS FSM will set ew_state = EW_YELLOW_RETURN when ready
                    break;

                case EW_YELLOW_RETURN:
                    if (ew_timer > 0) ew_timer--;
                    if (ew_timer == 0) {
                        ew_state = EW_GREEN;
                        cycle_count++;
                    }
                    break;
            }

            // ── NS FSM step ──────────────────────────────────────────────────
            switch (ns_state) {

                case NS_RED:
                    // Waiting — EW_RED_CLEAR will kick us to NS_GREEN
                    break;

                case NS_GREEN:
                    if (ns_timer > 0) ns_timer--;
                    if (ns_timer == 0) {
                        ns_state = NS_YELLOW;
                        ns_timer = NS_YELLOW_TICKS;
                    }
                    break;

                case NS_YELLOW:
                    if (ns_timer > 0) ns_timer--;
                    if (ns_timer == 0) {
                        ns_state = NS_RED_CLEAR;
                        ns_timer = NS_RED_CLEAR_TICKS;
                    }
                    break;

                case NS_RED_CLEAR:
                    // All-red buffer: NS red, EW still red
                    // When this expires, kick EW FSM to return
                    if (ns_timer > 0) ns_timer--;
                    if (ns_timer == 0) {
                        ns_state = NS_RED;
                        // Kick EW FSM back to yellow return
                        ew_state = EW_YELLOW_RETURN;
                        ew_timer = EW_YELLOW_TICKS;
                    }
                    break;
            }
        }

        // ── Signal T3 on any state change ────────────────────────────────────
        if (ew_state != prev_ew || ns_state != prev_ns) {
            xSemaphoreGive(g_fsm_event_72);
        }

        // ── Publish snapshot ──────────────────────────────────────────────────
        if (xSemaphoreTake(g_snap72_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            g_snap72.ew_state    = ew_state;
            g_snap72.ns_state    = ns_state;
            g_snap72.ew_timer    = ew_timer;
            g_snap72.ns_timer    = ns_timer;
            g_snap72.ns_request  = ns_request;
            g_snap72.emergency   = emergency;
            g_snap72.emrg_hold   = emrg_hold;
            g_snap72.cycle_count = cycle_count;
            g_snap72.uptime_ms   = (uint32_t)(xTaskGetTickCount()
                                    * portTICK_PERIOD_MS);
            xSemaphoreGive(g_snap72_mutex);
        } else {
            printf("[T2] WARN: snapshot mutex timeout\n");
        }

        // ── Step 2: fixed-period wait ─────────────────────────────────────────
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(FSM_PERIOD_MS));
    }
}