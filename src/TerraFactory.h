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
class TerraRelayPumpActuator;                               // Terra Relay Pump Actuator
class TerraVariableActuator;                                // Terra Variable Actuator
class TerraReservoir;                                       // Terra Reservoir
class TerraWaterReservoir;                                  // Terra Water Reservoir
class TerraThermalReservoir;                                // Terra Thermal Reservoir
class TerraInfiniteWaterReservoir;                          // Terra Infinite Water Reservoir
class TerraInfiniteThermalReservoir;                        // Terra Infinite Thermal Reservoir
class TerraRainCatchment;                                   // Terra Rain Catchment
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
                                                 const String &name = String());

    // Pin-backed sensor convenience builders.
    SharedPtr<TerraLeakSensor> addLeakIndicator(uint8_t inputPin,
                                                bool activeLow = true,
                                                const String &name = String());
    SharedPtr<TerraLevelSensor> addAnalogLevelSensor(uint8_t inputPin,
                                                     float rawMinimum = 0.0f,
                                                     float rawMaximum = 1.0f,
                                                     float levelMinimum = 0.0f,
                                                     float levelMaximum = 100.0f,
                                                     const String &name = String());
    SharedPtr<TerraTemperatureSensor> addAnalogTemperatureSensor(uint8_t inputPin,
                                                                 float rawMinimum,
                                                                 float rawMaximum,
                                                                 float temperatureMinimum,
                                                                 float temperatureMaximum,
                                                                 Terra_UnitsType unit = Terra_UnitsType_Temperature_Celsius,
                                                                 const String &name = String());
    SharedPtr<TerraPressureSensor> addAnalogPressureSensor(uint8_t inputPin,
                                                           float rawMinimum,
                                                           float rawMaximum,
                                                           float pressureMinimum,
                                                           float pressureMaximum,
                                                           Terra_UnitsType unit = Terra_UnitsType_Pressure_Kilopascals,
                                                           const String &name = String());

    // Pin-backed actuator convenience builders.
    SharedPtr<TerraRelayPumpActuator> addPumpRelay(uint8_t outputPin,
                                                    bool activeLow = false,
                                                    const String &name = String());
    SharedPtr<TerraRelayPumpActuator> addSumpPumpRelay(uint8_t outputPin,
                                                        bool activeLow = false,
                                                        const String &name = String());
    SharedPtr<TerraRelayPumpActuator> addCirculatorRelay(uint8_t outputPin,
                                                          bool activeLow = false,
                                                          const String &name = String());
    SharedPtr<TerraRelayPumpActuator> addValveRelay(uint8_t outputPin,
                                                     bool activeLow = false,
                                                     const String &name = String());
    SharedPtr<TerraRelayActuator> addFanRelay(uint8_t outputPin,
                                              bool activeLow = false,
                                              const String &name = String());
    SharedPtr<TerraRelayActuator> addHeaterRelay(uint8_t outputPin,
                                                 bool activeLow = false,
                                                 const String &name = String());

    // Reservoir and process object builders.
    SharedPtr<TerraWaterReservoir> addWaterReservoir(float maxVolume,
                                                     const String &name = String());
    SharedPtr<TerraThermalReservoir> addThermalReservoir(float maxTemperature,
                                                         const String &name = String());
    SharedPtr<TerraInfiniteWaterReservoir> addInfiniteWaterReservoir(bool alwaysFilled = true,
                                                                     const String &name = String());
    SharedPtr<TerraInfiniteThermalReservoir> addInfiniteThermalReservoir(bool alwaysFilled = true,
                                                                         const String &name = String());
    SharedPtr<TerraRainCatchment> addRainCatchment(float areaSquareMeters,
                                                   float collectionEfficiency = 0.85f,
                                                   const String &name = String());
    SharedPtr<TerraEnvironment> addEnvironment(const String &name = SFP(TStr_Environment));

    // Convenience builders for common power rails (shared, nullptr return -> failure).
    SharedPtr<TerraSimpleRail> addSimplePowerRail(Terra_RailType railType,
                                                  int maxActiveAtOnce = 2);
    SharedPtr<TerraRegulatedRail> addRegulatedPowerRail(Terra_RailType railType,
                                                        float maxPower);

    // Creates an object from persisted data (return ownership transfer - user code must delete or wrap returned object).
    static TerraObject *newObjectFromData(const TerraObjectData *dataIn);
};

#endif // /ifndef TerraFactory_H
