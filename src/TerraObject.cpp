/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Object
*/

#include "Terraduino.h"

TerraObject *newObjectFromData(const TerraData *dataIn)
{
    if (dataIn && !isValidType(dataIn->id.object.idType)) return nullptr;
    TERRA_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(TStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData()) {
        switch (dataIn->id.object.idType) {
            case (tid_t)TerraIdentity::Actuator:
                return newActuatorObjectFromData((TerraActuatorData *)dataIn);
            case (tid_t)TerraIdentity::Sensor:
                return newSensorObjectFromData((TerraSensorData *)dataIn);
            case (tid_t)TerraIdentity::Reservoir:
                return newReservoirObjectFromData((TerraReservoirData *)dataIn);
            case (tid_t)TerraIdentity::Rail:
                return newRailObjectFromData((TerraRailData *)dataIn);
            default: // Unable
                return nullptr;
        }
    }

    return nullptr;
}


tkey_t TerraIdentity::regenKey()
{
    switch (type) {
        case Actuator:
            keyString = actuatorTypeToString(objTypeAs.actuatorType, true);
            break;
        case Sensor:
            keyString = sensorTypeToString(objTypeAs.sensorType, true);
            break;
        case Reservoir:
            keyString = reservoirTypeToString(objTypeAs.reservoirType, true);
            break;
        case Rail:
            keyString = railTypeToString(objTypeAs.railType, true);
            break;
        default: // Unable
            return key;
    }
    keyString.concat(' ');
    keyString.concat('#');
    keyString.concat(positionIndexToString(posIndex, true));
    key = stringHash(keyString);
    return key;
}

String TerraIdentity::getDisplayString()
{
    switch (type) {
        case Actuator: return String(F("Actuator ")) + keyString;
        case Sensor: return String(F("Sensor ")) + keyString;
        case Reservoir: return String(F("Reservoir ")) + keyString;
        case Rail: return String(F("Rail ")) + keyString;
        default: return String(F("Unknown ")) + keyString;
    }
}


TerraObject::~TerraObject()
{
    if (_links) { delete [] _links; _links = nullptr; }
}

void TerraObject::update()
{ ; }

void TerraObject::handleLowMemory()
{
    if (_links && !_links[_linksSize >> 1].first) { allocateLinkages(_linksSize >> 1); } // shrink /2 if too big
}

TerraData *TerraObject::newSaveData()
{
    auto data = allocateData();
    TERRA_SOFT_ASSERT(data, SFP(TStr_Err_AllocationFailure));
    if (data) { saveToData(data); }
    return data;
}

void TerraObject::allocateLinkages(size_t size)
{
    if (_linksSize != size) {
        Pair<TerraObject *, int8_t> *newLinks = size ? new Pair<TerraObject *, int8_t>[size] : nullptr;

        if (size) {
            TERRA_HARD_ASSERT(newLinks, SFP(TStr_Err_AllocationFailure));

            size_t linksIndex = 0;
            if (_links) {
                for (; linksIndex < _linksSize && linksIndex < size; ++linksIndex) {
                    newLinks[linksIndex] = _links[linksIndex];
                }
            }
            for (; linksIndex < size; ++linksIndex) {
                newLinks[linksIndex] = make_pair((TerraObject *)nullptr, (int8_t)0);
            }
        }

        if (_links) { delete [] _links; }
        _links = newLinks;
        _linksSize = size;
    }
}

bool TerraObject::addLinkage(TerraObject *obj)
{
    if (!_links) { allocateLinkages(); }
    if (_links) {
        tposi_t linksIndex = 0;
        for (; linksIndex < _linksSize && _links[linksIndex].first; ++linksIndex) {
            if (_links[linksIndex].first == obj) {
                _links[linksIndex].second++;
                return true;
            }
        }
        if (linksIndex >= _linksSize) { allocateLinkages(_linksSize << 1); } // grow *2 if too small
        if (linksIndex < _linksSize) {
            _links[linksIndex] = make_pair(obj, (int8_t)1);
            return true;
        }
    }
    return false;
}

bool TerraObject::removeLinkage(TerraObject *obj)
{
    if (_links) {
        for (tposi_t linksIndex = 0; linksIndex < _linksSize && _links[linksIndex].first; ++linksIndex) {
            if (_links[linksIndex].first == obj) {
                if (--_links[linksIndex].second <= 0) {
                    for (int linksSubIndex = linksIndex; linksSubIndex < _linksSize - 1; ++linksSubIndex) {
                        _links[linksSubIndex] = _links[linksSubIndex + 1];
                    }
                    _links[_linksSize - 1] = make_pair((TerraObject *)nullptr, (int8_t)0);
                }
                return true;
            }
        }
    }
    return false;
}

bool TerraObject::hasLinkage(TerraObject *obj) const
{
    if (_links) {
        for (tposi_t linksIndex = 0; linksIndex < _linksSize && _links[linksIndex].first; ++linksIndex) {
            if (_links[linksIndex].first == obj) {
                return true;
            }
        }
    }
    return false;
}

void TerraObject::unresolveAny(TerraObject *obj)
{
    if (this == obj && _links) {
        TerraObject *lastObject = nullptr;
        for (tposi_t linksIndex = 0; linksIndex < _linksSize && _links[linksIndex].first; ++linksIndex) {
            TerraObject *object = _links[linksIndex].first;
            if (object != obj) {
                object->unresolveAny(obj); // may clobber indexing

                if (linksIndex && _links[linksIndex].first != object) {
                    while (linksIndex && _links[linksIndex].first != lastObject) { --linksIndex; }
                    object = lastObject;
                }
            }
            lastObject = object;
        }
    }
}

TerraIdentity TerraObject::getId() const
{
    return _id;
}

tkey_t TerraObject::getKey() const
{
    return _id.key;
}

String TerraObject::getKeyString() const
{
    return _id.keyString;
}

SharedPtr<TerraObjInterface> TerraObject::getSharedPtr() const
{
    return getController() ? static_pointer_cast<TerraObjInterface>(getController()->objectById(_id)) : nullptr;
}

SharedPtr<TerraObjInterface> TerraObject::getSharedPtrFor(const TerraObjInterface *obj) const
{
    return obj->isObject() ? obj->getSharedPtr() : nullptr;
}

bool TerraObject::isObject() const
{
    return true;
}

TerraData *TerraObject::allocateData() const
{
    TERRA_HARD_ASSERT(false, SFP(TStr_Err_UnsupportedOperation));
    return new TerraData();
}

void TerraObject::saveToData(TerraData *dataOut)
{
    dataOut->id.object.idType = (tid_t)_id.type;
    dataOut->id.object.objType = _id.objTypeAs.idType;
    dataOut->id.object.posIndex = _id.posIndex;
    dataOut->_revision = getRevision();
    if (_id.keyString.length()) {
        strncpy(((TerraObjectData *)dataOut)->name, _id.keyString.c_str(), TERRA_NAME_MAXSIZE);
    }
}


void TerraSubObject::unresolveAny(TerraObject *obj)
{ (void)obj; }

TerraIdentity TerraSubObject::getId() const
{
    return TerraIdentity(getKey());
}

tkey_t TerraSubObject::getKey() const
{
    return (tkey_t)(intptr_t)this;
}

String TerraSubObject::getKeyString() const
{
    return addressToString((uintptr_t)this);
}

SharedPtr<TerraObjInterface> TerraSubObject::getSharedPtr() const
{
    return _parent ? _parent->getSharedPtrFor((const TerraObjInterface *)this) : nullptr;
}

SharedPtr<TerraObjInterface> TerraSubObject::getSharedPtrFor(const TerraObjInterface *obj) const
{
    return obj->isObject() ? obj->getSharedPtr() : _parent ? _parent->getSharedPtrFor(obj) : nullptr;
}

bool TerraSubObject::isObject() const
{
    return false;
}

void TerraSubObject::setParent(TerraObjInterface *parent)
{
    _parent = parent;
}


TerraObjectData::TerraObjectData()
    : TerraData(), name{0}
{
    _size = sizeof(*this);
}

void TerraObjectData::toJSONObject(JsonObject &objectOut) const
{
    TerraData::toJSONObject(objectOut);

    if (name[0]) { objectOut[SFP(TStr_Key_Id)] = charsToString(name, TERRA_NAME_MAXSIZE); }
}

void TerraObjectData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraData::fromJSONObject(objectIn);

    const char *nameStr = objectIn[SFP(TStr_Key_Id)];
    if (nameStr && nameStr[0]) { strncpy(name, nameStr, TERRA_NAME_MAXSIZE); }
}
