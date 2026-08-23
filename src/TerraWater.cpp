/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Water Management
*/

#include "Terraduino.h"
#include <string.h>

TerraWaterStorage::TerraWaterStorage(float capacityLiters, tposi_t storageIndex, const TerraString &name,
                                     int classTypeIn)
    : TerraReservoir(Terra_ReservoirType_Water, storageIndex, name, classTypeIn),
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
    if (measurement.isSet() && measurement.units == Terra_UnitsType_Percentile_100) {
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
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
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
    : TerraWaterStorage(capacityLiters, storageIndex, name, TerraReservoir::Cistern),
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
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void TerraCistern::saveToData(TerraData *dataOut) const
{
    TerraWaterStorage::saveToData(dataOut);
    auto data = static_cast<TerraCisternData *>(dataOut);
    data->fillStartPercent = _fillStartPercent;
    data->fillStopPercent = _fillStopPercent;
    data->overflowPercent = _overflowPercent;
}


TerraWaterSource::TerraWaterSource(uint8_t priority, tposi_t sourceIndex, const TerraString &name)
    : TerraReservoir(Terra_ReservoirType_Water, sourceIndex, name, TerraReservoir::WaterSource),
      _priority(priority), _available(true), _maximumFlowLpm(0.0f), _levelSensor(this)
{
    _level = 100.0f;
    _reserveLevel = 0.0f;
    updateState();
}

TerraWaterSource::TerraWaterSource(const TerraWaterSourceData *dataIn)
    : TerraReservoir(dataIn), _priority(dataIn ? dataIn->priority : 0),
      _available(dataIn ? dataIn->available : true),
      _maximumFlowLpm(dataIn ? dataIn->maximumFlowLpm : 0.0f), _levelSensor(this)
{
    if (dataIn && dataIn->levelSensor[0]) { _levelSensor.initObject(dataIn->levelSensor); }
}

void TerraWaterSource::setReserveLevel(float reserve)
{
    float reserveLevel = constrain(reserve, 0.0f, 100.0f);
    if (!isFPEqual(_reserveLevel, reserveLevel)) {
        _reserveLevel = reserveLevel;
        updateState();
        bumpRevisionIfNeeded();
    }
}

void TerraWaterSource::update(uint32_t now)
{
    TerraReservoir::update(now);
    if (!_levelSensor.isSet()) { return; }

    TerraSingleMeasurement measurement = _levelSensor.getMeasurement(now, true);
    if (measurement.isSet() && measurement.units == Terra_UnitsType_Percentile_100) {
        setLevel(measurement.value);
    }
}

void TerraWaterSource::unresolveAny(TerraObject *object)
{
    _levelSensor.unresolveAny(object);
    TerraReservoir::unresolveAny(object);
}

TerraData *TerraWaterSource::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void TerraWaterSource::saveToData(TerraData *dataOut) const
{
    TerraReservoir::saveToData(dataOut);
    auto data = static_cast<TerraWaterSourceData *>(dataOut);
    data->priority = _priority;
    data->available = _available;
    data->maximumFlowLpm = _maximumFlowLpm;
    if (_levelSensor.isSet()) {
        strncpy(data->levelSensor, _levelSensor.getKeyString().c_str(), TERRA_NAME_MAXSIZE - 1);
        data->levelSensor[TERRA_NAME_MAXSIZE - 1] = '\0';
    }
}


TerraWaterRoute::TerraWaterRoute(tposi_t routeIndex, const TerraString &name)
    : TerraObject(TerraIdentity(Terra_ObjectType_WaterRoute, routeIndex), name), classType(Route),
      _destinationStartPercent(40.0f), _destinationStopPercent(90.0f),
      _minimumFlowLpm(0.0f), _maximumFlowLpm(0.0f),
      _routeState(Terra_RouteState_Idle), _balancer(this)
{ ; }

TerraWaterRoute::TerraWaterRoute(const TerraWaterRouteData *dataIn)
    : TerraObject(dataIn), classType(static_cast<decltype(Route)>(dataIn ? (int)dataIn->id.object.classType : (int)Unknown)),
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
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void TerraWaterRoute::saveToData(TerraData *dataOut) const
{
    TerraObject::saveToData(dataOut);
    dataOut->id.object.classType = (tid_t)classType;
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
    : TerraObject(TerraIdentity(Terra_ObjectType_RainCatchment, catchmentIndex), name), classType(Catchment),
      _areaSquareMeters(areaSquareMeters < 0.0f ? 0.0f : areaSquareMeters),
      _collectionEfficiency(constrain(collectionEfficiency, 0.0f, 1.0f))
{ ; }

TerraRainCatchment::TerraRainCatchment(const TerraRainCatchmentData *dataIn)
    : TerraObject(dataIn), classType(static_cast<decltype(Catchment)>(dataIn ? (int)dataIn->id.object.classType : (int)Unknown)),
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
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void TerraRainCatchment::saveToData(TerraData *dataOut) const
{
    TerraObject::saveToData(dataOut);
    dataOut->id.object.classType = (tid_t)classType;
    auto data = static_cast<TerraRainCatchmentData *>(dataOut);
    data->areaSquareMeters = _areaSquareMeters;
    data->collectionEfficiency = _collectionEfficiency;
}


TerraWaterRouteData::TerraWaterRouteData()
    : TerraObjectData(), source{0}, destination{0}, pump{0}, flowSensor{0},
      destinationStartPercent(40.0f), destinationStopPercent(90.0f),
      minimumFlowLpm(0.0f), maximumFlowLpm(0.0f)
{
    _size = sizeof(*this);
    id.object.idType = (tid_t)Terra_ObjectType_WaterRoute;
    id.object.objType = 0;
    id.object.posIndex = TERRA_POS_SEARCH_FROMBEG;
    id.object.classType = (tid_t)TerraWaterRoute::Route;
}

void TerraWaterRouteData::toJSONObject(JsonObject &objectOut) const
{
    TerraObjectData::toJSONObject(objectOut);
    if (source[0]) { objectOut["source"] = source; }
    if (destination[0]) { objectOut["destination"] = destination; }
    if (pump[0]) { objectOut["pump"] = pump; }
    if (flowSensor[0]) { objectOut["flowSensor"] = flowSensor; }
    objectOut["destinationStartPercent"] = destinationStartPercent;
    objectOut["destinationStopPercent"] = destinationStopPercent;
    objectOut["minimumFlowLpm"] = minimumFlowLpm;
    objectOut["maximumFlowLpm"] = maximumFlowLpm;
}

void TerraWaterRouteData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraObjectData::fromJSONObject(objectIn);
    auto copyString = [](char *destinationOut, JsonVariantConst sourceIn) {
        const char *value = sourceIn | nullptr;
        if (value) {
            strncpy(destinationOut, value, TERRA_NAME_MAXSIZE - 1);
            destinationOut[TERRA_NAME_MAXSIZE - 1] = '\0';
        }
    };
    copyString(source, objectIn["source"]);
    copyString(destination, objectIn["destination"]);
    copyString(pump, objectIn["pump"]);
    copyString(flowSensor, objectIn["flowSensor"]);
    destinationStartPercent = objectIn["destinationStartPercent"] | destinationStartPercent;
    destinationStopPercent = objectIn["destinationStopPercent"] | destinationStopPercent;
    minimumFlowLpm = objectIn["minimumFlowLpm"] | minimumFlowLpm;
    maximumFlowLpm = objectIn["maximumFlowLpm"] | maximumFlowLpm;
}

TerraRainCatchmentData::TerraRainCatchmentData()
    : TerraObjectData(), areaSquareMeters(0.0f), collectionEfficiency(0.85f)
{
    _size = sizeof(*this);
    id.object.idType = (tid_t)Terra_ObjectType_RainCatchment;
    id.object.objType = 0;
    id.object.posIndex = TERRA_POS_SEARCH_FROMBEG;
    id.object.classType = (tid_t)TerraRainCatchment::Catchment;
}

void TerraRainCatchmentData::toJSONObject(JsonObject &objectOut) const
{
    TerraObjectData::toJSONObject(objectOut);
    objectOut["areaSquareMeters"] = areaSquareMeters;
    objectOut["collectionEfficiency"] = collectionEfficiency;
}

void TerraRainCatchmentData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraObjectData::fromJSONObject(objectIn);
    areaSquareMeters = objectIn["areaSquareMeters"] | areaSquareMeters;
    collectionEfficiency = objectIn["collectionEfficiency"] | collectionEfficiency;
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
