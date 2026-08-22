/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Pins
*/

#ifndef TerraPins_H
#define TerraPins_H

struct TerraPin;
struct TerraDigitalPin;
struct TerraAnalogPin;
struct TerraPinData;

#include "TerraData.h"
#include "TerraInterfaces.h"

// Pin Base
struct TerraPin {
    enum : signed char { Digital, Analog, Unknown = -1 } type; // Pin type

    uint8_t pin;                                            // MCU pin number
    Terra_PinMode mode;                                     // Pin operating mode

    TerraPin(int classType = Unknown,
             uint8_t pinNumber = TERRA_INVALID_PIN,
             Terra_PinMode pinMode = Terra_PinMode_Undefined);
    TerraPin(const TerraPinData *dataIn);

    void init();
    void deinit();
    void saveToData(TerraPinData *dataOut) const;

    inline bool isValid() const { return pin != TERRA_INVALID_PIN && mode != Terra_PinMode_Undefined; }
    inline bool isInput() const { return mode == Terra_PinMode_Digital_Input ||
                                         mode == Terra_PinMode_Digital_Input_PullUp ||
                                         mode == Terra_PinMode_Digital_Input_PullDown ||
                                         mode == Terra_PinMode_Analog_Input; }
    inline bool isOutput() const { return mode == Terra_PinMode_Digital_Output ||
                                          mode == Terra_PinMode_Analog_Output; }
    inline bool canRead() const { return isValid() && isInput(); }
    inline bool canWrite() const { return isValid() && isOutput(); }
    inline bool isDigital() const { return mode == Terra_PinMode_Digital_Input ||
                                           mode == Terra_PinMode_Digital_Input_PullUp ||
                                           mode == Terra_PinMode_Digital_Input_PullDown ||
                                           mode == Terra_PinMode_Digital_Output; }
    inline bool isAnalog() const { return mode == Terra_PinMode_Analog_Input ||
                                          mode == Terra_PinMode_Analog_Output; }
};

// Digital Pin
struct TerraDigitalPin : public TerraPin {
    bool activeLow;                                         // Active-low trigger state

    TerraDigitalPin(uint8_t pinNumber = TERRA_INVALID_PIN,
                    Terra_PinMode pinMode = Terra_PinMode_Undefined,
                    bool isActiveLow = false);
    TerraDigitalPin(const TerraPinData *dataIn);

    void saveToData(TerraPinData *dataOut) const;

    bool digitalRead();
    void digitalWrite(bool status);
    inline bool isActive() { return digitalRead() == !activeLow; }
    inline void activate() { digitalWrite(!activeLow); }
    inline void deactivate() { digitalWrite(activeLow); }

#ifndef ARDUINO
    inline void setSimulatedState(bool active) { _lastState = activeLow ? !active : active; }
#endif

protected:
    bool _lastState;                                        // Last electrical pin state
};

// Analog Pin
struct TerraAnalogPin : public TerraPin {
    uint8_t bitRes;                                         // Analog bit resolution

    TerraAnalogPin(uint8_t pinNumber = TERRA_INVALID_PIN,
                   Terra_PinMode pinMode = Terra_PinMode_Undefined,
                   uint8_t analogBitRes = 10);
    TerraAnalogPin(const TerraPinData *dataIn);

    void saveToData(TerraPinData *dataOut) const;

    float analogRead();
    int analogRead_raw();
    void analogWrite(float amount);
    void analogWrite_raw(int amount);

#ifndef ARDUINO
    inline void setSimulatedValue(int value) { _lastValue = value; }
#endif

protected:
    int _lastValue;                                         // Last raw analog value
    inline int maximumRaw() const { return bitRes && bitRes < 16 ? (1 << bitRes) - 1 : 1023; }
};

// Pin Serialization Data
struct TerraPinData : public TerraSubData {
    uint8_t pin;                                            // MCU pin number
    Terra_PinMode mode;                                     // Pin operating mode
    bool activeLow;                                         // Active-low trigger state
    uint8_t bitRes;                                         // Analog bit resolution

    TerraPinData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

extern TerraPin *newPinObjectFromSubData(const TerraPinData *dataIn);

#endif // /ifndef TerraPins_H
