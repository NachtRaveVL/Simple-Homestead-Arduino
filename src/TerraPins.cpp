/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Pins
*/

#include "TerraPins.h"

TerraDigitalPin::TerraDigitalPin(const TerraPinSetup &setup)
    : _setup(setup), _lastState(false) { }

void TerraDigitalPin::begin()
{
#if defined(ARDUINO)
    if (!_setup.isValid()) return;
    switch (_setup.mode) {
        case Terra_PinMode_Digital_Input:
            pinMode(_setup.pin, INPUT);
            break;
        case Terra_PinMode_Digital_Input_PullUp:
            pinMode(_setup.pin, INPUT_PULLUP);
            break;
        case Terra_PinMode_Digital_Input_PullDown:
            #if defined(INPUT_PULLDOWN)
                pinMode(_setup.pin, INPUT_PULLDOWN);
            #else
                pinMode(_setup.pin, INPUT);
            #endif
            break;
        case Terra_PinMode_Digital_Output:
            pinMode(_setup.pin, OUTPUT);
            write(false);
            break;
        default:
            break;
    }
#endif
}

bool TerraDigitalPin::read() const
{
#if defined(ARDUINO)
    if (!_setup.isValid()) return false;
    const bool electricalHigh = digitalRead(_setup.pin) != LOW;
    _lastState = _setup.activeLow ? !electricalHigh : electricalHigh;
#endif
    return _lastState;
}

void TerraDigitalPin::write(bool active)
{
    if (!_setup.isValid() || !(_setup.mode == Terra_PinMode_Digital_Output)) return;
    _lastState = active;
#if defined(ARDUINO)
    const bool electricalHigh = _setup.activeLow ? !active : active;
    digitalWrite(_setup.pin, electricalHigh ? HIGH : LOW);
#endif
}

void TerraAnalogPin::begin()
{
#if defined(ARDUINO)
    if (_pin != TERRA_INVALID_PIN) pinMode(_pin, INPUT);
#endif
}

int TerraAnalogPin::read()
{
#if defined(ARDUINO)
    if (_pin != TERRA_INVALID_PIN) _lastValue = analogRead(_pin);
#endif
    return _lastValue;
}

void TerraAnalogPin::write(int value)
{
    _lastValue = value;
#if defined(ARDUINO)
    if (_pin != TERRA_INVALID_PIN) analogWrite(_pin, value);
#endif
}
