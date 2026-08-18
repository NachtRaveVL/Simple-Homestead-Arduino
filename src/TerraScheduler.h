/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Scheduler
*/

#ifndef TerraScheduler_H
#define TerraScheduler_H

#include "TerraDefines.h"
#include "TerraCallback.hh"

struct TerraScheduledTask {
    TerraTaskCallback callback;                             // Scheduled task callback
    void *context;
    uint32_t intervalMs;                                    // Task interval, milliseconds
    uint32_t lastRunAt;                                     // Last task run timestamp
    bool enabled;                                           // Enabled state
    bool daily;                                             // Daily-task flag
    uint16_t minuteOfDay;                                   // Scheduled minute of day
    int16_t lastDay;                                        // Last day task ran

    TerraScheduledTask()
        : callback(nullptr), context(nullptr), intervalMs(0), lastRunAt(0), enabled(false), daily(false), minuteOfDay(0), lastDay(-1) { }
};

class TerraScheduler {
public:
    TerraScheduler();
    int8_t addIntervalTask(TerraTaskCallback callback, void *context, uint32_t intervalMs, bool runImmediately = false);
    int8_t addDailyTask(TerraTaskCallback callback, void *context, uint16_t minuteOfDay);
    bool removeTask(uint8_t index);
    void enableTask(uint8_t index, bool enabled);
    void update(uint32_t nowMs = terraMillis(), uint16_t minuteOfDay = 0, int16_t dayNumber = 0);
    uint8_t count() const { return _count; }

protected:
    TerraScheduledTask _tasks[TERRA_MAX_SCHEDULE_TASKS];    // Scheduled task table
    uint8_t _count;                                         // Active entry count
};

#endif
