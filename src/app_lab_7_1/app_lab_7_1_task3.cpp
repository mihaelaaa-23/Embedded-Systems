#include "app_lab_7_1_task3.h"
#include "task_config.h"
#include "../srv_stdio_lcd/srv_stdio_lcd.h"
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>
#include <string.h>

// ── Helpers ───────────────────────────────────────────────────────────────────
static const char *fsm_state_str(LedFsmState_t state) {
    switch (state) {
        case FSM_STATE_OFF:      return "OFF  ";
        case FSM_STATE_BLINKING: return "BLINK";
        case FSM_STATE_ON:       return "ON   ";
        default:                 return "?????";
    }
}

static bool snapshot_changed(const App71Snapshot_t *a, const App71Snapshot_t *b) {
    return a->fsm_state      != b->fsm_state      ||
           a->led_physical   != b->led_physical   ||
           a->btn_stable     != b->btn_stable     ||
           a->blink_cycle    != b->blink_cycle    ||
           a->press_count    != b->press_count    ||
           a->timeout_ticks  != b->timeout_ticks  ||
           a->stuck_alert    != b->stuck_alert;
}

// ── Task body ─────────────────────────────────────────────────────────────────
void app_lab_7_1_task3(void *pvParameters) {
    (void)pvParameters;

    App71Snapshot_t  last      = {};
    bool             has_last  = false;
    TickType_t       last_tick = 0;
    const TickType_t hb_ticks  = pdMS_TO_TICKS(SERIAL_HEARTBEAT_MS);

    for (;;) {
        // Block until T2 signals a state change OR 500ms timeout
        xSemaphoreTake(g_fsm_event, pdMS_TO_TICKS(DISPLAY_PERIOD_MS));

        // Copy snapshot under mutex
        App71Snapshot_t snap = {};
        if (xSemaphoreTake(g_snap71_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            snap = g_snap71;
            xSemaphoreGive(g_snap71_mutex);
        } else {
            printf("[T3] WARN: snapshot mutex timeout\n");
        }

        // ── LCD: row0 = state + LED, row1 = BTN + timeout or blink progress
        const char *state_s = fsm_state_str(snap.fsm_state);
        const char *led_s   = snap.led_physical ? "ON " : "OFF";
        const char *btn_s   = snap.btn_stable   ? "ON " : "OFF";

        char row0[17] = {0};
        char row1[17] = {0};

        snprintf(row0, sizeof(row0), "ST:%-5s LED:%-3s", state_s, led_s);

        // Row 1 context-sensitive:
        //   LED_ON   → show auto-off countdown in seconds
        //   BLINKING → show blink cycle progress
        //   OFF      → show total press count
        if (snap.fsm_state == FSM_STATE_ON) {
            uint16_t secs = (uint16_t)((snap.timeout_ticks * FSM_PERIOD_MS) / 1000);
            snprintf(row1, sizeof(row1), "BTN:%-3s OFF:%3ds", btn_s, secs);
        } else if (snap.fsm_state == FSM_STATE_BLINKING) {
            snprintf(row1, sizeof(row1), "BTN:%-3s BLK:%d/%d", btn_s,
                     snap.blink_cycle, BLINK_COUNT);
        } else {
            snprintf(row1, sizeof(row1), "BTN:%-3s #%-5u", btn_s,
                     snap.press_count);
        }

        lcd.clear();
        lcd.setCursor(0, 0); lcd.print(row0);
        lcd.setCursor(0, 1); lcd.print(row1);

        // ── Serial: print on change or heartbeat ──────────────────────────────
        TickType_t now        = xTaskGetTickCount();
        bool changed          = !has_last || snapshot_changed(&snap, &last);
        bool heartbeat_due    = has_last && ((now - last_tick) >= hb_ticks);

        if (changed || heartbeat_due) {
            uint32_t uptime_s  = snap.uptime_ms / 1000;
            uint32_t uptime_ms = snap.uptime_ms % 1000;

            printf("==============================\n");
            printf(" Lab 7.1  |  T=%lus%03lums\n",
                   (unsigned long)uptime_s, (unsigned long)uptime_ms);
            printf("------------------------------\n");
            printf(" [FSM State]     %s\n", fsm_state_str(snap.fsm_state));
            printf(" [LED output]    %s\n", snap.led_physical ? "ON"  : "OFF");
            printf(" [Button raw]    %s\n", snap.btn_raw    ? "PRESSED" : "RELEASED");
            printf(" [Button stable] %s%s\n",
                   snap.btn_stable ? "PRESSED" : "RELEASED",
                   snap.stuck_alert ? "  !! STUCK !!" : "");
            printf(" [Press count]   %u\n", snap.press_count);

            if (snap.fsm_state == FSM_STATE_ON) {
                uint16_t secs = (uint16_t)((snap.timeout_ticks * FSM_PERIOD_MS)
                                           / 1000);
                printf(" [Auto-off]      %us remaining\n", secs);
            } else {
                printf(" [Auto-off]      inactive\n");
            }

            if (snap.fsm_state == FSM_STATE_BLINKING) {
                printf(" [Blink cycle]   %d / %d  (-> %s)\n",
                       snap.blink_cycle, BLINK_COUNT,
                       fsm_state_str(snap.target_state));
            }

            printf("==============================\n");

            last      = snap;
            has_last  = true;
            last_tick = now;
        }
    }
}