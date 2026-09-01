/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Utilities
*/

#ifndef TerraUtils_HPP
#define TerraUtils_HPP

#include "Terraduino.h"

inline TerraSingleMeasurement &TerraSingleMeasurement::toUnits(Terra_UnitsType outUnits, float convertParam)
{
    convertUnits(&value, &units, outUnits, convertParam);
    return *this;
}

inline TerraSingleMeasurement &TerraSingleMeasurement::wrapBy(float range)
{
    value = ::wrapBy<float>(value, range);
    return *this;
}

inline TerraSingleMeasurement &TerraSingleMeasurement::wrapBySplit(float range)
{
    value = ::wrapBySplit<float>(value, range);
    return *this;
}

inline TerraSingleMeasurement TerraSingleMeasurement::asUnits(Terra_UnitsType outUnits, float convertParam) const
{
    TerraSingleMeasurement out(*this);
    convertUnits(&out, outUnits, convertParam);
    return out;
}

inline TerraSingleMeasurement TerraSingleMeasurement::wrappedBy(float range) const
{
    TerraSingleMeasurement out(*this);
    out.value = ::wrapBy<float>(out.value, range);
    return out;
}

inline TerraSingleMeasurement TerraSingleMeasurement::wrappedBySplit(float range) const
{
    TerraSingleMeasurement out(*this);
    out.value = ::wrapBySplit<float>(out.value, range);
    return out;
}


#ifdef TERRA_USE_MULTITASKING

template<typename ParameterType, int Slots>
taskid_t scheduleSignalFireOnce(SharedPtr<TerraObjInterface> object, Signal<ParameterType,Slots> &signal, ParameterType fireParam)
{
    SignalFireTask<ParameterType,Slots> *fireTask = object ? new SignalFireTask<ParameterType,Slots>(object, signal, fireParam) : nullptr;
    TERRA_SOFT_ASSERT(!object || fireTask, SFP(TStr_Err_AllocationFailure));
    taskid_t retVal = fireTask ? taskManager.scheduleOnce(0, fireTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (fireTask ? (fireTask->taskId = retVal) : retVal);
}

template<typename ParameterType, int Slots>
taskid_t scheduleSignalFireOnce(Signal<ParameterType,Slots> &signal, ParameterType fireParam)
{
    SignalFireTask<ParameterType,Slots> *fireTask = new SignalFireTask<ParameterType,Slots>(nullptr, signal, fireParam);
    TERRA_SOFT_ASSERT(fireTask, SFP(TStr_Err_AllocationFailure));
    taskid_t retVal = fireTask ? taskManager.scheduleOnce(0, fireTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (fireTask ? (fireTask->taskId = retVal) : retVal);
}

template<class ObjectType, typename ParameterType>
taskid_t scheduleObjectMethodCallOnce(SharedPtr<ObjectType> object, void (ObjectType::*method)(ParameterType), ParameterType callParam)
{
    MethodSlotCallTask<ObjectType,ParameterType> *callTask = object ? new MethodSlotCallTask<ObjectType,ParameterType>(object, method, callParam) : nullptr;
    TERRA_SOFT_ASSERT(!object || callTask, SFP(TStr_Err_AllocationFailure));
    taskid_t retVal = callTask ? taskManager.scheduleOnce(0, callTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (callTask ? (callTask->taskId = retVal) : retVal);
}

template<class ObjectType, typename ParameterType>
taskid_t scheduleObjectMethodCallOnce(ObjectType *object, void (ObjectType::*method)(ParameterType), ParameterType callParam)
{
    MethodSlotCallTask<ObjectType,ParameterType> *callTask = object ? new MethodSlotCallTask<ObjectType,ParameterType>(object, method, callParam) : nullptr;
    TERRA_SOFT_ASSERT(!object || callTask, SFP(TStr_Err_AllocationFailure));
    taskid_t retVal = callTask ? taskManager.scheduleOnce(0, callTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (callTask ? (callTask->taskId = retVal) : retVal);
}

template<class ObjectType>
taskid_t scheduleObjectMethodCallWithTaskIdOnce(SharedPtr<ObjectType> object, void (ObjectType::*method)(taskid_t))
{
    MethodSlotCallTask<ObjectType,taskid_t> *callTask = object ? new MethodSlotCallTask<ObjectType,taskid_t>(object, method, (taskid_t)0) : nullptr;
    TERRA_SOFT_ASSERT(!object || callTask, SFP(TStr_Err_AllocationFailure));
    taskid_t retVal = callTask ? taskManager.scheduleOnce(0, callTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (callTask ? (callTask->taskId = (callTask->_callParam = retVal)) : retVal);
}

template<class ObjectType>
taskid_t scheduleObjectMethodCallWithTaskIdOnce(ObjectType *object, void (ObjectType::*method)(taskid_t))
{
    MethodSlotCallTask<ObjectType,taskid_t> *callTask = object ? new MethodSlotCallTask<ObjectType,taskid_t>(object, method, (taskid_t)0) : nullptr;
    TERRA_SOFT_ASSERT(!object || callTask, SFP(TStr_Err_AllocationFailure));
    taskid_t retVal = callTask ? taskManager.scheduleOnce(0, callTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (callTask ? (callTask->taskId = (callTask->_callParam = retVal)) : retVal);
}


template<typename ParameterType, int Slots>
void SignalFireTask<ParameterType,Slots>::exec() 
{
    _signal->fire(_param);
}


template<class ObjectType, typename ParameterType>
void MethodSlotCallTask<ObjectType,ParameterType>::exec()
{
    _methodSlot(_callParam);
}

#endif // /ifdef TERRA_USE_MULTITASKING


template<typename T>
String commaStringFromArray(const T *arrayIn, size_t length)
{
    if (!arrayIn || !length) { return String(SFP(TStr_null)); }
    String retVal; retVal.reserve(length << 1 + length >> 1 + 1);
    for (size_t index = 0; index < length; ++index) {
        if (retVal.length()) { retVal.concat(','); }
        retVal += String(arrayIn[index]);
    }
    return retVal.length() ? retVal : String(SFP(TStr_null));
}

template<typename T>
void commaStringToArray(String stringIn, T *arrayOut, size_t length)
{
    if (!stringIn.length() || !length || stringIn.equalsIgnoreCase(SFP(TStr_null))) { return; }
    int lastSepPos = -1;
    for (size_t index = 0; index < length; ++index) {
        int nextSepPos = stringIn.indexOf(',', lastSepPos+1);
        if (nextSepPos == -1) { nextSepPos = stringIn.length(); }
        String subString = stringIn.substring(lastSepPos+1, nextSepPos);
        if (nextSepPos < stringIn.length()) { lastSepPos = nextSepPos; }

        arrayOut[index] = static_cast<T>(subString.toInt());
    }
}

template<typename T>
void commaStringToArray(JsonVariantConst &variantIn, T *arrayOut, size_t length)
{
    if (variantIn.isNull() || variantIn.is<JsonObjectConst>() || variantIn.is<JsonArrayConst>()) { return; }
    commaStringToArray<T>(variantIn.as<String>(), arrayOut, length);
}

template<typename T>
bool arrayElementsEqual(const T *arrayIn, size_t length, T value)
{
    for (size_t index = 0; index < length; ++index) {
        if (!(arrayIn[index] == value)) {
            return false;
        }
    }
    return true;
}


inline bool convertUnits(float *valueInOut, Terra_UnitsType *unitsInOut, Terra_UnitsType outUnits, float convertParam)
{
    if (tryConvertUnits(*valueInOut, *unitsInOut, valueInOut, outUnits, convertParam)) {
        *unitsInOut = outUnits;
        return true;
    }
    return false;
}

inline bool convertUnits(float valueIn, float *valueOut, Terra_UnitsType unitsIn, Terra_UnitsType outUnits, Terra_UnitsType *unitsOut, float convertParam)
{
    if (tryConvertUnits(valueIn, unitsIn, valueOut, outUnits, convertParam)) {
        if (unitsOut) { *unitsOut = outUnits; }
        return true;
    }
    return false;
}

inline bool convertUnits(TerraSingleMeasurement *measureInOut, Terra_UnitsType outUnits, float convertParam)
{
    return convertUnits(&measureInOut->value, &measureInOut->units, outUnits, convertParam);
}

inline bool convertUnits(const TerraSingleMeasurement *measureIn, TerraSingleMeasurement *measureOut, Terra_UnitsType outUnits, float convertParam)
{
    return convertUnits(measureIn->value, &measureOut->value, measureIn->units, outUnits, &measureOut->units, convertParam);
}

template<> inline float wrapBy(float value, float range)
{
    value = fmodf(value, range);
    return !signbit(value) ? value : value + range;
}

template<> inline double wrapBy(double value, double range)
{
    value = fmod(value, range);
    return !signbit(value) ? value : value + range;
}


template<size_t N = TERRA_DEFAULT_MAXSIZE>
Vector<TerraObject *, N> linksFilterActuators(Pair<uint8_t, Pair<TerraObject *, int8_t> *> links)
{
    Vector<TerraObject *, N> retVal;

    for (tposi_t linksIndex = 0; linksIndex < links.first && links.second[linksIndex].first; ++linksIndex) {
        if (links.second[linksIndex].first->isActuatorType()) {
            retVal.push_back(links.second[linksIndex].first);
        }
    }

    return retVal;
}

template<size_t N = TERRA_DEFAULT_MAXSIZE>
void linksResolveActuatorsByType(Vector<TerraObject *, N> &actuatorsIn, Vector<TerraActuatorAttachment, N> &activationsOut, Terra_ActuatorType actuatorType)
{
    for (auto actIter = actuatorsIn.begin(); actIter != actuatorsIn.end(); ++actIter) {
        auto actuator = getSharedPtr<TerraActuator>(*actIter);
        TERRA_HARD_ASSERT(actuator, SFP(TStr_Err_OperationFailure));
        if (actuator->getActuatorType() == actuatorType) {
            activationsOut.push_back(TerraActuatorAttachment());
            activationsOut.back().setObject(actuator);
        }
    }
}

template<size_t N = TERRA_DEFAULT_MAXSIZE>
void linksResolveActuatorsToAttachments(Vector<TerraObject *, N> &actuatorsIn, TerraObjInterface *parent, tposi_t subIndex, Vector<TerraActuatorAttachment, N> &activationsOut)
{
    for (auto actIter = actuatorsIn.begin(); actIter != actuatorsIn.end(); ++actIter) {
        auto actuator = getSharedPtr<TerraActuator>(*actIter);
        TERRA_HARD_ASSERT(actuator, SFP(TStr_Err_OperationFailure));

        activationsOut.push_back(TerraActuatorAttachment());
        activationsOut.back().setParent(parent, subIndex);
        activationsOut.back().setObject(actuator);
    }
}


inline Terraduino *getController()
{
    return Terraduino::_activeInstance;
}

inline TerraScheduler *getScheduler()
{
    return Terraduino::_activeInstance ? &Terraduino::_activeInstance->scheduler : nullptr;
}

inline TerraLogger *getLogger()
{
    return Terraduino::_activeInstance ? &Terraduino::_activeInstance->logger : nullptr;
}

inline TerraPublisher *getPublisher()
{
    return Terraduino::_activeInstance ? &Terraduino::_activeInstance->publisher : nullptr;
}

#ifdef TERRA_USE_GUI

inline TerraUIInterface *getUI()
{
    return Terraduino::_activeInstance ? Terraduino::_activeInstance->_activeUIInstance : nullptr;
}

#endif


inline time_t unixTime(DateTime localTime)
{
    return localTime.unixtime() - (getController() ? getController()->getTimeZoneOffset() : 0);
}

inline DateTime localTime(time_t unixTime)
{
    return DateTime((uint32_t)(unixTime + (getController() ? getController()->getTimeZoneOffset() : 0)));
}

inline time_t unixDayStart(time_t unixTime)
{
    DateTime currTime = DateTime((uint32_t)unixTime);
    return DateTime(currTime.year(), currTime.month(), currTime.day()).unixtime();
}

inline DateTime localDayStart(time_t unixTime)
{
    DateTime currTime = localTime(unixTime);
    return DateTime(currTime.year(), currTime.month(), currTime.day());
}

extern void _setUnixTime(DateTime unixTime, bool isSigTime);

inline void setUnixTime(time_t unixTime, bool isSigTime)
{
    _setUnixTime(DateTime((uint32_t)unixTime), isSigTime);
}

inline void setLocalTime(DateTime localTime, bool isSigTime)
{
    _setUnixTime(DateTime((uint32_t)unixTime(localTime)), isSigTime);
}


inline bool checkPinIsDigital(pintype_t pin)
{
    #ifdef ESP32
        return true; // all digital pins are ADC capable
    #else // separate analog from digital pins
        return !checkPinIsAnalogInput(pin) && !checkPinIsAnalogOutput(pin);
    #endif
}

inline bool checkPinIsPWMOutput(pintype_t pin)
{
    #if defined(digitalPinHasPWM)
        return digitalPinHasPWM(pin);
    #else
        return checkPinIsDigital(pin); // all digital pins are PWM capable
    #endif
}

inline bool checkPinCanInterrupt(pintype_t pin)
{
    if (pin >= tpin_virtual) {
        #ifdef TERRA_USE_MULTITASKING
            return getController() && getController()->getPinExpander(expanderPosForPinNumber(pin)) &&
                   getController()->getPinExpander(expanderPosForPinNumber(pin))->getInterruptPin().isValid() &&
                   isValidPin(digitalPinToInterrupt(getController()->getPinExpander(expanderPosForPinNumber(pin))->getInterruptPin().pin));
        #else
            return false;
        #endif
    }
    return (getController() && getController()->getPinMuxer(pin) &&
            getController()->getPinMuxer(pin)->getInterruptPin().isValid() &&
            isValidPin(digitalPinToInterrupt(getController()->getPinMuxer(pin)->getInterruptPin().pin)))
           || isValidPin(digitalPinToInterrupt(pin));
}

#endif // /ifndef TerraUtils_HPP
