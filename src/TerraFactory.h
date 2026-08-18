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
class TerraActuator;                                        // Terra Actuator
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

class TerraFactory {
public:
    TerraFactory();
    virtual ~TerraFactory() { }

    bool registerObject(TerraObject *object);
    bool unregisterObject(TerraObject *object);
    TerraObject *findObjectByKey(uint32_t key) const;
    TerraObject *findObjectByName(const TerraString &name) const;
    TerraObject *findFirstByType(Terra_ObjectType type) const;
    uint8_t findByType(Terra_ObjectType type, TerraObject **output, uint8_t capacity) const;
    TerraObject *objectAt(uint8_t index) const { return index < _count ? _objects[index] : nullptr; }
    uint8_t objectCount() const { return _count; }
    uint32_t allocateKey(const TerraString &name = TerraString());
    void updateObjects(uint32_t now = terraMillis());

    static TerraSensor *newSensor(Terra_SensorType sensorType, Terra_Unit unit = Terra_Unit_Raw,
                                  uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString());  // Object key
    static TerraActuator *newActuator(Terra_ActuatorType actuatorType,
                                      uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString());  // Object key
    static TerraResource *newResource(Terra_ResourceType resourceType,
                                      uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString());  // Object key
    static TerraWaterStorage *newWaterStorage(Terra_WaterStorageType storageType, float capacityLiters,
                                              uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString());  // Object key
    static TerraWaterSource *newWaterSource(Terra_WaterSourceType sourceType, uint8_t priority = 0,
                                            uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString());  // Object key
    static TerraPowerRail *newPowerRail(Terra_RailType railType,
                                        uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString());  // Object key

    static TerraObject *newObjectFromData(const TerraObjectData *dataIn);
    static TerraObjectData *newDataFromObject(const TerraObject *objectIn);

protected:
    TerraObject *_objects[TERRA_MAX_OBJECTS];
    uint8_t _count;                                         // Active entry count
    uint32_t _nextKey;                                      // Next automatically assigned object key
};

#endif
