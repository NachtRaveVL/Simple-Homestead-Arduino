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
class TerraRemoteSensor;                                    // Terra Remote Sensor
class TerraActuator;                                        // Terra Actuator
class TerraPump;                                            // Terra Pump
class TerraSumpPump;                                        // Terra Sump Pump
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
