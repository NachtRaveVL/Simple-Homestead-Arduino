/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Drivers
*/

#include "Terraduino.h"
#include <math.h>

TerraCallbackInputDriver::TerraCallbackInputDriver(TerraReadCallback callback,
                                                   void *context,
                                                   Terra_Unit units)
    : _callback(callback), _context(context), _units(units)
{ ; }

void TerraCallbackInputDriver::setCallback(TerraReadCallback callback, void *context)
{
    _callback = callback;
    _context = context;
}

TerraSingleMeasurement TerraCallbackInputDriver::read(uint32_t now)
{
    if (!_callback) { return TerraSingleMeasurement(0.0f, _units, now, tframe_none); }
    float value = _callback(_context);
    return TerraSingleMeasurement(value, _units, now, isnan(value) ? tframe_none : 1);
}

TerraAnalogInputDriver::TerraAnalogInputDriver(uint8_t pin, uint8_t bitRes)
    : _pin(pin, Terra_PinMode_Analog_Input, bitRes)
{ ; }

void TerraAnalogInputDriver::begin()
{
    _pin.init();
}

TerraSingleMeasurement TerraAnalogInputDriver::read(uint32_t now)
{
    return TerraSingleMeasurement(_pin.analogRead(), Terra_Unit_Raw, now, 1);
}

bool TerraAnalogInputDriver::getPinData(TerraPinData &dataOut) const
{
    if (!_pin.isValid()) { return false; }
    _pin.saveToData(&dataOut);
    return true;
}

TerraDigitalInputDriver::TerraDigitalInputDriver(uint8_t pin,
                                                 bool activeLow,
                                                 Terra_PinMode pinMode,
                                                 Terra_Unit units)
    : _pin(pin, pinMode, activeLow), _units(units)
{ ; }

void TerraDigitalInputDriver::begin()
{
    _pin.init();
}

TerraSingleMeasurement TerraDigitalInputDriver::read(uint32_t now)
{
    return TerraSingleMeasurement(_pin.isActive() ? 1.0f : 0.0f, _units, now, 1);
}

bool TerraDigitalInputDriver::getPinData(TerraPinData &dataOut) const
{
    if (!_pin.isValid()) { return false; }
    _pin.saveToData(&dataOut);
    return true;
}

TerraCallbackOutputDriver::TerraCallbackOutputDriver(TerraWriteCallback callback, void *context)
    : _callback(callback), _context(context)
{ ; }

void TerraCallbackOutputDriver::setCallback(TerraWriteCallback callback, void *context)
{
    _callback = callback;
    _context = context;
}

bool TerraCallbackOutputDriver::write(float value)
{
    if (!_callback) { return false; }
    _callback(_context, value < 0.0f ? 0.0f : value > 1.0f ? 1.0f : value);
    return true;
}

TerraDigitalOutputDriver::TerraDigitalOutputDriver(uint8_t pin, bool activeLow)
    : _pin(pin, Terra_PinMode_Digital_Output, activeLow)
{ ; }

void TerraDigitalOutputDriver::begin()
{
    _pin.init();
}

bool TerraDigitalOutputDriver::write(float value)
{
    if (!_pin.isValid()) { return false; }
    if (value >= 0.5f) { _pin.activate(); }
    else { _pin.deactivate(); }
    return true;
}

bool TerraDigitalOutputDriver::getPinData(TerraPinData &dataOut) const
{
    if (!_pin.isValid()) { return false; }
    _pin.saveToData(&dataOut);
    return true;
}

TerraAnalogOutputDriver::TerraAnalogOutputDriver(uint8_t pin, uint8_t bitRes)
    : _pin(pin, Terra_PinMode_Analog_Output, bitRes)
{ ; }

void TerraAnalogOutputDriver::begin()
{
    _pin.init();
}

bool TerraAnalogOutputDriver::write(float value)
{
    if (!_pin.isValid()) { return false; }
    _pin.analogWrite(value < 0.0f ? 0.0f : value > 1.0f ? 1.0f : value);
    return true;
}

bool TerraAnalogOutputDriver::getPinData(TerraPinData &dataOut) const
{
    if (!_pin.isValid()) { return false; }
    _pin.saveToData(&dataOut);
    return true;
}

int TerraAnalogOutputDriver::getMaximumRaw() const
{
    return _pin.bitRes && _pin.bitRes < 16 ? (1 << _pin.bitRes) - 1 : 255;
}
