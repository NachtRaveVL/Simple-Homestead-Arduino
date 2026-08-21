/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Factory
*/

#ifndef TerraFactory_H
#define TerraFactory_H

#include "TerraDefines.h"
#include "TerraObject.h"
#include "TerraDatas.h"

class TerraSensor;                                          // Terra Sensor
class TerraBinarySensor;                                    // Terra Binary Sensor
class TerraAnalogSensor;                                    // Terra Analog Sensor
class TerraTemperatureSensor;                               // Terra Temperature Sensor
class TerraLevelSensor;                                     // Terra Level Sensor
class TerraPressureSensor;                                  // Terra Pressure Sensor
class TerraLeakSensor;                                      // Terra Leak Sensor
class TerraRemoteSensor;                                    // Terra Remote Sensor
class TerraActuator;                                        // Terra Actuator
class TerraPump;                                            // Terra Pump
class TerraSumpPump;                                        // Terra Sump Pump
class TerraValve;                                           // Terra Valve
class TerraHeater;                                          // Terra Heater
class TerraCirculator;                                      // Terra Circulator
class TerraResource;                                        // Terra Resource
class TerraWaterStorage;                                    // Terra Water Storage
class TerraCistern;                                         // Terra Cistern
class TerraWaterSource;                                     // Terra Water Source
class TerraWaterRoute;                                      // Terra Water Route
class TerraRainCatchment;                                   // Terra Rain Catchment
class TerraThermalStore;                                    // Terra Thermal Store
class TerraThermalLoop;                                     // Terra Thermal Loop
class TerraEnvironment;                                     // Terra Environment
class TerraPowerRail;                                       // Terra Power Rail

// Object Factory
// Contains convenience builders for system objects and stores registered main objects in
// the controller's shared object collection for attachment-point resolution.
class TerraFactory {
public:
    virtual ~TerraFactory() { }

    // Convenience builders for common sensors (shared, nullptr return -> failure).
    SharedPtr<TerraSensor> addSensor(Terra_SensorType sensorType,
                                     Terra_Unit unit = Terra_Unit_Raw,
                                     uint32_t key = TERRA_INVALID_KEY,
                                     const TerraString &name = TerraString());
    // Adds a remotely reported sensor to the system.
    SharedPtr<TerraRemoteSensor> addRemoteSensor(Terra_SensorType reportedType,
                                                 Terra_Unit unit = Terra_Unit_Raw,
                                                 uint32_t key = TERRA_INVALID_KEY,
                                                 const TerraString &name = TerraString());
    inline SharedPtr<TerraSensor> addSensor(Terra_SensorType sensorType, Terra_Unit unit, const TerraString &name)
        { return addSensor(sensorType, unit, TERRA_INVALID_KEY, name); }
    inline SharedPtr<TerraRemoteSensor> addRemoteSensor(Terra_SensorType reportedType, Terra_Unit unit, const TerraString &name)
        { return addRemoteSensor(reportedType, unit, TERRA_INVALID_KEY, name); }

    // Pin-backed sensor convenience builders.
    SharedPtr<TerraLeakSensor> addLeakIndicator(uint8_t inputPin,
                                                bool activeLow = true,
                                                const TerraString &name = TerraString());
    SharedPtr<TerraLevelSensor> addAnalogLevelSensor(uint8_t inputPin,
                                                     const TerraString &name = TerraString());
    SharedPtr<TerraTemperatureSensor> addAnalogTemperatureSensor(uint8_t inputPin,
                                                                 const TerraString &name = TerraString());
    SharedPtr<TerraPressureSensor> addAnalogPressureSensor(uint8_t inputPin,
                                                           const TerraString &name = TerraString());

    // Convenience builders for common actuators (shared, nullptr return -> failure).
    SharedPtr<TerraActuator> addActuator(Terra_ActuatorType actuatorType,
                                         uint32_t key = TERRA_INVALID_KEY,
                                         const TerraString &name = TerraString());
    // Adds a general water-transfer pump to the system.
    SharedPtr<TerraPump> addPump(uint32_t key = TERRA_INVALID_KEY,
                                 const TerraString &name = TerraString());
    // Adds a sump pump with attached level sensing and hysteresis control.
    SharedPtr<TerraSumpPump> addSumpPump(uint32_t key = TERRA_INVALID_KEY,
                                         const TerraString &name = TerraString());
    // Adds a thermal circulation pump to the system.
    SharedPtr<TerraCirculator> addCirculator(uint32_t key = TERRA_INVALID_KEY,
                                             const TerraString &name = TerraString());
    inline SharedPtr<TerraActuator> addActuator(Terra_ActuatorType actuatorType, const TerraString &name)
        { return addActuator(actuatorType, TERRA_INVALID_KEY, name); }
    inline SharedPtr<TerraPump> addPump(const TerraString &name)
        { return addPump(TERRA_INVALID_KEY, name); }
    inline SharedPtr<TerraSumpPump> addSumpPump(const TerraString &name)
        { return addSumpPump(TERRA_INVALID_KEY, name); }
    inline SharedPtr<TerraCirculator> addCirculator(const TerraString &name)
        { return addCirculator(TERRA_INVALID_KEY, name); }

    // Pin-backed actuator convenience builders.
    SharedPtr<TerraPump> addPumpRelay(uint8_t outputPin,
                                      bool activeLow = false,
                                      const TerraString &name = TerraString());
    SharedPtr<TerraSumpPump> addSumpPumpRelay(uint8_t outputPin,
                                              bool activeLow = false,
                                              const TerraString &name = TerraString());
    SharedPtr<TerraCirculator> addCirculatorRelay(uint8_t outputPin,
                                                  bool activeLow = false,
                                                  const TerraString &name = TerraString());
    SharedPtr<TerraValve> addValveRelay(uint8_t outputPin,
                                        bool activeLow = false,
                                        const TerraString &name = TerraString());
    SharedPtr<TerraHeater> addHeaterRelay(uint8_t outputPin,
                                          bool activeLow = false,
                                          const TerraString &name = TerraString());

    SharedPtr<TerraResource> addResource(Terra_ResourceType resourceType,
                                         uint32_t key = TERRA_INVALID_KEY,
                                         const TerraString &name = TerraString());
    SharedPtr<TerraWaterStorage> addWaterStorage(Terra_WaterStorageType storageType,
                                                 float capacityLiters,
                                                 uint32_t key = TERRA_INVALID_KEY,
                                                 const TerraString &name = TerraString());
    SharedPtr<TerraCistern> addCistern(float capacityLiters,
                                       uint32_t key = TERRA_INVALID_KEY,
                                       const TerraString &name = TerraString());
    SharedPtr<TerraWaterSource> addWaterSource(Terra_WaterSourceType sourceType,
                                               uint8_t priority = 0,
                                               uint32_t key = TERRA_INVALID_KEY,
                                               const TerraString &name = TerraString());
    SharedPtr<TerraWaterRoute> addWaterRoute(uint32_t key = TERRA_INVALID_KEY,
                                             const TerraString &name = TerraString());
    SharedPtr<TerraRainCatchment> addRainCatchment(float areaSquareMeters,
                                                   float collectionEfficiency = 0.85f,
                                                   uint32_t key = TERRA_INVALID_KEY,
                                                   const TerraString &name = TerraString());
    SharedPtr<TerraThermalStore> addThermalStore(uint32_t key = TERRA_INVALID_KEY,
                                                 const TerraString &name = TerraString());
    SharedPtr<TerraThermalLoop> addThermalLoop(uint32_t key = TERRA_INVALID_KEY,
                                               const TerraString &name = TerraString());
    SharedPtr<TerraEnvironment> addEnvironment(uint32_t key = TERRA_INVALID_KEY,
                                               const TerraString &name = TerraString("Environment"));
    SharedPtr<TerraPowerRail> addPowerRail(Terra_RailType railType,
                                           uint32_t key = TERRA_INVALID_KEY,
                                           const TerraString &name = TerraString());

    // Common object builders without an explicit persistence key.
    inline SharedPtr<TerraResource> addResource(Terra_ResourceType resourceType, const TerraString &name)
        { return addResource(resourceType, TERRA_INVALID_KEY, name); }
    inline SharedPtr<TerraWaterStorage> addWaterStorage(Terra_WaterStorageType storageType, float capacityLiters, const TerraString &name)
        { return addWaterStorage(storageType, capacityLiters, TERRA_INVALID_KEY, name); }
    inline SharedPtr<TerraCistern> addCistern(float capacityLiters, const TerraString &name)
        { return addCistern(capacityLiters, TERRA_INVALID_KEY, name); }
    inline SharedPtr<TerraWaterSource> addWaterSource(Terra_WaterSourceType sourceType, uint8_t priority, const TerraString &name)
        { return addWaterSource(sourceType, priority, TERRA_INVALID_KEY, name); }
    inline SharedPtr<TerraWaterSource> addWaterSource(Terra_WaterSourceType sourceType, const TerraString &name)
        { return addWaterSource(sourceType, 0, TERRA_INVALID_KEY, name); }
    inline SharedPtr<TerraWaterRoute> addWaterRoute(const TerraString &name)
        { return addWaterRoute(TERRA_INVALID_KEY, name); }
    inline SharedPtr<TerraRainCatchment> addRainCatchment(float areaSquareMeters, float collectionEfficiency, const TerraString &name)
        { return addRainCatchment(areaSquareMeters, collectionEfficiency, TERRA_INVALID_KEY, name); }
    inline SharedPtr<TerraThermalStore> addThermalStore(const TerraString &name)
        { return addThermalStore(TERRA_INVALID_KEY, name); }
    inline SharedPtr<TerraThermalLoop> addThermalLoop(const TerraString &name)
        { return addThermalLoop(TERRA_INVALID_KEY, name); }
    inline SharedPtr<TerraEnvironment> addEnvironment(const TerraString &name)
        { return addEnvironment(TERRA_INVALID_KEY, name); }
    inline SharedPtr<TerraPowerRail> addPowerRail(Terra_RailType railType, const TerraString &name)
        { return addPowerRail(railType, TERRA_INVALID_KEY, name); }

    // Creates an object from persisted data (return ownership transfer - user code must delete or wrap returned object).
    static TerraObject *newObjectFromData(const TerraObjectData *dataIn);
    // Creates persisted data from an object (return ownership transfer - user code must delete returned data).
    static TerraObjectData *newDataFromObject(const TerraObject *objectIn);

protected:
    static TerraSensor *newSensorObject(Terra_SensorType sensorType, Terra_Unit unit,
                                        uint32_t key, const TerraString &name);
    static TerraActuator *newActuatorObject(Terra_ActuatorType actuatorType,
                                            uint32_t key, const TerraString &name);
    static TerraResource *newResourceObject(Terra_ResourceType resourceType,
                                            uint32_t key, const TerraString &name);
    static TerraWaterStorage *newWaterStorageObject(Terra_WaterStorageType storageType,
                                                    float capacityLiters, uint32_t key,
                                                    const TerraString &name);
    static TerraWaterSource *newWaterSourceObject(Terra_WaterSourceType sourceType,
                                                  uint8_t priority, uint32_t key,
                                                  const TerraString &name);
    static TerraPowerRail *newPowerRailObject(Terra_RailType railType,
                                              uint32_t key, const TerraString &name);
    static void applyObjectData(TerraObject *object, const TerraObjectData *data);
};

#endif
