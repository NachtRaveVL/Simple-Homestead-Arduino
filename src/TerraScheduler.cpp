/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Scheduler
*/

#include "TerraScheduler.h"
#include "TerraUtils.h"

TerraScheduler::TerraScheduler() : _count(0) { }

int8_t TerraScheduler::addIntervalTask(TerraTaskCallback callback, void *context, uint32_t intervalMs, bool runImmediately) {
    if (!callback || !intervalMs || _count >= TERRA_MAX_SCHEDULE_TASKS) return -1;
    TerraScheduledTask &task = _tasks[_count];
    task.callback = callback; task.context = context; task.intervalMs = intervalMs; task.enabled = true; task.daily = false;
    task.lastRunAt = runImmediately ? terraMillis() - intervalMs : terraMillis();
    return (int8_t)_count++;
}

int8_t TerraScheduler::addDailyTask(TerraTaskCallback callback, void *context, uint16_t minuteOfDay) {
    if (!callback || minuteOfDay >= 1440 || _count >= TERRA_MAX_SCHEDULE_TASKS) return -1;
    TerraScheduledTask &task = _tasks[_count];
    task.callback = callback; task.context = context; task.enabled = true; task.daily = true; task.minuteOfDay = minuteOfDay; task.lastDay = -1;
    return (int8_t)_count++;
}

bool TerraScheduler::removeTask(uint8_t index) {
    if (index >= _count) return false;
    for (uint8_t i = index + 1; i < _count; ++i) _tasks[i - 1] = _tasks[i];
    --_count;
    return true;
}

void TerraScheduler::enableTask(uint8_t index, bool enabled) { if (index < _count) _tasks[index].enabled = enabled; }

void TerraScheduler::update(uint32_t nowMs, uint16_t minuteOfDay, int16_t dayNumber) {
    for (uint8_t i = 0; i < _count; ++i) {
        TerraScheduledTask &task = _tasks[i];
        if (!task.enabled || !task.callback) continue;
        if (task.daily) {
            if (minuteOfDay >= task.minuteOfDay && task.lastDay != dayNumber) {
                task.callback(task.context); task.lastDay = dayNumber;
            }
        } else if (terraElapsed(nowMs, task.lastRunAt, task.intervalMs)) {
            task.callback(task.context); task.lastRunAt = nowMs;
        }
    }
}
