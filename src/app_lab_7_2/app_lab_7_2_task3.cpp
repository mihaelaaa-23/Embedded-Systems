#include "app_lab_7_2_task3.h"
#include "task_config.h"
#include "../srv_stdio_lcd/srv_stdio_lcd.h"
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>
#include <string.h>

// ── Helpers ───────────────────────────────────────────────────────────────────
static const char *phase_name(TrafficPhase_t p) {
    switch (p) {
        case PHASE_EW_GREEN:  return "EW_GREEN ";
        case PHASE_EW_YELLOW: return "EW_YELLOW";
        case PHASE_NS_GREEN:  return "NS_GREEN ";
        case PHASE_NS_YELLOW: return "NS_YELLOW";
        case PHASE_EMERGENCY: return "EMERGENCY";
        default:              return "?????????";
    }
}

// EW and NS light colours derived from phase (Moore: output = f(state))
static const char *ew_colour(TrafficPhase_t p) {
    switch (p) {
        case PHASE_EW_GREEN:  return "GRN";
        case PHASE_EW_YELLOW: return "YLW";
        default:              return "RED";
    }
}

static const char *ns_colour(TrafficPhase_t p) {
    switch (p) {
        case PHASE_NS_GREEN:  return "GRN";
        case PHASE_NS_YELLOW: return "YLW";
        default:              return "RED";
    }
}

static bool snapshot_changed(const App72Snapshot_t *a,
                             const App72Snapshot_t *b) {
    return a->phase       != b->phase       ||
           a->timer_ticks != b->timer_ticks ||
           a->ns_request  != b->ns_request  ||
           a->emergency   != b->emergency;
}

// ── Task body ─────────────────────────────────────────────────────────────────
void app_lab_7_2_task3(void *pvParameters) {
    (void)pvParameters;

    App72Snapshot_t  last      = {};
    bool             has_last  = false;
    TickType_t       last_tick = 0;
    const TickType_t hb_ticks  = pdMS_TO_TICKS(SERIAL_HEARTBEAT_MS);

    for (;;) {
        // Block: wake on phase change OR 500ms timeout
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
        // Row 0: EW state + NS state
        // Row 1: timer remaining (s) + NS request flag
        char row0[17] = {0};
        char row1[17] = {0};

        snprintf(row0, sizeof(row0), "EW:%-3s   NS:%-3s",
                 ew_colour(snap.phase), ns_colour(snap.phase));

        uint16_t timer_s = (uint16_t)((snap.timer_ticks * FSM_PERIOD_MS) / 1000);

        if (snap.emergency) {
            snprintf(row1, sizeof(row1), "!! EMERGENCY !!!");
        } else if (snap.timer_ticks > 0) {
            snprintf(row1, sizeof(row1), "T:%-3us REQ:%s",
                     timer_s, snap.ns_request ? "YES" : "NO ");
        } else {
            snprintf(row1, sizeof(row1), "IDLE   REQ:%s",
                     snap.ns_request ? "YES" : "NO ");
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
            printf(" [Phase]         %s\n", phase_name(snap.phase));
            printf(" [EW light]      %s\n", ew_colour(snap.phase));
            printf(" [NS light]      %s\n", ns_colour(snap.phase));

            if (snap.timer_ticks > 0) {
                printf(" [Timer]         %us remaining (%d ticks)\n",
                       timer_s, snap.timer_ticks);
            } else {
                printf(" [Timer]         inactive (waiting for request)\n");
            }

            printf(" [NS request]    %s\n", snap.ns_request ? "PENDING" : "none");
            printf(" [Emergency]     %s\n", snap.emergency  ? "ACTIVE" : "clear");
            printf(" [Cycles done]   %u\n", snap.cycle_count);
            printf("==============================\n");

            last      = snap;
            has_last  = true;
            last_tick = now;
        }
    }
}