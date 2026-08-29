/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Attachment Points
*/

#include "Terraduino.h"
#include "TerraCoreLogic.h"

TerraDLinkObject::TerraDLinkObject()
    : _key(tkey_none), _obj(nullptr), _keyStr(nullptr)
{ ; }

TerraDLinkObject::TerraDLinkObject(const TerraDLinkObject &obj)
    : _key(obj._key), _obj(obj._obj), _keyStr(nullptr)
{
    if (obj._keyStr) {
        auto len = strnlen(obj._keyStr, TERRA_NAME_MAXSIZE);
        if (len) {
            _keyStr = (const char *)malloc(len + 1);
            strncpy((char *)_keyStr, obj._keyStr, len + 1);
        }
    }
}

TerraDLinkObject::~TerraDLinkObject()
{
    if (_keyStr) { free((void *)_keyStr); }
}

void TerraDLinkObject::unresolve()
{
    if (_obj && !_keyStr) {
        auto id = _obj->getId();
        auto len = id.keyString.length();
        if (len) {
            _keyStr = (const char *)malloc(len + 1);
            strncpy((char *)_keyStr, id.keyString.c_str(), len + 1);
        }
    }
    TERRA_HARD_ASSERT(!_obj || _key == _obj->getKey(), SFP(TStr_Err_OperationFailure));
    _obj = nullptr;
}

SharedPtr<TerraObjInterface> TerraDLinkObject::resolveObject()
{
    if (_obj || !isSet()) { return _obj; }
    if (Terraduino::_activeInstance) {
        _obj = static_pointer_cast<TerraObjInterface>(Terraduino::_activeInstance->_objects[_key]);
    }
    if (_obj && _keyStr) {
        free((void *)_keyStr); _keyStr = nullptr;
    }
    return _obj;
}


TerraAttachment::TerraAttachment(TerraObjInterface *parent, tposi_t subIndex)
    : TerraSubObject(parent), _obj(), _subIndex(subIndex)
{ ; }

TerraAttachment::TerraAttachment(const TerraAttachment &attachment)
    : TerraSubObject(attachment._parent), _obj(), _subIndex(attachment._subIndex)
{
    initObject(attachment._obj);
}

TerraAttachment::~TerraAttachment()
{
    if (isResolved() && _obj->isObject() && _parent && _parent->isObject()) {
        _obj.get<TerraObject>()->removeLinkage((TerraObject *)_parent);
    }
}

void TerraAttachment::attachObject()
{
    if (resolve() && _obj->isObject() && _parent && _parent->isObject()) { // purposeful resolve in front
        _obj.get<TerraObject>()->addLinkage((TerraObject *)_parent);
    }
}

void TerraAttachment::detachObject()
{
    if (isResolved() && _obj->isObject() && _parent && _parent->isObject()) {
        _obj.get<TerraObject>()->removeLinkage((TerraObject *)_parent);
    }
    // note: used to set _obj to nullptr here, but found that it's best not to -> avoids additional operator= calls during typical detach scenarios
}

void TerraAttachment::updateIfNeeded(bool poll)
{
    // intended to be overridden by derived classes, but not an error if left not implemented
}

void TerraAttachment::setParent(TerraObjInterface *parent)
{
    if (_parent != parent) {
        if (isResolved() && _obj->isObject() && _parent && _parent->isObject()) { _obj.get<TerraObject>()->removeLinkage((TerraObject *)_parent); }

        _parent = parent;

        if (isResolved() && _obj->isObject() && _parent && _parent->isObject()) { _obj.get<TerraObject>()->addLinkage((TerraObject *)_parent); }
    }
}

SharedPtr<TerraObjInterface> TerraAttachment::getSharedPtrFor(const TerraObjInterface *obj) const
{
    return obj->getKey() == getKey() ? _obj._obj : TerraSubObject::getSharedPtrFor(obj);
}


TerraActuatorAttachment::TerraActuatorAttachment(TerraObjInterface *parent, tposi_t subIndex)
    : TerraSignalAttachment<TerraActuator *, TERRA_ACTUATOR_SIGNAL_SLOTS>(parent, subIndex, &TerraActuator::getActivationSignal),
       _actHandle(), _actSetup(), _updateSlot(nullptr), _rateMultiplier(1.0f), _calledLastUpdate(false)
{ ; }

TerraActuatorAttachment::TerraActuatorAttachment(const TerraActuatorAttachment &attachment)
    : TerraSignalAttachment<TerraActuator *, TERRA_ACTUATOR_SIGNAL_SLOTS>(attachment),
      _actHandle(attachment._actHandle), _actSetup(attachment._actSetup),
      _updateSlot(attachment._updateSlot ? attachment._updateSlot->clone() : nullptr),
      _rateMultiplier(attachment._rateMultiplier), _calledLastUpdate(false)
{ ; }

TerraActuatorAttachment::~TerraActuatorAttachment()
{
    if (_updateSlot) { delete _updateSlot; _updateSlot = nullptr; }
}

void TerraActuatorAttachment::updateIfNeeded(bool poll)
{
    if (_actHandle.isValid()) {
        if (isActivated()) {
            _actHandle.elapseTo();
            if (_updateSlot) { (*_updateSlot)(this); }
            _calledLastUpdate = _actHandle.isDone();
        } else if (_actHandle.isDone() && !_calledLastUpdate) {
            if (_updateSlot) { (*_updateSlot)(this); }
            _calledLastUpdate = true;
        }
    }
}

void TerraActuatorAttachment::setupActivation(float value, millis_t duration, bool force)
{
    if (resolve()) {
        value = get()->calibrationInvTransform(value);

        if (get()->isBidirectionalType()) {
            // Keep direction selection tolerant of floating-point noise around a stopped command.
            setupActivation(TerraActivation(value > FLT_EPSILON ? Terra_DirectionMode_Forward :
                                            value < -FLT_EPSILON ? Terra_DirectionMode_Reverse : Terra_DirectionMode_Stop,
                                            fabsf(value), duration, (force ? Terra_ActivationFlags_Forced : Terra_ActivationFlags_None)));
            return;
        }
    }

    setupActivation(TerraActivation(Terra_DirectionMode_Forward, value, duration, (force ? Terra_ActivationFlags_Forced : Terra_ActivationFlags_None)));
}

void TerraActuatorAttachment::enableActivation()
{
    if (!_actHandle.actuator && _actSetup.isValid() && resolve()) {
        if (_actHandle.isDone()) { applySetup(); } // repeats existing setup
        _calledLastUpdate = false;
        _actHandle = getObject();
    }
}

void TerraActuatorAttachment::setUpdateSlot(const Slot<TerraActuatorAttachment *> &updateSlot)
{
    if (!_updateSlot || !_updateSlot->operator==(&updateSlot)) {
        if (_updateSlot) { delete _updateSlot; _updateSlot = nullptr; }
        _updateSlot = updateSlot.clone();
    }
}

void TerraActuatorAttachment::applySetup()
{
    if (_actSetup.isValid()) {
        if (isFPEqual(_rateMultiplier, 1.0f)) {
            _actHandle.activation = _actSetup;
        } else {
            _actHandle.activation.direction = _actSetup.direction;
            _actHandle.activation.flags = _actSetup.flags;

            if (resolve() && get()->isAnyBinaryClass()) { // Duration based change for rate multiplier
                _actHandle.activation.intensity = _actSetup.intensity;
                if (!_actHandle.isUntimed()) {
                    _actHandle.activation.duration = _actSetup.duration * _rateMultiplier;
                } else { // cannot directly use rate multiplier
                    _actHandle.activation.duration = _actSetup.duration;
                }
            } else { // Intensity based change for rate multiplier
                _actHandle.activation.intensity = _actSetup.intensity * _rateMultiplier;
                _actHandle.activation.duration = _actSetup.duration;
            }
        }

        if (isActivated() && resolve()) { get()->setNeedsUpdate(); }
    }
}


TerraSensorAttachment::TerraSensorAttachment(TerraObjInterface *parent, tposi_t subIndex, uint8_t measurementRow)
    : TerraSignalAttachment<const TerraMeasurement *, TERRA_SENSOR_SIGNAL_SLOTS>(parent, subIndex, &TerraSensor::getMeasurementSignal),
      _measurementRow(measurementRow), _convertParam(FLT_UNDEF), _needsMeasurement(true)
{
    setHandleMethod(&TerraSensorAttachment::handleMeasurement, this);
}

TerraSensorAttachment::TerraSensorAttachment(const TerraSensorAttachment &attachment)
    : TerraSignalAttachment<const TerraMeasurement *, TERRA_SENSOR_SIGNAL_SLOTS>(attachment),
      _measurement(attachment._measurement), _measurementRow(attachment._measurementRow),
      _convertParam(attachment._convertParam), _needsMeasurement(attachment._needsMeasurement)
{
    setHandleSlot(*attachment._handleSlot);
}

TerraSensorAttachment::~TerraSensorAttachment()
{ ; }

void TerraSensorAttachment::attachObject()
{
    TerraSignalAttachment<const TerraMeasurement *, TERRA_SENSOR_SIGNAL_SLOTS>::attachObject();

    if (_handleSlot) { (*_handleSlot)(get()->getMeasurement()); }
    else { handleMeasurement(get()->getMeasurement()); }
}

void TerraSensorAttachment::detachObject()
{
    TerraSignalAttachment<const TerraMeasurement *, TERRA_SENSOR_SIGNAL_SLOTS>::detachObject();

    setNeedsMeasurement();
}

void TerraSensorAttachment::updateIfNeeded(bool poll)
{
    if ((poll || _needsMeasurement) && resolve()) {
        if (_handleSlot) { (*_handleSlot)(get()->getMeasurement()); }
        else { handleMeasurement(get()->getMeasurement()); }

        get()->takeMeasurement((poll || _needsMeasurement)); // purposeful recheck
    }
}

void TerraSensorAttachment::setMeasurement(TerraSingleMeasurement measurement)
{
    auto outUnits = definedUnitsElse(getMeasurementUnits(), measurement.units);
    _measurement = measurement;
    _measurement.updateFrame(1);

    convertUnits(&_measurement, outUnits, _convertParam);
    _needsMeasurement = false;
}

void TerraSensorAttachment::setMeasurementRow(uint8_t measurementRow)
{
    if (_measurementRow != measurementRow) {
        _measurementRow = measurementRow;

        setNeedsMeasurement();
    }
}

void TerraSensorAttachment::setMeasurementUnits(Terra_UnitsType units, float convertParam)
{
    if (_measurement.units != units || !isFPEqual(_convertParam, convertParam)) {
        _convertParam = convertParam;
        convertUnits(&_measurement, units, _convertParam);

        setNeedsMeasurement();
    }
}

void TerraSensorAttachment::handleMeasurement(const TerraMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        setMeasurement(getAsSingleMeasurement(measurement, _measurementRow));
    }
}


TerraTriggerAttachment::TerraTriggerAttachment(TerraObjInterface *parent, tposi_t subIndex)
    : TerraSignalAttachment<Terra_TriggerState, TERRA_TRIGGER_SIGNAL_SLOTS>(parent, subIndex, &TerraTrigger::getTriggerSignal)
{ ; }

TerraTriggerAttachment::TerraTriggerAttachment(const TerraTriggerAttachment &attachment)
    : TerraSignalAttachment<Terra_TriggerState, TERRA_TRIGGER_SIGNAL_SLOTS>(attachment)
{ ; }

TerraTriggerAttachment::~TerraTriggerAttachment()
{ ; }

void TerraTriggerAttachment::updateIfNeeded(bool poll)
{
    if (poll && resolve()) { get()->update(); }
}


TerraDriverAttachment::TerraDriverAttachment(TerraObjInterface *parent, tposi_t subIndex)
    : TerraSignalAttachment<Terra_DrivingState, TERRA_DRIVER_SIGNAL_SLOTS>(parent, subIndex, &TerraDriver::getDrivingSignal)
{ ; }

TerraDriverAttachment::TerraDriverAttachment(const TerraDriverAttachment &attachment)
    : TerraSignalAttachment<Terra_DrivingState, TERRA_DRIVER_SIGNAL_SLOTS>(attachment)
{ ; }

TerraDriverAttachment::~TerraDriverAttachment()
{ ; }

void TerraDriverAttachment::updateIfNeeded(bool poll)
{
    if (poll && resolve()) { get()->update(); }
}
