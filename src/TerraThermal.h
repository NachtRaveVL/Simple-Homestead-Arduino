/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Thermal Management
*/

#ifndef TerraThermal_H
#define TerraThermal_H

struct TerraThermalLoopData;

#include "TerraReservoir.h"
#include "TerraAttachments.h"
#include "TerraBalancers.h"
#include "TerraDatas.h"

// Thermal Store
// Tracks stored heat temperature and normal/safety target ranges.
class TerraThermalStore : public TerraReservoir {
public:
    TerraThermalStore(tposi_t storeIndex = TERRA_POS_SEARCH_FROMBEG,
                      const TerraString &name = TerraString());
    TerraThermalStore(const TerraThermalStoreData *dataIn);

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

    virtual void update(uint32_t now = millis()) override;
    virtual void unresolveAny(TerraObject *object) override;

protected:
    float _temperatureC;                                    // Current temperature, degrees Celsius
    float _minimumTargetC;                                  // Minimum target temperature
    float _maximumTargetC;                                  // Maximum target temperature
    float _absoluteMaximumC;                                // Absolute temperature safety limit
    TerraSensorAttachment _temperatureSensor;               // Temperature sensor attachment point

    virtual TerraData *allocateData() const override;
    virtual void saveToData(TerraData *dataOut) const override;
};

// Thermal Loop
// Differential circulation state for moving heat between a source and storage.
class TerraThermalLoop : public TerraObject {
public:
    const enum : signed char { Loop, Unknown = -1 } classType; // Thermal loop class type

    TerraThermalLoop(tposi_t loopIndex = TERRA_POS_SEARCH_FROMBEG,
                     const TerraString &name = TerraString());
    TerraThermalLoop(const TerraThermalLoopData *dataIn);

    bool configure(float onDifferentialC, float offDifferentialC, float maxStoreTempC);

    template<class T> inline void setSourceTemperatureSensor(const SharedPtr<T> &sensor) { _balancer.setSourceTemperatureSensor(sensor); }
    template<class T> inline void setThermalStore(const SharedPtr<T> &store) { _balancer.setThermalStore(store); }
    template<class T> inline void setCirculator(const SharedPtr<T> &circulator) { _balancer.setCirculator(circulator); }

    virtual void setEnabled(bool enabled) override;

    inline bool isRunning() const { return _running; }
    inline float getOnDifferential() const { return _onDifferentialC; }
    inline float getOffDifferential() const { return _offDifferentialC; }
    inline float getMaxStoreTemperature() const { return _maxStoreTempC; }
    inline tkey_t getSourceTemperatureSensorKey() const { return _balancer.getSourceTemperatureAttachment().getKey(); }
    inline tkey_t getThermalStoreKey() const { return _balancer.getStoreAttachment().getKey(); }
    inline tkey_t getCirculatorKey() const { return _balancer.getCirculatorAttachment().getKey(); }

    inline TerraThermalBalancer &getBalancer() { return _balancer; }
    inline const TerraThermalBalancer &getBalancer() const { return _balancer; }

    virtual void update(uint32_t now = millis()) override;
    virtual void unresolveAny(TerraObject *object) override;

protected:
    float _onDifferentialC;                                 // Circulation-on temperature differential
    float _offDifferentialC;                                // Circulation-off temperature differential
    float _maxStoreTempC;                                   // Maximum allowed storage temperature
    bool _running;                                          // Current circulation state
    TerraThermalBalancer _balancer;                         // Thermal balancing process

    bool shouldCirculate(float sourceTempC, float storeTempC) const;
    void setRunning(bool running);

    virtual TerraData *allocateData() const override;
    virtual void saveToData(TerraData *dataOut) const override;

    friend class TerraThermalBalancer;
};


// Thermal Loop Serialization Data
struct TerraThermalLoopData : public TerraObjectData {
    char sourceTemperatureSensor[TERRA_NAME_MAXSIZE];       // Source temperature sensor attachment
    char store[TERRA_NAME_MAXSIZE];                         // Thermal store attachment
    char circulator[TERRA_NAME_MAXSIZE];                    // Circulator attachment
    float onDifferentialC;                                  // Circulation-on temperature differential
    float offDifferentialC;                                 // Circulation-off temperature differential
    float maxStoreTempC;                                    // Maximum storage temperature

    TerraThermalLoopData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef TerraThermal_H
