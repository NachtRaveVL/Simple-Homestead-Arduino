/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Pins
*/

#include "Astruino.h"
#include "AnalogDeviceAbstraction.h"

TerraPin *newPinObjectFromSubData(const TerraPinData *dataIn)
{
    if (!dataIn || !isValidType(dataIn->type)) return nullptr;
    TERRA_SOFT_ASSERT(dataIn && isValidType(dataIn->type), SFP(TStr_Err_InvalidParameter));

    if (dataIn) {
        switch (dataIn->type) {
            case TerraPin::Digital:
                return new TerraDigitalPin(dataIn);
            case TerraPin::Analog:
                return new TerraAnalogPin(dataIn);
            default: break;
        }
    }

    return nullptr;
}

TerraPin::TerraPin()
    : type(Unknown), pin(tpin_none), mode(Terra_PinMode_Undefined), channel(tpinchnl_none)
{ ; }

TerraPin::TerraPin(int classType, pintype_t pinNumber, Terra_PinMode pinMode, int8_t pinChannel)
    : type((typeof(type))classType), pin(pinNumber), mode(pinMode),
      channel(isValidChannel(pinChannel) ? pinChannel : (isValidPin(pinNumber) && pinNumber >= tpin_virtual ? pinChannelForExpanderChannel(pinNumber - tpin_virtual) : tpinchnl_none))
{ ; }

TerraPin::TerraPin(const TerraPinData *dataIn)
    : type((typeof(type))(dataIn->type)), pin(dataIn->pin), mode(dataIn->mode), channel(dataIn->channel)
{ ; }

TerraPin::operator TerraDigitalPin() const
{
    return (isDigitalType() || isDigital() || (!isUnknownType() && !isAnalog())) ? TerraDigitalPin(pin, mode, channel) : TerraDigitalPin();
}

TerraPin::operator TerraAnalogPin() const
{
    return (isAnalogType() || isAnalog() || (!isUnknownType() && !isDigital())) ? TerraAnalogPin(pin, mode, isOutput() ? DAC_RESOLUTION : ADC_RESOLUTION, channel) : TerraAnalogPin();
}

void TerraPin::saveToData(TerraPinData *dataOut) const
{
    dataOut->type = (int8_t)type;
    dataOut->pin = pin;
    dataOut->mode = mode;
    dataOut->channel = channel;
}

void TerraPin::init()
{
    #if !TERRA_SYS_DRY_RUN_ENABLE
        if (isValid()) {
            if (!(isExpanded() || isVirtual())) {
                TERRA_SOFT_ASSERT(!isMuxed() || channel == pinChannelForMuxerChannel(muxerChannelForPinChannel(channel)), SFP(TStr_Err_NotConfiguredProperly));

                switch (mode) {
                    case Terra_PinMode_Digital_Input:
                    case Terra_PinMode_Analog_Input:
                        pinMode(pin, INPUT);
                        break;

                    case Terra_PinMode_Digital_Input_PullUp:
                        pinMode(pin, INPUT_PULLUP);
                        break;

                    case Terra_PinMode_Digital_Input_PullDown:
                        #if HAS_INPUT_PULLDOWN
                            pinMode(pin, INPUT_PULLDOWN);
                        #else
                            pinMode(pin, INPUT);
                        #endif
                        break;

                    case Terra_PinMode_Digital_Output:
                    case Terra_PinMode_Digital_Output_PushPull:
                    case Terra_PinMode_Analog_Output:
                        pinMode(pin, OUTPUT);
                        break;

                    default:
                        break;
                }
            } else {
                #ifdef TERRA_USE_MULTITASKING
                    TERRA_SOFT_ASSERT(isVirtual() && pin == pinNumberForPinChannel(channel), SFP(TStr_Err_NotConfiguredProperly));
                    TERRA_SOFT_ASSERT(channel == pinChannelForExpanderChannel(channel), SFP(TStr_Err_NotConfiguredProperly));

                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderPosForPinChannel(channel) : expanderPosForPinNumber(pin)) : nullptr;
                    if (expander) {
                        #if HAS_INPUT_PULLDOWN
                            expander->getIoAbstraction()->pinDirection(channel % 16, isOutput() ? OUTPUT : mode == Terra_PinMode_Digital_Input_PullUp ? INPUT_PULLUP : mode == Terra_PinMode_Digital_Input_PullDown ? INPUT_PULLDOWN : INPUT);
                        #else
                            expander->getIoAbstraction()->pinDirection(channel % 16, isOutput() ? OUTPUT : mode == Terra_PinMode_Digital_Input_PullUp ? INPUT_PULLUP : INPUT);
                        #endif
                    }
                #else
                    TERRA_HARD_ASSERT(false, SFP(TStr_Err_NotConfiguredProperly));
                #endif
            }
        }
    #endif
}

void TerraPin::deinit()
{
    #if !TERRA_SYS_DRY_RUN_ENABLE
        if (isValid()) {
            if (!(isExpanded() || isVirtual())) {
                pinMode(pin, INPUT);
            } else {
                #ifdef TERRA_USE_MULTITASKING
                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderPosForPinChannel(channel) : expanderPosForPinNumber(pin)) : nullptr;
                    if (expander) {
                        expander->getIoAbstraction()->pinDirection(channel % 16, INPUT);
                    }
                #else
                    TERRA_HARD_ASSERT(false, SFP(TStr_Err_NotConfiguredProperly));
                #endif
            }
        }
    #endif
}

bool TerraPin::enablePin(int step)
{
    #if !TERRA_SYS_DRY_RUN_ENABLE
        if (isValid() && isValidChannel(channel)) {
            if (isMuxed()) {
                SharedPtr<TerraPinMuxer> muxer = getController() ? getController()->getPinMuxer(pin) : nullptr;
                if (muxer) {
                    switch (step) {
                        case 0: muxer->selectChannel(muxerChannelForPinChannel(channel)); muxer->activate(); return true;
                        case 1: muxer->selectChannel(muxerChannelForPinChannel(channel)); return true;
                        case 2: muxer->activate(); return true;
                        default: return false;
                    }
                }
            } else if (isExpanded() || isVirtual()) {
                #ifdef TERRA_USE_MULTITASKING
                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderPosForPinChannel(channel) : expanderPosForPinNumber(pin)) : nullptr;
                    return expander && expander->trySyncChannel();
                #else
                    TERRA_HARD_ASSERT(false, SFP(TStr_Err_NotConfiguredProperly));
                #endif
            }
        }
        return false;
    #else
        return isValid() && isValidChannel(channel);
    #endif
}


TerraDigitalPin::TerraDigitalPin()
    : TerraPin(Digital), activeLow(false)
{ ; }

TerraDigitalPin::TerraDigitalPin(pintype_t pinNumber, ard_pinmode_t pinMode, int8_t pinChannel)
    : TerraPin(Digital, pinNumber, pinMode != OUTPUT ? (pinMode != INPUT ? (pinMode == INPUT_PULLUP ? Terra_PinMode_Digital_Input_PullUp : Terra_PinMode_Digital_Input_PullDown)
                                                                         : Terra_PinMode_Digital_Input)
                                                     : (pinMode == OUTPUT ? Terra_PinMode_Digital_Output : Terra_PinMode_Digital_Output_PushPull), pinChannel),
      activeLow(pinMode == INPUT || pinMode == INPUT_PULLUP || pinMode == OUTPUT)
{ ; }

TerraDigitalPin::TerraDigitalPin(pintype_t pinNumber, Terra_PinMode pinMode, int8_t pinChannel)
    : TerraPin(Digital, pinNumber, pinMode, pinChannel),
      activeLow(pinMode == Terra_PinMode_Digital_Input ||
                pinMode == Terra_PinMode_Digital_Input_PullUp ||
                pinMode == Terra_PinMode_Digital_Output)
{ ; }

TerraDigitalPin::TerraDigitalPin(pintype_t pinNumber, ard_pinmode_t pinMode, bool isActiveLow, int8_t pinChannel)
    : TerraPin(Digital, pinNumber, pinMode != OUTPUT ? (isActiveLow ? Terra_PinMode_Digital_Input_PullUp : Terra_PinMode_Digital_Input_PullDown)
                                                     : (isActiveLow ? Terra_PinMode_Digital_Output : Terra_PinMode_Digital_Output_PushPull), pinChannel),
      activeLow(isActiveLow)
{ ; }

TerraDigitalPin::TerraDigitalPin(pintype_t pinNumber, Terra_PinMode pinMode, bool isActiveLow, int8_t pinChannel)
    : TerraPin(Digital, pinNumber, pinMode, pinChannel),
      activeLow(isActiveLow)
{ ; }

TerraDigitalPin::TerraDigitalPin(const TerraPinData *dataIn)
    : TerraPin(dataIn), activeLow(dataIn->dataAs.digitalPin.activeLow)
{ ; }

void TerraDigitalPin::saveToData(TerraPinData *dataOut) const
{
    TerraPin::saveToData(dataOut);

    dataOut->dataAs.digitalPin.activeLow = activeLow;
}

ard_pinstatus_t TerraDigitalPin::digitalRead()
{
    #if !TERRA_SYS_DRY_RUN_ENABLE
        if (isValid()) {
            if (isValidChannel(channel)) { selectAndActivatePin(); }
            if (!(isExpanded() || isVirtual())) {
                return ::digitalRead(pin);
            } else {
                #ifdef TERRA_USE_MULTITASKING
                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderPosForPinChannel(channel) : expanderPosForPinNumber(pin)) : nullptr;
                    if (expander) {
                        return (ard_pinstatus_t)(expander->getIoAbstraction()->readValue(channel % 16));
                    }
                #else
                    TERRA_HARD_ASSERT(false, SFP(TStr_Err_NotConfiguredProperly));
                #endif
            }
        }
    #endif
    return (ard_pinstatus_t)-1;
}

void TerraDigitalPin::digitalWrite(ard_pinstatus_t status)
{
    #if !TERRA_SYS_DRY_RUN_ENABLE
        if (isValid()) {
            if (!(isExpanded() || isVirtual())) {
                if (isMuxed()) { selectPin(); }
                ::digitalWrite(pin, status);
            } else {
                #ifdef TERRA_USE_MULTITASKING
                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderPosForPinChannel(channel) : expanderPosForPinNumber(pin)) : nullptr;
                    if (expander) {
                        expander->getIoAbstraction()->writeValue(channel % 16, (uint8_t)status);
                    }
                #else
                    TERRA_HARD_ASSERT(false, SFP(TStr_Err_NotConfiguredProperly));
                #endif
            }
            if (isValidChannel(channel)) { activatePin(); }
        }
    #endif
}


TerraAnalogPin::TerraAnalogPin()
    : TerraPin(Analog), bitRes(0)
#ifdef ESP32
      , pwmChannel(-1)
#endif
#ifdef ESP_PLATFORM
      , pwmFrequency(0)
#endif
{ ; }

TerraAnalogPin::TerraAnalogPin(pintype_t pinNumber, ard_pinmode_t pinMode, uint8_t analogBitRes,
#ifdef ESP32
                               uint8_t pinPWMChannel,
#endif
#ifdef ESP_PLATFORM
                               float pinPWMFrequency,
#endif
                               int8_t pinChannel)
    : TerraPin(Analog, pinNumber, pinMode != OUTPUT ? Terra_PinMode_Analog_Input : Terra_PinMode_Analog_Output, pinChannel),
      bitRes(analogBitRes ? analogBitRes : (pinMode == OUTPUT ? DAC_RESOLUTION : ADC_RESOLUTION))
#ifdef ESP32
      , pwmChannel(pinPWMChannel)
#endif
#ifdef ESP_PLATFORM
      , pwmFrequency(pinPWMFrequency)
#endif
{ ; }

TerraAnalogPin::TerraAnalogPin(pintype_t pinNumber, Terra_PinMode pinMode, uint8_t analogBitRes,
#ifdef ESP32
                               uint8_t pinPWMChannel,
#endif
#ifdef ESP_PLATFORM
                               float pinPWMFrequency,
#endif
                               int8_t pinChannel)
    : TerraPin(Analog, pinNumber, pinMode, pinChannel),
      bitRes(analogBitRes ? analogBitRes : (pinMode == Terra_PinMode_Analog_Output ? DAC_RESOLUTION : ADC_RESOLUTION))
#ifdef ESP32
      , pwmChannel(pinPWMChannel)
#endif
#ifdef ESP_PLATFORM
      , pwmFrequency(pinPWMFrequency)
#endif
{ ; }

TerraAnalogPin::TerraAnalogPin(const TerraPinData *dataIn)
    : TerraPin(dataIn), bitRes(dataIn->dataAs.analogPin.bitRes)
#ifdef ESP32
      , pwmChannel(dataIn->dataAs.analogPin.pwmChannel)
#endif
#ifdef ESP_PLATFORM
      , pwmFrequency(dataIn->dataAs.analogPin.pwmFrequency)
#endif
{ ; }

void TerraAnalogPin::init()
{
    #if !TERRA_SYS_DRY_RUN_ENABLE
        if (isValid()) {
            if (!(isExpanded() || isVirtual())) {
                TerraPin::init();

                #ifdef ESP32
                    ledcAttachPin(pin, pwmChannel);
                    ledcSetup(pwmChannel, pwmFrequency, bitRes.bits);
                #endif
            } else {
                #ifdef TERRA_USE_MULTITASKING
                    TERRA_SOFT_ASSERT(isVirtual() && pin == pinNumberForPinChannel(channel), SFP(TStr_Err_NotConfiguredProperly));
                    TERRA_SOFT_ASSERT(channel == pinChannelForExpanderChannel(channel), SFP(TStr_Err_NotConfiguredProperly));

                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderPosForPinChannel(channel) : expanderPosForPinNumber(pin)) : nullptr;
                    if (expander) {
                        auto ioDir = isOutput() ? AnalogDirection::DIR_OUT : AnalogDirection::DIR_IN;
                        auto analogIORef = (AnalogDevice *)(expander->getIoAbstraction());
                        analogIORef->initPin(channel % 16, ioDir);

                        auto ioRefBits = analogIORef->getBitDepth(ioDir, channel % 16);
                        if (bitRes.bits != ioRefBits) {
                            bitRes = BitResolution(ioRefBits);
                        }
                    }
                #else
                    TERRA_HARD_ASSERT(false, SFP(TStr_Err_NotConfiguredProperly));
                #endif
            }
        }
    #endif
}

void TerraAnalogPin::saveToData(TerraPinData *dataOut) const
{
    TerraPin::saveToData(dataOut);

    dataOut->dataAs.analogPin.bitRes = bitRes.bits;
    #ifdef ESP32
        dataOut->dataAs.analogPin.pwmChannel = pwmChannel;
    #endif
    #ifdef ESP_PLATFORM
        dataOut->dataAs.analogPin.pwmFrequency = pwmFrequency;
    #endif
}

float TerraAnalogPin::analogRead()
{
    return bitRes.transform(analogRead_raw());
}

int TerraAnalogPin::analogRead_raw()
{
    #if !TERRA_SYS_DRY_RUN_ENABLE
        if (isValid()) {
            if (isValidChannel(channel)) { selectAndActivatePin(); }
            if (!(isExpanded() || isVirtual())) {
                #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
                    analogReadResolution(bitRes.bits);
                #endif
                return ::analogRead(pin);
            } else {
                #ifdef TERRA_USE_MULTITASKING
                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderPosForPinChannel(channel) : expanderPosForPinNumber(pin)) : nullptr;
                    if (expander) {
                        auto analogIORef = (AnalogDevice *)(expander->getIoAbstraction());
                        analogIORef->getCurrentValue(channel % 16);
                    }
                #else
                    TERRA_HARD_ASSERT(false, SFP(TStr_Err_NotConfiguredProperly));
                #endif
            }
        }
    #endif
    return 0;
}

void TerraAnalogPin::analogWrite(float amount)
{
    analogWrite_raw(bitRes.inverseTransform(amount));
}

void TerraAnalogPin::analogWrite_raw(int amount)
{
    #if !TERRA_SYS_DRY_RUN_ENABLE
        if (isValid()) {
            if (!(isExpanded() || isVirtual())) {
                if (isMuxed()) { selectPin(); }
                #ifdef ESP32
                    ledcWrite(pwmChannel, amount);
                #else
                    #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
                        analogWriteResolution(bitRes.bits);
                    #elif defined(ESP8266)
                        analogWriteRange(bitRes.maxVal);
                        analogWriteFreq(pwmFrequency);
                    #endif
                    ::analogWrite(pin, amount);
                #endif
            } else {
                #ifdef TERRA_USE_MULTITASKING
                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderPosForPinChannel(channel) : expanderPosForPinNumber(pin)) : nullptr;
                    if (expander) {
                        auto analogIORef = (AnalogDevice *)(expander->getIoAbstraction());
                        analogIORef->setCurrentValue(channel % 16, amount);
                    }
                #else
                    TERRA_HARD_ASSERT(false, SFP(TStr_Err_NotConfiguredProperly));
                #endif
            }
            if (isValidChannel(channel)) { activatePin(); }
        }
    #endif
}


TerraPinData::TerraPinData()
    : TerraSubData((int8_t)TerraPin::Unknown), pin(tpin_none), mode(Terra_PinMode_Undefined), channel(tpinchnl_none), dataAs{0}
{ ; }

void TerraPinData::toJSONObject(JsonObject &objectOut) const
{
    TerraSubData::toJSONObject(objectOut);

    if (isValidPin(pin)) { objectOut[SFP(TStr_Key_Pin)] = pin; }
    if (mode != Terra_PinMode_Undefined) { objectOut[SFP(TStr_Key_Mode)] = pinModeToString(mode); }
    if (isValidChannel(channel)) { objectOut[SFP(TStr_Key_Channel)] = channel; }

    if (mode != Terra_PinMode_Undefined) {
        if (!(mode == Terra_PinMode_Analog_Input || mode == Terra_PinMode_Analog_Output)) {
            objectOut[SFP(TStr_Key_ActiveLow)] = dataAs.digitalPin.activeLow;
        } else {
            objectOut[SFP(TStr_Key_BitRes)] = dataAs.analogPin.bitRes;
            #ifdef ESP32
                objectOut[SFP(TStr_Key_PWMChannel)] = dataAs.analogPin.pwmChannel;
            #endif
            #ifdef ESP_PLATFORM
                objectOut[SFP(TStr_Key_PWMFrequency)] = dataAs.analogPin.pwmFrequency;
            #endif
        }
    }
}

void TerraPinData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraSubData::fromJSONObject(objectIn);

    pin = objectIn[SFP(TStr_Key_Pin)] | pin;
    mode = pinModeFromString(objectIn[SFP(TStr_Key_Mode)]);
    channel = objectIn[SFP(TStr_Key_Channel)] | channel;

    if (mode != Terra_PinMode_Undefined) {
        if (!(mode == Terra_PinMode_Analog_Input || mode == Terra_PinMode_Analog_Output)) {
            type = (int8_t)TerraPin::Digital;
            dataAs.digitalPin.activeLow = objectIn[SFP(TStr_Key_ActiveLow)] | dataAs.digitalPin.activeLow;
        } else {
            type = (int8_t)TerraPin::Analog;
            dataAs.analogPin.bitRes = objectIn[SFP(TStr_Key_BitRes)] | dataAs.analogPin.bitRes;
            #ifdef ESP32
                dataAs.analogPin.pwmChannel = objectIn[SFP(TStr_Key_PWMChannel)] | dataAs.analogPin.pwmChannel;
            #endif
            #ifdef ESP_PLATFORM
                dataAs.analogPin.pwmFrequency = objectIn[SFP(TStr_Key_PWMFrequency)] | dataAs.analogPin.pwmFrequency;
            #endif
        }
    } else {
        type = (int8_t)TerraPin::Unknown;
    }
}


TerraPinMuxer::TerraPinMuxer()
    : _signal(), _chipEnable(), _channelPins{tpin_none},
      _channelBits(0), _channelSelect(-1), _usingISR(false)
{
    _signal.channel = tpinchnl_none; // unused
}

TerraPinMuxer::TerraPinMuxer(TerraPin signalPin,
                             pintype_t *muxChannelPins, int8_t muxChannelBits,
                             TerraDigitalPin chipEnablePin, TerraDigitalPin interruptPin)
    : _signal(signalPin), _chipEnable(chipEnablePin), _interrupt(interruptPin),
      _channelPins{ muxChannelBits > 0 ? muxChannelPins[0] : tpin_none,
                    muxChannelBits > 1 ? muxChannelPins[1] : tpin_none,
                    muxChannelBits > 2 ? muxChannelPins[2] : tpin_none,
                    muxChannelBits > 3 ? muxChannelPins[3] : tpin_none },
      _channelBits(muxChannelBits), _channelSelect(-1), _usingISR(false)
{
    _signal.channel = tpinchnl_none; // unused
}

void TerraPinMuxer::init()
{
    _signal.deinit();
    _chipEnable.init();
    _chipEnable.deactivate();

    if (isValidPin(_channelPins[0])) {
        pinMode(_channelPins[0], OUTPUT);
        ::digitalWrite(_channelPins[0], LOW);

        if (isValidPin(_channelPins[1])) {
            pinMode(_channelPins[1], OUTPUT);
            ::digitalWrite(_channelPins[1], LOW);

            if (isValidPin(_channelPins[2])) {
                pinMode(_channelPins[2], OUTPUT);
                ::digitalWrite(_channelPins[2], LOW);

                if (isValidPin(_channelPins[3])) {
                    pinMode(_channelPins[3], OUTPUT);
                    ::digitalWrite(_channelPins[3], LOW);
                }
            }
        }
    }
    _channelSelect = 0;
}

bool TerraPinMuxer::tryRegisterISR(bool anyChange)
{
    #ifdef TERRA_USE_MULTITASKING
        if (!_usingISR && _interrupt.isValid() && checkPinCanInterrupt(_interrupt.pin)) {
            taskManager.addInterrupt(&interruptImpl, _interrupt.pin, !anyChange ? (_interrupt.activeLow ? FALLING : RISING) : CHANGE);
            _usingISR = true;
        }
    #endif
    return _usingISR;
}

void TerraPinMuxer::selectChannel(uint8_t channelNumber)
{
    if (_channelSelect != channelNumber) {
        #if TERRA_MUXERS_SHARED_ADDR_BUS
            if (getController()) { getController()->deactivatePinMuxers(); }
        #endif

        if (isValidPin(_channelPins[0])) {
            ::digitalWrite(_channelPins[0], (channelNumber >> 0) & 1 ? HIGH : LOW);

            if (isValidPin(_channelPins[1])) {
                ::digitalWrite(_channelPins[1], (channelNumber >> 1) & 1 ? HIGH : LOW);

                if (isValidPin(_channelPins[2])) {
                    ::digitalWrite(_channelPins[2], (channelNumber >> 2) & 1 ? HIGH : LOW);

                    if (isValidPin(_channelPins[3])) {
                        ::digitalWrite(_channelPins[3], (channelNumber >> 3) & 1 ? HIGH : LOW);
                    }
                }
            }
        }
        _channelSelect = channelNumber;
    }
}

void TerraPinMuxer::setIsActive(bool isActive)
{
    if (isActive) {
        _signal.init();
        _chipEnable.activate();
    } else {
        _chipEnable.deactivate();
        _signal.deinit();
    }
}

#ifdef TERRA_USE_MULTITASKING

TerraPinExpander::TerraPinExpander()
    : _expander(0), _channelBits(0), _ioRef(nullptr), _interrupt(), _usingISR(false)
{ ; }

TerraPinExpander::TerraPinExpander(tposi_t expanderPos, uint8_t channelBits, IoAbstractionRef ioRef, TerraDigitalPin interruptPin)
    : _expander(expanderPos), _channelBits(channelBits), _ioRef(ioRef), _interrupt(interruptPin), _usingISR(false)
{ ; }

bool TerraPinExpander::tryRegisterISR(bool anyChange)
{
    #ifdef TERRA_USE_MULTITASKING
        if (!_usingISR && _interrupt.isValid() && checkPinCanInterrupt(_interrupt.pin)) {
            taskManager.addInterrupt(&interruptImpl, _interrupt.pin, !anyChange ? (_interrupt.activeLow ? FALLING : RISING) : CHANGE);
            _usingISR = true;
        }
    #endif
    return _usingISR;
}

bool TerraPinExpander::trySyncChannel()
{
    return _ioRef->sync();
}

#endif // /ifdef TERRA_USE_MULTITASKING
