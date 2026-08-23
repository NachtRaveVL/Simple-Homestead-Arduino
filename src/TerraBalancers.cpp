/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Balancers
*/

#include "Terraduino.h"
#include "TerraActuators.h"
#include "TerraSensors.h"
#include "TerraThermal.h"
#include "TerraWater.h"

TerraWaterBalancer::TerraWaterBalancer(TerraWaterRoute *route)
    : _route(route), _source(route), _destination(route), _pump(route), _flowSensor(route)
{ ; }

void TerraWaterBalancer::setParent(TerraWaterRoute *route)
{
    _route = route;
    _source.setParent(route);
    _destination.setParent(route);
    _pump.setParent(route);
    _flowSensor.setParent(route);
}

void TerraWaterBalancer::update(uint32_t now)
{
    if (!_route || !_route->isEnabled() || _route->hasFault()) {
        _pump.off();
        return;
    }

    SharedPtr<TerraWaterSource> source = _source.getObject<TerraWaterSource>();
    SharedPtr<TerraWaterStorage> destination = _destination.getObject<TerraWaterStorage>();
    if (!source || !destination || !_pump.isSet()) {
        _pump.off();
        return;
    }

    bool run = false;
    bool pumpWasActive = _pump.isActive();
    const TerraCistern *cistern = destination->classType == TerraReservoir::Cistern
                                ? static_cast<const TerraCistern *>(destination.get())
                                : nullptr;

    if (cistern && cistern->isOverflowDetected()) {
        _route->setRouteState(Terra_RouteState_Fault);
    } else if (!destination->isEnabled() || destination->hasFault()) {
        _route->setRouteState(Terra_RouteState_Fault);
    } else if (!source->isAvailable() || source->getLevel() <= source->getReserveLevel()) {
        _route->setRouteState(Terra_RouteState_Idle);
    } else {
        float startPercent = cistern ? cistern->getFillStartPercent() : _route->getDestinationStartPercent();
        float stopPercent = cistern ? cistern->getFillStopPercent() : _route->getDestinationStopPercent();
        bool routeWasActive = _route->getRouteState() == Terra_RouteState_Requested ||
                              _route->getRouteState() == Terra_RouteState_Active;

        if (destination->getLevel() >= stopPercent) {
            _route->setRouteState(Terra_RouteState_Complete);
        } else if (routeWasActive || pumpWasActive) {
            _route->setRouteState(Terra_RouteState_Active);
            run = true;
        } else if (destination->getLevel() <= startPercent) {
            _route->setRouteState(Terra_RouteState_Requested);
            run = true;
        } else {
            _route->setRouteState(Terra_RouteState_Idle);
        }
    }

    if (_flowSensor.isSet()) {
        TerraSingleMeasurement flow = _flowSensor.getMeasurement(now, true);
        if (flow.isSet() && flow.units != Terra_Unit_LitersPerMinute && canConvertUnits(flow.units, Terra_Unit_LitersPerMinute)) {
            flow.toUnits(Terra_Unit_LitersPerMinute);
        }
        if (!flow.isSet() || flow.units != Terra_Unit_LitersPerMinute) {
            run = false;
        } else if ((run && pumpWasActive && !_route->validateFlow(flow.value, true)) ||
                   (!run && !_route->validateFlow(flow.value, false))) {
            run = false;
        }
    }

    if (run) { _pump.setOutput(1.0f, (millis_t)-1, now); }
    else { _pump.off(); }
}

void TerraWaterBalancer::unresolveAny(TerraObject *object)
{
    _source.unresolveAny(object);
    _destination.unresolveAny(object);
    _pump.unresolveAny(object);
    _flowSensor.unresolveAny(object);
}

const TerraWaterSource *TerraWaterBalancer::selectSource(const TerraWaterSource *const *sources, uint8_t count) const
{
    if (!sources || !count) return nullptr;
    const TerraWaterSource *selected = nullptr;
    for (uint8_t index = 0; index < count; ++index) {
        const TerraWaterSource *source = sources[index];
        if (!source || !source->isAvailable() || source->getLevel() <= source->getReserveLevel()) continue;
        if (!selected || source->getPriority() < selected->getPriority()) selected = source;
    }
    return selected;
}

TerraCistern *TerraWaterBalancer::selectFillCistern(TerraCistern *const *cisterns, uint8_t count) const
{
    if (!cisterns || !count) return nullptr;
    TerraCistern *selected = nullptr;
    for (uint8_t index = 0; index < count; ++index) {
        TerraCistern *cistern = cisterns[index];
        if (!cistern || !cistern->needsFill(false)) continue;
        if (!selected || cistern->getLevel() < selected->getLevel()) selected = cistern;
    }
    return selected;
}

const TerraCistern *TerraWaterBalancer::selectSupplyCistern(const TerraCistern *const *cisterns, uint8_t count) const
{
    if (!cisterns || !count) return nullptr;
    const TerraCistern *selected = nullptr;
    for (uint8_t index = 0; index < count; ++index) {
        const TerraCistern *cistern = cisterns[index];
        if (!cistern || !cistern->canSupplyWater()) continue;
        if (!selected || cistern->availableAboveReserveLiters() > selected->availableAboveReserveLiters()) selected = cistern;
    }
    return selected;
}

float TerraWaterBalancer::transferAllowance(const TerraCistern &source, const TerraCistern &destination, float requestedLiters) const
{
    if (!source.canSupplyWater() || !destination.canAcceptWater()) return 0.0f;
    if (requestedLiters <= 0.0f) { return 0.0f; }
    const float sourceReserve = source.getCapacityLiters() * (source.getReserveLevel() / 100.0f);
    const float sourceAvailable = source.getStoredLiters() > sourceReserve
                                ? source.getStoredLiters() - sourceReserve : 0.0f;
    const float destinationTarget = destination.getCapacityLiters() * (destination.getFillStopPercent() / 100.0f);
    const float destinationRoom = destinationTarget > destination.getStoredLiters()
                                ? destinationTarget - destination.getStoredLiters() : 0.0f;
    const float sourceLimited = sourceAvailable < requestedLiters ? sourceAvailable : requestedLiters;
    return destinationRoom < sourceLimited ? destinationRoom : sourceLimited;
}

TerraThermalBalancer::TerraThermalBalancer(TerraThermalLoop *loop)
    : _loop(loop), _sourceTemperature(loop), _store(loop), _circulator(loop)
{ ; }

void TerraThermalBalancer::setParent(TerraThermalLoop *loop)
{
    _loop = loop;
    _sourceTemperature.setParent(loop);
    _store.setParent(loop);
    _circulator.setParent(loop);
}

void TerraThermalBalancer::update(uint32_t now)
{
    if (!_loop || !_loop->isEnabled() || _loop->hasFault()) {
        _circulator.off();
        if (_loop) { _loop->setRunning(false); }
        return;
    }

    SharedPtr<TerraThermalStore> store = _store.getObject<TerraThermalStore>();
    if (!store || !_sourceTemperature.isSet() || !_circulator.isSet()) {
        _circulator.off();
        _loop->setRunning(false);
        return;
    }

    TerraSingleMeasurement source = _sourceTemperature.getMeasurement(now, true);
    if (source.isSet() && source.units != Terra_UnitsType_Temperature_Celsius && canConvertUnits(source.units, Terra_UnitsType_Temperature_Celsius)) {
        source.toUnits(Terra_UnitsType_Temperature_Celsius);
    }
    if (!source.isSet() || source.units != Terra_UnitsType_Temperature_Celsius) {
        _circulator.off();
        _loop->setRunning(false);
        return;
    }

    bool run = _loop->shouldCirculate(source.value, store->getTemperature());
    _loop->setRunning(run);
    if (run) { _circulator.setOutput(1.0f, (millis_t)-1, now); }
    else { _circulator.off(); }
}

void TerraThermalBalancer::unresolveAny(TerraObject *object)
{
    _sourceTemperature.unresolveAny(object);
    _store.unresolveAny(object);
    _circulator.unresolveAny(object);
}
