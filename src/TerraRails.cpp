/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Power Rails
*/

#include "Terraduino.h"

TerraPowerRail::TerraPowerRail(float nominalVoltage, tposi_t railIndex, const TerraString &name, Terra_RailType railType)
    : TerraObject(TerraIdentity(railType, railIndex), name), _nominalVoltage(nominalVoltage), _measuredVoltage(0.0f),
      _railEnabled(false)
{ ; }

TerraPowerRail::TerraPowerRail(const TerraPowerRailData *dataIn)
    : TerraObject(dataIn), _nominalVoltage(dataIn ? dataIn->nominalVoltage : 0.0f), _measuredVoltage(0.0f),
      _railEnabled(false)
{ ; }

void TerraPowerRail::setEnabledState(bool enabled)
{
    _railEnabled = enabled;
}

TerraData *TerraPowerRail::allocateData() const
{
    return new TerraPowerRailData();
}

void TerraPowerRail::saveToData(TerraData *dataOut) const
{
    TerraObject::saveToData(dataOut);
    static_cast<TerraPowerRailData *>(dataOut)->nominalVoltage = _nominalVoltage;
}
