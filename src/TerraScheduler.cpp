/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Scheduler
*/

#include "Terraduino.h"

TerraScheduler::TerraScheduler()
    : _needsScheduling(false), _lastDay{0}
{ ; }

void TerraScheduler::update()
{
    if (hasSchedulerData()) {
        DateTime currTime = localTime(unixNow());

        if (!(_lastDay[0] == currTime.year()-2000 &&
              _lastDay[1] == currTime.month() &&
              _lastDay[2] == currTime.day())) {
            // only log uptime upon actual day change and if uptime has been at least 1d
            if (getLogger()->getSystemUptime() >= SECS_PER_DAY) {
                getLogger()->logSystemUptime();
            }
            broadcastDateChange();
        }

        if (needsScheduling()) { performScheduling(); }
    }
}

void TerraScheduler::updateDayTracking()
{
    DateTime currTime = localTime(unixNow());
    _lastDay[0] = currTime.year()-2000;
    _lastDay[1] = currTime.month();
    _lastDay[2] = currTime.day();

    setNeedsScheduling();
    Terraduino::_activeInstance->setNeedsRedraw();
}

void TerraScheduler::performScheduling()
{
    // Terraduino's current water and thermal processes are demand driven by their
    // registered route/loop objects. Scheduling refresh therefore only acknowledges
    // time-boundary changes until a domain process requires an explicit schedule.
    _needsScheduling = false;
}

void TerraScheduler::broadcastDateChange()
{
    updateDayTracking();

    #ifdef TERRA_USE_MULTITASKING
        // these can take a while to complete
        taskManager.scheduleOnce(0, []{
            if (getController()) {
                getController()->broadcastDateChanged();
            }
            yield();
            if (getLogger()) {
                getLogger()->notifyDateChanged();
            }
            yield();
            if (getPublisher()) {
                getPublisher()->notifyDateChanged();
            }
            yield();
        });
    #else
        if (getController()) {
            getController()->broadcastDateChanged();
        }
        if (getLogger()) {
            getLogger()->notifyDateChanged();
        }
        if (getPublisher()) {
            getPublisher()->notifyDateChanged();
        }
    #endif
}


TerraSchedulerSubData::TerraSchedulerSubData()
    : TerraSubData(0)
{ ; }

void TerraSchedulerSubData::toJSONObject(JsonObject &objectOut) const
{
    (void)objectOut;
}

void TerraSchedulerSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    (void)objectIn;
}
