/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Scheduler
*/

#ifndef TerraScheduler_H
#define TerraScheduler_H

class TerraScheduler;
struct TerraSchedulerSubData;

#include "TerraData.h"

class TerraScheduler {
public:
    TerraScheduler();

    void update();
    inline void setNeedsScheduling() { _needsScheduling = hasSchedulerData(); }
    inline bool needsScheduling() const { return _needsScheduling; }

protected:
    bool _needsScheduling;                                  // Domain scheduling refresh needed
    tposi_t _lastDay[3];                                    // Last day tracking for rescheduling (Y-2k,M,D)

    friend class Terraduino;

    inline TerraSchedulerSubData *schedulerData() const;
    inline bool hasSchedulerData() const;

    void updateDayTracking();
    void performScheduling();
    void broadcastDateChange();
};


// Scheduler Serialization Sub Data
// A part of TSYS system data.
struct TerraSchedulerSubData : public TerraSubData {
    TerraSchedulerSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef TerraScheduler_H
