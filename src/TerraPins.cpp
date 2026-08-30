/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Pins
*/

#include "Terraduino.h"

TerraPin *newPinObjectFromSubData(const TerraPinData *dataIn)
{
    if (!dataIn || !dataIn->isSet()) { return nullptr; }
    switch (dataIn->type) {
        case TerraPin::Digital: return new TerraDigitalPin(dataIn);
        case TerraPin::Analog: return new TerraAnalogPin(dataIn);
        default: break;
    }
    return nullptr;
}

TerraPin::TerraPin(int classType, uint8_t pinNumber, Terra_PinMode pinMode)
    : type((decltype(type))classType), pin(pinNumber), mode(pinMode)
{ ; }

TerraPin::TerraPin(const TerraPinData *dataIn)
    : type(dataIn ? (decltype(type))dataIn->type : Unknown),
      pin(dataIn ? dataIn->pin : TERRA_INVALID_PIN),
      mode(dataIn ? dataIn->mode : Terra_PinMode_Undefined)
{ ; }

void TerraPin::init()
{
#ifdef ARDUINO
    if (!isValid()) { return; }
    switch (mode) {
        case Terra_PinMode_Digital_Input: pinMode(pin, INPUT); break;
        case Terra_PinMode_Digital_Input_PullUp: pinMode(pin, INPUT_PULLUP); break;
        case Terra_PinMode_Digital_Input_PullDown:
#ifdef INPUT_PULLDOWN
            pinMode(pin, INPUT_PULLDOWN);
#else
            pinMode(pin, INPUT);
#endif
            break;
        case Terra_PinMode_Digital_Output: pinMode(pin, OUTPUT); break;
        case Terra_PinMode_Analog_Input: pinMode(pin, INPUT); break;
        case Terra_PinMode_Analog_Output: pinMode(pin, OUTPUT); break;
        default: break;
    }
#endif
}

void TerraPin::deinit()
{
#ifdef ARDUINO
    if (pin != TERRA_INVALID_PIN) { pinMode(pin, INPUT); }
#endif
}

void TerraPin::saveToData(TerraPinData *dataOut) const
{
    if (!dataOut) { return; }
    dataOut->type = type;
    dataOut->pin = pin;
    dataOut->mode = mode;
}

TerraDigitalPin::TerraDigitalPin(uint8_t pinNumber, Terra_PinMode pinMode, bool isActiveLow)
    : TerraPin(Digital, pinNumber, pinMode), activeLow(isActiveLow), _lastState(false)
{ ; }

TerraDigitalPin::TerraDigitalPin(const TerraPinData *dataIn)
    : TerraPin(dataIn), activeLow(dataIn ? dataIn->activeLow : false), _lastState(false)
{ ; }

void TerraDigitalPin::saveToData(TerraPinData *dataOut) const
{
    TerraPin::saveToData(dataOut);
    if (dataOut) { dataOut->activeLow = activeLow; }
}

bool TerraDigitalPin::digitalRead()
{
#ifdef ARDUINO
    if (canRead()) { _lastState = ::digitalRead(pin) != LOW; }
#endif
    return _lastState;
}

void TerraDigitalPin::digitalWrite(bool status)
{
    if (!canWrite()) { return; }
    _lastState = status;
#ifdef ARDUINO
    ::digitalWrite(pin, status ? HIGH : LOW);
#endif
}

TerraAnalogPin::TerraAnalogPin(uint8_t pinNumber, Terra_PinMode pinMode, uint8_t analogBitRes)
    : TerraPin(Analog, pinNumber, pinMode), bitRes(analogBitRes), _lastValue(0)
{ ; }

TerraAnalogPin::TerraAnalogPin(const TerraPinData *dataIn)
    : TerraPin(dataIn), bitRes(dataIn ? dataIn->bitRes : 10), _lastValue(0)
{ ; }

void TerraAnalogPin::saveToData(TerraPinData *dataOut) const
{
    TerraPin::saveToData(dataOut);
    if (dataOut) { dataOut->bitRes = bitRes; }
}

int TerraAnalogPin::analogRead_raw()
{
#ifdef ARDUINO
    if (canRead()) { _lastValue = ::analogRead(pin); }
#endif
    return _lastValue;
}

float TerraAnalogPin::analogRead()
{
    int maxRaw = maximumRaw();
    return maxRaw > 0 ? constrain((float)analogRead_raw() / (float)maxRaw, 0.0f, 1.0f) : 0.0f;
}

void TerraAnalogPin::analogWrite_raw(int amount)
{
    if (!canWrite()) { return; }
    _lastValue = constrain(amount, 0, maximumRaw());
#ifdef ARDUINO
    ::analogWrite(pin, _lastValue);
#endif
}

void TerraAnalogPin::analogWrite(float amount)
{
    analogWrite_raw((int)(constrain(amount, 0.0f, 1.0f) * maximumRaw() + 0.5f));
}

TerraPinData::TerraPinData()
    : TerraSubData(TerraPin::Unknown), pin(TERRA_INVALID_PIN),
      mode(Terra_PinMode_Undefined), activeLow(false), bitRes(10)
{ ; }

void TerraPinData::toJSONObject(JsonObject &objectOut) const
{
    TerraSubData::toJSONObject(objectOut);
    objectOut[SFP(TStr_Key_Pin)] = pin;
    objectOut[SFP(TStr_Key_Mode)] = terraPinModeToString(mode);
    if (activeLow) { objectOut[SFP(TStr_Key_ActiveLow)] = true; }
    if (type == TerraPin::Analog) { objectOut[SFP(TStr_Key_BitRes)] = bitRes; }
}

void TerraPinData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraSubData::fromJSONObject(objectIn);
    pin = objectIn[SFP(TStr_Key_Pin)] | pin;
    const char *modeString = objectIn[SFP(TStr_Key_Mode)] | nullptr;
    if (modeString) { mode = terraPinModeFromString(modeString); }
    activeLow = objectIn[SFP(TStr_Key_ActiveLow)] | activeLow;
    bitRes = objectIn[SFP(TStr_Key_BitRes)] | bitRes;
}
