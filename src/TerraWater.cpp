/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Water Management
*/

#include "TerraWater.h"
#include "TerraUtils.h"
#include "TerraCoreLogic.h"

TerraWaterStorage::TerraWaterStorage(float capacityLiters, uint32_t key, const TerraString &name,
                                         Terra_WaterStorageType storageType)
    : TerraResource(Terra_ResourceType_Water, key, name),
      _capacityLiters(capacityLiters < 0.0f ? 0.0f : capacityLiters),
      _storageType(storageType), _attachments() {
    _objectType = Terra_ObjectType_WaterStorage;
}

void TerraWaterStorage::setStoredLiters(float liters) {
    if (_capacityLiters <= 0.0f) { setLevel(0.0f); return; }
    setLevel(liters / _capacityLiters * 100.0f);
}

float TerraWaterStorage::availableAboveReserveLiters() const {
    float reserveLiters = _capacityLiters * (_reserveLevel / 100.0f);
    float available = getStoredLiters() - reserveLiters;
    return available > 0.0f ? available : 0.0f;
}

TerraCistern::TerraCistern(float capacityLiters, uint32_t key, const TerraString &name)
    : TerraWaterStorage(capacityLiters, key, name, Terra_WaterStorageType_Cistern),
      _fillStartPercent(35.0f), _fillStopPercent(90.0f), _overflowPercent(99.0f),
      _overflowDetected(false), _totalInflowLiters(0.0f), _totalOutflowLiters(0.0f),
      _overflowLiters(0.0f) { }

bool TerraCistern::configureFillBand(float startPercent, float stopPercent, float overflowPercent) {
    if (startPercent < 0.0f || startPercent >= stopPercent ||
        stopPercent >= overflowPercent || overflowPercent > 100.0f) return false;
    _fillStartPercent = startPercent;
    _fillStopPercent = stopPercent;
    _overflowPercent = overflowPercent;
    return true;
}

bool TerraCistern::canAcceptWater() const {
    return isEnabled() && !hasFault() && !_overflowDetected && getLevel() < _overflowPercent;
}

bool TerraCistern::canSupplyWater() const {
    return isEnabled() && !hasFault() && availableAboveReserveLiters() > 0.0f;
}

bool TerraCistern::needsFill(bool currentlyFilling) const {
    if (!canAcceptWater()) return false;
    return currentlyFilling ? getLevel() < _fillStopPercent : getLevel() <= _fillStartPercent;
}

float TerraCistern::fillRequestLiters() const {
    if (!canAcceptWater() || _capacityLiters <= 0.0f) return 0.0f;
    const float targetLiters = _capacityLiters * (_fillStopPercent / 100.0f);
    const float requested = targetLiters - getStoredLiters();
    return requested > 0.0f ? requested : 0.0f;
}

float TerraCistern::safeFillCapacityLiters() const {
    if (_capacityLiters <= 0.0f) return 0.0f;
    const float safeLimitLiters = _capacityLiters * (_overflowPercent / 100.0f);
    const float free = safeLimitLiters - getStoredLiters();
    return free > 0.0f ? free : 0.0f;
}

float TerraCistern::receiveWater(float liters) {
    if (liters <= 0.0f || !canAcceptWater() || _capacityLiters <= 0.0f) return 0.0f;
    const float accepted = terraMin(liters, safeFillCapacityLiters());
    const float overflow = liters - accepted;
    setStoredLiters(getStoredLiters() + accepted);
    _totalInflowLiters += accepted;
    if (overflow > 0.0f) _overflowLiters += overflow;
    return accepted;
}

float TerraCistern::drawWater(float liters, bool allowReserve) {
    if (liters <= 0.0f || !isEnabled() || hasFault() || _capacityLiters <= 0.0f) return 0.0f;
    const float available = allowReserve ? getStoredLiters() : availableAboveReserveLiters();
    const float delivered = terraMin(liters, available);
    setStoredLiters(getStoredLiters() - delivered);
    _totalOutflowLiters += delivered;
    return delivered;
}

void TerraCistern::resetFlowTotals() {
    _totalInflowLiters = 0.0f;
    _totalOutflowLiters = 0.0f;
    _overflowLiters = 0.0f;
}

TerraWaterSource::TerraWaterSource(Terra_WaterSourceType type, uint8_t priority, uint32_t key, const TerraString &name)
    : TerraObject(Terra_ObjectType_WaterSource, key, name), _type(type), _priority(priority), _available(true),
      _level(100.0f), _reserveLevel(0.0f), _maximumFlowLpm(0.0f) { }

TerraWaterRoute::TerraWaterRoute(uint32_t key, const TerraString &name)
    : TerraObject(Terra_ObjectType_WaterRoute, key, name), _sourceKey(0), _destinationKey(0),
      _destinationStartPercent(40.0f), _destinationStopPercent(90.0f), _minimumFlowLpm(0.0f),
      _maximumFlowLpm(0.0f), _routeState(Terra_RouteState_Idle), _attachments() { }

void TerraWaterRoute::configure(uint32_t sourceKey, uint32_t destinationKey, float destinationStartPercent, float destinationStopPercent) {
    _sourceKey = sourceKey;
    _destinationKey = destinationKey;
    _destinationStartPercent = terraClamp(destinationStartPercent, 0.0f, 100.0f);
    _destinationStopPercent = terraClamp(destinationStopPercent, _destinationStartPercent, 100.0f);
}

TerraTransferDecision TerraWaterRoute::evaluate(const TerraWaterSource &source, const TerraWaterStorage &destination) {
    TerraTransferDecision decision = terraEvaluateWaterTransfer(source.isAvailable(), source.getLevel(), source.getReserveLevel(),
                                                               destination.getLevel(), _destinationStartPercent, _destinationStopPercent,
                                                               hasFault(), _routeState == Terra_RouteState_Requested || _routeState == Terra_RouteState_Active);
    _routeState = decision.state;
    return decision;
}

TerraTransferDecision TerraWaterRoute::evaluate(const TerraWaterSource &source, const TerraCistern &destination) {
    if (destination.isOverflowDetected()) {
        _routeState = Terra_RouteState_Fault;
        return TerraTransferDecision(false, _routeState, "cistern overflow detected");
    }
    if (!destination.isEnabled() || destination.hasFault()) {
        _routeState = Terra_RouteState_Fault;
        return TerraTransferDecision(false, _routeState, "cistern unavailable");
    }
    TerraTransferDecision decision = terraEvaluateWaterTransfer(source.isAvailable(), source.getLevel(), source.getReserveLevel(),
                                                               destination.getLevel(), destination.getFillStartPercent(), destination.getFillStopPercent(),
                                                               hasFault(), _routeState == Terra_RouteState_Requested || _routeState == Terra_RouteState_Active);
    _routeState = decision.state;
    return decision;
}

bool TerraWaterRoute::validateFlow(float measuredFlowLpm, bool commandedOn) {
    bool fault = terraFlowFault(commandedOn, measuredFlowLpm, _minimumFlowLpm, _maximumFlowLpm);
    if (fault) {
        _routeState = Terra_RouteState_Fault;
        setFault(commandedOn ? TerraString("route flow outside limits") : TerraString("unexpected flow while route idle"));
    } else if (hasFault()) {
        clearFault();
    }
    return !fault;
}

TerraRainCatchment::TerraRainCatchment(float areaSquareMeters, float collectionEfficiency,
                                         uint32_t key, const TerraString &name)
    : TerraObject(Terra_ObjectType_RainCatchment, key, name),
      _areaSquareMeters(areaSquareMeters < 0.0f ? 0.0f : areaSquareMeters),
      _collectionEfficiency(terraClamp(collectionEfficiency, 0.0f, 1.0f)), _attachments() { }

float TerraRainCatchment::estimateCaptureLiters(float rainfallMm) const
{
    if (rainfallMm <= 0.0f || _areaSquareMeters <= 0.0f) return 0.0f;
    // 1 mm of rain over 1 m^2 equals 1 liter of water.
    return rainfallMm * _areaSquareMeters * _collectionEfficiency;
}

float TerraRainCatchment::estimateCaptureGallons(float rainfallInches) const
{
    if (rainfallInches <= 0.0f) return 0.0f;
    const float rainfallMm = rainfallInches * 25.4f;
    return estimateCaptureLiters(rainfallMm) / 3.785411784f;
}

TerraRainCollectionResult TerraRainCatchment::collectInto(TerraCistern &cistern, float rainfallMm,
                                                          TerraFirstFlushController *firstFlush) const
{
    TerraRainCollectionResult result;
    result.capturedLiters = estimateCaptureLiters(rainfallMm);
    if (result.capturedLiters <= 0.0f) return result;

    if (firstFlush && firstFlush->shouldDivert()) {
        result.discardedLiters = terraMin(result.capturedLiters, firstFlush->getRemainingLiters());
        firstFlush->recordFlow(result.discardedLiters);
    }

    const float storageInput = result.capturedLiters - result.discardedLiters;
    result.storedLiters = cistern.receiveWater(storageInput);
    result.overflowLiters = storageInput - result.storedLiters;
    return result;
}

TerraFirstFlushController::TerraFirstFlushController(float discardLiters)
    : _discardLiters(discardLiters < 0.0f ? 0.0f : discardLiters), _discardedLiters(0.0f) { }

void TerraFirstFlushController::reset() { _discardedLiters = 0.0f; }
void TerraFirstFlushController::recordFlow(float liters) { if (liters > 0.0f) _discardedLiters += liters; }
float TerraFirstFlushController::getRemainingLiters() const { float remain = _discardLiters - _discardedLiters; return remain > 0.0f ? remain : 0.0f; }
