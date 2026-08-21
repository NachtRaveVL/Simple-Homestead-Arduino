/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Interfaces
*/

#ifndef TerraInterfaces_H
#define TerraInterfaces_H

#include "TerraMeasurements.h"

class TerraUpdatable {
public:
    virtual ~TerraUpdatable() { }
    virtual void update(uint32_t now = terraMillis()) = 0;
};

class TerraMeasurementProvider {
public:
    virtual ~TerraMeasurementProvider() { }
    virtual TerraMeasurement getMeasurement() const = 0;
};

class TerraTextSink {
public:
    virtual ~TerraTextSink() { }
    virtual void write(const TerraString &text) = 0;
};

class TerraSerializable {
public:
    virtual ~TerraSerializable() { }
    virtual TerraString toJSON() const = 0;
};

#ifdef ARDUINO
// RTC Interface
// Common interface used by supported RTClib real-time clock implementations.
class TerraRTCInterface {
public:
    virtual ~TerraRTCInterface() { }
    virtual bool begin(TwoWire *wire = TERRA_USE_WIRE) = 0;
    virtual void adjust(const DateTime &dt) = 0;
    virtual bool lostPower() = 0;
    virtual DateTime now() = 0;
};
#endif

#endif
