/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Resources
*/

#include "Terraduino.h"
#include "TerraUtils.h"

TerraResource::TerraResource(Terra_ResourceType type, tposi_t resourceIndex, const TerraString &name)
    : TerraObject(TerraIdentity(type, resourceIndex), name), _state(Terra_ResourceState_Reserve),
      _level(0.0f), _reserveLevel(10.0f), _lowLevel(25.0f), _highLevel(90.0f)
{ ; }


TerraResource::TerraResource(TerraIdentity id, const TerraString &name)
    : TerraObject(id, name), _state(Terra_ResourceState_Reserve),
      _level(0.0f), _reserveLevel(10.0f), _lowLevel(25.0f), _highLevel(90.0f)
{ ; }

TerraResource::TerraResource(const TerraResourceData *dataIn)
    : TerraObject(dataIn), _state(Terra_ResourceState_Reserve),
      _level(dataIn ? dataIn->level : 0.0f),
      _reserveLevel(dataIn ? dataIn->reserveLevel : 10.0f),
      _lowLevel(dataIn ? dataIn->lowLevel : 25.0f),
      _highLevel(dataIn ? dataIn->highLevel : 90.0f)
{
    updateState();
}

bool TerraResource::setThresholds(float reserveLevel, float lowLevel, float highLevel)
{
    if (reserveLevel < 0.0f || reserveLevel > lowLevel || lowLevel >= highLevel || highLevel > 100.0f) return false;
    _reserveLevel = reserveLevel;
    _lowLevel = lowLevel;
    _highLevel = highLevel;
    updateState();
    bumpRevisionIfNeeded();
    return true;
}

void TerraResource::setLevel(float level)
{
    float constrainedLevel = constrain(level, 0.0f, 100.0f);
    if (!isFPEqual(_level, constrainedLevel)) {
        _level = constrainedLevel;
        updateState();
        bumpRevisionIfNeeded();
    }
}

void TerraResource::setFault(const TerraString &message)
{
    TerraObject::setFault(message);
    updateState();
}

void TerraResource::clearFault()
{
    TerraObject::clearFault();
    updateState();
}

void TerraResource::updateState()
{
    if (_fault) { _state = Terra_ResourceState_Fault; }
    else if (_level <= _reserveLevel) { _state = Terra_ResourceState_Reserve; }
    else if (_level <= _lowLevel) { _state = Terra_ResourceState_Low; }
    else if (_level >= _highLevel) { _state = Terra_ResourceState_High; }
    else { _state = Terra_ResourceState_Normal; }
}

TerraData *TerraResource::allocateData() const
{
    return new TerraResourceData();
}

void TerraResource::saveToData(TerraData *dataOut) const
{
    TerraObject::saveToData(dataOut);
    auto data = static_cast<TerraResourceData *>(dataOut);
    data->level = _level;
    data->reserveLevel = _reserveLevel;
    data->lowLevel = _lowLevel;
    data->highLevel = _highLevel;
}
