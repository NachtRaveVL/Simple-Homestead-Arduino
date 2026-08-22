/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Attachment Points
*/

#include "Terraduino.h"

TerraDLinkObject::TerraDLinkObject()
    : _key(tkey_none), _obj(nullptr), _keyString()
{ ; }

TerraDLinkObject::TerraDLinkObject(const TerraDLinkObject &object)
    : _key(object._key), _obj(object._obj), _keyString(object._keyString)
{ ; }

TerraDLinkObject &TerraDLinkObject::operator=(const TerraDLinkObject &rhs)
{
    if (this != &rhs) {
        _key = rhs._key;
        _obj = rhs._obj;
        _keyString = rhs._keyString;
    }
    return *this;
}

void TerraDLinkObject::unresolve()
{
#ifdef ARDUINO
    if (_obj && !_keyString.length()) { _keyString = _obj->getKeyString(); }
#else
    if (_obj && _keyString.empty()) { _keyString = _obj->getKeyString(); }
#endif
    _obj = nullptr;
}

SharedPtr<TerraObjInterface> TerraDLinkObject::resolveObject()
{
    if (_obj || !isSet()) { return _obj; }
    if (getController()) {
        _obj = static_pointer_cast<TerraObjInterface>(getController()->objectById(getId()));
    }
    if (_obj) { _keyString = TerraString(); }
    return _obj;
}

TerraIdentity TerraDLinkObject::getId() const
{
#ifdef ARDUINO
    return _obj ? _obj->getId() : (_keyString.length() ? TerraIdentity(_keyString.c_str()) : TerraIdentity(_key));
#else
    return _obj ? _obj->getId() : (!_keyString.empty() ? TerraIdentity(_keyString.c_str()) : TerraIdentity(_key));
#endif
}

TerraString TerraDLinkObject::getKeyString() const
{
#ifdef ARDUINO
    return _keyString.length() ? _keyString : (_obj ? _obj->getKeyString() : TerraString());
#else
    return !_keyString.empty() ? _keyString : (_obj ? _obj->getKeyString() : TerraString());
#endif
}

TerraAttachment::TerraAttachment(TerraObjInterface *parent)
    : TerraSubObject(parent), _obj()
{ ; }

TerraAttachment::TerraAttachment(const TerraAttachment &attachment)
    : TerraSubObject(attachment._parent), _obj()
{
    initObject(attachment._obj);
}

TerraAttachment::~TerraAttachment()
{
    if (isResolved() && _obj->isObject() && _parent && _parent->isObject()) {
        _obj.get<TerraObject>()->removeLinkage(static_cast<TerraObject *>(_parent));
    }
}

void TerraAttachment::attachObject()
{
    if (resolve() && _obj->isObject() && _parent && _parent->isObject()) {
        _obj.get<TerraObject>()->addLinkage(static_cast<TerraObject *>(_parent));
    }
}

void TerraAttachment::detachObject()
{
    if (isResolved() && _obj->isObject() && _parent && _parent->isObject()) {
        _obj.get<TerraObject>()->removeLinkage(static_cast<TerraObject *>(_parent));
    }
}

void TerraAttachment::updateIfNeeded(bool poll)
{
    (void)poll;
}

void TerraAttachment::unresolve()
{
    detachObject();
    _obj.unresolve();
}

void TerraAttachment::setParent(TerraObjInterface *parent)
{
    if (_parent != parent) {
        detachObject();
        _parent = parent;
        if (isResolved()) { attachObject(); }
    }
}

SharedPtr<TerraObjInterface> TerraAttachment::getSharedPtrFor(const TerraObjInterface *object) const
{
    return object && object->getKey() == getKey() ? _obj._obj : TerraSubObject::getSharedPtrFor(object);
}

TerraSensorAttachment::TerraSensorAttachment(TerraObjInterface *parent, uint8_t measurementRow)
    : SignalAttachment<const TerraMeasurement *, TERRA_SENSOR_SIGNAL_SLOTS>(parent, &TerraSensor::getMeasurementSignal),
      _measurement(), _measurementRow(measurementRow), _convertParam(0.0f), _needsMeasurement(true)
{
    setHandleMethod(&TerraSensorAttachment::handleMeasurement, this);
}

TerraSensorAttachment::TerraSensorAttachment(const TerraSensorAttachment &attachment)
    : SignalAttachment<const TerraMeasurement *, TERRA_SENSOR_SIGNAL_SLOTS>(attachment), _measurement(attachment._measurement),
      _measurementRow(attachment._measurementRow), _convertParam(attachment._convertParam),
      _needsMeasurement(attachment._needsMeasurement)
{ ; }

void TerraSensorAttachment::attachObject()
{
    SignalAttachment<const TerraMeasurement *, TERRA_SENSOR_SIGNAL_SLOTS>::attachObject();
    if (_handleSlot) { (*_handleSlot)(get()->getMeasurement()); }
    else { handleMeasurement(get()->getMeasurement()); }
}

void TerraSensorAttachment::detachObject()
{
    SignalAttachment<const TerraMeasurement *, TERRA_SENSOR_SIGNAL_SLOTS>::detachObject();
    setNeedsMeasurement();
}

void TerraSensorAttachment::updateIfNeeded(bool poll)
{
    if ((poll || _needsMeasurement) && resolve()) {
        if (_handleSlot) { (*_handleSlot)(get()->getMeasurement()); }
        else { handleMeasurement(get()->getMeasurement()); }
        get()->takeMeasurement(poll || _needsMeasurement);
    }
}

void TerraSensorAttachment::setMeasurement(TerraSingleMeasurement measurement)
{
    if (_measurement.units != Terra_Unit_Undefined && measurement.units != _measurement.units &&
        canConvertUnits(measurement.units, _measurement.units)) {
        measurement.toUnits(_measurement.units);
    }
    _measurement = measurement;
    _needsMeasurement = false;
}

void TerraSensorAttachment::setMeasurementRow(uint8_t measurementRow)
{
    if (_measurementRow != measurementRow) {
        _measurementRow = measurementRow;
        setNeedsMeasurement();
        bumpRevisionIfNeeded();
    }
}

void TerraSensorAttachment::setMeasurementUnits(Terra_Unit units, float convertParam)
{
    if (_measurement.units != units || !isFPEqual(_convertParam, convertParam)) {
        _measurement.units = units;
        _convertParam = convertParam;
        setNeedsMeasurement();
        bumpRevisionIfNeeded();
    }
}

TerraSingleMeasurement TerraSensorAttachment::getMeasurement(uint32_t now, bool poll)
{
    (void)now;
    updateIfNeeded(poll);
    return _measurement;
}

void TerraSensorAttachment::handleMeasurement(const TerraMeasurement *measurement)
{
    if (measurement && measurement->isSet()) { setMeasurement(getAsSingleMeasurement(measurement, _measurementRow)); }
}

TerraActuatorAttachment::TerraActuatorAttachment(TerraObjInterface *parent)
    : TerraAttachment(parent), _activation()
{ ; }

TerraActuatorAttachment::TerraActuatorAttachment(const TerraActuatorAttachment &attachment)
    : TerraAttachment(attachment), _activation()
{ ; }

void TerraActuatorAttachment::updateIfNeeded(bool poll)
{
    (void)poll;
    if (_activation.isActive()) { _activation.elapseTo(); }
}

void TerraActuatorAttachment::setOutput(float intensity, millis_t duration, uint32_t now)
{
    SharedPtr<TerraActuator> actuator = getObject();
    if (!actuator || intensity <= FLT_EPSILON) {
        off();
        return;
    }

    _activation.activation = TerraActivation(Terra_DirectionMode_Forward, intensity, duration, Terra_ActivationFlags_None);
    _activation = actuator;
    actuator->setNeedsUpdate();
    actuator->update(now);
}

void TerraActuatorAttachment::off()
{
    SharedPtr<TerraActuator> actuator = _activation.actuator;
    _activation.unset();
    if (actuator) { actuator->setNeedsUpdate(); actuator->update(); }
}
