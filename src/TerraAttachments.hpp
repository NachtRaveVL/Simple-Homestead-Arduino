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
    SharedPtr<TerraObject> object = terraReinterpretPointerCast<TerraObject>(_object);
    if (object && _parent && object.get() != _parent) object->addLinkage(_parent);
}

// Detaches a resolved object while retaining its stable key for later resolution.
template<class TObject>
void TerraAttachment<TObject>::detachObject()
{
    SharedPtr<TerraObject> object = terraReinterpretPointerCast<TerraObject>(_object);
    if (object && _parent && object.get() != _parent) object->removeLinkage(_parent);
    if (object) _key = object->getKey();
    _object.reset();
}

template<class TObject>
void TerraAttachment<TObject>::setObjectImpl(const SharedPtr<TObject> &object)
{
    if (_object == object) return;
    detachObject();
    _object = object;
    SharedPtr<TerraObject> baseObject = terraReinterpretPointerCast<TerraObject>(_object);
    _key = baseObject ? baseObject->getKey() : TERRA_INVALID_KEY;
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
    setObjectImpl(registered ? terraReinterpretPointerCast<TObject>(registered) : SharedPtr<TObject>());
}

template<class TObject>
SharedPtr<TObject> TerraAttachment<TObject>::getObject()
{
    if (!_object && _key != TERRA_INVALID_KEY) {
        SharedPtr<TerraObject> registered = terraObjectByKey(_key);
        if (registered) {
            _object = terraReinterpretPointerCast<TObject>(registered);
            attachObject();
        }
    }
    return _object;
}

template<class TObject>
void TerraAttachment<TObject>::unresolve()
{
    detachObject();
}

template<class TObject>
void TerraAttachment<TObject>::unresolveAny(TerraObject *object)
{
    SharedPtr<TerraObject> baseObject = terraReinterpretPointerCast<TerraObject>(_object);
    if (baseObject && baseObject.get() == object) detachObject();
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
