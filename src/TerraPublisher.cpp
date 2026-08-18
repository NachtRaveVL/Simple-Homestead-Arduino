/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Publisher
*/

#include "TerraPublisher.h"
#include "TerraMeasurements.h"
#include "TerraUtils.h"

static bool terraStringEqualExact(const TerraString &a, const TerraString &b) {
#if defined(ARDUINO)
    return a == b;
#else
    return a.compare(b) == 0;
#endif
}

TerraPublisher::TerraPublisher()
    : _count(0), _callback(nullptr), _context(nullptr), _intervalMs(TERRA_DEFAULT_PUBLISH_INTERVAL), _lastPublishAt(0) { }

bool TerraPublisher::addChannel(const TerraString &name, const TerraMeasurementProvider *provider, Terra_Unit outputUnit) {
    if (!provider || _count >= TERRA_MAX_PUBLISH_CHANNELS) return false;
    for (uint8_t i = 0; i < _count; ++i) if (terraStringEqualExact(_channels[i].name, name)) return false;
    _channels[_count].name = name;
    _channels[_count].provider = provider;
    _channels[_count].outputUnit = outputUnit;
    ++_count;
    return true;
}

bool TerraPublisher::removeChannel(const TerraString &name) {
    for (uint8_t i = 0; i < _count; ++i) {
        if (terraStringEqualExact(_channels[i].name, name)) {
            for (uint8_t j = i + 1; j < _count; ++j) _channels[j - 1] = _channels[j];
            --_count;
            return true;
        }
    }
    return false;
}

void TerraPublisher::publish(uint32_t now) {
    if (!_callback) return;
    for (uint8_t i = 0; i < _count; ++i) {
        TerraMeasurement measurement = _channels[i].provider->getMeasurement();
        if (_channels[i].outputUnit != Terra_Unit_Undefined && measurement.unit != _channels[i].outputUnit)
            measurement = terraConvertMeasurement(measurement, _channels[i].outputUnit);
        _callback(_context, _channels[i].name.c_str(), measurement);
    }
    _lastPublishAt = now;
}

void TerraPublisher::update(uint32_t now) {
    if (!_lastPublishAt || terraElapsed(now, _lastPublishAt, _intervalMs)) publish(now);
}
