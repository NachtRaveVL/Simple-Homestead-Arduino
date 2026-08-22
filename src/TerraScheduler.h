/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Scheduler
*/

#ifndef TerraScheduler_H
#define TerraScheduler_H

class TerraScheduler {
public:
    TerraScheduler();

    void update();
    inline void setNeedsScheduling() { _needsScheduling = true; }
    inline bool needsScheduling() const { return _needsScheduling; }

protected:
    bool _needsScheduling;                                  // Domain scheduling refresh needed
    int32_t _lastDay;                                       // Last local date processed

    void updateDayTracking();
    void performScheduling();
};

#endif // /ifndef TerraScheduler_H
