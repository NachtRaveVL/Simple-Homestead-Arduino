/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Thermal Management
*/

#ifndef TerraThermal_H
#define TerraThermal_H

#include "TerraResource.h"
#include "TerraAttachments.h"
#include "TerraBalancers.h"

// Thermal Store
// Tracks stored heat temperature and normal/safety target ranges.
class TerraThermalStore : public TerraResource {
public:
    TerraThermalStore(uint32_t key = TERRA_INVALID_KEY,
                      const TerraString &name = TerraString());

    inline void setTemperature(float celsius) { _temperatureC = celsius; }
    inline float getTemperature() const { return _temperatureC; }

    bool setTargetRange(float minimumC, float maximumC);
    bool setAbsoluteMaximum(float maximumC);

    inline float getMinimumTarget() const { return _minimumTargetC; }
    inline float getMaximumTarget() const { return _maximumTargetC; }
    inline float getAbsoluteMaximum() const { return _absoluteMaximumC; }
    inline bool needsHeat() const { return _temperatureC < _minimumTargetC; }
    inline bool isOverTemperature() const { return _temperatureC > _maximumTargetC; }
    inline bool isSafetyLimitExceeded() const { return _temperatureC >= _absoluteMaximumC; }

    // Temperature Sensor Attachment Point
    template<class T> inline void setTemperatureSensor(const SharedPtr<T> &sensor) { _temperatureSensor.setObject(sensor); }
    inline TerraSensorAttachment &getTemperatureSensorAttachment() { return _temperatureSensor; }
    inline const TerraSensorAttachment &getTemperatureSensorAttachment() const { return _temperatureSensor; }

    virtual void update(uint32_t now = terraMillis()) override;
    virtual void unresolveAny(TerraObject *object) override;

protected:
    float _temperatureC;                                    // Current temperature, degrees Celsius
    float _minimumTargetC;                                  // Minimum target temperature
    float _maximumTargetC;                                  // Maximum target temperature
    float _absoluteMaximumC;                                // Absolute temperature safety limit
    TerraSensorAttachment _temperatureSensor;               // Temperature sensor attachment point

    inline void initTemperatureSensorKey(uint32_t key) { _temperatureSensor.initObject(key); }

    friend class TerraFactory;
};

// Thermal Loop
// Differential circulation state for moving heat between a source and storage.
class TerraThermalLoop : public TerraObject {
public:
    TerraThermalLoop(uint32_t key = TERRA_INVALID_KEY,
                     const TerraString &name = TerraString());

    bool configure(float onDifferentialC, float offDifferentialC, float maxStoreTempC);

    template<class T> inline void setSourceTemperatureSensor(const SharedPtr<T> &sensor) { _balancer.setSourceTemperatureSensor(sensor); }
    template<class T> inline void setThermalStore(const SharedPtr<T> &store) { _balancer.setThermalStore(store); }
    template<class T> inline void setCirculator(const SharedPtr<T> &circulator) { _balancer.setCirculator(circulator); }

    virtual void setEnabled(bool enabled) override;

    inline bool isRunning() const { return _running; }
    inline float getOnDifferential() const { return _onDifferentialC; }
    inline float getOffDifferential() const { return _offDifferentialC; }
    inline float getMaxStoreTemperature() const { return _maxStoreTempC; }
    inline uint32_t getSourceTemperatureSensorKey() const { return _balancer.getSourceTemperatureAttachment().getKey(); }
    inline uint32_t getThermalStoreKey() const { return _balancer.getStoreAttachment().getKey(); }
    inline uint32_t getCirculatorKey() const { return _balancer.getCirculatorAttachment().getKey(); }

    inline TerraThermalBalancer &getBalancer() { return _balancer; }
    inline const TerraThermalBalancer &getBalancer() const { return _balancer; }

    virtual void update(uint32_t now = terraMillis()) override;
    virtual void unresolveAny(TerraObject *object) override;

protected:
    float _onDifferentialC;                                 // Circulation-on temperature differential
    float _offDifferentialC;                                // Circulation-off temperature differential
    float _maxStoreTempC;                                   // Maximum allowed storage temperature
    bool _running;                                          // Current circulation state
    TerraThermalBalancer _balancer;                         // Thermal balancing process

    bool shouldCirculate(float sourceTempC, float storeTempC) const;
    void setRunning(bool running);

    inline void initSourceTemperatureKey(uint32_t key) { _balancer.initSourceTemperatureKey(key); }
    inline void initStoreKey(uint32_t key) { _balancer.initStoreKey(key); }
    inline void initCirculatorKey(uint32_t key) { _balancer.initCirculatorKey(key); }

    friend class TerraThermalBalancer;
    friend class TerraFactory;
};

#endif // /ifndef TerraThermal_H
