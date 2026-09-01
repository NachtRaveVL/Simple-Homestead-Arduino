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
                                                 Terra_UnitsType unit = Terra_UnitsType_Raw_1);

    // Pin-backed sensor convenience builders.
    SharedPtr<TerraLeakSensor> addLeakIndicator(pintype_t inputPin,
                                                bool activeLow = true);
    SharedPtr<TerraLevelSensor> addAnalogLevelSensor(pintype_t inputPin,
                                                     float rawMinimum = 0.0f,
                                                     float rawMaximum = 1.0f,
                                                     float levelMinimum = 0.0f,
                                                     float levelMaximum = 100.0f);
    SharedPtr<TerraTemperatureSensor> addAnalogTemperatureSensor(pintype_t inputPin,
                                                                 float rawMinimum,
                                                                 float rawMaximum,
                                                                 float temperatureMinimum,
                                                                 float temperatureMaximum,
                                                                 Terra_UnitsType unit = Terra_UnitsType_Temperature_Celsius);
    SharedPtr<TerraPressureSensor> addAnalogPressureSensor(pintype_t inputPin,
                                                           float rawMinimum,
                                                           float rawMaximum,
                                                           float pressureMinimum,
                                                           float pressureMaximum,
                                                           Terra_UnitsType unit = Terra_UnitsType_Pressure_Kilopascals);

    // Pin-backed actuator convenience builders.
    SharedPtr<TerraRelayPumpActuator> addPumpRelay(pintype_t outputPin,
                                                    bool activeLow = false);
    SharedPtr<TerraRelayPumpActuator> addSumpPumpRelay(pintype_t outputPin,
                                                        bool activeLow = false);
    SharedPtr<TerraRelayPumpActuator> addCirculatorRelay(pintype_t outputPin,
                                                          bool activeLow = false);
    SharedPtr<TerraRelayPumpActuator> addValveRelay(pintype_t outputPin,
                                                     bool activeLow = false);
    SharedPtr<TerraRelayActuator> addFanRelay(pintype_t outputPin,
                                              bool activeLow = false);
    SharedPtr<TerraRelayActuator> addHeaterRelay(pintype_t outputPin,
                                                 bool activeLow = false);

    // Reservoir and process object builders.
    SharedPtr<TerraWaterReservoir> addWaterReservoir(float maxVolume);
    SharedPtr<TerraThermalReservoir> addThermalReservoir(float maxTemperature);
    SharedPtr<TerraInfiniteWaterReservoir> addInfiniteWaterReservoir(bool alwaysFilled = true);
    SharedPtr<TerraInfiniteThermalReservoir> addInfiniteThermalReservoir(bool alwaysFilled = true);

    // Convenience builders for common power rails (shared, nullptr return -> failure).
    SharedPtr<TerraSimpleRail> addSimplePowerRail(Terra_RailType railType,
                                                  int maxActiveAtOnce = 2);
    SharedPtr<TerraRegulatedRail> addRegulatedPowerRail(Terra_RailType railType,
                                                        float maxPower);

    // Creates an object from persisted data (return ownership transfer - user code must delete or wrap returned object).
    static TerraObject *newObjectFromData(const TerraObjectData *dataIn);
};

#endif // /ifndef TerraFactory_H
