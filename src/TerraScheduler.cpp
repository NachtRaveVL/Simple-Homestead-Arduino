/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Scheduler
*/

#include "Terraduino.h"
#include "TerraUtils.h"

TerraScheduler::TerraScheduler()
    : _count(0)
{ ; }

int8_t TerraScheduler::addIntervalTask(TerraTaskCallback callback, void *context,
                                       uint32_t intervalMs, bool runImmediately)
{
    if (!callback || !intervalMs || _count >= TERRA_MAX_SCHEDULE_TASKS) { return -1; }

    TerraScheduledTask &task = _tasks[_count];
    task.callback = callback;
    task.context = context;
    task.intervalMs = intervalMs;
    task.enabled = true;
    task.daily = false;
    task.lastRunAt = runImmediately ? millis() - intervalMs : millis();
    return (int8_t)_count++;
}

int8_t TerraScheduler::addDailyTask(TerraTaskCallback callback, void *context, uint16_t minuteOfDay)
{
    if (!callback || minuteOfDay >= 1440 || _count >= TERRA_MAX_SCHEDULE_TASKS) { return -1; }

    TerraScheduledTask &task = _tasks[_count];
    task.callback = callback;
    task.context = context;
    task.enabled = true;
    task.daily = true;
    task.minuteOfDay = minuteOfDay;
    task.lastDay = -1;
    return (int8_t)_count++;
}

bool TerraScheduler::removeTask(uint8_t index)
{
    if (index >= _count) { return false; }

    for (uint8_t i = index + 1; i < _count; ++i) {
        _tasks[i - 1] = _tasks[i];
    }
    --_count;
    return true;
}

void TerraScheduler::enableTask(uint8_t index, bool enabled)
{
    if (index < _count) { _tasks[index].enabled = enabled; }
}

void TerraScheduler::update()
{
    uint32_t nowMs = millis();
    uint16_t minuteOfDay = 0;
    int32_t dayNumber = -1;

#ifdef ARDUINO
    DateTime localTime = terraLocalNow();
    minuteOfDay = (uint16_t)localTime.hour() * 60U + localTime.minute();
    dayNumber = (int32_t)localTime.year() * 512L +
                (int32_t)localTime.month() * 32L + localTime.day();
#else
    time_t localTimestamp = terraLocalNow();
    struct tm *localTime = gmtime(&localTimestamp);
    if (localTime) {
        minuteOfDay = (uint16_t)localTime->tm_hour * 60U + (uint16_t)localTime->tm_min;
        dayNumber = (int32_t)(localTime->tm_year + 1900) * 512L +
                    (int32_t)(localTime->tm_mon + 1) * 32L + localTime->tm_mday;
    }
#endif

    for (uint8_t i = 0; i < _count; ++i) {
        TerraScheduledTask &task = _tasks[i];
        if (!task.enabled || !task.callback) { continue; }

        if (task.daily) {
            if (dayNumber >= 0 && minuteOfDay >= task.minuteOfDay && task.lastDay != dayNumber) {
                task.lastDay = dayNumber;
                task.callback(task.context);
            }
        } else if (task.intervalMs && (uint32_t)(nowMs - task.lastRunAt) >= task.intervalMs) {
            task.lastRunAt = nowMs;
            task.callback(task.context);
        }
    }
}
