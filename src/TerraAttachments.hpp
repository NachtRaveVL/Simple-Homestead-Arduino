/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Attachment Inlines
*/

#ifndef TerraAttachments_HPP
#define TerraAttachments_HPP

#include "Terraduino.h"

inline TerraDLinkObject &TerraDLinkObject::operator=(TerraIdentity rhs)
{
    _key = rhs.key;
    _obj = nullptr;
    if (_keyStr) { free((void *)_keyStr); _keyStr = nullptr; }

    auto len = rhs.keyString.length();
    if (len) {
        _keyStr = (const char *)malloc(len + 1);
        strncpy((char *)_keyStr, rhs.keyString.c_str(), len + 1);
    }
    return *this;
}

inline TerraDLinkObject &TerraDLinkObject::operator=(const char *rhs)
{
    _key = stringHash(rhs);
    _obj = nullptr;
    if (_keyStr) { free((void *)_keyStr); _keyStr = nullptr; }

    auto len = strnlen(rhs, TERRA_NAME_MAXSIZE);
    if (len) {
        _keyStr = (const char *)malloc(len + 1);
        strncpy((char *)_keyStr, rhs, len + 1);
    }
    return *this;
}

inline TerraDLinkObject &TerraDLinkObject::operator=(const TerraObjInterface *rhs)
{
    _key = rhs ? rhs->getKey() : tkey_none;
    _obj = rhs ? rhs->getSharedPtr() : nullptr;
    if (_keyStr) { free((void *)_keyStr); _keyStr = nullptr; }

    return *this;
}

inline TerraDLinkObject &TerraDLinkObject::operator=(const TerraAttachment *rhs)
{
    _key = rhs ? rhs->getKey() : tkey_none;
    _obj = rhs && rhs->isResolved() ? rhs->getSharedPtr() : nullptr;
    if (_keyStr) { free((void *)_keyStr); _keyStr = nullptr; }

    if (rhs && !rhs->isResolved()) {
        String keyString = rhs->getKeyString();
        auto len = keyString.length();
        if (len) {
            _keyStr = (const char *)malloc(len + 1);
            strncpy((char *)_keyStr, keyString.c_str(), len + 1);
        }
    }
    return *this;
}

template<class U>
inline TerraDLinkObject &TerraDLinkObject::operator=(SharedPtr<U> &rhs)
{
    _key = rhs ? rhs->getKey() : tkey_none;
    _obj = rhs ? static_pointer_cast<TerraObjInterface>(rhs) : nullptr;
    if (_keyStr) { free((void *)_keyStr); _keyStr = nullptr; }

    return *this;
}


template<class U>
void TerraAttachment::setObject(U obj, bool modify)
{
    if (!(_obj == obj)) {
        if (_obj.isResolved()) { detachObject(); }

        _obj = obj; // will be replaced by templated operator= inline

        if (_obj.isResolved()) { attachObject(); }

        if (modify && _parent) {
            if (_parent->isObject()) {
                ((TerraObject *)_parent)->bumpRevisionIfNeeded();
            } else {
                ((TerraSubObject *)_parent)->bumpRevisionIfNeeded();
            }
        }
    }
}

template<class U>
SharedPtr<U> TerraAttachment::getObject()
{
    if (_obj) { return _obj.getObject<U>(); }
    else if (!_obj.isSet()) { return nullptr; }
    else if (_obj.needsResolved() && _obj.resolveObject()) {
        attachObject();
    }
    return _obj.getObject<U>();
}


template<class ParameterType, int Slots> template<class U>
TerraSignalAttachment<ParameterType,Slots>::TerraSignalAttachment(TerraObjInterface *parent, tposi_t subIndex, Signal<ParameterType,Slots> &(U::*signalGetter)(void))
    : TerraAttachment(parent, subIndex), _signalGetter((SignalGetterPtr)signalGetter), _handleSlot(nullptr)
{ ; }

template<class ParameterType, int Slots>
TerraSignalAttachment<ParameterType,Slots>::TerraSignalAttachment(const TerraSignalAttachment<ParameterType,Slots> &attachment)
    : TerraAttachment(attachment), _signalGetter((SignalGetterPtr)attachment._signalGetter),
      _handleSlot(attachment._handleSlot ? attachment._handleSlot->clone() : nullptr)
{ ; }

template<class ParameterType, int Slots>
TerraSignalAttachment<ParameterType,Slots>::~TerraSignalAttachment()
{
    if (isResolved() && _handleSlot && _signalGetter) {
        (get()->*_signalGetter)().detach(*_handleSlot);
    }
    if (_handleSlot) {
        delete _handleSlot; _handleSlot = nullptr;
    }
}

template<class ParameterType, int Slots>
void TerraSignalAttachment<ParameterType,Slots>::attachObject()
{
    TerraAttachment::attachObject();

    if (isResolved() && _handleSlot && _signalGetter) {
        (get()->*_signalGetter)().attach(*_handleSlot);
    }
}

template<class ParameterType, int Slots>
void TerraSignalAttachment<ParameterType,Slots>::detachObject()
{
    if (isResolved() && _handleSlot && _signalGetter) {
        (get()->*_signalGetter)().detach(*_handleSlot);
    }

    TerraAttachment::detachObject();
}

template<class ParameterType, int Slots> template<class U>
void TerraSignalAttachment<ParameterType,Slots>::setSignalGetter(Signal<ParameterType,Slots> &(U::*signalGetter)(void))
{
    if (_signalGetter != signalGetter) {
        if (isResolved() && _handleSlot && _signalGetter) { (get()->*_signalGetter)().detach(*_handleSlot); }

        _signalGetter = signalGetter;

        if (isResolved() && _handleSlot && _signalGetter) { (get()->*_signalGetter)().attach(*_handleSlot); }
    }
}

template<class ParameterType, int Slots>
void TerraSignalAttachment<ParameterType,Slots>::setHandleSlot(const Slot<ParameterType> &handleSlot)
{
    if (!_handleSlot || !_handleSlot->operator==(&handleSlot)) {
        if (isResolved() && _handleSlot && _signalGetter) { (get()->*_signalGetter)().detach(*_handleSlot); }

        if (_handleSlot) { delete _handleSlot; _handleSlot = nullptr; }
        _handleSlot = handleSlot.clone();

        if (isResolved() && _handleSlot && _signalGetter) { (get()->*_signalGetter)().attach(*_handleSlot); }
    }
}


inline Terra_UnitsType TerraActuatorAttachment::getActivationUnits()
{
    return resolve() && get()->getUserCalibrationData() ? get()->getUserCalibrationData()->calibrationUnits : Terra_UnitsType_Raw_1;
}

inline float TerraActuatorAttachment::getActiveDriveIntensity()
{
    return resolve() ? get()->getDriveIntensity() : 0.0f;
}

inline float TerraActuatorAttachment::getActiveCalibratedValue()
{
    return resolve() ? get()->getCalibratedValue() : 0.0f;
}

inline float TerraActuatorAttachment::getSetupDriveIntensity() const
{
    return _actSetup.intensity;
}

inline float TerraActuatorAttachment::getSetupCalibratedValue()
{
    return resolve() ? get()->calibrationTransform(_actSetup.intensity) : 0.0f;
}


inline Terra_TriggerState TerraTriggerAttachment::getTriggerState(bool poll)
{
    return resolve() ? get()->getTriggerState(poll) : Terra_TriggerState_Undefined;
}


inline Terra_DrivingState TerraDriverAttachment::getDrivingState(bool poll)
{
    return resolve() ? get()->getDrivingState(poll) : Terra_DrivingState_Undefined;
}

#endif // /ifndef TerraAttachments_HPP
