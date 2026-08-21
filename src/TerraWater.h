/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Water Management
*/

#ifndef TerraWater_H
#define TerraWater_H

#include "TerraResource.h"
#include "TerraUtils.h"
#include "TerraAttachments.h"
#include "TerraBalancers.h"

// First-Flush Controller
// Tracks the initial diverted volume from a rainfall event before storage is allowed.
class TerraFirstFlushController;                            // Terra First Flush Controller

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
class TerraWaterStorage : public TerraResource {
public:
    TerraWaterStorage(float capacityLiters = 0.0f,
                      uint32_t key = TERRA_INVALID_KEY,
                      const TerraString &name = TerraString(),
                      Terra_WaterStorageType storageType = Terra_WaterStorageType_Undefined);  // Water storage type

    void setCapacityLiters(float capacityLiters) { _capacityLiters = capacityLiters < 0.0f ? 0.0f : capacityLiters; }
    float getCapacityLiters() const { return _capacityLiters; }
    Terra_WaterStorageType getStorageType() const { return _storageType; }
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

    virtual void update(uint32_t now = terraMillis()) override;
    virtual void unresolveAny(TerraObject *object) override;

protected:
    float _capacityLiters;                                  // Storage capacity, liters
    Terra_WaterStorageType _storageType;                    // Water storage type
    TerraSensorAttachment _levelSensor;                     // Level sensor attachment point

    inline void initLevelSensorKey(uint32_t key) { _levelSensor.initObject(key); }

    friend class TerraFactory;
};

// Cistern
// Water storage with fill hysteresis, protected reserve, overflow tracking, and flow totals.
class TerraCistern : public TerraWaterStorage {
public:
    TerraCistern(float capacityLiters = 0.0f,
                 uint32_t key = TERRA_INVALID_KEY,
                 const TerraString &name = TerraString());

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
};

// Water Source
// Describes a usable water source and its priority, reserve, availability, and flow limit.
class TerraWaterSource : public TerraObject {
public:
    TerraWaterSource(Terra_WaterSourceType type = Terra_WaterSourceType_Undefined,
                     uint8_t priority = 0,
                     uint32_t key = TERRA_INVALID_KEY,
                     const TerraString &name = TerraString());

    Terra_WaterSourceType getType() const { return _type; }
    uint8_t getPriority() const { return _priority; }
    bool isAvailable() const { return _available && !_fault && _enabled; }
    bool isConfiguredAvailable() const { return _available; }
    float getLevel() const { return _level; }
    float getReserveLevel() const { return _reserveLevel; }
    float getMaximumFlowLpm() const { return _maximumFlowLpm; }

    void setPriority(uint8_t priority) { _priority = priority; }
    void setAvailable(bool available) { _available = available; }
    void setLevel(float level) { _level = terraClamp(level, 0.0f, 100.0f); }
    void setReserveLevel(float reserve) { _reserveLevel = terraClamp(reserve, 0.0f, 100.0f); }
    void setMaximumFlowLpm(float flow) { _maximumFlowLpm = flow < 0.0f ? 0.0f : flow; }

    // Level Sensor Attachment Point
    template<class T> inline void setLevelSensor(const SharedPtr<T> &sensor) { _levelSensor.setObject(sensor); }
    inline TerraSensorAttachment &getLevelSensorAttachment() { return _levelSensor; }
    inline const TerraSensorAttachment &getLevelSensorAttachment() const { return _levelSensor; }

    virtual void update(uint32_t now = terraMillis()) override;
    virtual void unresolveAny(TerraObject *object) override;

protected:
    Terra_WaterSourceType _type;                            // Resource/source type
    uint8_t _priority;                                      // Source priority
    bool _available;                                        // Configured availability
    float _level;                                           // Normalized level, percent
    float _reserveLevel;                                    // Protected reserve level, percent
    float _maximumFlowLpm;                                  // Maximum allowed flow
    TerraSensorAttachment _levelSensor;                     // Level sensor attachment point

    inline void initLevelSensorKey(uint32_t key) { _levelSensor.initObject(key); }

    friend class TerraFactory;
};

// Water Route
// Defines one controlled transfer path between a source and destination storage object.
class TerraWaterRoute : public TerraObject {
public:
    TerraWaterRoute(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString());

    template<class T> inline void setSource(const SharedPtr<T> &source) { _balancer.setSource(source); }
    template<class T> inline void setDestination(const SharedPtr<T> &destination) { _balancer.setDestination(destination); }
    template<class T> inline void setPump(const SharedPtr<T> &pump) { _balancer.setPump(pump); }
    template<class T> inline void setFlowSensor(const SharedPtr<T> &sensor) { _balancer.setFlowSensor(sensor); }

    bool setDestinationBand(float startPercent, float stopPercent);
    void setMinimumFlow(float lpm) { _minimumFlowLpm = lpm < 0.0f ? 0.0f : lpm; }
    void setMaximumFlow(float lpm) { _maximumFlowLpm = lpm < 0.0f ? 0.0f : lpm; }
    uint32_t getSourceKey() const { return _balancer.getSourceAttachment().getKey(); }
    uint32_t getDestinationKey() const { return _balancer.getDestinationAttachment().getKey(); }
    uint32_t getPumpKey() const { return _balancer.getPumpAttachment().getKey(); }
    uint32_t getFlowSensorKey() const { return _balancer.getFlowSensorAttachment().getKey(); }
    float getDestinationStartPercent() const { return _destinationStartPercent; }
    float getDestinationStopPercent() const { return _destinationStopPercent; }
    float getMinimumFlow() const { return _minimumFlowLpm; }
    float getMaximumFlow() const { return _maximumFlowLpm; }
    Terra_RouteState getRouteState() const { return _routeState; }

    inline TerraWaterBalancer &getBalancer() { return _balancer; }
    inline const TerraWaterBalancer &getBalancer() const { return _balancer; }

    virtual void update(uint32_t now = terraMillis()) override;
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

    inline void initSourceKey(uint32_t key) { _balancer.initSourceKey(key); }
    inline void initDestinationKey(uint32_t key) { _balancer.initDestinationKey(key); }
    inline void initPumpKey(uint32_t key) { _balancer.initPumpKey(key); }
    inline void initFlowSensorKey(uint32_t key) { _balancer.initFlowSensorKey(key); }

    friend class TerraWaterBalancer;
    friend class TerraFactory;
};

// Rain Catchment
// Converts rainfall over a catchment area into water delivered to a cistern.
class TerraRainCatchment : public TerraObject {
public:
    TerraRainCatchment(float areaSquareMeters = 0.0f,
                       float collectionEfficiency = 0.85f,
                       uint32_t key = TERRA_INVALID_KEY,
                       const TerraString &name = TerraString());

    void setAreaSquareMeters(float area) { _areaSquareMeters = area < 0.0f ? 0.0f : area; }
    void setCollectionEfficiency(float efficiency) { _collectionEfficiency = terraClamp(efficiency, 0.0f, 1.0f); }
    float getAreaSquareMeters() const { return _areaSquareMeters; }
    float getCollectionEfficiency() const { return _collectionEfficiency; }
    float estimateCaptureLiters(float rainfallMm) const;
    float estimateCaptureGallons(float rainfallInches) const;
    TerraRainCollectionResult collectInto(TerraCistern &cistern, float rainfallMm,
                                          TerraFirstFlushController *firstFlush = nullptr) const;

protected:
    float _areaSquareMeters;                                // Effective collection area, square meters
    float _collectionEfficiency;                            // Fraction of rainfall reaching storage
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
