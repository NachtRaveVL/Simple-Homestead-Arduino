/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Object
*/

#include "Terraduino.h"

TerraObject::TerraObject(Terra_ObjectType objectType, uint32_t key, const TerraString &name)
    : _objectType(objectType), _key(key), _name(name), _enabled(true), _fault(false),
      _faultMessage(), _linkages()
{ }

void TerraObject::setFault(const TerraString &message)
{
    _fault = true;
    _faultMessage = message;
}

void TerraObject::clearFault()
{
    _fault = false;
    _faultMessage = TerraString();
}

void TerraObject::setEnabled(bool enabled)
{
    _enabled = enabled;
}

bool TerraObject::addLinkage(TerraObject *object)
{
    if (!object) return false;

    for (uint8_t index = 0; index < TERRA_MAX_ATTACHMENTS; ++index) {
        if (_linkages[index].object == object) {
            ++_linkages[index].count;
            return true;
        }
        if (!_linkages[index].object) {
            _linkages[index] = TerraObjectLink(object, 1);
            return true;
        }
    }
    return false;
}

bool TerraObject::removeLinkage(TerraObject *object)
{
    if (!object) return false;

    for (uint8_t index = 0; index < TERRA_MAX_ATTACHMENTS && _linkages[index].object; ++index) {
        if (_linkages[index].object != object) continue;

        if (_linkages[index].count > 1) {
            --_linkages[index].count;
        } else {
            for (uint8_t subIndex = index; subIndex + 1 < TERRA_MAX_ATTACHMENTS; ++subIndex) {
                _linkages[subIndex] = _linkages[subIndex + 1];
            }
            _linkages[TERRA_MAX_ATTACHMENTS - 1] = TerraObjectLink();
        }
        return true;
    }
    return false;
}

bool TerraObject::hasLinkage(TerraObject *object) const
{
    if (!object) return false;

    for (uint8_t index = 0; index < TERRA_MAX_ATTACHMENTS && _linkages[index].object; ++index) {
        if (_linkages[index].object == object) return true;
    }
    return false;
}

void TerraObject::unresolveAny(TerraObject *object)
{
    if (this != object) return;

    TerraObject *lastObject = nullptr;
    for (uint8_t index = 0; index < TERRA_MAX_ATTACHMENTS && _linkages[index].object; ++index) {
        TerraObject *linkedObject = _linkages[index].object;
        if (linkedObject != object) {
            linkedObject->unresolveAny(object);             // May clobber linkage indexing.
            if (index && _linkages[index].object != linkedObject) {
                while (index && _linkages[index].object != lastObject) --index;
                linkedObject = lastObject;
            }
        }
        lastObject = linkedObject;
    }
}
