/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Actuators
*/

#include "Terraduino.h"
#include "TerraSensors.h"

TerraActuator *newActuatorObjectFromData(const TerraActuatorData *dataIn)
{
    if (dataIn && !isValidType(dataIn->id.object.idType)) return nullptr;
    TERRA_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(TStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData()) {
        switch (dataIn->id.object.classType) {
            case (tid_t)TerraActuator::Relay:
                return new TerraRelayActuator((const TerraActuatorData *)dataIn);
            case (tid_t)TerraActuator::RelayPump:
                return new TerraRelayPumpActuator((const TerraPumpActuatorData *)dataIn);
            case (tid_t)TerraActuator::Variable:
                return new TerraVariableActuator((const TerraActuatorData *)dataIn);
            case (tid_t)TerraActuator::VariablePump:
                //return new TerraVariablePumpActuator((const TerraPumpActuatorData *)dataIn);
            default: break;
        }
    }

    return nullptr;
}


TerraActuator::TerraActuator(Terra_ActuatorType actuatorType, tposi_t actuatorIndex, int classTypeIn)
    : TerraObject(TerraIdentity(actuatorType, actuatorIndex)), classType(static_cast<decltype(classType)>(classTypeIn)),
      _enabled(false), _needsUpdate(false), _enableMode(Terra_EnableMode_Undefined),
      _contPowerUsage(), _parentRail(this), _parentReservoir(this), _calibrationData(nullptr)
{ ; }

TerraActuator::TerraActuator(const TerraActuatorData *dataIn)
    : TerraObject(dataIn), classType(static_cast<decltype(classType)>(dataIn->id.object.classType)),
      _enabled(false), _needsUpdate(false), _enableMode(dataIn->enableMode),
      _contPowerUsage(dataIn->contPowerUsage.value, dataIn->contPowerUsage.units,
                      dataIn->contPowerUsage.timestamp,
                      dataIn->contPowerUsage.units != Terra_UnitsType_Undefined ? 1 : tframe_none),
      _parentRail(this), _parentReservoir(this), _calibrationData(nullptr)
{
    _parentRail.initObject(dataIn->railName);
    _parentReservoir.initObject(dataIn->reservoirName);
}

TerraActuator::~TerraActuator()
{
    while (_handles.size()) {
        TerraActivationHandle *handle = _handles.front();
        if (handle) { handle->unset(); }
        else { _handles.erase(_handles.begin()); }
    }
}

void TerraActuator::update(uint32_t now)
{
    TerraObject::update(now);

    _parentRail.resolve();
    _parentReservoir.resolve();

    // Update running handles and elapse them as needed, determine forced status, and remove invalid/finished handles
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

    // Enablement checking
    bool canEnable = _handles.size() && (forced || getCanEnable());

    if (!canEnable && (_enabled || _needsUpdate)) { // If enabled and shouldn't be (unless force enabled)
        _disableActuator();
    } else if (canEnable && (!_enabled || _needsUpdate)) { // If can enable and isn't (maybe force enabled)
        float drivingIntensity = 0.0f;

        // Determine what driving intensity [-1,1] actuator should use
        switch (_enableMode) {
            case Terra_EnableMode_Highest:
            case Terra_EnableMode_DescOrder: {
                drivingIntensity = -__FLT_MAX__;
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    if ((*handleIter)->isValid() && !(*handleIter)->isDone()) {
                        auto handleIntensity = (*handleIter)->getDriveIntensity();
                        if (handleIntensity > drivingIntensity) { drivingIntensity = handleIntensity; }
                    }
                }
            } break;

            case Terra_EnableMode_Lowest:
            case Terra_EnableMode_AscOrder: {
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

        // Enable/disable activation handles as needed (serial modes only select 1 at a time)
        switch (_enableMode) {
            case Terra_EnableMode_InOrder:
            case Terra_EnableMode_DescOrder: {
                bool selected = false;
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    if (!selected && (*handleIter)->isValid() && !(*handleIter)->isDone() && isFPEqual((*handleIter)->activation.intensity, getDriveIntensity())) {
                        selected = true; (*handleIter)->checkTime = now;
                    } else if ((*handleIter)->checkTime != 0) {
                        (*handleIter)->checkTime = 0;
                    }
                }
            } break;

            case Terra_EnableMode_RevOrder:
            case Terra_EnableMode_AscOrder: {
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
    if (!TerraObject::_enabled) { return false; }
    if (getParentRail() && !getParentRail()->canActivate(this)) { return false; }
    if (getParentReservoir() && !getParentReservoir()->canActivate(this)) { return false; }
    return true;
}

void TerraActuator::setEnableMode(Terra_EnableMode mode)
{
    if (_enableMode != mode) {
        _enableMode = mode;
        setNeedsUpdate();
        bumpRevisionIfNeeded();
    }
}

void TerraActuator::setContinuousPowerUsage(TerraSingleMeasurement contPowerUsage)
{
    _contPowerUsage = contPowerUsage;
    _contPowerUsage.updateFrame(1);
    bumpRevisionIfNeeded();
}

const TerraSingleMeasurement &TerraActuator::getContinuousPowerUsage()
{
    return _contPowerUsage;
}

TerraAttachment &TerraActuator::getParentRailAttachment()
{
    return _parentRail;
}

TerraAttachment &TerraActuator::getParentReservoirAttachment()
{
    return _parentReservoir;
}

void TerraActuator::setUserCalibrationData(const TerraCalibrationData *userCalibrationData)
{
    if (_calibrationData && _calibrationData != userCalibrationData) { bumpRevisionIfNeeded(); }
    if (getController()) {
        if (userCalibrationData && getController()->setUserCalibrationData(userCalibrationData)) {
            _calibrationData = getController()->getUserCalibrationData(_id.key);
        } else if (!userCalibrationData && _calibrationData && getController()->dropUserCalibrationData(_calibrationData)) {
            _calibrationData = nullptr;
        }
    } else {
        _calibrationData = userCalibrationData;
    }
}

Signal<TerraActuator *, TERRA_ACTUATOR_SIGNAL_SLOTS> &TerraActuator::getActivationSignal()
{
    return _activateSignal;
}

TerraData *TerraActuator::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void TerraActuator::saveToData(TerraData *dataOut) const
{
    TerraObject::saveToData(dataOut);

    dataOut->id.object.classType = (int8_t)classType;
    auto actuatorData = (TerraActuatorData *)dataOut;
    actuatorData->enableMode = _enableMode;
    if (_contPowerUsage.isSet()) {
        actuatorData->contPowerUsage.value = _contPowerUsage.value;
        actuatorData->contPowerUsage.units = _contPowerUsage.units;
        actuatorData->contPowerUsage.timestamp = _contPowerUsage.timestamp;
    }
    if (_parentReservoir.isSet()) {
        strncpy(actuatorData->reservoirName, _parentReservoir.getKeyString().c_str(), TERRA_NAME_MAXSIZE - 1);
        actuatorData->reservoirName[TERRA_NAME_MAXSIZE - 1] = '\0';
    }
    if (_parentRail.isSet()) {
        strncpy(actuatorData->railName, _parentRail.getKeyString().c_str(), TERRA_NAME_MAXSIZE - 1);
        actuatorData->railName[TERRA_NAME_MAXSIZE - 1] = '\0';
    }
}

void TerraActuator::handleActivation()
{
    if (_enabled) {
        if (getLogger()) { getLogger()->logActivation(this); }
    } else {
        for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
            if ((*handleIter)->checkTime) { (*handleIter)->checkTime = 0; }
        }

        if (getLogger()) { getLogger()->logDeactivation(this); }
    }

    #ifdef TERRA_USE_MULTITASKING
        scheduleSignalFireOnce<TerraActuator *>(getSharedPtr(), _activateSignal, this);
    #else
        _activateSignal.fire(this);
    #endif
}


TerraRelayActuator::TerraRelayActuator(Terra_ActuatorType actuatorType, tposi_t actuatorIndex,
                                       TerraDigitalPin outputPin, int classTypeIn)
    : TerraActuator(actuatorType, actuatorIndex, classTypeIn), _outputPin(outputPin)
{
    TERRA_HARD_ASSERT(_outputPin.isValid(), SFP(TStr_Err_InvalidPinOrType));
    _outputPin.init();
    _outputPin.deactivate();
}

TerraRelayActuator::TerraRelayActuator(const TerraActuatorData *dataIn)
    : TerraActuator(dataIn), _outputPin(&dataIn->outputPin)
{
    TERRA_HARD_ASSERT(_outputPin.isValid(), SFP(TStr_Err_InvalidPinOrType));
    _outputPin.init();
    _outputPin.deactivate();
}

TerraRelayActuator::~TerraRelayActuator()
{
    if (_enabled) {
        _enabled = false;
        _outputPin.deactivate();
    }
}

bool TerraRelayActuator::getCanEnable()
{
    return _outputPin.isValid() && TerraActuator::getCanEnable();
}

float TerraRelayActuator::getDriveIntensity() const
{
    return _enabled ? 1.0f : 0.0f;
}

bool TerraRelayActuator::isEnabled(float tolerance) const
{
    (void)tolerance;
    return _enabled;
}

void TerraRelayActuator::_enableActuator(float intensity)
{
    bool wasEnabled = _enabled;

    if (_outputPin.isValid()) {
        if (intensity > FLT_EPSILON) {
            _enabled = true;
            _outputPin.activate();
        } else {
            _enabled = false;
            _outputPin.deactivate();
        }

        if (wasEnabled != _enabled) { handleActivation(); }
    }
}

void TerraRelayActuator::_disableActuator()
{
    bool wasEnabled = _enabled;

    if (_outputPin.isValid()) {
        _enabled = false;
        _outputPin.deactivate();

        if (wasEnabled) { handleActivation(); }
    }
}

void TerraRelayActuator::saveToData(TerraData *dataOut) const
{
    TerraActuator::saveToData(dataOut);
    _outputPin.saveToData(&((TerraActuatorData *)dataOut)->outputPin);
}


TerraRelayPumpActuator::TerraRelayPumpActuator(Terra_ActuatorType actuatorType, tposi_t actuatorIndex,
                                               TerraDigitalPin outputPin, int classTypeIn)
    : TerraRelayActuator(actuatorType, actuatorIndex, outputPin, classTypeIn),
      TerraFlowRateUnitsInterfaceStorage(defaultFlowRateUnits()),
      _contFlowRate(), _flowRate(this), _destReservoir(this),
      _pumpVolumeAccum(0.0f), _pumpTimeStart(0), _pumpTimeAccum(0)
{
    _flowRate.setMeasurementUnits(getFlowRateUnits());
}

TerraRelayPumpActuator::TerraRelayPumpActuator(const TerraPumpActuatorData *dataIn)
    : TerraRelayActuator(dataIn),
      TerraFlowRateUnitsInterfaceStorage(definedUnitsElse(dataIn->flowRateUnits, defaultFlowRateUnits())),
      _contFlowRate(dataIn->contFlowRate.value, dataIn->contFlowRate.units,
                    dataIn->contFlowRate.timestamp,
                    dataIn->contFlowRate.units != Terra_UnitsType_Undefined ? 1 : tframe_none),
      _flowRate(this), _destReservoir(this),
      _pumpVolumeAccum(0.0f), _pumpTimeStart(0), _pumpTimeAccum(0)
{
    _flowRate.setMeasurementUnits(getFlowRateUnits());
    _destReservoir.initObject(dataIn->destReservoir);
    _flowRate.initObject(dataIn->flowRateSensor);
}

void TerraRelayPumpActuator::update(uint32_t now)
{
    TerraActuator::update(now);

    _destReservoir.resolve();
    _flowRate.updateIfNeeded(true);

    if (_pumpTimeStart && _pumpTimeAccum < now) {
        handlePumpTime(now);
    }
}

bool TerraRelayPumpActuator::getCanEnable()
{
    if (TerraRelayActuator::getCanEnable()) {
        if (getDestinationReservoir() && !getDestinationReservoir()->canActivate(this)) { return false; }
        return true;
    }
    return false;
}

bool TerraRelayPumpActuator::canPump(float volume, Terra_UnitsType volumeUnits)
{
    auto sourceReservoir = getSourceReservoir();
    if (!sourceReservoir || _contFlowRate.value <= FLT_EPSILON || sourceReservoir->isEmpty(true)) { return false; }

    if (sourceReservoir->isWaterPipeClass()) { return true; }
    if (!sourceReservoir->isWaterClass()) { return false; }

    auto waterReservoir = static_pointer_cast<TerraWaterReservoir>(sourceReservoir);
    auto waterVolume = waterReservoir->getWaterVolumeSensorAttachment().getMeasurement(true);
    if (!waterVolume.isSet()) { return false; }

    volumeUnits = definedUnitsElse(volumeUnits, getVolumeUnits());
    TerraSingleMeasurement requestedVolume(volume, volumeUnits, 0, 1);
    requestedVolume.toUnits(waterVolume.units);
    return requestedVolume.isSet() && requestedVolume.value <= waterVolume.value + FLT_EPSILON;
}

TerraActivationHandle TerraRelayPumpActuator::pump(float volume, Terra_UnitsType volumeUnits)
{
    if (getSourceReservoir() && _contFlowRate.value > FLT_EPSILON) {
        volumeUnits = definedUnitsElse(volumeUnits, getVolumeUnits());
        if (volumeUnits != getVolumeUnits() && !convertUnits(&volume, &volumeUnits, getVolumeUnits())) {
            return TerraActivationHandle();
        }
        return pump((millis_t)((volume / _contFlowRate.value) * secondsToMillis(SECS_PER_MIN)));
    }
    return TerraActivationHandle();
}

bool TerraRelayPumpActuator::canPump(millis_t time)
{
    if (getSourceReservoir() && _contFlowRate.value > FLT_EPSILON) {
        return canPump(_contFlowRate.value * (time / (float)secondsToMillis(SECS_PER_MIN)), getVolumeUnits());
    }
    return false;
}

TerraActivationHandle TerraRelayPumpActuator::pump(millis_t time)
{
    if (getSourceReservoir()) {
        if (getLogger()) {
            getLogger()->logStatus(this, SFP(TStr_Log_CalculatedPumping));
            if (getSourceReservoir()) { getLogger()->logMessage(SFP(TStr_Log_Field_Source_Reservoir), getSourceReservoir()->getId().getDisplayString()); }
            if (getDestinationReservoir()) { getLogger()->logMessage(SFP(TStr_Log_Field_Destination_Reservoir), getDestinationReservoir()->getId().getDisplayString()); }
            if (_contFlowRate.value > FLT_EPSILON) {
                getLogger()->logMessage(SFP(TStr_Log_Field_Vol_Calculated), measurementToString(_contFlowRate.value * (time / (float)secondsToMillis(SECS_PER_MIN)), baseUnits(getFlowRateUnits()), 1));
            }
            getLogger()->logMessage(SFP(TStr_Log_Field_Time_Calculated), roundToString(time / 1000.0f, 1), String('s'));
        }
        return enableActuator(time);
    }
    return TerraActivationHandle();
}

void TerraRelayPumpActuator::setFlowRateUnits(Terra_UnitsType flowRateUnits)
{
    if (_flowRateUnits != flowRateUnits) {
        _flowRateUnits = flowRateUnits;
        convertUnits(&_contFlowRate, getFlowRateUnits());
        _flowRate.setMeasurementUnits(getFlowRateUnits());
        bumpRevisionIfNeeded();
    }
}

TerraAttachment &TerraRelayPumpActuator::getSourceReservoirAttachment()
{
    return _parentReservoir;
}

TerraAttachment &TerraRelayPumpActuator::getDestinationReservoirAttachment()
{
    return _destReservoir;
}

void TerraRelayPumpActuator::setContinuousFlowRate(TerraSingleMeasurement contFlowRate)
{
    _contFlowRate = contFlowRate;
    _contFlowRate.updateFrame(1);
    convertUnits(&_contFlowRate, getFlowRateUnits());
    bumpRevisionIfNeeded();
}

const TerraSingleMeasurement &TerraRelayPumpActuator::getContinuousFlowRate()
{
    return _contFlowRate;
}

TerraSensorAttachment &TerraRelayPumpActuator::getFlowRateSensorAttachment()
{
    return _flowRate;
}

void TerraRelayPumpActuator::saveToData(TerraData *dataOut) const
{
    TerraRelayActuator::saveToData(dataOut);

    auto pumpData = (TerraPumpActuatorData *)dataOut;
    pumpData->flowRateUnits = _flowRateUnits;
    if (_contFlowRate.isSet()) {
        pumpData->contFlowRate.value = _contFlowRate.value;
        pumpData->contFlowRate.units = _contFlowRate.units;
        pumpData->contFlowRate.timestamp = _contFlowRate.timestamp;
    }
    if (_destReservoir.isSet()) {
        strncpy(pumpData->destReservoir, _destReservoir.getKeyString().c_str(), TERRA_NAME_MAXSIZE - 1);
        pumpData->destReservoir[TERRA_NAME_MAXSIZE - 1] = '\0';
    }
    if (_flowRate.isSet()) {
        strncpy(pumpData->flowRateSensor, _flowRate.getKeyString().c_str(), TERRA_NAME_MAXSIZE - 1);
        pumpData->flowRateSensor[TERRA_NAME_MAXSIZE - 1] = '\0';
    }
}

void TerraRelayPumpActuator::handleActivation()
{
    millis_t time = nzMillis();
    TerraActuator::handleActivation();

    if (_enabled) {
        _pumpVolumeAccum = 0.0f;
        _pumpTimeStart = _pumpTimeAccum = time;
    } else if (_pumpTimeStart) {
        if (_pumpTimeAccum < time) { handlePumpTime(time); }
        _pumpTimeAccum = 0;

        if (getLogger()) {
            float duration = time - _pumpTimeStart;
            getLogger()->logStatus(this, SFP(TStr_Log_MeasuredPumping));
            if (getSourceReservoir()) { getLogger()->logMessage(SFP(TStr_Log_Field_Source_Reservoir), getSourceReservoir()->getId().getDisplayString()); }
            if (getDestinationReservoir()) { getLogger()->logMessage(SFP(TStr_Log_Field_Destination_Reservoir), getDestinationReservoir()->getId().getDisplayString()); }
            getLogger()->logMessage(SFP(TStr_Log_Field_Vol_Measured), measurementToString(_pumpVolumeAccum, baseUnits(getFlowRateUnits()), 1));
            getLogger()->logMessage(SFP(TStr_Log_Field_Time_Measured), roundToString(duration / 1000.0f, 1), String('s'));
        }

        _pumpTimeStart = 0;
    }
}

void TerraRelayPumpActuator::handlePumpTime(millis_t time)
{
    auto flowRate = getFlowRateSensor(true) ? _flowRate.getMeasurement() : _contFlowRate;
    if (flowRate.isSet() && flowRate.value > FLT_EPSILON) {
        flowRate.toUnits(getFlowRateUnits());
        if (flowRate.isSet()) {
            auto timeDelta = (time - _pumpTimeAccum) / (float)secondsToMillis(SECS_PER_MIN);
            _pumpVolumeAccum += flowRate.value * timeDelta;
        }
    }
    _pumpTimeAccum = time;
}


TerraVariableActuator::TerraVariableActuator(Terra_ActuatorType actuatorType, tposi_t actuatorIndex,
                                             TerraAnalogPin outputPin, int classTypeIn)
    : TerraActuator(actuatorType, actuatorIndex, classTypeIn), _outputPin(outputPin), _intensity(0.0f)
{
    TERRA_HARD_ASSERT(_outputPin.isValid(), SFP(TStr_Err_InvalidPinOrType));
    _outputPin.init();
    _outputPin.analogWrite(0.0f);
}

TerraVariableActuator::TerraVariableActuator(const TerraActuatorData *dataIn)
    : TerraActuator(dataIn), _outputPin(&dataIn->outputPin), _intensity(0.0f)
{
    TERRA_HARD_ASSERT(_outputPin.isValid(), SFP(TStr_Err_InvalidPinOrType));
    _outputPin.init();
    _outputPin.analogWrite(0.0f);
}

TerraVariableActuator::~TerraVariableActuator()
{
    if (_enabled) {
        _enabled = false;
        _outputPin.analogWrite(0.0f);
    }
}

bool TerraVariableActuator::getCanEnable()
{
    return _outputPin.isValid() && TerraActuator::getCanEnable();
}

float TerraVariableActuator::getDriveIntensity() const
{
    return _intensity;
}

bool TerraVariableActuator::isEnabled(float tolerance) const
{
    return _enabled && _intensity >= tolerance - FLT_EPSILON;
}

void TerraVariableActuator::_enableActuator(float intensity)
{
    bool wasEnabled = _enabled;
    intensity = constrain(intensity, 0.0f, 1.0f);

    if (_outputPin.isValid()) {
        _enabled = true;
        _outputPin.analogWrite((_intensity = intensity));

        if (!wasEnabled) { handleActivation(); }
    }
}

void TerraVariableActuator::_disableActuator()
{
    bool wasEnabled = _enabled;

    if (_outputPin.isValid()) {
        _intensity = 0.0f;
        _enabled = false;
        _outputPin.analogWrite(0.0f);

        if (wasEnabled) { handleActivation(); }
    }
}

void TerraVariableActuator::saveToData(TerraData *dataOut) const
{
    TerraActuator::saveToData(dataOut);
    _outputPin.saveToData(&((TerraActuatorData *)dataOut)->outputPin);
}


TerraActuatorData::TerraActuatorData()
    : TerraObjectData(), outputPin(), enableMode(Terra_EnableMode_Undefined),
      contPowerUsage(), railName{0}, reservoirName{0}
{
    _size = sizeof(*this);
}

void TerraActuatorData::toJSONObject(JsonObject &objectOut) const
{
    TerraObjectData::toJSONObject(objectOut);

    if (outputPin.isSet()) {
        JsonObject outputPinObj = objectOut.createNestedObject(SFP(TStr_Key_OutputPin));
        outputPin.toJSONObject(outputPinObj);
    }
    if (enableMode != Terra_EnableMode_Undefined) { objectOut[SFP(TStr_Key_EnableMode)] = terraEnableModeToString(enableMode); }
    if (contPowerUsage.units != Terra_UnitsType_Undefined) {
        JsonObject contPowerUsageObj = objectOut.createNestedObject(SFP(TStr_Key_ContinuousPowerUsage));
        contPowerUsage.toJSONObject(contPowerUsageObj);
    }
    if (railName[0]) { objectOut[SFP(TStr_Key_RailName)] = charsToString(railName, TERRA_NAME_MAXSIZE); }
    if (reservoirName[0]) { objectOut[SFP(TStr_Key_ReservoirName)] = charsToString(reservoirName, TERRA_NAME_MAXSIZE); }
}

void TerraActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraObjectData::fromJSONObject(objectIn);

    JsonObjectConst outputPinObj = objectIn[SFP(TStr_Key_OutputPin)];
    if (!outputPinObj.isNull()) { outputPin.fromJSONObject(outputPinObj); }
    const char *enableModeStr = objectIn[SFP(TStr_Key_EnableMode)] | nullptr;
    if (enableModeStr) { enableMode = terraEnableModeFromString(TerraString(enableModeStr)); }
    JsonVariantConst contPowerUsageVar = objectIn[SFP(TStr_Key_ContinuousPowerUsage)];
    if (!contPowerUsageVar.isNull()) { contPowerUsage.fromJSONVariant(contPowerUsageVar); }
    const char *railNameStr = objectIn[SFP(TStr_Key_RailName)] | nullptr;
    if (railNameStr && railNameStr[0]) { strncpy(railName, railNameStr, TERRA_NAME_MAXSIZE - 1); railName[TERRA_NAME_MAXSIZE - 1] = '\0'; }
    const char *reservoirNameStr = objectIn[SFP(TStr_Key_ReservoirName)] | nullptr;
    if (reservoirNameStr && reservoirNameStr[0]) { strncpy(reservoirName, reservoirNameStr, TERRA_NAME_MAXSIZE - 1); reservoirName[TERRA_NAME_MAXSIZE - 1] = '\0'; }
}

TerraPumpActuatorData::TerraPumpActuatorData()
    : TerraActuatorData(), flowRateUnits(Terra_UnitsType_Undefined),
      contFlowRate(), destReservoir{0}, flowRateSensor{0}
{
    _size = sizeof(*this);
}

void TerraPumpActuatorData::toJSONObject(JsonObject &objectOut) const
{
    TerraActuatorData::toJSONObject(objectOut);

    if (flowRateUnits != Terra_UnitsType_Undefined) { objectOut[SFP(TStr_Key_FlowRateUnits)] = terraUnitToString(flowRateUnits); }
    if (contFlowRate.units != Terra_UnitsType_Undefined) {
        JsonObject contFlowRateObj = objectOut.createNestedObject(SFP(TStr_Key_ContinuousFlowRate));
        contFlowRate.toJSONObject(contFlowRateObj);
    }
    if (destReservoir[0]) { objectOut[SFP(TStr_Key_OutputReservoir)] = charsToString(destReservoir, TERRA_NAME_MAXSIZE); }
    if (flowRateSensor[0]) { objectOut[SFP(TStr_Key_FlowRateSensor)] = charsToString(flowRateSensor, TERRA_NAME_MAXSIZE); }
}

void TerraPumpActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraActuatorData::fromJSONObject(objectIn);

    const char *flowRateUnitsStr = objectIn[SFP(TStr_Key_FlowRateUnits)] | nullptr;
    if (flowRateUnitsStr) { flowRateUnits = unitsTypeFromSymbol(TerraString(flowRateUnitsStr)); }
    JsonVariantConst contFlowRateVar = objectIn[SFP(TStr_Key_ContinuousFlowRate)];
    if (!contFlowRateVar.isNull()) { contFlowRate.fromJSONVariant(contFlowRateVar); }
    const char *destReservoirStr = objectIn[SFP(TStr_Key_OutputReservoir)] | nullptr;
    if (destReservoirStr && destReservoirStr[0]) { strncpy(destReservoir, destReservoirStr, TERRA_NAME_MAXSIZE - 1); destReservoir[TERRA_NAME_MAXSIZE - 1] = '\0'; }
    const char *flowRateSensorStr = objectIn[SFP(TStr_Key_FlowRateSensor)] | nullptr;
    if (flowRateSensorStr && flowRateSensorStr[0]) { strncpy(flowRateSensor, flowRateSensorStr, TERRA_NAME_MAXSIZE - 1); flowRateSensor[TERRA_NAME_MAXSIZE - 1] = '\0'; }
}
