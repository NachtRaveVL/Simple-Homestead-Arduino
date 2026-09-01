/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Scheduler
*/

#ifndef TerraScheduler_H
#define TerraScheduler_H

class TerraScheduler;
struct TerraSchedulerSubData;
struct TerraProcess;
struct TerraTracking;

#include "Terraduino.h"

// Scheduler
// The Scheduler acts as the system's main scheduling attendant, who looks through all
// the various equipment and reservoirs you have programmed in, and figures out the best
// case tracking processes that should occur to support them. It is also responsible
// for setting up and maintaining the drivers that get assigned to reservoirs (such as the
// various pump and valve devices in use), as well as determining when significant time
// or event changes have occurred and broadcasting such out.
class TerraScheduler {
public:
    TerraScheduler();
    ~TerraScheduler();

    void update();

    inline void setNeedsScheduling() { _needsScheduling = hasSchedulerData(); }
    inline bool needsScheduling() { return _needsScheduling; }
    inline bool inDaytimeMode() const { return _inDaytimeMode; }

    void setReportInterval(TimeSpan reportInterval);

    TimeSpan getReportInterval() const;

    inline const Twilight &getDailyTwilight() const { return _dailyTwilight; }

protected:
    Twilight _dailyTwilight;                                // Daily twilight settings
    bool _needsScheduling;                                  // Needs rescheduling tracking flag
    bool _inDaytimeMode;                                    // Daytime mode flag
    tposi_t _lastDay[3];                                    // Last day tracking for rescheduling (Y-2k,M,D)
    Map<tkey_t, TerraTracking *, TERRA_SCH_PROCS_MAXSIZE> _trackings; // Reservoir tracking processes

    friend class Terraduino;
    friend struct TerraProcess;
    friend struct TerraTracking;

    inline TerraSchedulerSubData *schedulerData() const;
    inline bool hasSchedulerData() const;

    void updateDayTracking();
    void performScheduling();
    void broadcastDateChange();
};

// Scheduler Process Base
// Processes are created and managed by Scheduler to manage the daily control
// sequences necessary for resource alignment.
struct TerraProcess {
    SharedPtr<TerraReservoir> reservoir;                    // Reservoir

    Vector<TerraActuatorAttachment, TERRA_SCH_REQACTS_MAXSIZE> actuatorReqs; // Actuators required for this stage (keep-enabled list)

    time_t stageStart;                                      // Stage start time

    TerraProcess(SharedPtr<TerraReservoir> reservoir);

    void clearActuatorReqs();
    void setActuatorReqs(const Vector<TerraActuatorAttachment, TERRA_SCH_REQACTS_MAXSIZE> &actuatorReqsIn);
};

// Scheduler Tracking Process
struct TerraTracking : public TerraProcess {
    enum : signed char {Init,Assess,Fill,Condition,Distribute,Settle} stage; // Current tracking stage

    time_t canProcessAfter;                                 // Time next processing can occur (unix/UTC), else 0/disabled
    time_t lastEnvReport;                                   // Last time an environment report was generated (unix/UTC)
    bool stormingReported;                                  // Flag for storming reported
    bool nightSeqReported;                                  // Flag for night sequence reported
    bool coverSeqReported;                                  // Flag for cover sequence reported

    TerraTracking(SharedPtr<TerraReservoir> reservoir);
    ~TerraTracking();

    void setupStaging();
    void update();

private:
    void reset();
};


// Scheduler Serialization Sub Data
// A part of TSYS system data.
struct TerraSchedulerSubData : public TerraSubData {
    // TODO: additional stored params.
    time_t reportInterval;                                  // Interval between environmental sensor reports, in seconds (default: 8hrs)

    TerraSchedulerSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef TerraScheduler_H
