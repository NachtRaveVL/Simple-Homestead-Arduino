/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Reservoirs
*/

#include "Terraduino.h"

TerraReservoir *newReservoirObjectFromData(const TerraReservoirData *dataIn)
{
    if (dataIn && !isValidType(dataIn->id.object.idType)) return nullptr;
    TERRA_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(TStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData()) {
        switch (dataIn->id.object.classType) {
            case (tid_t)TerraReservoir::Water:
                return new TerraWaterReservoir((const TerraWaterReservoirData *)dataIn);
            case (tid_t)TerraReservoir::Thermal:
                return new TerraThermalReservoir((const TerraThermalReservoirData *)dataIn);
            case (tid_t)TerraReservoir::WaterPipe:
                return new TerraInfiniteWaterReservoir((const TerraInfiniteWaterReservoirData *)dataIn);
            case (tid_t)TerraReservoir::ThermalPipe:
                return new TerraInfiniteThermalReservoir((const TerraInfiniteThermalReservoirData *)dataIn);
            default: break;
        }
    }

    return nullptr;
}


TerraReservoir::TerraReservoir(Terra_ReservoirType reservoirType, tposi_t reservoirIndex, int classTypeIn)
    : TerraObject(TerraIdentity(reservoirType, reservoirIndex)), classType((typeof(classType))classTypeIn),
      _filledState(Terra_TriggerState_Disabled), _highState(Terra_TriggerState_Disabled),
      _lowState(Terra_TriggerState_Disabled), _emptyState(Terra_TriggerState_Disabled)
{ ; }

TerraReservoir::TerraReservoir(const TerraReservoirData *dataIn)
    : TerraObject(dataIn), classType((typeof(classType))(dataIn->id.object.classType)),
      _filledState(Terra_TriggerState_Disabled), _highState(Terra_TriggerState_Disabled),
      _lowState(Terra_TriggerState_Disabled), _emptyState(Terra_TriggerState_Disabled)
{ ; }

bool TerraReservoir::canActivate(TerraActuator *actuator)
{
    if (!actuator) { return false; }

    if (actuator->isPumpType()) {
        bool doEmptyCheck = actuator->getParentReservoir().get() == this;
        return doEmptyCheck ? !isEmpty(true) : !isFilled(true);
    }

    return true;
}

Signal<TerraReservoir *, TERRA_RESERVOIR_SIGNAL_SLOTS> &TerraReservoir::getFilledSignal()
{
    return _filledSignal;
}

Signal<TerraReservoir *, TERRA_RESERVOIR_SIGNAL_SLOTS> &TerraReservoir::getHighSignal()
{
    return _highSignal;
}

Signal<TerraReservoir *, TERRA_RESERVOIR_SIGNAL_SLOTS> &TerraReservoir::getLowSignal()
{
    return _lowSignal;
}

Signal<TerraReservoir *, TERRA_RESERVOIR_SIGNAL_SLOTS> &TerraReservoir::getEmptySignal()
{
    return _emptySignal;
}

TerraData *TerraReservoir::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void TerraReservoir::saveToData(TerraData *dataOut)
{
    TerraObject::saveToData(dataOut);

    dataOut->id.object.classType = (int8_t)classType;
}

void TerraReservoir::handleFilled(Terra_TriggerState filledState)
{
    if (filledState == Terra_TriggerState_Disabled || filledState == Terra_TriggerState_Undefined) { return; }

    if (_filledState != filledState) {
        _filledState = filledState;

        if (triggerStateToBool(_filledState)) {
            #ifdef TERRA_USE_MULTITASKING
                scheduleSignalFireOnce<TerraReservoir *>(getSharedPtr(), _filledSignal, this);
            #else
                _filledSignal.fire(this);
            #endif
        }
    }
}

void TerraReservoir::handleHigh(Terra_TriggerState highState)
{
    if (highState == Terra_TriggerState_Disabled || highState == Terra_TriggerState_Undefined) { return; }

    if (_highState != highState) {
        _highState = highState;

        if (triggerStateToBool(_highState)) {
            #ifdef TERRA_USE_MULTITASKING
                scheduleSignalFireOnce<TerraReservoir *>(getSharedPtr(), _highSignal, this);
            #else
                _highSignal.fire(this);
            #endif
        }
    }
}

void TerraReservoir::handleLow(Terra_TriggerState lowState)
{
    if (lowState == Terra_TriggerState_Disabled || lowState == Terra_TriggerState_Undefined) { return; }

    if (_lowState != lowState) {
        _lowState = lowState;

        if (triggerStateToBool(_lowState)) {
            #ifdef TERRA_USE_MULTITASKING
                scheduleSignalFireOnce<TerraReservoir *>(getSharedPtr(), _lowSignal, this);
            #else
                _lowSignal.fire(this);
            #endif
        }
    }
}

void TerraReservoir::handleEmpty(Terra_TriggerState emptyState)
{
    if (emptyState == Terra_TriggerState_Disabled || emptyState == Terra_TriggerState_Undefined) { return; }

    if (_emptyState != emptyState) {
        _emptyState = emptyState;

        if (triggerStateToBool(_emptyState)) {
            #ifdef TERRA_USE_MULTITASKING
                scheduleSignalFireOnce<TerraReservoir *>(getSharedPtr(), _emptySignal, this);
            #else
                _emptySignal.fire(this);
            #endif
        }
    }
}


TerraWaterReservoir::TerraWaterReservoir(tposi_t reservoirIndex, float maxVolume, int classTypeIn)
    : TerraReservoir(Terra_ReservoirType_Water, reservoirIndex, classTypeIn),
      TerraVolumeUnitsInterfaceStorage(defaultVolumeUnits()),
      _maxVolume(maxVolume), _waterVolume(this), _waterTemperature(this),
      _filledTrigger(this), _highTrigger(this), _lowTrigger(this), _emptyTrigger(this)
{
    _waterVolume.setMeasurementUnits(getVolumeUnits());

    _filledTrigger.setHandleMethod(&TerraWaterReservoir::handleFilled, this);
    _highTrigger.setHandleMethod(&TerraWaterReservoir::handleHigh, this);
    _lowTrigger.setHandleMethod(&TerraWaterReservoir::handleLow, this);
    _emptyTrigger.setHandleMethod(&TerraWaterReservoir::handleEmpty, this);
}

TerraWaterReservoir::TerraWaterReservoir(const TerraWaterReservoirData *dataIn)
    : TerraReservoir(dataIn),
      TerraVolumeUnitsInterfaceStorage(definedUnitsElse(dataIn->volumeUnits, defaultVolumeUnits())),
      _maxVolume(dataIn->maxVolume), _waterVolume(this), _waterTemperature(this),
      _filledTrigger(this), _highTrigger(this), _lowTrigger(this), _emptyTrigger(this)
{
    _waterVolume.setMeasurementUnits(getVolumeUnits());
    _waterVolume.initObject(dataIn->volumeSensor);
    _waterTemperature.initObject(dataIn->waterTemperatureSensor);

    _filledTrigger.setHandleMethod(&TerraWaterReservoir::handleFilled, this);
    _filledTrigger.setObject(newTriggerObjectFromSubData(&(dataIn->filledTrigger)));
    TERRA_SOFT_ASSERT(!dataIn->filledTrigger.isSet() || _filledTrigger, SFP(TStr_Err_AllocationFailure));

    _highTrigger.setHandleMethod(&TerraWaterReservoir::handleHigh, this);
    _highTrigger.setObject(newTriggerObjectFromSubData(&(dataIn->highTrigger)));
    TERRA_SOFT_ASSERT(!dataIn->highTrigger.isSet() || _highTrigger, SFP(TStr_Err_AllocationFailure));

    _lowTrigger.setHandleMethod(&TerraWaterReservoir::handleLow, this);
    _lowTrigger.setObject(newTriggerObjectFromSubData(&(dataIn->lowTrigger)));
    TERRA_SOFT_ASSERT(!dataIn->lowTrigger.isSet() || _lowTrigger, SFP(TStr_Err_AllocationFailure));

    _emptyTrigger.setHandleMethod(&TerraWaterReservoir::handleEmpty, this);
    _emptyTrigger.setObject(newTriggerObjectFromSubData(&(dataIn->emptyTrigger)));
    TERRA_SOFT_ASSERT(!dataIn->emptyTrigger.isSet() || _emptyTrigger, SFP(TStr_Err_AllocationFailure));
}

void TerraWaterReservoir::update()
{
    TerraObject::update();

    _waterVolume.updateIfNeeded(true);
    _waterTemperature.updateIfNeeded(true);

    _filledTrigger.updateIfNeeded(true);
    _highTrigger.updateIfNeeded(true);
    _lowTrigger.updateIfNeeded(true);
    _emptyTrigger.updateIfNeeded(true);
}

SharedPtr<TerraObjInterface> TerraWaterReservoir::getSharedPtrFor(const TerraObjInterface *obj) const
{
    return obj->getKey() == _filledTrigger.getKey() ? _filledTrigger.getSharedPtrFor(obj) :
           obj->getKey() == _highTrigger.getKey() ? _highTrigger.getSharedPtrFor(obj) :
           obj->getKey() == _lowTrigger.getKey() ? _lowTrigger.getSharedPtrFor(obj) :
           obj->getKey() == _emptyTrigger.getKey() ? _emptyTrigger.getSharedPtrFor(obj) :
           TerraObject::getSharedPtrFor(obj);
}

float TerraWaterReservoir::getLevel(bool poll)
{
    if (_maxVolume <= FLT_EPSILON) { return 0.0f; }
    return constrain((_waterVolume.getMeasurementValue(poll) / _maxVolume) * 100.0f, 0.0f, 100.0f);
}

Terra_ResourceState TerraWaterReservoir::getState(bool poll)
{
    if (hasFault()) { return Terra_ResourceState_Fault; }
    if (isFilled(poll) || isHigh(poll)) { return Terra_ResourceState_High; }
    if (isEmpty(poll)) { return Terra_ResourceState_Reserve; }
    if (isLow(poll)) { return Terra_ResourceState_Low; }
    return Terra_ResourceState_Normal;
}

bool TerraWaterReservoir::isFilled(bool poll)
{
    return _filledTrigger.isTriggered(poll);
}

bool TerraWaterReservoir::isHigh(bool poll)
{
    return _highTrigger.isTriggered(poll);
}

bool TerraWaterReservoir::isLow(bool poll)
{
    return _lowTrigger.isTriggered(poll);
}

bool TerraWaterReservoir::isEmpty(bool poll)
{
    return _emptyTrigger.isTriggered(poll);
}

void TerraWaterReservoir::setVolumeUnits(Terra_UnitsType volumeUnits)
{
    if (_volumeUnits != volumeUnits) {
        _volumeUnits = volumeUnits;

        _waterVolume.setMeasurementUnits(volumeUnits);
        bumpRevisionIfNeeded();
    }
}

TerraSensorAttachment &TerraWaterReservoir::getWaterVolumeSensorAttachment()
{
    return _waterVolume;
}

TerraSensorAttachment &TerraWaterReservoir::getWaterTemperatureSensorAttachment()
{
    return _waterTemperature;
}

TerraTriggerAttachment &TerraWaterReservoir::getFilledTriggerAttachment()
{
    return _filledTrigger;
}

TerraTriggerAttachment &TerraWaterReservoir::getHighTriggerAttachment()
{
    return _highTrigger;
}

TerraTriggerAttachment &TerraWaterReservoir::getLowTriggerAttachment()
{
    return _lowTrigger;
}

TerraTriggerAttachment &TerraWaterReservoir::getEmptyTriggerAttachment()
{
    return _emptyTrigger;
}

void TerraWaterReservoir::saveToData(TerraData *dataOut)
{
    TerraReservoir::saveToData(dataOut);

    ((TerraWaterReservoirData *)dataOut)->volumeUnits = _volumeUnits;
    ((TerraWaterReservoirData *)dataOut)->maxVolume = _maxVolume;
    if (_waterVolume.isSet()) {
        strncpy(((TerraWaterReservoirData *)dataOut)->volumeSensor, _waterVolume.getKeyString().c_str(), TERRA_NAME_MAXSIZE);
    }
    if (_waterTemperature.isSet()) {
        strncpy(((TerraWaterReservoirData *)dataOut)->waterTemperatureSensor, _waterTemperature.getKeyString().c_str(), TERRA_NAME_MAXSIZE);
    }
    if (_filledTrigger.isSet()) {
        _filledTrigger->saveToData(&(((TerraWaterReservoirData *)dataOut)->filledTrigger));
    }
    if (_highTrigger.isSet()) {
        _highTrigger->saveToData(&(((TerraWaterReservoirData *)dataOut)->highTrigger));
    }
    if (_lowTrigger.isSet()) {
        _lowTrigger->saveToData(&(((TerraWaterReservoirData *)dataOut)->lowTrigger));
    }
    if (_emptyTrigger.isSet()) {
        _emptyTrigger->saveToData(&(((TerraWaterReservoirData *)dataOut)->emptyTrigger));
    }
}

void TerraWaterReservoir::handleFilled(Terra_TriggerState filledState)
{
    TerraReservoir::handleFilled(filledState);
}

void TerraWaterReservoir::handleHigh(Terra_TriggerState highState)
{
    TerraReservoir::handleHigh(highState);
}

void TerraWaterReservoir::handleLow(Terra_TriggerState lowState)
{
    TerraReservoir::handleLow(lowState);
}

void TerraWaterReservoir::handleEmpty(Terra_TriggerState emptyState)
{
    TerraReservoir::handleEmpty(emptyState);
}


TerraThermalReservoir::TerraThermalReservoir(tposi_t reservoirIndex, float maxTemperature, int classTypeIn)
    : TerraReservoir(Terra_ReservoirType_Thermal, reservoirIndex, classTypeIn),
      TerraTemperatureUnitsInterfaceStorage(defaultTemperatureUnits()),
      _maxTemperature(maxTemperature), _temperatureSensor(this),
      _filledTrigger(this), _highTrigger(this), _lowTrigger(this), _emptyTrigger(this)
{
    _temperatureSensor.setMeasurementUnits(getTemperatureUnits());

    _filledTrigger.setHandleMethod(&TerraThermalReservoir::handleFilled, this);
    _highTrigger.setHandleMethod(&TerraThermalReservoir::handleHigh, this);
    _lowTrigger.setHandleMethod(&TerraThermalReservoir::handleLow, this);
    _emptyTrigger.setHandleMethod(&TerraThermalReservoir::handleEmpty, this);
}

TerraThermalReservoir::TerraThermalReservoir(const TerraThermalReservoirData *dataIn)
    : TerraReservoir(dataIn),
      TerraTemperatureUnitsInterfaceStorage(definedUnitsElse(dataIn->temperatureUnits, defaultTemperatureUnits())),
      _maxTemperature(dataIn->maxTemperature), _temperatureSensor(this),
      _filledTrigger(this), _highTrigger(this), _lowTrigger(this), _emptyTrigger(this)
{
    _temperatureSensor.setMeasurementUnits(getTemperatureUnits());
    _temperatureSensor.initObject(dataIn->temperatureSensor);

    _filledTrigger.setHandleMethod(&TerraThermalReservoir::handleFilled, this);
    _filledTrigger.setObject(newTriggerObjectFromSubData(&(dataIn->filledTrigger)));
    TERRA_SOFT_ASSERT(!dataIn->filledTrigger.isSet() || _filledTrigger, SFP(TStr_Err_AllocationFailure));

    _highTrigger.setHandleMethod(&TerraThermalReservoir::handleHigh, this);
    _highTrigger.setObject(newTriggerObjectFromSubData(&(dataIn->highTrigger)));
    TERRA_SOFT_ASSERT(!dataIn->highTrigger.isSet() || _highTrigger, SFP(TStr_Err_AllocationFailure));

    _lowTrigger.setHandleMethod(&TerraThermalReservoir::handleLow, this);
    _lowTrigger.setObject(newTriggerObjectFromSubData(&(dataIn->lowTrigger)));
    TERRA_SOFT_ASSERT(!dataIn->lowTrigger.isSet() || _lowTrigger, SFP(TStr_Err_AllocationFailure));

    _emptyTrigger.setHandleMethod(&TerraThermalReservoir::handleEmpty, this);
    _emptyTrigger.setObject(newTriggerObjectFromSubData(&(dataIn->emptyTrigger)));
    TERRA_SOFT_ASSERT(!dataIn->emptyTrigger.isSet() || _emptyTrigger, SFP(TStr_Err_AllocationFailure));
}

void TerraThermalReservoir::update()
{
    TerraObject::update();

    _temperatureSensor.updateIfNeeded(true);

    _filledTrigger.updateIfNeeded(true);
    _highTrigger.updateIfNeeded(true);
    _lowTrigger.updateIfNeeded(true);
    _emptyTrigger.updateIfNeeded(true);
}

SharedPtr<TerraObjInterface> TerraThermalReservoir::getSharedPtrFor(const TerraObjInterface *obj) const
{
    return obj->getKey() == _filledTrigger.getKey() ? _filledTrigger.getSharedPtrFor(obj) :
           obj->getKey() == _highTrigger.getKey() ? _highTrigger.getSharedPtrFor(obj) :
           obj->getKey() == _lowTrigger.getKey() ? _lowTrigger.getSharedPtrFor(obj) :
           obj->getKey() == _emptyTrigger.getKey() ? _emptyTrigger.getSharedPtrFor(obj) :
           TerraObject::getSharedPtrFor(obj);
}

float TerraThermalReservoir::getLevel(bool poll)
{
    if (_maxTemperature <= FLT_EPSILON) { return 0.0f; }
    return constrain((_temperatureSensor.getMeasurementValue(poll) / _maxTemperature) * 100.0f, 0.0f, 100.0f);
}

Terra_ResourceState TerraThermalReservoir::getState(bool poll)
{
    if (hasFault()) { return Terra_ResourceState_Fault; }
    if (isFilled(poll) || isHigh(poll)) { return Terra_ResourceState_High; }
    if (isEmpty(poll)) { return Terra_ResourceState_Reserve; }
    if (isLow(poll)) { return Terra_ResourceState_Low; }
    return Terra_ResourceState_Normal;
}

bool TerraThermalReservoir::isFilled(bool poll)
{
    return _filledTrigger.isTriggered(poll);
}

bool TerraThermalReservoir::isHigh(bool poll)
{
    return _highTrigger.isTriggered(poll);
}

bool TerraThermalReservoir::isLow(bool poll)
{
    return _lowTrigger.isTriggered(poll);
}

bool TerraThermalReservoir::isEmpty(bool poll)
{
    return _emptyTrigger.isTriggered(poll);
}

void TerraThermalReservoir::setTemperatureUnits(Terra_UnitsType temperatureUnits)
{
    if (_tempUnits != temperatureUnits) {
        _tempUnits = temperatureUnits;

        _temperatureSensor.setMeasurementUnits(temperatureUnits);
        bumpRevisionIfNeeded();
    }
}

TerraSensorAttachment &TerraThermalReservoir::getMediumTemperatureSensorAttachment()
{
    return _temperatureSensor;
}

TerraTriggerAttachment &TerraThermalReservoir::getFilledTriggerAttachment()
{
    return _filledTrigger;
}

TerraTriggerAttachment &TerraThermalReservoir::getHighTriggerAttachment()
{
    return _highTrigger;
}

TerraTriggerAttachment &TerraThermalReservoir::getLowTriggerAttachment()
{
    return _lowTrigger;
}

TerraTriggerAttachment &TerraThermalReservoir::getEmptyTriggerAttachment()
{
    return _emptyTrigger;
}

void TerraThermalReservoir::saveToData(TerraData *dataOut)
{
    TerraReservoir::saveToData(dataOut);

    ((TerraThermalReservoirData *)dataOut)->temperatureUnits = _tempUnits;
    ((TerraThermalReservoirData *)dataOut)->maxTemperature = _maxTemperature;
    if (_temperatureSensor.isSet()) {
        strncpy(((TerraThermalReservoirData *)dataOut)->temperatureSensor, _temperatureSensor.getKeyString().c_str(), TERRA_NAME_MAXSIZE);
    }
    if (_filledTrigger.isSet()) {
        _filledTrigger->saveToData(&(((TerraThermalReservoirData *)dataOut)->filledTrigger));
    }
    if (_highTrigger.isSet()) {
        _highTrigger->saveToData(&(((TerraThermalReservoirData *)dataOut)->highTrigger));
    }
    if (_lowTrigger.isSet()) {
        _lowTrigger->saveToData(&(((TerraThermalReservoirData *)dataOut)->lowTrigger));
    }
    if (_emptyTrigger.isSet()) {
        _emptyTrigger->saveToData(&(((TerraThermalReservoirData *)dataOut)->emptyTrigger));
    }
}

void TerraThermalReservoir::handleFilled(Terra_TriggerState filledState)
{
    TerraReservoir::handleFilled(filledState);
}

void TerraThermalReservoir::handleHigh(Terra_TriggerState highState)
{
    TerraReservoir::handleHigh(highState);
}

void TerraThermalReservoir::handleLow(Terra_TriggerState lowState)
{
    TerraReservoir::handleLow(lowState);
}

void TerraThermalReservoir::handleEmpty(Terra_TriggerState emptyState)
{
    TerraReservoir::handleEmpty(emptyState);
}


TerraInfiniteWaterReservoir::TerraInfiniteWaterReservoir(tposi_t reservoirIndex, bool alwaysFilled)
    : TerraWaterReservoir(reservoirIndex, 0.0f, WaterPipe), _alwaysFilled(alwaysFilled)
{ ; }

TerraInfiniteWaterReservoir::TerraInfiniteWaterReservoir(const TerraInfiniteWaterReservoirData *dataIn)
    : TerraWaterReservoir(dataIn), _alwaysFilled(dataIn->alwaysFilled)
{ ; }

void TerraInfiniteWaterReservoir::update()
{
    TerraObject::update();

    handleFilled(triggerStateFromBool(isFilled()));
    handleHigh(triggerStateFromBool(false));
    handleLow(triggerStateFromBool(false));
    handleEmpty(triggerStateFromBool(isEmpty()));
}

float TerraInfiniteWaterReservoir::getLevel(bool poll)
{
    (void)poll;
    return FLT_UNDEF;
}

bool TerraInfiniteWaterReservoir::isFilled(bool poll)
{
    (void)poll;
    return _alwaysFilled;
}

bool TerraInfiniteWaterReservoir::isHigh(bool poll)
{
    (void)poll;
    return false;
}

bool TerraInfiniteWaterReservoir::isLow(bool poll)
{
    (void)poll;
    return false;
}

bool TerraInfiniteWaterReservoir::isEmpty(bool poll)
{
    (void)poll;
    return !_alwaysFilled;
}

void TerraInfiniteWaterReservoir::saveToData(TerraData *dataOut)
{
    TerraWaterReservoir::saveToData(dataOut);

    ((TerraInfiniteWaterReservoirData *)dataOut)->alwaysFilled = _alwaysFilled;
}


TerraInfiniteThermalReservoir::TerraInfiniteThermalReservoir(tposi_t reservoirIndex, bool alwaysFilled)
    : TerraThermalReservoir(reservoirIndex, 0.0f, ThermalPipe), _alwaysFilled(alwaysFilled)
{ ; }

TerraInfiniteThermalReservoir::TerraInfiniteThermalReservoir(const TerraInfiniteThermalReservoirData *dataIn)
    : TerraThermalReservoir(dataIn), _alwaysFilled(dataIn->alwaysFilled)
{ ; }

void TerraInfiniteThermalReservoir::update()
{
    TerraObject::update();

    handleFilled(triggerStateFromBool(isFilled()));
    handleHigh(triggerStateFromBool(false));
    handleLow(triggerStateFromBool(false));
    handleEmpty(triggerStateFromBool(isEmpty()));
}

float TerraInfiniteThermalReservoir::getLevel(bool poll)
{
    (void)poll;
    return FLT_UNDEF;
}

bool TerraInfiniteThermalReservoir::isFilled(bool poll)
{
    (void)poll;
    return _alwaysFilled;
}

bool TerraInfiniteThermalReservoir::isHigh(bool poll)
{
    (void)poll;
    return false;
}

bool TerraInfiniteThermalReservoir::isLow(bool poll)
{
    (void)poll;
    return false;
}

bool TerraInfiniteThermalReservoir::isEmpty(bool poll)
{
    (void)poll;
    return !_alwaysFilled;
}

void TerraInfiniteThermalReservoir::saveToData(TerraData *dataOut)
{
    TerraThermalReservoir::saveToData(dataOut);

    ((TerraInfiniteThermalReservoirData *)dataOut)->alwaysFilled = _alwaysFilled;
}


TerraReservoirData::TerraReservoirData()
    : TerraObjectData()
{
    _size = sizeof(*this);
    id.object.idType = (tid_t)Terra_ObjectType_Reservoir;
    id.object.objType = (tid_t)Terra_ReservoirType_Undefined;
    id.object.posIndex = TERRA_POS_SEARCH_FROMBEG;
    id.object.classType = (tid_t)TerraReservoir::Unknown;
}

void TerraReservoirData::toJSONObject(JsonObject &objectOut) const
{
    TerraObjectData::toJSONObject(objectOut);

    if (filledTrigger.isSet()) {
        JsonObject filledTriggerObj = objectOut.createNestedObject(SFP(TStr_Key_FilledTrigger));
        filledTrigger.toJSONObject(filledTriggerObj);
    }
    if (highTrigger.isSet()) {
        JsonObject highTriggerObj = objectOut.createNestedObject(SFP(TStr_Key_HighTrigger));
        highTrigger.toJSONObject(highTriggerObj);
    }
    if (lowTrigger.isSet()) {
        JsonObject lowTriggerObj = objectOut.createNestedObject(SFP(TStr_Key_LowTrigger));
        lowTrigger.toJSONObject(lowTriggerObj);
    }
    if (emptyTrigger.isSet()) {
        JsonObject emptyTriggerObj = objectOut.createNestedObject(SFP(TStr_Key_EmptyTrigger));
        emptyTrigger.toJSONObject(emptyTriggerObj);
    }
}

void TerraReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraObjectData::fromJSONObject(objectIn);

    JsonObjectConst filledTriggerObj = objectIn[SFP(TStr_Key_FilledTrigger)];
    if (!filledTriggerObj.isNull()) { filledTrigger.fromJSONObject(filledTriggerObj); }
    JsonObjectConst highTriggerObj = objectIn[SFP(TStr_Key_HighTrigger)];
    if (!highTriggerObj.isNull()) { highTrigger.fromJSONObject(highTriggerObj); }
    JsonObjectConst lowTriggerObj = objectIn[SFP(TStr_Key_LowTrigger)];
    if (!lowTriggerObj.isNull()) { lowTrigger.fromJSONObject(lowTriggerObj); }
    JsonObjectConst emptyTriggerObj = objectIn[SFP(TStr_Key_EmptyTrigger)];
    if (!emptyTriggerObj.isNull()) { emptyTrigger.fromJSONObject(emptyTriggerObj); }
}


TerraWaterReservoirData::TerraWaterReservoirData()
    : TerraReservoirData(), volumeUnits(Terra_UnitsType_Undefined), maxVolume(0.0f), volumeSensor{0}, waterTemperatureSensor{0}
{
    _size = sizeof(*this);
    id.object.objType = (tid_t)Terra_ReservoirType_Water;
    id.object.classType = (tid_t)TerraReservoir::Water;
}

void TerraWaterReservoirData::toJSONObject(JsonObject &objectOut) const
{
    TerraReservoirData::toJSONObject(objectOut);

    if (volumeUnits != Terra_UnitsType_Undefined) { objectOut[SFP(TStr_Key_VolumeUnits)] = unitsTypeToSymbol(volumeUnits); }
    objectOut[SFP(TStr_Key_MaxVolume)] = maxVolume;
    if (volumeSensor[0]) { objectOut[SFP(TStr_Key_VolumeSensor)] = charsToString(volumeSensor, TERRA_NAME_MAXSIZE); }
    if (waterTemperatureSensor[0]) { objectOut[SFP(TStr_Key_WaterTemperatureSensor)] = charsToString(waterTemperatureSensor, TERRA_NAME_MAXSIZE); }
}

void TerraWaterReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraReservoirData::fromJSONObject(objectIn);

    volumeUnits = unitsTypeFromSymbol(objectIn[SFP(TStr_Key_VolumeUnits)]);
    maxVolume = objectIn[SFP(TStr_Key_MaxVolume)] | maxVolume;
    const char *volumeSensorStr = objectIn[SFP(TStr_Key_VolumeSensor)];
    if (volumeSensorStr && volumeSensorStr[0]) { strncpy(volumeSensor, volumeSensorStr, TERRA_NAME_MAXSIZE); }
    const char *waterTemperatureSensorStr = objectIn[SFP(TStr_Key_WaterTemperatureSensor)];
    if (waterTemperatureSensorStr && waterTemperatureSensorStr[0]) { strncpy(waterTemperatureSensor, waterTemperatureSensorStr, TERRA_NAME_MAXSIZE); }
}


TerraThermalReservoirData::TerraThermalReservoirData()
    : TerraReservoirData(), temperatureUnits(Terra_UnitsType_Undefined), maxTemperature(0.0f), temperatureSensor{0}
{
    _size = sizeof(*this);
    id.object.objType = (tid_t)Terra_ReservoirType_Thermal;
    id.object.classType = (tid_t)TerraReservoir::Thermal;
}

void TerraThermalReservoirData::toJSONObject(JsonObject &objectOut) const
{
    TerraReservoirData::toJSONObject(objectOut);

    if (temperatureUnits != Terra_UnitsType_Undefined) { objectOut[SFP(TStr_Key_TemperatureUnits)] = unitsTypeToSymbol(temperatureUnits); }
    objectOut[SFP(TStr_Key_MaxTemperature)] = maxTemperature;
    if (temperatureSensor[0]) { objectOut[SFP(TStr_Key_TemperatureSensor)] = charsToString(temperatureSensor, TERRA_NAME_MAXSIZE); }
}

void TerraThermalReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraReservoirData::fromJSONObject(objectIn);

    temperatureUnits = unitsTypeFromSymbol(objectIn[SFP(TStr_Key_TemperatureUnits)]);
    maxTemperature = objectIn[SFP(TStr_Key_MaxTemperature)] | maxTemperature;
    const char *temperatureSensorStr = objectIn[SFP(TStr_Key_TemperatureSensor)];
    if (temperatureSensorStr && temperatureSensorStr[0]) { strncpy(temperatureSensor, temperatureSensorStr, TERRA_NAME_MAXSIZE); }
}


TerraInfiniteWaterReservoirData::TerraInfiniteWaterReservoirData()
    : TerraWaterReservoirData(), alwaysFilled(true)
{
    _size = sizeof(*this);
    id.object.classType = (tid_t)TerraReservoir::WaterPipe;
}

void TerraInfiniteWaterReservoirData::toJSONObject(JsonObject &objectOut) const
{
    TerraWaterReservoirData::toJSONObject(objectOut);

    objectOut[SFP(TStr_Key_AlwaysFilled)] = alwaysFilled;
}

void TerraInfiniteWaterReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraWaterReservoirData::fromJSONObject(objectIn);

    alwaysFilled = objectIn[SFP(TStr_Key_AlwaysFilled)] | alwaysFilled;
}


TerraInfiniteThermalReservoirData::TerraInfiniteThermalReservoirData()
    : TerraThermalReservoirData(), alwaysFilled(true)
{
    _size = sizeof(*this);
    id.object.classType = (tid_t)TerraReservoir::ThermalPipe;
}

void TerraInfiniteThermalReservoirData::toJSONObject(JsonObject &objectOut) const
{
    TerraThermalReservoirData::toJSONObject(objectOut);

    objectOut[SFP(TStr_Key_AlwaysFilled)] = alwaysFilled;
}

void TerraInfiniteThermalReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraThermalReservoirData::fromJSONObject(objectIn);

    alwaysFilled = objectIn[SFP(TStr_Key_AlwaysFilled)] | alwaysFilled;
}
