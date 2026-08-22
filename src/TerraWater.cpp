/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Water Management
*/

#include "Terraduino.h"
#include <string.h>

TerraWaterStorage::TerraWaterStorage(float capacityLiters, tposi_t storageIndex, const TerraString &name,
                                     Terra_WaterStorageType storageType)
    : TerraReservoir(TerraIdentity(storageType, storageIndex), name),
      _capacityLiters(capacityLiters < 0.0f ? 0.0f : capacityLiters), _levelSensor(this)
{ ; }

TerraWaterStorage::TerraWaterStorage(const TerraWaterStorageData *dataIn)
    : TerraReservoir(dataIn), _capacityLiters(dataIn ? dataIn->capacityLiters : 0.0f), _levelSensor(this)
{
    if (dataIn && dataIn->levelSensor[0]) { _levelSensor.initObject(dataIn->levelSensor); }
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
    TerraReservoir::update(now);
    if (!_levelSensor.isSet()) { return; }

    TerraSingleMeasurement measurement = _levelSensor.getMeasurement(now, true);
    if (measurement.isSet() && measurement.units == Terra_Unit_Percent) {
        setLevel(measurement.value);
    }
}

void TerraWaterStorage::unresolveAny(TerraObject *object)
{
    _levelSensor.unresolveAny(object);
    TerraReservoir::unresolveAny(object);
}

TerraData *TerraWaterStorage::allocateData() const
{
    return new TerraWaterStorageData();
}

void TerraWaterStorage::saveToData(TerraData *dataOut) const
{
    TerraReservoir::saveToData(dataOut);
    auto data = static_cast<TerraWaterStorageData *>(dataOut);
    data->capacityLiters = _capacityLiters;
    if (_levelSensor.isSet()) {
        strncpy(data->levelSensor, _levelSensor.getKeyString().c_str(), TERRA_NAME_MAXSIZE - 1);
        data->levelSensor[TERRA_NAME_MAXSIZE - 1] = '\0';
    }
}


TerraCistern::TerraCistern(float capacityLiters, tposi_t storageIndex, const TerraString &name)
    : TerraWaterStorage(capacityLiters, storageIndex, name, Terra_WaterStorageType_Cistern),
      _fillStartPercent(35.0f), _fillStopPercent(90.0f), _overflowPercent(99.0f),
      _overflowDetected(false), _totalInflowLiters(0.0f), _totalOutflowLiters(0.0f),
      _overflowLiters(0.0f)
{ ; }

TerraCistern::TerraCistern(const TerraCisternData *dataIn)
    : TerraWaterStorage(dataIn),
      _fillStartPercent(dataIn ? dataIn->fillStartPercent : 35.0f),
      _fillStopPercent(dataIn ? dataIn->fillStopPercent : 90.0f),
      _overflowPercent(dataIn ? dataIn->overflowPercent : 99.0f),
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
    bumpRevisionIfNeeded();
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

TerraData *TerraCistern::allocateData() const
{
    return new TerraCisternData();
}

void TerraCistern::saveToData(TerraData *dataOut) const
{
    TerraWaterStorage::saveToData(dataOut);
    auto data = static_cast<TerraCisternData *>(dataOut);
    data->fillStartPercent = _fillStartPercent;
    data->fillStopPercent = _fillStopPercent;
    data->overflowPercent = _overflowPercent;
}


TerraWaterSource::TerraWaterSource(Terra_WaterSourceType type, uint8_t priority,
                                   tposi_t sourceIndex, const TerraString &name)
    : TerraObject(TerraIdentity(type, sourceIndex), name),
      _priority(priority), _available(true), _level(100.0f), _reserveLevel(0.0f),
      _maximumFlowLpm(0.0f), _levelSensor(this)
{ ; }

TerraWaterSource::TerraWaterSource(const TerraWaterSourceData *dataIn)
    : TerraObject(dataIn), _priority(dataIn ? dataIn->priority : 0),
      _available(dataIn ? dataIn->available : true), _level(dataIn ? dataIn->level : 100.0f),
      _reserveLevel(dataIn ? dataIn->reserveLevel : 0.0f),
      _maximumFlowLpm(dataIn ? dataIn->maximumFlowLpm : 0.0f), _levelSensor(this)
{
    if (dataIn && dataIn->levelSensor[0]) { _levelSensor.initObject(dataIn->levelSensor); }
}

void TerraWaterSource::update(uint32_t now)
{
    TerraObject::update(now);
    if (!_levelSensor.isSet()) { return; }

    TerraSingleMeasurement measurement = _levelSensor.getMeasurement(now, true);
    if (measurement.isSet() && measurement.units == Terra_Unit_Percent) {
        setLevel(measurement.value);
    }
}

void TerraWaterSource::unresolveAny(TerraObject *object)
{
    _levelSensor.unresolveAny(object);
    TerraObject::unresolveAny(object);
}

TerraData *TerraWaterSource::allocateData() const
{
    return new TerraWaterSourceData();
}

void TerraWaterSource::saveToData(TerraData *dataOut) const
{
    TerraObject::saveToData(dataOut);
    auto data = static_cast<TerraWaterSourceData *>(dataOut);
    data->priority = _priority;
    data->available = _available;
    data->level = _level;
    data->reserveLevel = _reserveLevel;
    data->maximumFlowLpm = _maximumFlowLpm;
    if (_levelSensor.isSet()) {
        strncpy(data->levelSensor, _levelSensor.getKeyString().c_str(), TERRA_NAME_MAXSIZE - 1);
        data->levelSensor[TERRA_NAME_MAXSIZE - 1] = '\0';
    }
}


TerraWaterRoute::TerraWaterRoute(tposi_t routeIndex, const TerraString &name)
    : TerraObject(TerraIdentity(Terra_ObjectType_WaterRoute, routeIndex), name),
      _destinationStartPercent(40.0f), _destinationStopPercent(90.0f),
      _minimumFlowLpm(0.0f), _maximumFlowLpm(0.0f),
      _routeState(Terra_RouteState_Idle), _balancer(this)
{ ; }

TerraWaterRoute::TerraWaterRoute(const TerraWaterRouteData *dataIn)
    : TerraObject(dataIn),
      _destinationStartPercent(dataIn ? dataIn->destinationStartPercent : 40.0f),
      _destinationStopPercent(dataIn ? dataIn->destinationStopPercent : 90.0f),
      _minimumFlowLpm(dataIn ? dataIn->minimumFlowLpm : 0.0f),
      _maximumFlowLpm(dataIn ? dataIn->maximumFlowLpm : 0.0f),
      _routeState(Terra_RouteState_Idle), _balancer(this)
{
    if (dataIn) {
        if (dataIn->source[0]) { _balancer.getSourceAttachment().initObject(dataIn->source); }
        if (dataIn->destination[0]) { _balancer.getDestinationAttachment().initObject(dataIn->destination); }
        if (dataIn->pump[0]) { _balancer.getPumpAttachment().initObject(dataIn->pump); }
        if (dataIn->flowSensor[0]) { _balancer.getFlowSensorAttachment().initObject(dataIn->flowSensor); }
    }
}

bool TerraWaterRoute::setDestinationBand(float startPercent, float stopPercent)
{
    if (startPercent < 0.0f || startPercent >= stopPercent || stopPercent > 100.0f) { return false; }

    _destinationStartPercent = startPercent;
    _destinationStopPercent = stopPercent;
    bumpRevisionIfNeeded();
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
    TerraObject::update(now);
    _balancer.update(now);
}

void TerraWaterRoute::unresolveAny(TerraObject *object)
{
    _balancer.unresolveAny(object);
    TerraObject::unresolveAny(object);
}

TerraData *TerraWaterRoute::allocateData() const
{
    return new TerraWaterRouteData();
}

void TerraWaterRoute::saveToData(TerraData *dataOut) const
{
    TerraObject::saveToData(dataOut);
    auto data = static_cast<TerraWaterRouteData *>(dataOut);
    auto copyAttachment = [](char *destination, const TerraAttachment &attachment) {
        if (attachment.isSet()) {
            strncpy(destination, attachment.getKeyString().c_str(), TERRA_NAME_MAXSIZE - 1);
            destination[TERRA_NAME_MAXSIZE - 1] = '\0';
        }
    };
    copyAttachment(data->source, _balancer.getSourceAttachment());
    copyAttachment(data->destination, _balancer.getDestinationAttachment());
    copyAttachment(data->pump, _balancer.getPumpAttachment());
    copyAttachment(data->flowSensor, _balancer.getFlowSensorAttachment());
    data->destinationStartPercent = _destinationStartPercent;
    data->destinationStopPercent = _destinationStopPercent;
    data->minimumFlowLpm = _minimumFlowLpm;
    data->maximumFlowLpm = _maximumFlowLpm;
}


TerraRainCatchment::TerraRainCatchment(float areaSquareMeters, float collectionEfficiency,
                                       tposi_t catchmentIndex, const TerraString &name)
    : TerraObject(TerraIdentity(Terra_ObjectType_RainCatchment, catchmentIndex), name),
      _areaSquareMeters(areaSquareMeters < 0.0f ? 0.0f : areaSquareMeters),
      _collectionEfficiency(constrain(collectionEfficiency, 0.0f, 1.0f))
{ ; }

TerraRainCatchment::TerraRainCatchment(const TerraRainCatchmentData *dataIn)
    : TerraObject(dataIn),
      _areaSquareMeters(dataIn ? dataIn->areaSquareMeters : 0.0f),
      _collectionEfficiency(dataIn ? dataIn->collectionEfficiency : 0.85f)
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

TerraData *TerraRainCatchment::allocateData() const
{
    return new TerraRainCatchmentData();
}

void TerraRainCatchment::saveToData(TerraData *dataOut) const
{
    TerraObject::saveToData(dataOut);
    auto data = static_cast<TerraRainCatchmentData *>(dataOut);
    data->areaSquareMeters = _areaSquareMeters;
    data->collectionEfficiency = _collectionEfficiency;
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
