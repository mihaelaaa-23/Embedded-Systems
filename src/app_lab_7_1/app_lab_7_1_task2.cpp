#include "app_lab_7_1_task2.h"
#include "app_lab_7_1_task1.h"
#include "task_config.h"
#include "../dd_led/dd_led.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

// ── Shared globals (defined here, declared extern in task_config.h) ───────────
App71Snapshot_t g_snap71 = {};
SemaphoreHandle_t g_snap71_mutex = NULL;
SemaphoreHandle_t g_fsm_event = NULL;

// ── Table-driven Moore FSM ────────────────────────────────────────────────────
// Matches the theory structure from Lab 7.1, Listing 7.3 / Table 7.1.
//
//  Num │ Name      │ Out   │ In=0      │ In=1
//  ────┼───────────┼───────┼───────────┼──────────
//   0  │ LED_OFF   │ false │ LED_OFF   │ BLINKING
//   1  │ BLINKING  │ phase │ BLINKING  │ BLINKING
//   2  │ LED_ON    │ true  │ LED_ON    │ BLINKING
//
// Two additional autonomous transitions bypass the table:
//   LED_ON → LED_OFF  when auto-off timer expires (no press required)
//   Stuck alert flag  when button raw stays PRESSED > STUCK_TICKS steps

typedef struct
{
    bool out;
    LedFsmState_t next[2]; // next[0]=no press, next[1]=press confirmed
} FsmState_t;

static const FsmState_t FSM[3] = {
    /* FSM_STATE_OFF      */ {false, {FSM_STATE_OFF, FSM_STATE_BLINKING}},
    /* FSM_STATE_BLINKING */ {false, {FSM_STATE_BLINKING, FSM_STATE_BLINKING}},
    /* FSM_STATE_ON       */ {true, {FSM_STATE_ON, FSM_STATE_BLINKING}},
};

// ── Blink sub-state ───────────────────────────────────────────────────────────
static LedFsmState_t s_blink_target = FSM_STATE_ON;
static int s_blink_cycle = 0;
static int s_blink_ticks = 0;
static bool s_blink_phase = false;

static bool fsm_blink_tick(LedFsmState_t *state)
{
    s_blink_ticks++;
    if (s_blink_ticks >= BLINK_HALF_TICKS)
    {
        s_blink_ticks = 0;
        s_blink_phase = !s_blink_phase;
        if (!s_blink_phase)
            s_blink_cycle++;
        if (s_blink_cycle >= BLINK_COUNT)
        {
            *state = s_blink_target;
            return (*state == FSM_STATE_ON);
        }
    }
    return !s_blink_phase;
}

static void fsm_blink_enter(LedFsmState_t from)
{
    s_blink_target = (from == FSM_STATE_OFF) ? FSM_STATE_ON : FSM_STATE_OFF;
    s_blink_cycle = 0;
    s_blink_ticks = 0;
    s_blink_phase = false;
}

// ── Auto-off timer ────────────────────────────────────────────────────────────
// Counts FSM steps while in LED_ON with no press.
// Resets on every press or whenever FSM leaves LED_ON.
// At AUTO_OFF_TICKS, forces FSM → LED_OFF (autonomous timed transition).
static int s_autooff_ticks = 0;

// ── Stuck-button detection ────────────────────────────────────────────────────
// Counts consecutive FSM steps where raw button is PRESSED.
// Clears immediately when button is released.
static int s_stuck_ticks = 0;
static bool s_stuck_alert = false;

// ── Press counter ─────────────────────────────────────────────────────────────
static uint16_t s_press_count = 0;

// ── Green LED warning blink counter ─────────────────────────────────────────
// Tracks half-period phase steps for the auto-off warning blink.
// Resets when warning window is exited (steady ON resumes).
static int s_green_blink_ticks = 0;      // steps in current half-period
static bool s_green_blink_phase = false; // false=ON, true=OFF

// ── 3-sample debounce ─────────────────────────────────────────────────────────
static bool s_stable = false;
static bool s_candidate = false;
static int s_deb_count = 0;
static bool s_was_stable = false;

static void debounce_step(bool raw)
{
    if (raw == s_candidate)
    {
        if (s_deb_count < BTN_PERSISTENCE_SAMPLES)
            s_deb_count++;
    }
    else
    {
        s_candidate = raw;
        s_deb_count = 1;
    }
    if (s_deb_count >= BTN_PERSISTENCE_SAMPLES)
        s_stable = s_candidate;
}

// ── LED output helper ─────────────────────────────────────────────────────────
static void fsm_set_led(bool on)
{
    on ? dd_led_turn_on() : dd_led_turn_off();
    on ? dd_led_2_turn_on() : dd_led_2_turn_off();
}

// ── Task init ─────────────────────────────────────────────────────────────────
void app_lab_7_1_task2_init()
{
    g_snap71_mutex = xSemaphoreCreateMutex();
    g_fsm_event = xSemaphoreCreateBinary();
}

// ── Task body — 4-step Moore evaluation (matches Lab 7.1 Listing 7.5) ────────
void app_lab_7_1_task2(void *pvParameters)
{
    (void)pvParameters;

    LedFsmState_t s_fsm_state = FSM_STATE_OFF;
    TickType_t last_wake = xTaskGetTickCount();

    for (;;)
    {
        LedFsmState_t prev_state = s_fsm_state;

        // ── Step 1: Apply output based on current state (Moore: out = f(state))
        bool led_out;
        if (s_fsm_state == FSM_STATE_BLINKING)
        {
            led_out = fsm_blink_tick(&s_fsm_state);
        }
        else
        {
            led_out = FSM[s_fsm_state].out;
        }
        fsm_set_led(led_out);

        // ── Step 2: Wait — handled by vTaskDelayUntil at end of loop

        // ── Step 3: Read input — debounced rising edge from T1
        bool raw = app_lab_7_1_task1_get_raw();
        debounce_step(raw);
        bool rising_edge = (s_stable && !s_was_stable);
        bool input = rising_edge && (s_fsm_state != FSM_STATE_BLINKING);
        s_was_stable = s_stable;

        // ── Stuck-button detection (independent of FSM state)
        // Raw button held continuously for STUCK_MS → raise alert
        if (raw)
        {
            if (s_stuck_ticks < STUCK_TICKS)
                s_stuck_ticks++;
            s_stuck_alert = (s_stuck_ticks >= STUCK_TICKS);
        }
        else
        {
            s_stuck_ticks = 0;
            s_stuck_alert = false;
        }

        // ── Press counter
        if (input)
            s_press_count++;

        // ── Step 4: Compute next state ────────────────────────────────────────
        //
        // Priority order:
        //   (a) Auto-off timeout in LED_ON — timed autonomous transition
        //   (b) Normal table lookup       — input-driven transition
        //
        // Auto-off resets on press (so pressing before timeout is clean).

        bool auto_off_fired = false;

        if (s_fsm_state == FSM_STATE_ON)
        {
            if (input)
            {
                // Press received — reset timer, table handles the transition
                s_autooff_ticks = 0;
            }
            else
            {
                s_autooff_ticks++;
                if (s_autooff_ticks >= AUTO_OFF_TICKS)
                {
                    // Timed autonomous transition: LED_ON → LED_OFF
                    s_autooff_ticks = 0;
                    s_fsm_state = FSM_STATE_OFF;
                    auto_off_fired = true;
                    fsm_set_led(false); // apply OFF immediately
                    printf("[AUTO-OFF] LED off after %ds idle\n",
                           AUTO_OFF_MS / 1000);
                }
            }
        }
        else
        {
            s_autooff_ticks = 0; // reset whenever outside LED_ON
        }

        if (!auto_off_fired)
        {
            // Normal table-driven transition
            if (input)
                fsm_blink_enter(s_fsm_state);
            s_fsm_state = FSM[s_fsm_state].next[(int)input];
        }

        // ── Green LED: steady ON normally, fast blink during auto-off warning
        // Warning window: remaining ticks <= AUTO_OFF_WARN_TICKS (last 3s).
        // Outside warning: reset blink counter so it restarts cleanly next time.
        bool in_warning = (s_fsm_state == FSM_STATE_ON) &&
                          ((AUTO_OFF_TICKS - s_autooff_ticks) <= AUTO_OFF_WARN_TICKS);

        if (in_warning)
        {
            // Advance half-period counter, toggle phase at threshold
            s_green_blink_ticks++;
            if (s_green_blink_ticks >= GREEN_WARN_BLINK_TICKS)
            {
                s_green_blink_ticks = 0;
                s_green_blink_phase = !s_green_blink_phase;
            }
            s_green_blink_phase ? dd_led_1_turn_off() : dd_led_1_turn_on();
        }
        else
        {
            // Steady ON — reset blink state for a clean restart next warning
            s_green_blink_ticks = 0;
            s_green_blink_phase = false;
            dd_led_1_turn_on();
        }
        dd_led_apply();

        // ── Signal T3 immediately if FSM state changed
        if (s_fsm_state != prev_state)
            xSemaphoreGive(g_fsm_event);

        // ── Publish snapshot for T3
        if (xSemaphoreTake(g_snap71_mutex, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            g_snap71.fsm_state = s_fsm_state;
            g_snap71.led_physical = dd_led_is_on();
            g_snap71.btn_raw = raw;
            g_snap71.btn_stable = s_stable;
            g_snap71.blink_cycle = s_blink_cycle;
            g_snap71.target_state = s_blink_target;
            g_snap71.press_count = s_press_count;
            g_snap71.uptime_ms = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
            // timeout_ticks: remaining steps until auto-off (0 when inactive)
            g_snap71.timeout_ticks = (s_fsm_state == FSM_STATE_ON)
                                         ? (AUTO_OFF_TICKS - s_autooff_ticks)
                                         : 0;
            g_snap71.stuck_alert = s_stuck_alert;
            xSemaphoreGive(g_snap71_mutex);
        }
        else
        {
            printf("[T2] WARN: snapshot mutex timeout\n");
        }

        // ── Step 2 (FreeRTOS): fixed-period wait
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(FSM_PERIOD_MS));
    }
}