/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Power Rails
*/

#ifndef TerraRails_H
#define TerraRails_H

#include "TerraObject.h"
#include "TerraCallback.hh"

class TerraPowerRail : public TerraObject {
public:
    TerraPowerRail(float nominalVoltage = 0.0f,
                   uint32_t key = TERRA_INVALID_KEY,
                   const TerraString &name = TerraString(),
                   Terra_RailType railType = Terra_RailType_Custom);  // Power rail type

    void setEnableCallback(TerraWriteCallback callback, void *context = nullptr) { _callback = callback; _context = context; }
    void setEnabledState(bool enabled);
    bool isRailEnabled() const { return _railEnabled; }
    float getNominalVoltage() const { return _nominalVoltage; }
    Terra_RailType getRailType() const { return _railType; }
    void setRailType(Terra_RailType railType) { _railType = railType; }
    void setMeasuredVoltage(float voltage) { _measuredVoltage = voltage; }
    float getMeasuredVoltage() const { return _measuredVoltage; }

protected:
    Terra_RailType _railType;                               // Power rail type
    float _nominalVoltage;                                  // Nominal rail voltage
    float _measuredVoltage;                                 // Latest measured rail voltage
    bool _railEnabled;                                      // Rail enabled state
    TerraWriteCallback _callback;                           // Configured callback
    void *_context;
};

#endif
