/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Scheduler
*/

#include "Terraduino.h"

TerraScheduler::TerraScheduler()
    : _needsScheduling(true), _lastDay(-1)
{ ; }

void TerraScheduler::updateDayTracking()
{
#ifdef ARDUINO
    DateTime localTime = terraLocalNow();
    int32_t dayNumber = (int32_t)localTime.year() * 512L +
                        (int32_t)localTime.month() * 32L + localTime.day();
#else
    time_t localTimestamp = terraLocalNow();
    struct tm *localTime = gmtime(&localTimestamp);
    int32_t dayNumber = localTime ?
                        (int32_t)(localTime->tm_year + 1900) * 512L +
                        (int32_t)(localTime->tm_mon + 1) * 32L + localTime->tm_mday : -1;
#endif
    if (dayNumber >= 0 && dayNumber != _lastDay) {
        _lastDay = dayNumber;
        setNeedsScheduling();
    }
}

void TerraScheduler::performScheduling()
{
    // Terraduino's current water and thermal processes are demand driven by their
    // registered route/loop objects. Scheduling refresh therefore only acknowledges
    // time-boundary changes until a domain process requires an explicit schedule.
    _needsScheduling = false;
}

void TerraScheduler::update()
{
    updateDayTracking();
    if (_needsScheduling) { performScheduling(); }
}
