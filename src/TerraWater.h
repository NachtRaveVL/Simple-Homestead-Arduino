/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Water Management
*/

#ifndef TerraWater_H
#define TerraWater_H

class TerraFirstFlushController;                            // Terra First Flush Controller

struct TerraWaterStorageData;
struct TerraCisternData;
struct TerraWaterSourceData;
struct TerraWaterRouteData;
struct TerraRainCatchmentData;

#include "TerraReservoir.h"
#include "TerraUtils.h"
#include "TerraAttachments.h"
#include "TerraBalancers.h"
#include "TerraDatas.h"

// Rainwater Collection Result
// Summarizes one rainfall capture event after first-flush and cistern limits are applied.
struct TerraRainCollectionResult {
    float capturedLiters;                                   // Captured rainfall volume, liters
    float discardedLiters;                                  // First-flush discarded volume, liters
    float storedLiters;                                     // Volume stored in cistern, liters
    float overflowLiters;                                   // Overflow volume, liters

    TerraRainCollectionResult(float captured = 0.0f, float discarded = 0.0f,
                              float stored = 0.0f, float overflow = 0.0f)
        : capturedLiters(captured), discardedLiters(discarded),
          storedLiters(stored), overflowLiters(overflow) { }
};

// Water Storage
// Base water inventory with capacity, reserve, and attachment relationships.
class TerraWaterStorage : public TerraReservoir {
public:
    TerraWaterStorage(float capacityLiters = 0.0f,
                      tposi_t storageIndex = TERRA_POS_SEARCH_FROMBEG,
                      const TerraString &name = TerraString(),
                      Terra_WaterStorageType storageType = Terra_WaterStorageType_Undefined);  // Water storage type
    TerraWaterStorage(const TerraWaterStorageData *dataIn);

    void setCapacityLiters(float capacityLiters) { _capacityLiters = capacityLiters < 0.0f ? 0.0f : capacityLiters; bumpRevisionIfNeeded(); }
    float getCapacityLiters() const { return _capacityLiters; }
    Terra_WaterStorageType getStorageType() const { return _id.objTypeAs.waterStorageType; }
    float getStoredLiters() const { return _capacityLiters * (getLevel() / 100.0f); }
    void setStoredLiters(float liters);
    float availableAboveReserveLiters() const;
    float freeCapacityLiters() const;
    bool isAtReserve() const { return getLevel() <= getReserveLevel(); }
    bool isNearFull() const { return getLevel() >= getHighLevel(); }

    // Level Sensor Attachment Point
    template<class T> inline void setLevelSensor(const SharedPtr<T> &sensor) { _levelSensor.setObject(sensor); }
    inline TerraSensorAttachment &getLevelSensorAttachment() { return _levelSensor; }
    inline const TerraSensorAttachment &getLevelSensorAttachment() const { return _levelSensor; }

    virtual void update(uint32_t now = millis()) override;
    virtual void unresolveAny(TerraObject *object) override;

protected:
    float _capacityLiters;                                  // Storage capacity, liters
    TerraSensorAttachment _levelSensor;                     // Level sensor attachment point

    virtual TerraData *allocateData() const override;
    virtual void saveToData(TerraData *dataOut) const override;
};

// Cistern
// Water storage with fill hysteresis, protected reserve, overflow tracking, and flow totals.
class TerraCistern : public TerraWaterStorage {
public:
    TerraCistern(float capacityLiters = 0.0f,
                 tposi_t storageIndex = TERRA_POS_SEARCH_FROMBEG,
                 const TerraString &name = TerraString());
    TerraCistern(const TerraCisternData *dataIn);

    bool configureFillBand(float startPercent, float stopPercent, float overflowPercent = 99.0f);
    float getFillStartPercent() const { return _fillStartPercent; }
    float getFillStopPercent() const { return _fillStopPercent; }
    float getOverflowPercent() const { return _overflowPercent; }

    void setOverflowDetected(bool detected) { _overflowDetected = detected; }
    bool isOverflowDetected() const { return _overflowDetected; }
    bool canAcceptWater() const;
    bool canSupplyWater() const;
    bool needsFill(bool currentlyFilling = false) const;
    float fillRequestLiters() const;
    float safeDrawLiters() const { return availableAboveReserveLiters(); }
    float safeFillCapacityLiters() const;

    float receiveWater(float liters);
    float drawWater(float liters, bool allowReserve = false);
    float getTotalInflowLiters() const { return _totalInflowLiters; }
    float getTotalOutflowLiters() const { return _totalOutflowLiters; }
    float getOverflowLiters() const { return _overflowLiters; }
    void resetFlowTotals();

protected:
    float _fillStartPercent;                                // Fill-start threshold, percent
    float _fillStopPercent;                                 // Fill-stop threshold, percent
    float _overflowPercent;                                 // Overflow safety threshold, percent
    bool _overflowDetected;                                 // External overflow detection state
    float _totalInflowLiters;                               // Accumulated accepted inflow, liters
    float _totalOutflowLiters;                              // Accumulated delivered outflow, liters
    float _overflowLiters;                                  // Accumulated overflow volume, liters

    virtual TerraData *allocateData() const override;
    virtual void saveToData(TerraData *dataOut) const override;
};

// Water Source
// Describes a usable water source and its priority, reserve, availability, and flow limit.
class TerraWaterSource : public TerraObject {
public:
    TerraWaterSource(Terra_WaterSourceType type = Terra_WaterSourceType_Undefined,
                     uint8_t priority = 0,
                     tposi_t sourceIndex = TERRA_POS_SEARCH_FROMBEG,
                     const TerraString &name = TerraString());
    TerraWaterSource(const TerraWaterSourceData *dataIn);

    Terra_WaterSourceType getType() const { return _id.objTypeAs.waterSourceType; }
    uint8_t getPriority() const { return _priority; }
    bool isAvailable() const { return _available && !_fault && _enabled; }
    bool isConfiguredAvailable() const { return _available; }
    float getLevel() const { return _level; }
    float getReserveLevel() const { return _reserveLevel; }
    float getMaximumFlowLpm() const { return _maximumFlowLpm; }

    void setPriority(uint8_t priority) { _priority = priority; bumpRevisionIfNeeded(); }
    void setAvailable(bool available) { _available = available; bumpRevisionIfNeeded(); }
    void setLevel(float level) { _level = constrain(level, 0.0f, 100.0f); bumpRevisionIfNeeded(); }
    void setReserveLevel(float reserve) { _reserveLevel = constrain(reserve, 0.0f, 100.0f); bumpRevisionIfNeeded(); }
    void setMaximumFlowLpm(float flow) { _maximumFlowLpm = flow < 0.0f ? 0.0f : flow; bumpRevisionIfNeeded(); }

    // Level Sensor Attachment Point
    template<class T> inline void setLevelSensor(const SharedPtr<T> &sensor) { _levelSensor.setObject(sensor); }
    inline TerraSensorAttachment &getLevelSensorAttachment() { return _levelSensor; }
    inline const TerraSensorAttachment &getLevelSensorAttachment() const { return _levelSensor; }

    virtual void update(uint32_t now = millis()) override;
    virtual void unresolveAny(TerraObject *object) override;

protected:
    uint8_t _priority;                                      // Source priority
    bool _available;                                        // Configured availability
    float _level;                                           // Normalized level, percent
    float _reserveLevel;                                    // Protected reserve level, percent
    float _maximumFlowLpm;                                  // Maximum allowed flow
    TerraSensorAttachment _levelSensor;                     // Level sensor attachment point

    virtual TerraData *allocateData() const override;
    virtual void saveToData(TerraData *dataOut) const override;
};

// Water Route
// Defines one controlled transfer path between a source and destination storage object.
class TerraWaterRoute : public TerraObject {
public:
    TerraWaterRoute(tposi_t routeIndex = TERRA_POS_SEARCH_FROMBEG, const TerraString &name = TerraString());
    TerraWaterRoute(const TerraWaterRouteData *dataIn);

    template<class T> inline void setSource(const SharedPtr<T> &source) { _balancer.setSource(source); }
    template<class T> inline void setDestination(const SharedPtr<T> &destination) { _balancer.setDestination(destination); }
    template<class T> inline void setPump(const SharedPtr<T> &pump) { _balancer.setPump(pump); }
    template<class T> inline void setFlowSensor(const SharedPtr<T> &sensor) { _balancer.setFlowSensor(sensor); }

    bool setDestinationBand(float startPercent, float stopPercent);
    void setMinimumFlow(float lpm) { _minimumFlowLpm = lpm < 0.0f ? 0.0f : lpm; bumpRevisionIfNeeded(); }
    void setMaximumFlow(float lpm) { _maximumFlowLpm = lpm < 0.0f ? 0.0f : lpm; bumpRevisionIfNeeded(); }
    tkey_t getSourceKey() const { return _balancer.getSourceAttachment().getKey(); }
    tkey_t getDestinationKey() const { return _balancer.getDestinationAttachment().getKey(); }
    tkey_t getPumpKey() const { return _balancer.getPumpAttachment().getKey(); }
    tkey_t getFlowSensorKey() const { return _balancer.getFlowSensorAttachment().getKey(); }
    float getDestinationStartPercent() const { return _destinationStartPercent; }
    float getDestinationStopPercent() const { return _destinationStopPercent; }
    float getMinimumFlow() const { return _minimumFlowLpm; }
    float getMaximumFlow() const { return _maximumFlowLpm; }
    Terra_RouteState getRouteState() const { return _routeState; }

    inline TerraWaterBalancer &getBalancer() { return _balancer; }
    inline const TerraWaterBalancer &getBalancer() const { return _balancer; }

    virtual void update(uint32_t now = millis()) override;
    virtual void unresolveAny(TerraObject *object) override;

protected:
    float _destinationStartPercent;                         // Destination fill-start threshold
    float _destinationStopPercent;                          // Destination fill-stop threshold
    float _minimumFlowLpm;                                  // Minimum expected flow
    float _maximumFlowLpm;                                  // Maximum allowed flow
    Terra_RouteState _routeState;                           // Current route state
    TerraWaterBalancer _balancer;                           // Route balancing process

    bool validateFlow(float measuredFlowLpm, bool commandedOn);
    inline void setRouteState(Terra_RouteState state) { _routeState = state; }

    virtual TerraData *allocateData() const override;
    virtual void saveToData(TerraData *dataOut) const override;

    friend class TerraWaterBalancer;
};

// Rain Catchment
// Converts rainfall over a catchment area into water delivered to a cistern.
class TerraRainCatchment : public TerraObject {
public:
    TerraRainCatchment(float areaSquareMeters = 0.0f,
                       float collectionEfficiency = 0.85f,
                       tposi_t catchmentIndex = TERRA_POS_SEARCH_FROMBEG,
                       const TerraString &name = TerraString());
    TerraRainCatchment(const TerraRainCatchmentData *dataIn);

    void setAreaSquareMeters(float area) { _areaSquareMeters = area < 0.0f ? 0.0f : area; bumpRevisionIfNeeded(); }
    void setCollectionEfficiency(float efficiency) { _collectionEfficiency = constrain(efficiency, 0.0f, 1.0f); bumpRevisionIfNeeded(); }
    float getAreaSquareMeters() const { return _areaSquareMeters; }
    float getCollectionEfficiency() const { return _collectionEfficiency; }
    float estimateCaptureLiters(float rainfallMm) const;
    float estimateCaptureGallons(float rainfallInches) const;
    TerraRainCollectionResult collectInto(TerraCistern &cistern, float rainfallMm,
                                          TerraFirstFlushController *firstFlush = nullptr) const;

protected:
    float _areaSquareMeters;                                // Effective collection area, square meters
    float _collectionEfficiency;                            // Fraction of rainfall reaching storage

    virtual TerraData *allocateData() const override;
    virtual void saveToData(TerraData *dataOut) const override;
};

// First-Flush Controller
// Diverts the configured initial volume from each rainfall event before storage begins.
class TerraFirstFlushController {
public:
    TerraFirstFlushController(float discardLiters = 0.0f);
    inline void setDiscardLiters(float liters)
    {
        _discardLiters = liters < 0.0f ? 0.0f : liters;
        reset();
    }
    void reset();
    void recordFlow(float liters);
    bool shouldDivert() const { return _discardedLiters < _discardLiters; }
    float getRemainingLiters() const;

protected:
    float _discardLiters;                                   // Required first-flush discard volume
    float _discardedLiters;                                 // Current event discarded volume
};

#endif
