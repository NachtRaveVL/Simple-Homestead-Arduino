/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Object
*/

#include "Terraduino.h"
#include "TerraUtils.h"
#include <string.h>
#include "TerraStrings.h"
#include <stdio.h>

static TerraString terraPositionIndexToString(tposi_t positionIndex)
{
#ifdef ARDUINO
    return String((int)positionIndex);
#else
    return std::to_string((int)positionIndex);
#endif
}

TerraIdentity::TerraIdentity(tkey_t keyIn)
    : type(Terra_ObjectType_Undefined), objTypeAs(), posIndex(TERRA_POS_SEARCH_FROMBEG), keyString(), key(keyIn)
{ objTypeAs.idType = -1; }

TerraIdentity::TerraIdentity(const char *keyStringIn)
    : type(Terra_ObjectType_Undefined), objTypeAs(), posIndex(TERRA_POS_SEARCH_FROMBEG),
      keyString(keyStringIn ? keyStringIn : ""), key(keyStringIn ? terraHashString(keyStringIn) : tkey_none)
{ objTypeAs.idType = -1; }

TerraIdentity::TerraIdentity(Terra_SensorType sensorTypeIn, tposi_t positionIndex)
    : type(Terra_ObjectType_Sensor), objTypeAs(), posIndex(positionIndex), keyString(), key(tkey_none)
{ objTypeAs.sensorType = sensorTypeIn; regenKey(); }

TerraIdentity::TerraIdentity(Terra_ActuatorType actuatorTypeIn, tposi_t positionIndex)
    : type(Terra_ObjectType_Actuator), objTypeAs(), posIndex(positionIndex), keyString(), key(tkey_none)
{ objTypeAs.actuatorType = actuatorTypeIn; regenKey(); }

TerraIdentity::TerraIdentity(Terra_ResourceType resourceTypeIn, tposi_t positionIndex)
    : type(Terra_ObjectType_Resource), objTypeAs(), posIndex(positionIndex), keyString(), key(tkey_none)
{ objTypeAs.resourceType = resourceTypeIn; regenKey(); }

TerraIdentity::TerraIdentity(Terra_WaterStorageType storageTypeIn, tposi_t positionIndex)
    : type(Terra_ObjectType_WaterStorage), objTypeAs(), posIndex(positionIndex), keyString(), key(tkey_none)
{ objTypeAs.waterStorageType = storageTypeIn; regenKey(); }

TerraIdentity::TerraIdentity(Terra_WaterSourceType sourceTypeIn, tposi_t positionIndex)
    : type(Terra_ObjectType_WaterSource), objTypeAs(), posIndex(positionIndex), keyString(), key(tkey_none)
{ objTypeAs.waterSourceType = sourceTypeIn; regenKey(); }

TerraIdentity::TerraIdentity(Terra_RailType railTypeIn, tposi_t positionIndex)
    : type(Terra_ObjectType_PowerRail), objTypeAs(), posIndex(positionIndex), keyString(), key(tkey_none)
{ objTypeAs.railType = railTypeIn; regenKey(); }

TerraIdentity::TerraIdentity(Terra_ObjectType objectTypeIn, tposi_t positionIndex)
    : type(objectTypeIn), objTypeAs(), posIndex(positionIndex), keyString(), key(tkey_none)
{ objTypeAs.idType = 0; regenKey(); }

TerraIdentity::TerraIdentity(int objectType, int16_t subType, tposi_t positionIndex)
    : type((Terra_ObjectType)objectType), objTypeAs(), posIndex(positionIndex), keyString(), key(tkey_none)
{ objTypeAs.idType = subType; regenKey(); }

tkey_t TerraIdentity::regenKey()
{
    switch (type) {
        case Terra_ObjectType_Sensor: keyString = terraSensorTypeToString(objTypeAs.sensorType); break;
        case Terra_ObjectType_Actuator: keyString = terraActuatorTypeToString(objTypeAs.actuatorType); break;
        case Terra_ObjectType_Resource: keyString = terraResourceTypeToString(objTypeAs.resourceType); break;
        case Terra_ObjectType_WaterStorage: keyString = terraWaterStorageTypeToString(objTypeAs.waterStorageType); break;
        case Terra_ObjectType_WaterSource: keyString = terraWaterSourceTypeToString(objTypeAs.waterSourceType); break;
        case Terra_ObjectType_WaterRoute: keyString = SFP(TStr_WaterRoute); break;
        case Terra_ObjectType_RainCatchment: keyString = SFP(TStr_RainCatchment); break;
        case Terra_ObjectType_ThermalStore: keyString = SFP(TStr_ThermalStore); break;
        case Terra_ObjectType_ThermalLoop: keyString = SFP(TStr_ThermalLoop); break;
        case Terra_ObjectType_Environment: keyString = SFP(TStr_Environment); break;
        case Terra_ObjectType_PowerRail: keyString = terraRailTypeToString(objTypeAs.railType); break;
        default: return key;
    }

    if (posIndex >= 0) {
        keyString += " #";
        keyString += terraPositionIndexToString(posIndex);
    }
    key = terraHashString(keyString.c_str());
    return key;
}

TerraString TerraIdentity::getDisplayString() const
{
    TerraString display = terraObjectTypeToString(type);
    display += " ";
    display += keyString;
    return display;
}

TerraObject::TerraObject(TerraIdentity id, const TerraString &name)
    : _id(id), _name(name), _revision(-1), _enabled(true), _fault(false),
      _faultMessage(), _linksSize(0), _links(nullptr)
{ }

TerraObject::TerraObject(const TerraObjectData *dataIn)
    : _id(dataIn ? TerraIdentity(dataIn->id.object.idType, dataIn->id.object.objType, dataIn->id.object.posIndex) : TerraIdentity()),
      _name(dataIn && dataIn->name[0] ? TerraString(dataIn->name) : TerraString()),
      _revision(dataIn ? (int8_t)dataIn->getRevision() : -1), _enabled(dataIn ? dataIn->enabled : true),
      _fault(false), _faultMessage(), _linksSize(0), _links(nullptr)
{ ; }

TerraObject::~TerraObject()
{
    if (_links) { delete [] _links; _links = nullptr; }
}

TerraData *TerraObject::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, tid_none);
}

void TerraObject::saveToData(TerraData *dataOut) const
{
    if (!dataOut) { return; }
    dataOut->id.object.idType = (tid_t)_id.type;
    dataOut->id.object.objType = (tid_t)_id.objTypeAs.idType;
    dataOut->id.object.posIndex = _id.posIndex;
    dataOut->_revision = (int8_t)getRevision();
    auto objectData = static_cast<TerraObjectData *>(dataOut);
    strncpy(objectData->name, _name.c_str(), TERRA_NAME_MAXSIZE - 1);
    objectData->name[TERRA_NAME_MAXSIZE - 1] = '\0';
    objectData->enabled = _enabled;
}

TerraObjectData *TerraObject::newSaveData() const
{
    TerraData *data = allocateData();
    if (data) { saveToData(data); }
    return static_cast<TerraObjectData *>(data);
}

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
    if (_enabled != enabled) {
        _enabled = enabled;
        bumpRevisionIfNeeded();
    }
}

void TerraObject::allocateLinkages(size_t size)
{
    if (_linksSize == size) { return; }
    TerraObjectLink *newLinks = size ? new TerraObjectLink[size] : nullptr;
    if (size && !newLinks) { return; }

    size_t copyCount = _linksSize < size ? _linksSize : size;
    for (size_t index = 0; index < copyCount; ++index) { newLinks[index] = _links[index]; }
    if (_links) { delete [] _links; }
    _links = newLinks;
    _linksSize = size;
}

bool TerraObject::addLinkage(TerraObject *object)
{
    if (!object) { return false; }
    if (!_links) { allocateLinkages(); }
    if (!_links) { return false; }

    size_t index = 0;
    for (; index < _linksSize && _links[index].object; ++index) {
        if (_links[index].object == object) {
            ++_links[index].count;
            return true;
        }
    }
    if (index >= _linksSize) {
        size_t oldSize = _linksSize;
        allocateLinkages(_linksSize ? _linksSize << 1 : 1);
        index = oldSize;
    }
    if (index < _linksSize) {
        _links[index] = TerraObjectLink(object, 1);
        return true;
    }
    return false;
}

bool TerraObject::removeLinkage(TerraObject *object)
{
    if (!_links || !object) { return false; }
    for (size_t index = 0; index < _linksSize && _links[index].object; ++index) {
        if (_links[index].object == object) {
            if (--_links[index].count <= 0) {
                for (size_t subIndex = index; subIndex + 1 < _linksSize; ++subIndex) { _links[subIndex] = _links[subIndex + 1]; }
                _links[_linksSize - 1] = TerraObjectLink();
            }
            return true;
        }
    }
    return false;
}

bool TerraObject::hasLinkage(TerraObject *object) const
{
    if (!_links || !object) { return false; }
    for (size_t index = 0; index < _linksSize && _links[index].object; ++index) {
        if (_links[index].object == object) { return true; }
    }
    return false;
}

void TerraObject::unresolveAny(TerraObject *object)
{
    if (this == object && _links) {
        TerraObject *lastObject = nullptr;
        for (size_t index = 0; index < _linksSize && _links[index].object; ++index) {
            TerraObject *linkedObject = _links[index].object;
            if (linkedObject != object) {
                linkedObject->unresolveAny(object);         // May clobber linkage indexing.

                if (index && _links[index].object != linkedObject) {
                    while (index && _links[index].object != lastObject) { --index; }
                    linkedObject = lastObject;
                }
            }
            lastObject = linkedObject;
        }
    }
}

SharedPtr<TerraObjInterface> TerraObject::getSharedPtr() const
{
    return getController() ? getController()->objectById(_id) : nullptr;
}

SharedPtr<TerraObjInterface> TerraObject::getSharedPtrFor(const TerraObjInterface *object) const
{
    return object == this ? getSharedPtr() : nullptr;
}

tkey_t TerraSubObject::getKey() const
{
    uintptr_t address = (uintptr_t)this;
    return (tkey_t)((address ^ (address >> 16)) & 0xffffffffUL);
}

TerraIdentity TerraSubObject::getId() const
{
    return TerraIdentity(getKey());
}

TerraString TerraSubObject::getKeyString() const
{
    char buffer[2 * sizeof(void *) + 3];
    snprintf(buffer, sizeof(buffer), "%p", (const void *)this);
    return TerraString(buffer);
}

SharedPtr<TerraObjInterface> TerraSubObject::getSharedPtr() const
{
    return _parent ? _parent->getSharedPtrFor(this) : nullptr;
}

SharedPtr<TerraObjInterface> TerraSubObject::getSharedPtrFor(const TerraObjInterface *object) const
{
    return object->isObject() ? object->getSharedPtr() : _parent ? _parent->getSharedPtrFor(object) : nullptr;
}
