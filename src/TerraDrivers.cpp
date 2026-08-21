/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Drivers
*/

#include "TerraDrivers.h"
#include "TerraUtils.h"

TerraCallbackInputDriver::TerraCallbackInputDriver(TerraReadCallback callback,
                                                   void *context,
                                                   Terra_Unit unit)
    : _callback(callback), _context(context), _unit(unit) { }

void TerraCallbackInputDriver::setCallback(TerraReadCallback callback, void *context) {
    _callback = callback;
    _context = context;
}

TerraMeasurement TerraCallbackInputDriver::read(uint32_t now) {
    if (!_callback) return TerraMeasurement(0.0f, _unit, now, false);
    float value = _callback(_context);
    return TerraMeasurement(value, _unit, now, !isnan(value));
}

TerraAnalogInputDriver::TerraAnalogInputDriver(uint8_t pin)
    : _pin(pin) { }

void TerraAnalogInputDriver::begin() {
    _pin.begin();
}

TerraMeasurement TerraAnalogInputDriver::read(uint32_t now) {
    return TerraMeasurement((float)_pin.read(), Terra_Unit_Raw, now, true);
}

bool TerraAnalogInputDriver::getPinSetup(TerraPinSetup &setup) const {
    setup = TerraPinSetup(_pin.getPin(), Terra_PinMode_Analog_Input, false);
    return setup.isValid();
}

TerraDigitalInputDriver::TerraDigitalInputDriver(const TerraPinSetup &setup, Terra_Unit unit)
    : _pin(setup), _unit(unit) { }

void TerraDigitalInputDriver::begin() {
    _pin.begin();
}

TerraMeasurement TerraDigitalInputDriver::read(uint32_t now) {
    return TerraMeasurement(_pin.read() ? 1.0f : 0.0f, _unit, now, true);
}

TerraCallbackOutputDriver::TerraCallbackOutputDriver(TerraWriteCallback callback, void *context)
    : _callback(callback), _context(context) { }

void TerraCallbackOutputDriver::setCallback(TerraWriteCallback callback, void *context) {
    _callback = callback;
    _context = context;
}

bool TerraCallbackOutputDriver::write(float value) {
    if (!_callback) return false;
    _callback(_context, terraClamp(value, 0.0f, 1.0f));
    return true;
}

TerraDigitalOutputDriver::TerraDigitalOutputDriver(const TerraPinSetup &setup)
    : _pin(setup) { }

void TerraDigitalOutputDriver::begin() {
    _pin.begin();
}

bool TerraDigitalOutputDriver::write(float value) {
    _pin.write(value >= 0.5f);
    return _pin.getSetup().isValid();
}

TerraAnalogOutputDriver::TerraAnalogOutputDriver(uint8_t pin, int maximumRaw)
    : _pin(pin), _maximumRaw(maximumRaw > 0 ? maximumRaw : 255) { }

void TerraAnalogOutputDriver::begin()
{
    _pin.begin();
}

bool TerraAnalogOutputDriver::write(float value)
{
    if (_pin.getPin() == TERRA_INVALID_PIN) return false;
    const float normalized = terraClamp(value, 0.0f, 1.0f);
    const int raw = (int)(normalized * (float)_maximumRaw + 0.5f);
    _pin.write(raw);
    return true;
}
