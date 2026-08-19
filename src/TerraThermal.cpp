/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Thermal Management
*/

#include "TerraThermal.h"
#include "TerraCoreLogic.h"

TerraThermalStore::TerraThermalStore(uint32_t key, const TerraString &name)
    : TerraResource(Terra_ResourceType_Thermal, key, name), _temperatureC(0.0f), _minimumTargetC(0.0f),
      _maximumTargetC(80.0f), _absoluteMaximumC(95.0f), _attachments() {
    _objectType = Terra_ObjectType_ThermalStore;
}

bool TerraThermalStore::setTargetRange(float minimumC, float maximumC) {
    if (minimumC > maximumC || maximumC > _absoluteMaximumC) return false;
    _minimumTargetC = minimumC;
    _maximumTargetC = maximumC;
    return true;
}

TerraThermalLoop::TerraThermalLoop(uint32_t key, const TerraString &name)
    : TerraObject(Terra_ObjectType_ThermalLoop, key, name), _onDifferentialC(8.0f), _offDifferentialC(3.0f),
      _maxStoreTempC(80.0f), _running(false), _attachments() { }


bool TerraThermalStore::setAbsoluteMaximum(float maximumC) {
    if (maximumC < _maximumTargetC) return false;
    _absoluteMaximumC = maximumC;
    return true;
}

bool TerraThermalLoop::configure(float onDifferentialC, float offDifferentialC, float maxStoreTempC) {
    if (offDifferentialC < 0.0f || onDifferentialC < offDifferentialC) return false;
    _onDifferentialC = onDifferentialC;
    _offDifferentialC = offDifferentialC;
    _maxStoreTempC = maxStoreTempC;
    return true;
}

bool TerraThermalLoop::shouldCirculate(float sourceTempC, float storeTempC) const {
    return terraThermalLoopShouldRun(sourceTempC, storeTempC, _onDifferentialC, _offDifferentialC, _maxStoreTempC, _running);
}
