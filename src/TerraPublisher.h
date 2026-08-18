/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Publisher
*/

#ifndef TerraPublisher_H
#define TerraPublisher_H

#include "TerraDefines.h"
#include "TerraCallback.hh"
#include "TerraInterfaces.h"

struct TerraPublishChannel {
    TerraString name;                                       // Display name
    const TerraMeasurementProvider *provider;
    Terra_Unit outputUnit;                                  // Output Unit

    TerraPublishChannel() : name(), provider(nullptr), outputUnit(Terra_Unit_Undefined) { }
};

class TerraPublisher {
public:
    TerraPublisher();

    bool addChannel(const TerraString &name, const TerraMeasurementProvider *provider, Terra_Unit outputUnit = Terra_Unit_Undefined);
    bool removeChannel(const TerraString &name);
    void setCallback(TerraPublishCallback callback, void *context = nullptr) { _callback = callback; _context = context; }
    void setInterval(uint32_t intervalMs) { _intervalMs = intervalMs; }
    uint32_t getInterval() const { return _intervalMs; }
    void publish(uint32_t now = terraMillis());
    void update(uint32_t now = terraMillis());
    uint8_t count() const { return _count; }

protected:
    TerraPublishChannel _channels[TERRA_MAX_PUBLISH_CHANNELS];  // Publisher channel table
    uint8_t _count;                                         // Active entry count
    TerraPublishCallback _callback;                         // Configured callback
    void *_context;
    uint32_t _intervalMs;                                   // Update/publish interval, milliseconds
    uint32_t _lastPublishAt;                                // Last publish timestamp
};

#endif
