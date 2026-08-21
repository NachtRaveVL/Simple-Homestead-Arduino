/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Object Data
*/

#ifndef TerraDatas_H
#define TerraDatas_H

#include "TerraTypes.h"
#include "TerraDefines.h"
#include "TerraEnvironment.h"
#include "TerraPins.h"
#include "TerraAttachments.h"

struct TerraObjectData {
    uint32_t key;                                           // Object key
    Terra_ObjectType objectType;                            // Object type
    TerraString name;                                       // Display name
    bool enabled;                                           // Enabled state
    TerraAttachmentData attachments[TERRA_MAX_ATTACHMENTS];     // Object attachment records
    uint8_t attachmentCount;                                // Attachment record count

    TerraObjectData();
    virtual ~TerraObjectData() { }
    virtual TerraString toJSON() const;
    virtual bool fromJSON(const TerraString &json);
};


struct TerraSensorData : public TerraObjectData {
    Terra_SensorType sensorType;                            // Sensor type
    Terra_SensorType reportedType;                          // Reported measurement type
    Terra_Unit unit;                                        // Measurement unit
    uint32_t updateIntervalMs;                              // Controller update interval, milliseconds
    uint32_t staleAfterMs;                                  // Remote stale timeout, milliseconds
    bool hasPinDriver;                                      // Pin-backed driver configured flag
    TerraPinSetup pinSetup;                                 // Saved pin setup
    bool driverCalibrated;                                  // Driver calibration available flag
    float driverRawMinimum;                                 // Driver raw calibration minimum
    float driverRawMaximum;                                 // Driver raw calibration maximum
    float driverValueMinimum;                               // Driver calibrated minimum
    float driverValueMaximum;                               // Driver calibrated maximum
    bool sensorCalibrated;                                  // Sensor calibration available flag
    float sensorRawMinimum;                                 // Sensor raw calibration minimum
    float sensorRawMaximum;                                 // Sensor raw calibration maximum
    float sensorValueMinimum;                               // Sensor calibrated minimum
    float sensorValueMaximum;                               // Sensor calibrated maximum

    TerraSensorData();
    TerraString toJSON() const;
    bool fromJSON(const TerraString &json);
};

struct TerraActuatorData : public TerraObjectData {
    Terra_ActuatorType actuatorType;                        // Actuator type
    Terra_EnableMode enableMode;                            // Actuator request aggregation mode
    uint32_t maxContinuousMs;                               // Maximum continuous runtime, milliseconds
    bool hasPinDriver;                                      // Pin-backed driver configured flag
    TerraPinSetup pinSetup;                                 // Saved pin setup
    int maximumRaw;                                         // Maximum raw output value
    float sumpStartPercent;                                 // Sump pump start level, percent
    float sumpStopPercent;                                  // Sump pump stop level, percent
    float sumpAlarmPercent;                                 // Sump high-water alarm level, percent

    TerraActuatorData();
    TerraString toJSON() const;
    bool fromJSON(const TerraString &json);
};

struct TerraResourceData : public TerraObjectData {
    Terra_ResourceType resourceType;                        // Resource type
    float level;                                            // Normalized resource level, percent
    float reserveLevel;                                     // Protected reserve level, percent
    float lowLevel;                                         // Low threshold, percent
    float highLevel;                                        // High threshold, percent

    TerraResourceData();
    TerraString toJSON() const;
    bool fromJSON(const TerraString &json);
};

struct TerraWaterStorageData : public TerraResourceData {
    Terra_WaterStorageType storageType;                     // Water storage type
    float capacityLiters;                                   // Storage capacity, liters
    float fillStartPercent;                                 // Fill-start threshold, percent
    float fillStopPercent;                                  // Fill-stop threshold, percent
    float overflowPercent;                                  // Overflow safety threshold, percent

    TerraWaterStorageData();
    TerraString toJSON() const;
    bool fromJSON(const TerraString &json);
};

struct TerraCisternData : public TerraWaterStorageData {
    TerraCisternData();
    TerraString toJSON() const;
    bool fromJSON(const TerraString &json);
};

struct TerraWaterSourceData : public TerraObjectData {
    Terra_WaterSourceType sourceType;                       // Water source type
    uint8_t priority;                                       // Source priority
    bool available;                                         // Configured source availability
    float level;                                            // Normalized resource level, percent
    float reserveLevel;                                     // Protected reserve level, percent
    float maximumFlowLpm;                                   // Maximum source flow, liters per minute

    TerraWaterSourceData();
    TerraString toJSON() const;
    bool fromJSON(const TerraString &json);
};

struct TerraWaterRouteData : public TerraObjectData {
    uint32_t sourceKey;                                     // Source object key
    uint32_t destinationKey;                                // Destination object key
    float destinationStartPercent;                          // Destination fill-start threshold, percent
    float destinationStopPercent;                           // Destination fill-stop threshold, percent
    float minimumFlowLpm;                                   // Minimum expected flow, liters per minute
    float maximumFlowLpm;                                   // Maximum source flow, liters per minute
    Terra_RouteState routeState;                            // Current route state

    TerraWaterRouteData();
    TerraString toJSON() const;
    bool fromJSON(const TerraString &json);
};

struct TerraRainCatchmentData : public TerraObjectData {
    float areaSquareMeters;                  ///< Effective catchment area
    float collectionEfficiency;              ///< Fraction of rainfall reaching storage

    TerraRainCatchmentData();
    TerraString toJSON() const;
    bool fromJSON(const TerraString &json);
};

struct TerraThermalStoreData : public TerraResourceData {
    float temperatureC;                                     // Temperature, degrees Celsius
    float minimumTargetC;                                   // Minimum target temperature, degrees Celsius
    float maximumTargetC;                                   // Maximum target temperature, degrees Celsius
    float absoluteMaximumC;                                 // Absolute safety limit, degrees Celsius

    TerraThermalStoreData();
    TerraString toJSON() const;
    bool fromJSON(const TerraString &json);
};


struct TerraThermalLoopData : public TerraObjectData {
    float onDifferentialC;                                  // Circulation-on temperature differential
    float offDifferentialC;                                 // Circulation-off temperature differential
    float maxStoreTempC;                                    // Maximum storage temperature

    TerraThermalLoopData();
    TerraString toJSON() const;
    bool fromJSON(const TerraString &json);
};

struct TerraPowerRailData : public TerraObjectData {
    Terra_RailType railType;                                // Power rail type
    float nominalVoltage;                                   // Nominal rail voltage

    TerraPowerRailData();
    TerraString toJSON() const;
    bool fromJSON(const TerraString &json);
};

struct TerraEnvironmentData : public TerraObjectData {
    TerraEnvironmentData();
    TerraString toJSON() const;
    bool fromJSON(const TerraString &json);
};

#endif
