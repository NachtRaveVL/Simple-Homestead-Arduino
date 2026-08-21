/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Attachment Points
*/

#ifndef TerraAttachments_HPP
#define TerraAttachments_HPP

// Attaches a resolved object and registers the parent linkage.
template<class TObject>
void TerraAttachment<TObject>::attachObject()
{
    if (_object && _parent && _object.get() != _parent) _object->addLinkage(_parent);
}

// Detaches a resolved object while retaining its stable key for later resolution.
template<class TObject>
void TerraAttachment<TObject>::detachObject()
{
    if (_object && _parent && _object.get() != _parent) _object->removeLinkage(_parent);
    if (_object) _key = _object->getKey();
    _object.reset();
}

template<class TObject>
void TerraAttachment<TObject>::setObjectImpl(const SharedPtr<TerraObject> &object)
{
    if (_object == object) return;
    detachObject();
    _object = object;
    _key = _object ? _object->getKey() : TERRA_INVALID_KEY;
    attachObject();
}

template<class TObject>
void TerraAttachment<TObject>::setObject(TObject *object)
{
    if (!object) {
        detachObject();
        _key = TERRA_INVALID_KEY;
        return;
    }

    TerraObject *baseObject = reinterpret_cast<TerraObject *>(object);
    SharedPtr<TerraObject> registered = terraObjectByKey(baseObject->getKey());
    setObjectImpl(registered);
}

template<class TObject>
SharedPtr<TObject> TerraAttachment<TObject>::getObject()
{
    if (!_object && _key != TERRA_INVALID_KEY) {
        _object = terraObjectByKey(_key);
        if (_object) attachObject();
    }
    return _object ? terraReinterpretPointerCast<TObject>(_object) : SharedPtr<TObject>();
}

template<class TObject>
void TerraAttachment<TObject>::unresolve()
{
    detachObject();
}

template<class TObject>
void TerraAttachment<TObject>::unresolveAny(TerraObject *object)
{
    if (_object && _object.get() == object) detachObject();
}

template<class TObject>
void TerraAttachment<TObject>::setParent(TerraObject *parent)
{
    if (_parent == parent) return;
    detachObject();
    _parent = parent;
    getObject();
}

#endif
