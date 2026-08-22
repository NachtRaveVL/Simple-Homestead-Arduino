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
class TerraRelayActuator;                                   // Terra Relay Actuator
class TerraVariableActuator;                                // Terra Variable Actuator
class TerraPump;                                            // Terra Pump
class TerraSumpPump;                                        // Terra Sump Pump
class TerraValve;                                           // Terra Valve
class TerraHeater;                                          // Terra Heater
class TerraCirculator;                                      // Terra Circulator
class TerraReservoir;                                        // Terra Resource
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
// Contains convenience builders for system objects. Objects created through the factory
// receive their position-based identity from the controller and are registered immediately.
class TerraFactory {
public:
    virtual ~TerraFactory() { ; }

    // Convenience builders for common sensors (shared, nullptr return -> failure).
    SharedPtr<TerraSensor> addSensor(Terra_SensorType sensorType,
                                     Terra_Unit unit = Terra_Unit_Raw,
                                     const TerraString &name = TerraString());
    SharedPtr<TerraRemoteSensor> addRemoteSensor(Terra_SensorType reportedType,
                                                 Terra_Unit unit = Terra_Unit_Raw,
                                                 const TerraString &name = TerraString());

    // Pin-backed sensor convenience builders.
    SharedPtr<TerraLeakSensor> addLeakIndicator(uint8_t inputPin,
                                                bool activeLow = true,
                                                const TerraString &name = TerraString());
    SharedPtr<TerraLevelSensor> addAnalogLevelSensor(uint8_t inputPin,
                                                     float rawMinimum = 0.0f,
                                                     float rawMaximum = 1.0f,
                                                     float levelMinimum = 0.0f,
                                                     float levelMaximum = 100.0f,
                                                     const TerraString &name = TerraString());
    SharedPtr<TerraTemperatureSensor> addAnalogTemperatureSensor(uint8_t inputPin,
                                                                 float rawMinimum,
                                                                 float rawMaximum,
                                                                 float temperatureMinimum,
                                                                 float temperatureMaximum,
                                                                 Terra_Unit unit = Terra_Unit_Celsius,
                                                                 const TerraString &name = TerraString());
    SharedPtr<TerraPressureSensor> addAnalogPressureSensor(uint8_t inputPin,
                                                           float rawMinimum,
                                                           float rawMaximum,
                                                           float pressureMinimum,
                                                           float pressureMaximum,
                                                           Terra_Unit unit = Terra_Unit_Kilopascals,
                                                           const TerraString &name = TerraString());

    // Pin-backed actuator convenience builders.
    SharedPtr<TerraRelayActuator> addRelayActuator(Terra_ActuatorType actuatorType,
                                                   uint8_t outputPin,
                                                   bool activeLow = false,
                                                   const TerraString &name = TerraString());
    SharedPtr<TerraVariableActuator> addVariableActuator(Terra_ActuatorType actuatorType,
                                                         uint8_t outputPin,
                                                         uint8_t outputBitRes = 8,
                                                         const TerraString &name = TerraString());
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

    // Resource and process object builders.
    SharedPtr<TerraReservoir> addResource(Terra_ReservoirType ReservoirType,
                                         const TerraString &name = TerraString());
    SharedPtr<TerraWaterStorage> addWaterStorage(Terra_WaterStorageType storageType,
                                                 float capacityLiters,
                                                 const TerraString &name = TerraString());
    SharedPtr<TerraCistern> addCistern(float capacityLiters,
                                       const TerraString &name = TerraString());
    SharedPtr<TerraWaterSource> addWaterSource(Terra_WaterSourceType sourceType,
                                               uint8_t priority = 0,
                                               const TerraString &name = TerraString());
    SharedPtr<TerraWaterRoute> addWaterRoute(const TerraString &name = TerraString());
    SharedPtr<TerraRainCatchment> addRainCatchment(float areaSquareMeters,
                                                   float collectionEfficiency = 0.85f,
                                                   const TerraString &name = TerraString());
    SharedPtr<TerraThermalStore> addThermalStore(const TerraString &name = TerraString());
    SharedPtr<TerraThermalLoop> addThermalLoop(const TerraString &name = TerraString());
    SharedPtr<TerraEnvironment> addEnvironment(const TerraString &name = TerraString("Environment"));
    SharedPtr<TerraPowerRail> addPowerRail(Terra_RailType railType,
                                           const TerraString &name = TerraString());

    // Creates an object from persisted data (return ownership transfer - user code must delete or wrap returned object).
    static TerraObject *newObjectFromData(const TerraObjectData *dataIn);
    // Creates persisted data from an object (return ownership transfer - user code must delete returned data).
    static TerraObjectData *newDataFromObject(const TerraObject *objectIn);
};

#endif
