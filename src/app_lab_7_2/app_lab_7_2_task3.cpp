#include "app_lab_7_2_task3.h"
#include "task_config.h"
#include "../srv_stdio_lcd/srv_stdio_lcd.h"
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>
#include <string.h>

// ── State name helpers ────────────────────────────────────────────────────────
static const char *ew_state_name(EwState_t s) {
    switch (s) {
        case EW_GREEN:         return "EW_GREEN        ";
        case EW_YELLOW:        return "EW_YELLOW       ";
        case EW_RED_CLEAR:     return "EW_RED_CLEAR    ";
        case EW_RED:           return "EW_RED          ";
        case EW_YELLOW_RETURN: return "EW_YELLOW_RETURN";
        default:               return "????????????????";
    }
}

static const char *ns_state_name(NsState_t s) {
    switch (s) {
        case NS_RED:       return "NS_RED      ";
        case NS_GREEN:     return "NS_GREEN    ";
        case NS_YELLOW:    return "NS_YELLOW   ";
        case NS_RED_CLEAR: return "NS_RED_CLEAR";
        default:           return "????????????";
    }
}

static const char *ew_colour(EwState_t s) {
    switch (s) {
        case EW_GREEN:         return "GRN";
        case EW_YELLOW:        return "YLW";
        case EW_YELLOW_RETURN: return "YLW";
        default:               return "RED";
    }
}

static const char *ns_colour(NsState_t s) {
    switch (s) {
        case NS_GREEN:  return "GRN";
        case NS_YELLOW: return "YLW";
        default:        return "RED";
    }
}

static bool snapshot_changed(const App72Snapshot_t *a,
                             const App72Snapshot_t *b) {
    return a->ew_state   != b->ew_state   ||
           a->ns_state   != b->ns_state   ||
           a->ew_timer   != b->ew_timer   ||
           a->ns_timer   != b->ns_timer   ||
           a->ns_request != b->ns_request ||
           a->emergency  != b->emergency;
}

// ── Task body ─────────────────────────────────────────────────────────────────
void app_lab_7_2_task3(void *pvParameters) {
    (void)pvParameters;

    App72Snapshot_t  last      = {};
    bool             has_last  = false;
    TickType_t       last_tick = 0;
    const TickType_t hb_ticks  = pdMS_TO_TICKS(SERIAL_HEARTBEAT_MS);

    for (;;) {
        // Block: wake on state change OR 1s timeout (for countdown update)
        xSemaphoreTake(g_fsm_event_72, pdMS_TO_TICKS(DISPLAY_PERIOD_MS));

        // Copy snapshot under mutex
        App72Snapshot_t snap = {};
        if (xSemaphoreTake(g_snap72_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            snap = g_snap72;
            xSemaphoreGive(g_snap72_mutex);
        } else {
            printf("[T3] WARN: snapshot mutex timeout\n");
        }

        // ── LCD ───────────────────────────────────────────────────────────────
        // Row 0: EW colour | NS colour
        // Row 1: context — emergency / active timer / idle
        char row0[17] = {0};
        char row1[17] = {0};

        if (snap.emergency) {
            snprintf(row0, sizeof(row0), "EW:RED   NS:RED ");
            uint16_t secs = (uint16_t)((snap.ew_timer * FSM_PERIOD_MS) / 1000);
            snprintf(row1, sizeof(row1), "!! EMRG %2ds    ", secs);
        } else {
            snprintf(row0, sizeof(row0), "EW:%-3s   NS:%-3s ",
                     ew_colour(snap.ew_state), ns_colour(snap.ns_state));

            // Show the active countdown — whichever timer is nonzero
            int active_timer = snap.ew_timer > 0 ? snap.ew_timer : snap.ns_timer;
            uint16_t secs = (uint16_t)((active_timer * FSM_PERIOD_MS) / 1000);

            if (active_timer > 0) {
                snprintf(row1, sizeof(row1), "T:%-3us REQ:%-3s ",
                         secs, snap.ns_request ? "YES" : "NO ");
            } else {
                snprintf(row1, sizeof(row1), "IDLE   REQ:%-3s ",
                         snap.ns_request ? "YES" : "NO ");
            }
        }

        lcd.clear();
        lcd.setCursor(0, 0); lcd.print(row0);
        lcd.setCursor(0, 1); lcd.print(row1);

        // ── Serial ────────────────────────────────────────────────────────────
        TickType_t now        = xTaskGetTickCount();
        bool changed          = !has_last || snapshot_changed(&snap, &last);
        bool heartbeat_due    = has_last && ((now - last_tick) >= hb_ticks);

        if (changed || heartbeat_due) {
            uint32_t uptime_s  = snap.uptime_ms / 1000;
            uint32_t uptime_ms = snap.uptime_ms % 1000;

            printf("==============================\n");
            printf(" Lab 7.2  |  T=%lus%03lums\n",
                   (unsigned long)uptime_s, (unsigned long)uptime_ms);
            printf("------------------------------\n");

            if (snap.emergency) {
                uint16_t secs = (uint16_t)(
                    (snap.ew_timer * FSM_PERIOD_MS) / 1000);
                printf(" [EMERGENCY]     ACTIVE — %ds remaining\n", secs);
                printf(" [EW light]      RED\n");
                printf(" [NS light]      RED\n");
                printf(" [D3 held]       %d/%d ticks\n",
                       snap.emrg_hold, EMRG_HOLD_TICKS);
            } else {
                printf(" [EW state]      %s\n", ew_state_name(snap.ew_state));
                printf(" [NS state]      %s\n", ns_state_name(snap.ns_state));
                printf(" [EW light]      %s\n", ew_colour(snap.ew_state));
                printf(" [NS light]      %s\n", ns_colour(snap.ns_state));
                if (snap.ew_timer > 0) {
                    printf(" [EW timer]      %ds remaining (%d ticks)\n",
                           (snap.ew_timer * FSM_PERIOD_MS) / 1000,
                           snap.ew_timer);
                }
                if (snap.ns_timer > 0) {
                    printf(" [NS timer]      %ds remaining (%d ticks)\n",
                           (snap.ns_timer * FSM_PERIOD_MS) / 1000,
                           snap.ns_timer);
                }
                if (snap.ew_timer == 0 && snap.ns_timer == 0) {
                    printf(" [Timer]         inactive\n");
                }
            }

            printf(" [NS request]    %s\n", snap.ns_request ? "PENDING" : "none");
            printf(" [Cycles done]   %u\n", snap.cycle_count);
            printf("==============================\n");

            last      = snap;
            has_last  = true;
            last_tick = now;
        }
    }
}