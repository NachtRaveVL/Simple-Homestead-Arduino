/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Water Management
*/

#include "Terraduino.h"

TerraWaterStorage::TerraWaterStorage(float capacityLiters, uint32_t key, const TerraString &name,
                                     Terra_WaterStorageType storageType)
    : TerraResource(Terra_ResourceType_Water, key, name),
      _capacityLiters(capacityLiters < 0.0f ? 0.0f : capacityLiters),
      _storageType(storageType), _levelSensor(this)
{
    _objectType = Terra_ObjectType_WaterStorage;
}

void TerraWaterStorage::setStoredLiters(float liters)
{
    if (_capacityLiters <= 0.0f) {
        setLevel(0.0f);
        return;
    }

    setLevel(liters / _capacityLiters * 100.0f);
}

float TerraWaterStorage::availableAboveReserveLiters() const
{
    float reserveLiters = _capacityLiters * (_reserveLevel / 100.0f);
    float available = getStoredLiters() - reserveLiters;
    return available > 0.0f ? available : 0.0f;
}

float TerraWaterStorage::freeCapacityLiters() const
{
    float free = _capacityLiters - getStoredLiters();
    return free > 0.0f ? free : 0.0f;
}

void TerraWaterStorage::update(uint32_t now)
{
    if (!_levelSensor.isSet()) { return; }

    TerraMeasurement measurement = _levelSensor.getMeasurement(now, true);
    if (measurement.valid && measurement.unit == Terra_Unit_Percent) {
        setLevel(measurement.value);
    }
}

void TerraWaterStorage::unresolveAny(TerraObject *object)
{
    _levelSensor.unresolveAny(object);
    TerraResource::unresolveAny(object);
}


TerraCistern::TerraCistern(float capacityLiters, uint32_t key, const TerraString &name)
    : TerraWaterStorage(capacityLiters, key, name, Terra_WaterStorageType_Cistern),
      _fillStartPercent(35.0f), _fillStopPercent(90.0f), _overflowPercent(99.0f),
      _overflowDetected(false), _totalInflowLiters(0.0f), _totalOutflowLiters(0.0f),
      _overflowLiters(0.0f)
{ ; }

bool TerraCistern::configureFillBand(float startPercent, float stopPercent, float overflowPercent)
{
    if (startPercent < 0.0f || startPercent >= stopPercent ||
        stopPercent >= overflowPercent || overflowPercent > 100.0f) { return false; }

    _fillStartPercent = startPercent;
    _fillStopPercent = stopPercent;
    _overflowPercent = overflowPercent;
    return true;
}

bool TerraCistern::canAcceptWater() const
{
    return isEnabled() && !hasFault() && !_overflowDetected && getLevel() < _overflowPercent;
}

bool TerraCistern::canSupplyWater() const
{
    return isEnabled() && !hasFault() && availableAboveReserveLiters() > 0.0f;
}

bool TerraCistern::needsFill(bool currentlyFilling) const
{
    if (!canAcceptWater()) { return false; }
    return currentlyFilling ? getLevel() < _fillStopPercent : getLevel() <= _fillStartPercent;
}

float TerraCistern::fillRequestLiters() const
{
    if (!canAcceptWater() || _capacityLiters <= 0.0f) { return 0.0f; }

    float targetLiters = _capacityLiters * (_fillStopPercent / 100.0f);
    float requested = targetLiters - getStoredLiters();
    return requested > 0.0f ? requested : 0.0f;
}

float TerraCistern::safeFillCapacityLiters() const
{
    if (_capacityLiters <= 0.0f) { return 0.0f; }

    float safeLimitLiters = _capacityLiters * (_overflowPercent / 100.0f);
    float free = safeLimitLiters - getStoredLiters();
    return free > 0.0f ? free : 0.0f;
}

float TerraCistern::receiveWater(float liters)
{
    if (liters <= 0.0f || !canAcceptWater() || _capacityLiters <= 0.0f) { return 0.0f; }

    float accepted = min(liters, safeFillCapacityLiters());
    float overflow = liters - accepted;

    setStoredLiters(getStoredLiters() + accepted);
    _totalInflowLiters += accepted;
    if (overflow > 0.0f) { _overflowLiters += overflow; }
    return accepted;
}

float TerraCistern::drawWater(float liters, bool allowReserve)
{
    if (liters <= 0.0f || !isEnabled() || hasFault() || _capacityLiters <= 0.0f) { return 0.0f; }

    float available = allowReserve ? getStoredLiters() : availableAboveReserveLiters();
    float delivered = min(liters, available);

    setStoredLiters(getStoredLiters() - delivered);
    _totalOutflowLiters += delivered;
    return delivered;
}

void TerraCistern::resetFlowTotals()
{
    _totalInflowLiters = 0.0f;
    _totalOutflowLiters = 0.0f;
    _overflowLiters = 0.0f;
}


TerraWaterSource::TerraWaterSource(Terra_WaterSourceType type, uint8_t priority,
                                   uint32_t key, const TerraString &name)
    : TerraObject(Terra_ObjectType_WaterSource, key, name),
      _type(type), _priority(priority), _available(true),
      _level(100.0f), _reserveLevel(0.0f), _maximumFlowLpm(0.0f),
      _levelSensor(this)
{ ; }

void TerraWaterSource::update(uint32_t now)
{
    if (!_levelSensor.isSet()) { return; }

    TerraMeasurement measurement = _levelSensor.getMeasurement(now, true);
    if (measurement.valid && measurement.unit == Terra_Unit_Percent) {
        setLevel(measurement.value);
    }
}

void TerraWaterSource::unresolveAny(TerraObject *object)
{
    _levelSensor.unresolveAny(object);
    TerraObject::unresolveAny(object);
}


TerraWaterRoute::TerraWaterRoute(uint32_t key, const TerraString &name)
    : TerraObject(Terra_ObjectType_WaterRoute, key, name),
      _destinationStartPercent(40.0f), _destinationStopPercent(90.0f),
      _minimumFlowLpm(0.0f), _maximumFlowLpm(0.0f),
      _routeState(Terra_RouteState_Idle), _balancer(this)
{ ; }

bool TerraWaterRoute::setDestinationBand(float startPercent, float stopPercent)
{
    if (startPercent < 0.0f || startPercent >= stopPercent || stopPercent > 100.0f) { return false; }

    _destinationStartPercent = startPercent;
    _destinationStopPercent = stopPercent;
    return true;
}

bool TerraWaterRoute::validateFlow(float measuredFlowLpm, bool commandedOn)
{
    bool fault = !commandedOn ? measuredFlowLpm > _minimumFlowLpm
                                : measuredFlowLpm < _minimumFlowLpm ||
                                  (_maximumFlowLpm > 0.0f && measuredFlowLpm > _maximumFlowLpm);
    if (fault) {
        _routeState = Terra_RouteState_Fault;
        setFault(commandedOn ? TerraString("route flow outside limits")
                             : TerraString("unexpected flow while route idle"));
    }
    return !fault;
}

void TerraWaterRoute::update(uint32_t now)
{
    _balancer.update(now);
}

void TerraWaterRoute::unresolveAny(TerraObject *object)
{
    _balancer.unresolveAny(object);
    TerraObject::unresolveAny(object);
}


TerraRainCatchment::TerraRainCatchment(float areaSquareMeters, float collectionEfficiency,
                                       uint32_t key, const TerraString &name)
    : TerraObject(Terra_ObjectType_RainCatchment, key, name),
      _areaSquareMeters(areaSquareMeters < 0.0f ? 0.0f : areaSquareMeters),
      _collectionEfficiency(constrain(collectionEfficiency, 0.0f, 1.0f))
{ ; }

float TerraRainCatchment::estimateCaptureLiters(float rainfallMm) const
{
    if (rainfallMm <= 0.0f || _areaSquareMeters <= 0.0f) { return 0.0f; }

    // 1 mm of rain over 1 m^2 equals 1 liter of water.
    return rainfallMm * _areaSquareMeters * _collectionEfficiency;
}

float TerraRainCatchment::estimateCaptureGallons(float rainfallInches) const
{
    if (rainfallInches <= 0.0f) { return 0.0f; }

    float rainfallMm = rainfallInches * 25.4f;
    return estimateCaptureLiters(rainfallMm) / 3.785411784f;
}

TerraRainCollectionResult TerraRainCatchment::collectInto(TerraCistern &cistern, float rainfallMm,
                                                          TerraFirstFlushController *firstFlush) const
{
    TerraRainCollectionResult result;
    result.capturedLiters = estimateCaptureLiters(rainfallMm);
    if (result.capturedLiters <= 0.0f) { return result; }

    if (firstFlush && firstFlush->shouldDivert()) {
        result.discardedLiters = min(result.capturedLiters, firstFlush->getRemainingLiters());
        firstFlush->recordFlow(result.discardedLiters);
    }

    float storageInput = result.capturedLiters - result.discardedLiters;
    result.storedLiters = cistern.receiveWater(storageInput);
    result.overflowLiters = storageInput - result.storedLiters;
    return result;
}


TerraFirstFlushController::TerraFirstFlushController(float discardLiters)
    : _discardLiters(discardLiters < 0.0f ? 0.0f : discardLiters),
      _discardedLiters(0.0f)
{ ; }

void TerraFirstFlushController::reset()
{
    _discardedLiters = 0.0f;
}

void TerraFirstFlushController::recordFlow(float liters)
{
    if (liters > 0.0f) { _discardedLiters += liters; }
}

float TerraFirstFlushController::getRemainingLiters() const
{
    float remain = _discardLiters - _discardedLiters;
    return remain > 0.0f ? remain : 0.0f;
}
