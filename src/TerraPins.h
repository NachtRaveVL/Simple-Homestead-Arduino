/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Pins
*/

#ifndef TerraPins_H
#define TerraPins_H

#include "TerraDefines.h"
#include "TerraTypes.h"

struct TerraPinSetup {
    uint8_t pin;                              ///< MCU pin number
    Terra_PinMode mode;                       ///< Portable pin mode
    bool activeLow;                           ///< Logical active state is electrically low

    TerraPinSetup(uint8_t pinIn = TERRA_INVALID_PIN,
                  Terra_PinMode modeIn = Terra_PinMode_Digital_Input,
                  bool activeLowIn = false)
        : pin(pinIn), mode(modeIn), activeLow(activeLowIn) { }

    // Compatibility constructor for the earlier output/input boolean setup.
    TerraPinSetup(uint8_t pinIn, bool outputIn, bool activeLowIn)
        : pin(pinIn), mode(outputIn ? Terra_PinMode_Digital_Output : Terra_PinMode_Digital_Input),
          activeLow(activeLowIn) { }

    bool isValid() const { return pin != TERRA_INVALID_PIN && mode != Terra_PinMode_Undefined; }
    bool isOutput() const { return mode == Terra_PinMode_Digital_Output || mode == Terra_PinMode_Analog_Output; }
    bool isAnalog() const { return mode == Terra_PinMode_Analog_Input || mode == Terra_PinMode_Analog_Output; }
};

class TerraDigitalPin {
public:
    TerraDigitalPin(const TerraPinSetup &setup = TerraPinSetup());

    void begin();
    bool read() const;
#if !defined(ARDUINO)
    void setSimulatedState(bool active) { _lastState = active; }
#endif
    void write(bool active);
    bool getLastState() const { return _lastState; }
    const TerraPinSetup &getSetup() const { return _setup; }

protected:
    TerraPinSetup _setup;                     ///< Pin configuration
    mutable bool _lastState;                  ///< Last logical pin state
};

class TerraAnalogPin {
public:
    TerraAnalogPin(uint8_t pin = TERRA_INVALID_PIN)
        : _pin(pin), _lastValue(0) { }

    void begin();
    int read();
    void write(int value);
#if !defined(ARDUINO)
    void setSimulatedValue(int value) { _lastValue = value; }
#endif
    int getLastValue() const { return _lastValue; }
    uint8_t getPin() const { return _pin; }

protected:
    uint8_t _pin;                             ///< MCU pin number
    int _lastValue;                           ///< Last raw analog/PWM value
};

#endif // /ifndef TerraPins_H
