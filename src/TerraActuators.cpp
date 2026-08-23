/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Actuators
*/

#include "Terraduino.h"
#include "TerraSensors.h"
#include <string.h>

TerraActuator::TerraActuator(Terra_ActuatorType actuatorType, tposi_t actuatorIndex, int classTypeIn)
    : TerraObject(TerraIdentity(actuatorType, actuatorIndex)), classType(static_cast<decltype(Relay)>(classTypeIn)),
      _enabled(false), _needsUpdate(false), _enableMode(Terra_EnableMode_Highest),
      _handles(), _directActivation()
{ ; }

TerraActuator::TerraActuator(const TerraActuatorData *dataIn)
    : TerraObject((const TerraObjectData *)dataIn),
      classType(static_cast<decltype(Relay)>(dataIn ? (int)dataIn->id.object.classType : (int)Unknown)),
      _enabled(false), _needsUpdate(false),
      _enableMode(dataIn ? dataIn->enableMode : Terra_EnableMode_Highest),
      _handles(), _directActivation()
{ ; }

TerraActuator::~TerraActuator()
{
    _directActivation.unset();
    while (_handles.size()) {
        TerraActivationHandle *handle = _handles.front();
        if (handle) { handle->unset(); }
        else { _handles.erase(_handles.begin()); }
    }
}

void TerraActuator::setEnableMode(Terra_EnableMode mode)
{
    if (_enableMode != mode) {
        _enableMode = mode;
        setNeedsUpdate();
        bumpRevisionIfNeeded();
    }
}

void TerraActuator::update(uint32_t now)
{
    bool forced = false;
    if (_handles.size()) {
        for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
            if (_enabled && (*handleIter)->isActive()) {
                (*handleIter)->elapseTo(now);
            }
            if ((*handleIter)->actuator.get() != this || !(*handleIter)->isValid() || (*handleIter)->isDone()) {
                if ((*handleIter)->actuator.get() == this) { (*handleIter)->actuator = nullptr; }
                handleIter = _handles.erase(handleIter) - 1;
                setNeedsUpdate();
                continue;
            }
            forced = forced || (*handleIter)->isForced();
        }
    }

    bool canEnable = _handles.size() && (forced || getCanEnable());

    if (!canEnable && (_enabled || _needsUpdate)) {
        _disableActuator();
    } else if (canEnable && (!_enabled || _needsUpdate)) {
        float drivingIntensity = 0.0f;

        switch (_enableMode) {
            case Terra_EnableMode_Highest: {
                drivingIntensity = -__FLT_MAX__;
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    if ((*handleIter)->isValid() && !(*handleIter)->isDone()) {
                        auto handleIntensity = (*handleIter)->getDriveIntensity();
                        if (handleIntensity > drivingIntensity) { drivingIntensity = handleIntensity; }
                    }
                }
            } break;

            case Terra_EnableMode_Lowest: {
                drivingIntensity = __FLT_MAX__;
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    if ((*handleIter)->isValid() && !(*handleIter)->isDone()) {
                        auto handleIntensity = (*handleIter)->getDriveIntensity();
                        if (handleIntensity < drivingIntensity) { drivingIntensity = handleIntensity; }
                    }
                }
            } break;

            case Terra_EnableMode_Average: {
                int handleCount = 0;
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    if ((*handleIter)->isValid() && !(*handleIter)->isDone()) {
                        drivingIntensity += (*handleIter)->getDriveIntensity();
                        ++handleCount;
                    }
                }
                if (handleCount) { drivingIntensity /= handleCount; }
            } break;

            case Terra_EnableMode_Multiply: {
                drivingIntensity = (*_handles.begin())->getDriveIntensity();
                for (auto handleIter = _handles.begin() + 1; handleIter != _handles.end(); ++handleIter) {
                    if ((*handleIter)->isValid() && !(*handleIter)->isDone()) {
                        drivingIntensity *= (*handleIter)->getDriveIntensity();
                    }
                }
            } break;

            case Terra_EnableMode_InOrder: {
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    if ((*handleIter)->isValid() && !(*handleIter)->isDone()) {
                        drivingIntensity += (*handleIter)->getDriveIntensity();
                        break;
                    }
                }
            } break;

            case Terra_EnableMode_RevOrder: {
                for (auto handleIter = _handles.end() - 1; handleIter != _handles.begin() - 1; --handleIter) {
                    if ((*handleIter)->isValid() && !(*handleIter)->isDone()) {
                        drivingIntensity += (*handleIter)->getDriveIntensity();
                        break;
                    }
                }
            } break;

            default:
                break;
        }

        switch (_enableMode) {
            case Terra_EnableMode_InOrder: {
                bool selected = false;
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    if (!selected && (*handleIter)->isValid() && !(*handleIter)->isDone() && isFPEqual((*handleIter)->activation.intensity, getDriveIntensity())) {
                        selected = true; (*handleIter)->checkTime = now;
                    } else if ((*handleIter)->checkTime != 0) {
                        (*handleIter)->checkTime = 0;
                    }
                }
            } break;

            case Terra_EnableMode_RevOrder: {
                bool selected = false;
                for (auto handleIter = _handles.end() - 1; handleIter != _handles.begin() - 1; --handleIter) {
                    if (!selected && (*handleIter)->isValid() && !(*handleIter)->isDone() && isFPEqual((*handleIter)->activation.intensity, getDriveIntensity())) {
                        selected = true; (*handleIter)->checkTime = now;
                    } else if ((*handleIter)->checkTime != 0) {
                        (*handleIter)->checkTime = 0;
                    }
                }
            } break;

            default: {
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    if ((*handleIter)->isValid() && !(*handleIter)->isDone() && (*handleIter)->checkTime == 0) {
                        (*handleIter)->checkTime = now;
                    }
                }
            } break;
        }

        _enableActuator(drivingIntensity);
    }
    _needsUpdate = false;
}

bool TerraActuator::getCanEnable()
{
    return true;
}

TerraActivationHandle TerraActuator::activate(Terra_DirectionMode direction, float intensity,
                                              millis_t duration, bool force)
{
    return TerraActivationHandle(static_pointer_cast<TerraActuator>(getSharedPtr()), direction, intensity, duration, force);
}

void TerraActuator::setOutput(float intensity, millis_t duration, uint32_t now)
{
    _directActivation.activation = TerraActivation(intensity > FLT_EPSILON ? Terra_DirectionMode_Forward :
                                                   intensity < -FLT_EPSILON ? Terra_DirectionMode_Reverse : Terra_DirectionMode_Stop,
                                                   fabsf(intensity), duration, Terra_ActivationFlags_None);
    if (_directActivation.activation.isValid()) {
        _directActivation = static_pointer_cast<TerraActuator>(getSharedPtr());
    } else {
        _directActivation.unset();
    }
    setNeedsUpdate();
    update(now);
}

void TerraActuator::off()
{
    _directActivation.unset();
    setNeedsUpdate();
    update();
}

TerraData *TerraActuator::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void TerraActuator::saveToData(TerraData *dataOut) const
{
    TerraObject::saveToData(dataOut);
    dataOut->id.object.classType = (tid_t)classType;
    static_cast<TerraActuatorData *>(dataOut)->enableMode = _enableMode;
}

TerraRelayActuator::TerraRelayActuator(Terra_ActuatorType actuatorType, tposi_t actuatorIndex,
                                       TerraDigitalPin outputPin, int classTypeIn)
    : TerraActuator(actuatorType, actuatorIndex, classTypeIn), _outputPin(outputPin)
{
    _outputPin.init();
    _outputPin.deactivate();
}

TerraRelayActuator::TerraRelayActuator(const TerraActuatorData *dataIn)
    : TerraActuator(dataIn), _outputPin(&dataIn->outputPin)
{
    _outputPin.init();
    _outputPin.deactivate();
}

TerraRelayActuator::~TerraRelayActuator()
{
    _enabled = false;
    _outputPin.deactivate();
}

bool TerraRelayActuator::getCanEnable()
{
    return _outputPin.isValid() && TerraActuator::getCanEnable();
}

void TerraRelayActuator::_enableActuator(float intensity)
{
    if (!_outputPin.isValid()) { return; }
    if (intensity > FLT_EPSILON) {
        _enabled = true;
        _outputPin.activate();
    } else {
        _disableActuator();
    }
}

void TerraRelayActuator::_disableActuator()
{
    _enabled = false;
    if (_outputPin.isValid()) { _outputPin.deactivate(); }
}

void TerraRelayActuator::saveToData(TerraData *dataOut) const
{
    TerraActuator::saveToData(dataOut);
    _outputPin.saveToData(&static_cast<TerraActuatorData *>(dataOut)->outputPin);
}

TerraVariableActuator::TerraVariableActuator(Terra_ActuatorType actuatorType, tposi_t actuatorIndex,
                                             TerraAnalogPin outputPin)
    : TerraActuator(actuatorType, actuatorIndex, Variable), _outputPin(outputPin), _intensity(0.0f)
{
    _outputPin.init();
    _outputPin.analogWrite(0.0f);
}

TerraVariableActuator::TerraVariableActuator(const TerraActuatorData *dataIn)
    : TerraActuator(dataIn), _outputPin(&dataIn->outputPin), _intensity(0.0f)
{
    _outputPin.init();
    _outputPin.analogWrite(0.0f);
}

TerraVariableActuator::~TerraVariableActuator()
{
    _disableActuator();
}

bool TerraVariableActuator::getCanEnable()
{
    return _outputPin.isValid() && TerraActuator::getCanEnable();
}

void TerraVariableActuator::_enableActuator(float intensity)
{
    if (!_outputPin.isValid()) { return; }
    _intensity = constrain(intensity, 0.0f, 1.0f);
    _enabled = _intensity > FLT_EPSILON;
    _outputPin.analogWrite(_intensity);
}

void TerraVariableActuator::_disableActuator()
{
    _intensity = 0.0f;
    _enabled = false;
    if (_outputPin.isValid()) { _outputPin.analogWrite(0.0f); }
}

void TerraVariableActuator::saveToData(TerraData *dataOut) const
{
    TerraActuator::saveToData(dataOut);
    _outputPin.saveToData(&static_cast<TerraActuatorData *>(dataOut)->outputPin);
}

TerraPump::TerraPump(tposi_t actuatorIndex, TerraDigitalPin outputPin,
                     Terra_ActuatorType actuatorType, int classTypeIn)
    : TerraRelayActuator(actuatorType, actuatorIndex, outputPin, classTypeIn),
      _maxContinuousMs(0), _startedAt(0)
{ ; }

TerraPump::TerraPump(const TerraActuatorData *dataIn)
    : TerraRelayActuator(dataIn), _maxContinuousMs(dataIn->maxContinuousMs), _startedAt(0)
{ ; }

void TerraPump::update(uint32_t now)
{
    TerraActuator::update(now);
    if (isEnabled() && !_startedAt) { _startedAt = now; }
    if (!isEnabled()) { _startedAt = 0; }
    if (isEnabled() && _maxContinuousMs && _startedAt && (uint32_t)(now - _startedAt) >= _maxContinuousMs) {
        while (_handles.size()) { _handles.front()->unset(); }
        setFault(TerraString("maximum continuous runtime exceeded"));
        setNeedsUpdate();
        TerraActuator::update(now);
    }
}

void TerraPump::saveToData(TerraData *dataOut) const
{
    TerraRelayActuator::saveToData(dataOut);
    static_cast<TerraActuatorData *>(dataOut)->maxContinuousMs = _maxContinuousMs;
}

TerraSumpPump::TerraSumpPump(tposi_t actuatorIndex, TerraDigitalPin outputPin)
    : TerraPump(actuatorIndex, outputPin, Terra_ActuatorType_SumpPump), _levelSensor(this),
      _startLevelPercent(TERRA_SUMP_START_LEVEL_PERCENT), _stopLevelPercent(TERRA_SUMP_STOP_LEVEL_PERCENT),
      _alarmLevelPercent(TERRA_SUMP_ALARM_LEVEL_PERCENT), _lastLevelPercent(0.0f),
      _levelValid(false), _highWaterAlarm(false)
{ ; }

TerraSumpPump::TerraSumpPump(const TerraActuatorData *dataIn)
    : TerraPump(dataIn), _levelSensor(this), _startLevelPercent(dataIn->sumpStartPercent),
      _stopLevelPercent(dataIn->sumpStopPercent), _alarmLevelPercent(dataIn->sumpAlarmPercent),
      _lastLevelPercent(0.0f), _levelValid(false), _highWaterAlarm(false)
{
    _levelSensor.initObject(dataIn->levelSensor);
}

bool TerraSumpPump::configureLevels(float startPercent, float stopPercent, float alarmPercent)
{
    if (stopPercent < 0.0f || stopPercent >= startPercent || startPercent >= alarmPercent || alarmPercent > 100.0f) { return false; }
    _startLevelPercent = startPercent;
    _stopLevelPercent = stopPercent;
    _alarmLevelPercent = alarmPercent;
    bumpRevisionIfNeeded();
    return true;
}

void TerraSumpPump::update(uint32_t now)
{
    if (_levelSensor.isSet()) {
        TerraSingleMeasurement level = _levelSensor.getMeasurement(now, true);
        if (!level.isSet() || level.units != Terra_UnitsType_Percentile_100) {
            _levelValid = false;
            _highWaterAlarm = false;
            off();
            setFault(TerraString("sump level invalid"));
        } else {
            if (hasFault() && getFaultMessage() == TerraString("sump level invalid")) { clearFault(); }
            _lastLevelPercent = constrain(level.value, 0.0f, 100.0f);
            _levelValid = true;
            _highWaterAlarm = _lastLevelPercent >= _alarmLevelPercent;
            if (isEnabled()) {
                if (_lastLevelPercent <= _stopLevelPercent) { off(); }
            } else if (_lastLevelPercent >= _startLevelPercent) {
                setOutput(1.0f, (millis_t)-1, now);
            }
        }
    }
    TerraPump::update(now);
}

void TerraSumpPump::unresolveAny(TerraObject *object)
{
    _levelSensor.unresolveAny(object);
    TerraPump::unresolveAny(object);
}

void TerraSumpPump::saveToData(TerraData *dataOut) const
{
    TerraPump::saveToData(dataOut);
    auto actuatorData = static_cast<TerraActuatorData *>(dataOut);
    if (_levelSensor.isSet()) {
        strncpy(actuatorData->levelSensor, _levelSensor.getKeyString().c_str(), TERRA_NAME_MAXSIZE - 1);
        actuatorData->levelSensor[TERRA_NAME_MAXSIZE - 1] = '\0';
    }
    actuatorData->sumpStartPercent = _startLevelPercent;
    actuatorData->sumpStopPercent = _stopLevelPercent;
    actuatorData->sumpAlarmPercent = _alarmLevelPercent;
}

TerraActuator *newActuatorObjectFromData(const TerraActuatorData *dataIn)
{
    if (!dataIn) { return nullptr; }

    switch (dataIn->id.object.classType) {
        case (tid_t)TerraActuator::Relay:
            switch ((Terra_ActuatorType)dataIn->id.object.objType) {
                case Terra_ActuatorType_Valve:
                    return new TerraValve(dataIn);
                case Terra_ActuatorType_Diverter:
                    return new TerraDiverter(dataIn);
                case Terra_ActuatorType_Heater:
                    return new TerraHeater(dataIn);
                default:
                    return new TerraRelayActuator(dataIn);
            }
        case (tid_t)TerraActuator::RelayPump:
            switch ((Terra_ActuatorType)dataIn->id.object.objType) {
                case Terra_ActuatorType_SumpPump:
                    return new TerraSumpPump(dataIn);
                case Terra_ActuatorType_Circulator:
                    return new TerraCirculator(dataIn);
                default:
                    return new TerraPump(dataIn);
            }
        case (tid_t)TerraActuator::Variable:
            return new TerraVariableActuator(dataIn);
        default:
            return nullptr;
    }
}

TerraActuatorData::TerraActuatorData()
    : TerraObjectData(), enableMode(Terra_EnableMode_Highest), outputPin(), maxContinuousMs(0),
      levelSensor{0}, sumpStartPercent(TERRA_SUMP_START_LEVEL_PERCENT),
      sumpStopPercent(TERRA_SUMP_STOP_LEVEL_PERCENT), sumpAlarmPercent(TERRA_SUMP_ALARM_LEVEL_PERCENT)
{
    _size = sizeof(*this);
}

void TerraActuatorData::toJSONObject(JsonObject &objectOut) const
{
    TerraObjectData::toJSONObject(objectOut);
    objectOut["enableMode"] = (int)enableMode;
    if (outputPin.isSet()) {
        JsonObject pinObj = objectOut.createNestedObject("outputPin");
        outputPin.toJSONObject(pinObj);
    }
    if (maxContinuousMs) { objectOut["maxContinuousMs"] = maxContinuousMs; }
    if (levelSensor[0]) { objectOut["levelSensor"] = levelSensor; }
    objectOut["sumpStartPercent"] = sumpStartPercent;
    objectOut["sumpStopPercent"] = sumpStopPercent;
    objectOut["sumpAlarmPercent"] = sumpAlarmPercent;
}

void TerraActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraObjectData::fromJSONObject(objectIn);
    enableMode = (Terra_EnableMode)(objectIn["enableMode"] | (int)enableMode);
    JsonObjectConst pinObj = objectIn["outputPin"].as<JsonObjectConst>();
    if (!pinObj.isNull()) { outputPin.fromJSONObject(pinObj); }
    maxContinuousMs = objectIn["maxContinuousMs"] | maxContinuousMs;
    const char *levelSensorIn = objectIn["levelSensor"] | nullptr;
    if (levelSensorIn) {
        strncpy(levelSensor, levelSensorIn, TERRA_NAME_MAXSIZE - 1);
        levelSensor[TERRA_NAME_MAXSIZE - 1] = '\0';
    }
    sumpStartPercent = objectIn["sumpStartPercent"] | sumpStartPercent;
    sumpStopPercent = objectIn["sumpStopPercent"] | sumpStopPercent;
    sumpAlarmPercent = objectIn["sumpAlarmPercent"] | sumpAlarmPercent;
}
