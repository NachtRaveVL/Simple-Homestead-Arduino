/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Power Rails
*/

#include "Terraduino.h"

TerraRail *newRailObjectFromData(const TerraRailData *dataIn)
{
    if (dataIn && !isValidType(dataIn->id.object.idType)) return nullptr;
    TERRA_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(TStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData()) {
        switch (dataIn->id.object.classType) {
            case (tid_t)TerraRail::Simple:
                return new TerraSimpleRail((const TerraSimpleRailData *)dataIn);
            case (tid_t)TerraRail::Regulated:
                return new TerraRegulatedRail((const TerraRegulatedRailData *)dataIn);
            default: break;
        }
    }

    return nullptr;
}


TerraRail::TerraRail(Terra_RailType railType, tposi_t railIndex, int classTypeIn)
    : TerraObject(TerraIdentity(railType, railIndex)), TerraPowerUnitsInterfaceStorage(defaultPowerUnits()),
      classType(static_cast<decltype(Simple)>(classTypeIn)), _limitState(Terra_TriggerState_Undefined)
{
    allocateLinkages(TERRA_RAILS_LINKS_BASESIZE);
}

TerraRail::TerraRail(const TerraRailData *dataIn)
    : TerraObject(dataIn), TerraPowerUnitsInterfaceStorage(definedUnitsElse(dataIn->powerUnits, defaultPowerUnits())),
      classType(static_cast<decltype(Simple)>(dataIn->id.object.classType)), _limitState(Terra_TriggerState_Undefined)
{
    allocateLinkages(TERRA_RAILS_LINKS_BASESIZE);
}

TerraRail::~TerraRail()
{
    if (_links) {
        auto actuators = linksFilterActuators(getLinkages());
        for (auto iter = actuators.begin(); iter != actuators.end(); ++iter) { removeLinkage(*iter); }
    }
}

void TerraRail::update()
{
    TerraObject::update();

    handleLimit(triggerStateFromBool(getCapacity(true) >= 1.0f - FLT_EPSILON));
}

bool TerraRail::addLinkage(TerraObject *object)
{
    if (TerraObject::addLinkage(object)) {
        if (object->isActuatorType()) {
            TERRA_HARD_ASSERT(isSimpleClass() || isRegulatedClass(), SFP(TStr_Err_OperationFailure));
            if (isSimpleClass()) {
                auto methodSlot = MethodSlot<TerraSimpleRail, TerraActuator *>((TerraSimpleRail *)this, &TerraSimpleRail::handleActivation);
                ((TerraActuator *)object)->getActivationSignal().attach(methodSlot);
            } else if (isRegulatedClass()) {
                auto methodSlot = MethodSlot<TerraRegulatedRail, TerraActuator *>((TerraRegulatedRail *)this, &TerraRegulatedRail::handleActivation);
                ((TerraActuator *)object)->getActivationSignal().attach(methodSlot);
            }
        }
        return true;
    }
    return false;
}

bool TerraRail::removeLinkage(TerraObject *object)
{
    if (TerraObject::removeLinkage(object)) {
        if (((TerraObject *)object)->isActuatorType()) {
            TERRA_HARD_ASSERT(isSimpleClass() || isRegulatedClass(), SFP(TStr_Err_OperationFailure));
            if (isSimpleClass()) {
                auto methodSlot = MethodSlot<TerraSimpleRail, TerraActuator *>((TerraSimpleRail *)this, &TerraSimpleRail::handleActivation);
                ((TerraActuator *)object)->getActivationSignal().detach(methodSlot);
            } else if (isRegulatedClass()) {
                auto methodSlot = MethodSlot<TerraRegulatedRail, TerraActuator *>((TerraRegulatedRail *)this, &TerraRegulatedRail::handleActivation);
                ((TerraActuator *)object)->getActivationSignal().detach(methodSlot);
            }
        }
        return true;
    }
    return false;
}

Signal<TerraRail *, TERRA_RAIL_SIGNAL_SLOTS> &TerraRail::getCapacitySignal()
{
    return _capacitySignal;
}

TerraData *TerraRail::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void TerraRail::saveToData(TerraData *dataOut)
{
    TerraObject::saveToData(dataOut);

    dataOut->id.object.classType = (int8_t)classType;

    ((TerraRailData *)dataOut)->powerUnits = _powerUnits;
}

void TerraRail::handleLimit(Terra_TriggerState limitState)
{
    if (limitState == Terra_TriggerState_Disabled || limitState == Terra_TriggerState_Undefined) { return; }

    if (_limitState != limitState) {
        _limitState = limitState;

        if (_limitState == Terra_TriggerState_NotTriggered) {
            #ifdef TERRA_USE_MULTITASKING
                scheduleSignalFireOnce<TerraRail *>(getSharedPtr(), _capacitySignal, this);
            #else
                _capacitySignal.fire(this);
            #endif
        }
    }
}


TerraSimpleRail::TerraSimpleRail(Terra_RailType railType, tposi_t railIndex, int maxActiveAtOnce, int classType)
    : TerraRail(railType, railIndex, classType), _activeCount(0), _maxActiveAtOnce(maxActiveAtOnce)
{ ; }

TerraSimpleRail::TerraSimpleRail(const TerraSimpleRailData *dataIn)
    : TerraRail(dataIn), _activeCount(0), _maxActiveAtOnce(dataIn->maxActiveAtOnce)
{ ; }

bool TerraSimpleRail::canActivate(TerraActuator *actuator)
{
    (void)actuator;
    return _activeCount < _maxActiveAtOnce;
}

float TerraSimpleRail::getCapacity(bool poll)
{
    (void)poll;
    return _activeCount / (float)_maxActiveAtOnce;
}

void TerraSimpleRail::setPowerUnits(Terra_UnitsType powerUnits)
{
    if (_powerUnits != powerUnits) {
        _powerUnits = powerUnits;
    }
}

void TerraSimpleRail::saveToData(TerraData *dataOut)
{
    TerraRail::saveToData(dataOut);

    ((TerraSimpleRailData *)dataOut)->maxActiveAtOnce = _maxActiveAtOnce;
}

void TerraSimpleRail::handleActivation(TerraActuator *actuator)
{
    bool activeCountBefore = _activeCount;

    if (actuator->isEnabled()) {
        _activeCount++;
    } else {
        _activeCount--;
    }

    if (_activeCount < activeCountBefore) {
        #ifdef TERRA_USE_MULTITASKING
            scheduleSignalFireOnce<TerraRail *>(getSharedPtr(), _capacitySignal, this);
        #else
            _capacitySignal.fire(this);
        #endif
    }
}


TerraRegulatedRail::TerraRegulatedRail(Terra_RailType railType, tposi_t railIndex, float maxPower, int classType)
    : TerraRail(railType, railIndex, classType), _maxPower(maxPower), _powerUsage(this), _limitTrigger(this)
{
    _powerUsage.setMeasurementUnits(getPowerUnits(), getRailVoltage());
    _powerUsage.setHandleMethod(&TerraRegulatedRail::handlePower, this);

    _limitTrigger.setHandleMethod(&TerraRail::handleLimit, this);
}

TerraRegulatedRail::TerraRegulatedRail(const TerraRegulatedRailData *dataIn)
    : TerraRail(dataIn),
      _maxPower(dataIn->maxPower),
      _powerUsage(this), _limitTrigger(this)
{
    _powerUsage.setMeasurementUnits(TerraRail::getPowerUnits(), getRailVoltage());
    _powerUsage.setHandleMethod(&TerraRegulatedRail::handlePower, this);
    _powerUsage.initObject(dataIn->powerUsageSensor);

    _limitTrigger.setHandleMethod(&TerraRail::handleLimit, this);
    _limitTrigger.setObject(newTriggerObjectFromSubData(&(dataIn->limitTrigger)));
    TERRA_SOFT_ASSERT(_limitTrigger, SFP(TStr_Err_AllocationFailure));
}

void TerraRegulatedRail::update()
{
    TerraRail::update();

    _powerUsage.updateIfNeeded(true);

    _limitTrigger.updateIfNeeded();
}

SharedPtr<TerraObjInterface> TerraRegulatedRail::getSharedPtrFor(const TerraObjInterface *obj) const
{
    return obj->getKey() == _limitTrigger.getKey() ? _limitTrigger.getSharedPtrFor(obj) :
           TerraObject::getSharedPtrFor(obj);
}

bool TerraRegulatedRail::canActivate(TerraActuator *actuator)
{
    if (_limitTrigger.isTriggered()) { return false; }
    TerraSingleMeasurement powerReq = actuator->getContinuousPowerUsage().asUnits(getPowerUnits(), getRailVoltage());
    return _powerUsage.getMeasurementValue(true) + powerReq.value < (TERRA_RAILS_FRACTION_SATURATED * _maxPower) - FLT_EPSILON;
}

float TerraRegulatedRail::getCapacity(bool poll)
{
    if (_limitTrigger.isTriggered(poll)) { return 1.0f; }
    return _powerUsage.getMeasurementValue(poll) / (TERRA_RAILS_FRACTION_SATURATED * _maxPower);
}

void TerraRegulatedRail::setPowerUnits(Terra_UnitsType powerUnits)
{
    if (_powerUnits != powerUnits) {
        _powerUnits = powerUnits;

        _powerUsage.setMeasurementUnits(getPowerUnits(), getRailVoltage());
        bumpRevisionIfNeeded();
    }
}

TerraSensorAttachment &TerraRegulatedRail::getPowerUsageSensorAttachment()
{
    return _powerUsage;
}

TerraTriggerAttachment &TerraRegulatedRail::getLimitTriggerAttachment()
{
    return _limitTrigger;
}

void TerraRegulatedRail::saveToData(TerraData *dataOut)
{
    TerraRail::saveToData(dataOut);

    ((TerraRegulatedRailData *)dataOut)->maxPower = roundForExport(_maxPower, 1);
    if (_powerUsage.isSet()) {
        strncpy(((TerraRegulatedRailData *)dataOut)->powerUsageSensor, _powerUsage.getKeyString().c_str(), TERRA_NAME_MAXSIZE);
    }
    if (_limitTrigger.isSet()) {
        _limitTrigger->saveToData(&(((TerraRegulatedRailData *)dataOut)->limitTrigger));
    }
}

void TerraRegulatedRail::handleActivation(TerraActuator *actuator)
{
    if (!getPowerUsageSensor(true) && actuator) {
        auto powerReq = actuator->getContinuousPowerUsage().asUnits(getPowerUnits(), getRailVoltage());
        auto powerUsage = getPowerUsageSensorAttachment().getMeasurement(true);
        bool enabled = actuator->isEnabled();

        if (enabled) {
            powerUsage.value += powerReq.value;
        } else {
            powerUsage.value -= powerReq.value;
        }

        getPowerUsageSensorAttachment().setMeasurement(powerUsage);

        if (!enabled) {
            #ifdef TERRA_USE_MULTITASKING
                scheduleSignalFireOnce<TerraRail *>(getSharedPtr(), _capacitySignal, this);
            #else
                _capacitySignal.fire(this);
            #endif
        }
    }
}

void TerraRegulatedRail::handlePower(const TerraMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        float capacityBefore = getCapacity();

        getPowerUsageSensorAttachment().setMeasurement(getAsSingleMeasurement(measurement, _powerUsage.getMeasurementRow(), _maxPower, getPowerUnits()));

        if (getCapacity() < capacityBefore - FLT_EPSILON) {
            #ifdef TERRA_USE_MULTITASKING
                scheduleSignalFireOnce<TerraRail *>(getSharedPtr(), _capacitySignal, this);
            #else
                _capacitySignal.fire(this);
            #endif
        }
    }
}


TerraRailData::TerraRailData()
    : TerraObjectData(), powerUnits(Terra_UnitsType_Undefined)
{
    _size = sizeof(*this);
}

void TerraRailData::toJSONObject(JsonObject &objectOut) const
{
    TerraObjectData::toJSONObject(objectOut);

    if (powerUnits != Terra_UnitsType_Undefined) { objectOut[SFP(TStr_Key_PowerUnits)] = unitsTypeToSymbol(powerUnits); }
}

void TerraRailData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraObjectData::fromJSONObject(objectIn);

    powerUnits = unitsTypeFromSymbol(objectIn[SFP(TStr_Key_PowerUnits)]);
}

TerraSimpleRailData::TerraSimpleRailData()
    : TerraRailData(), maxActiveAtOnce(2)
{
    _size = sizeof(*this);
}

void TerraSimpleRailData::toJSONObject(JsonObject &objectOut) const
{
    TerraRailData::toJSONObject(objectOut);

    if (maxActiveAtOnce != 2) { objectOut[SFP(TStr_Key_MaxActiveAtOnce)] = maxActiveAtOnce; }
}

void TerraSimpleRailData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraRailData::fromJSONObject(objectIn);

    maxActiveAtOnce = objectIn[SFP(TStr_Key_MaxActiveAtOnce)] | maxActiveAtOnce;
}

TerraRegulatedRailData::TerraRegulatedRailData()
    : TerraRailData(), maxPower(0), powerUsageSensor{0}, limitTrigger()
{
    _size = sizeof(*this);
}

void TerraRegulatedRailData::toJSONObject(JsonObject &objectOut) const
{
    TerraRailData::toJSONObject(objectOut);

    objectOut[SFP(TStr_Key_MaxPower)] = maxPower;
    if (powerUsageSensor[0]) { objectOut[SFP(TStr_Key_PowerUsageSensor)] = charsToString(powerUsageSensor, TERRA_NAME_MAXSIZE); }
    if (isValidType(limitTrigger.type)) {
        JsonObject limitTriggerObj = objectOut.createNestedObject(SFP(TStr_Key_LimitTrigger));
        limitTrigger.toJSONObject(limitTriggerObj);
    }
}

void TerraRegulatedRailData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraRailData::fromJSONObject(objectIn);

    maxPower = objectIn[SFP(TStr_Key_MaxPower)] | maxPower;
    const char *powerUsageSensorStr = objectIn[SFP(TStr_Key_PowerUsageSensor)];
    if (powerUsageSensorStr && powerUsageSensorStr[0]) { strncpy(powerUsageSensor, powerUsageSensorStr, TERRA_NAME_MAXSIZE); }
    JsonObjectConst limitTriggerObj = objectIn[SFP(TStr_Key_LimitTrigger)];
    if (!limitTriggerObj.isNull()) { limitTrigger.fromJSONObject(limitTriggerObj); }
}
