/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Attachment Inlines
*/
#ifndef TerraAttachments_HPP
#define TerraAttachments_HPP
inline TerraDLinkObject &TerraDLinkObject::operator=(TerraIdentity rhs) { _key = rhs.key; _obj = nullptr; _keyString = rhs.keyString; return *this; }
inline TerraDLinkObject &TerraDLinkObject::operator=(const char *rhs) { _key = rhs ? terraHashString(rhs) : tkey_none; _obj = nullptr; _keyString = rhs ? TerraString(rhs) : TerraString(); return *this; }
inline TerraDLinkObject &TerraDLinkObject::operator=(const TerraObjInterface *rhs) { _key = rhs ? rhs->getKey() : tkey_none; _obj = rhs ? rhs->getSharedPtr() : nullptr; _keyString = rhs && !_obj ? rhs->getKeyString() : TerraString(); return *this; }
inline TerraDLinkObject &TerraDLinkObject::operator=(const TerraAttachment *rhs) { _key = rhs ? rhs->getKey() : tkey_none; _obj = rhs && rhs->isResolved() ? const_cast<TerraAttachment *>(rhs)->getObject<TerraObjInterface>() : nullptr; _keyString = rhs && !rhs->isResolved() ? rhs->getKeyString() : TerraString(); return *this; }
template<class U> inline TerraDLinkObject &TerraDLinkObject::operator=(SharedPtr<U> rhs) { _key = rhs ? rhs->getKey() : tkey_none; _obj = rhs ? static_pointer_cast<TerraObjInterface>(rhs) : nullptr; _keyString = TerraString(); return *this; }
template<class U> void TerraAttachment::setObject(U object, bool modify) { if (!(_obj == object)) { if (_obj.isResolved()) { detachObject(); } _obj = object; if (_obj.isResolved()) { attachObject(); } if (modify) { bumpRevisionIfNeeded(); } } }
template<class U> SharedPtr<U> TerraAttachment::getObject() { if (_obj) { return _obj.getObject<U>(); } if (!_obj.isSet()) { return nullptr; } if (_obj.needsResolved() && _obj.resolveObject()) { attachObject(); } return _obj.getObject<U>(); }


template<class ParameterType, int Slots>
template<class U>
SignalAttachment<ParameterType, Slots>::SignalAttachment(TerraObjInterface *parent, Signal<ParameterType, Slots> &(U::*signalGetter)(void))
    : TerraAttachment(parent), _signalGetter(nullptr), _handleSlot(nullptr)
{
    setSignalGetter(signalGetter);
}

template<class ParameterType, int Slots>
SignalAttachment<ParameterType, Slots>::SignalAttachment(const SignalAttachment<ParameterType, Slots> &attachment)
    : TerraAttachment(attachment), _signalGetter(attachment._signalGetter), _handleSlot(nullptr)
{
    if (attachment._handleSlot) { _handleSlot = attachment._handleSlot->clone(); }
}

template<class ParameterType, int Slots>
SignalAttachment<ParameterType, Slots>::~SignalAttachment()
{
    if (isResolved()) { detachObject(); }
    if (_handleSlot) { delete _handleSlot; _handleSlot = nullptr; }
}

template<class ParameterType, int Slots>
void SignalAttachment<ParameterType, Slots>::attachObject()
{
    TerraAttachment::attachObject();
    if (_signalGetter && _handleSlot && resolve()) { (get()->*_signalGetter)().attach(*_handleSlot); }
}

template<class ParameterType, int Slots>
void SignalAttachment<ParameterType, Slots>::detachObject()
{
    if (_signalGetter && _handleSlot && isResolved()) { (get()->*_signalGetter)().detach(*_handleSlot); }
    TerraAttachment::detachObject();
}

template<class ParameterType, int Slots>
template<class U>
void SignalAttachment<ParameterType, Slots>::setSignalGetter(Signal<ParameterType, Slots> &(U::*signalGetter)(void))
{
    _signalGetter = reinterpret_cast<SignalGetterPtr>(signalGetter);
}

template<class ParameterType, int Slots>
void SignalAttachment<ParameterType, Slots>::setHandleSlot(const Slot<ParameterType> &handleSlot)
{
    bool attached = isResolved();
    if (attached) { detachObject(); }
    if (_handleSlot) { delete _handleSlot; _handleSlot = nullptr; }
    _handleSlot = handleSlot.clone();
    if (attached) { attachObject(); }
}

#endif // /ifndef TerraAttachments_HPP
