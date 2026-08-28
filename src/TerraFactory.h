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
class TerraReservoir;                                       // Terra Reservoir
class TerraWaterStorage;                                    // Terra Water Storage
class TerraCistern;                                         // Terra Cistern
class TerraWaterSource;                                     // Terra Water Source
class TerraWaterRoute;                                      // Terra Water Route
class TerraRainCatchment;                                   // Terra Rain Catchment
class TerraThermalReservoir;                                    // Terra Thermal Store
class TerraThermalLoop;                                     // Terra Thermal Loop
class TerraEnvironment;                                     // Terra Environment
class TerraSimpleRail;                                      // Terra Simple Power Rail
class TerraRegulatedRail;                                   // Terra Regulated Power Rail

// Object Factory
// Contains convenience builders for system objects. Objects created through the factory
// receive their position-based identity from the controller and are registered immediately.
class TerraFactory {
public:
    virtual ~TerraFactory() { ; }

    // Convenience builders for common sensors (shared, nullptr return -> failure).
    SharedPtr<TerraRemoteSensor> addRemoteSensor(Terra_SensorType reportedType,
                                                 Terra_UnitsType unit = Terra_UnitsType_Raw_1,
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
                                                                 Terra_UnitsType unit = Terra_UnitsType_Temperature_Celsius,
                                                                 const TerraString &name = TerraString());
    SharedPtr<TerraPressureSensor> addAnalogPressureSensor(uint8_t inputPin,
                                                           float rawMinimum,
                                                           float rawMaximum,
                                                           float pressureMinimum,
                                                           float pressureMaximum,
                                                           Terra_UnitsType unit = Terra_UnitsType_Pressure_Kilopascals,
                                                           const TerraString &name = TerraString());

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

    // Reservoir and process object builders.
    SharedPtr<TerraReservoir> addReservoir(Terra_ReservoirType reservoirType,
                                           const TerraString &name = TerraString());
    SharedPtr<TerraWaterStorage> addWaterStorage(float capacityLiters,
                                                 const TerraString &name = TerraString());
    SharedPtr<TerraCistern> addCistern(float capacityLiters,
                                       const TerraString &name = TerraString());
    SharedPtr<TerraWaterSource> addWaterSource(uint8_t priority = 0,
                                               const TerraString &name = TerraString());
    SharedPtr<TerraWaterRoute> addWaterRoute(const TerraString &name = TerraString());
    SharedPtr<TerraRainCatchment> addRainCatchment(float areaSquareMeters,
                                                   float collectionEfficiency = 0.85f,
                                                   const TerraString &name = TerraString());
    SharedPtr<TerraThermalReservoir> addThermalReservoir(const TerraString &name = TerraString());
    SharedPtr<TerraThermalLoop> addThermalLoop(const TerraString &name = TerraString());
    SharedPtr<TerraEnvironment> addEnvironment(const TerraString &name = SFP(TStr_Environment));

    // Convenience builders for common power rails (shared, nullptr return -> failure).
    SharedPtr<TerraSimpleRail> addSimplePowerRail(Terra_RailType railType,
                                                  int maxActiveAtOnce = 2);
    SharedPtr<TerraRegulatedRail> addRegulatedPowerRail(Terra_RailType railType,
                                                        float maxPower);

    // Creates an object from persisted data (return ownership transfer - user code must delete or wrap returned object).
    static TerraObject *newObjectFromData(const TerraObjectData *dataIn);
};

#endif // /ifndef TerraFactory_H
