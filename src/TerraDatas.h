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


// Calibration Data
// Stores a simple Ax+B linear transformation mapping for sensor values.
struct TerraCalibrationData {
    uint32_t ownerKey;                                      // Owning object key
    Terra_Unit calibrationUnits;                            // Calibration output units
    float multiplier;                                       // Linear calibration multiplier
    float offset;                                           // Linear calibration offset

    TerraCalibrationData();
    TerraCalibrationData(uint32_t ownerKeyIn, Terra_Unit calibrationUnitsIn = Terra_Unit_Undefined);

    // Transforms value from raw (or initial) value into calibrated (or transformed) value.
    inline float transform(float value) const { return (value * multiplier) + offset; }
    // Transforms value in-place from raw (or initial) value into calibrated (or transformed) value, with optional units write out.
    inline void transform(float *valueInOut, Terra_Unit *unitsOut = nullptr) const { *valueInOut = transform(*valueInOut);
                                                                                   if (unitsOut) { *unitsOut = calibrationUnits; } }

    // Inverse transforms value from calibrated (or transformed) value back into raw (or initial) value.
    inline float inverseTransform(float value) const { return (value - offset) / multiplier; }
    // Inverse transforms value in-place from calibrated (or transformed) value back into raw (or initial) value, with optional units write out.
    inline void inverseTransform(float *valueInOut, Terra_Unit *unitsOut = nullptr) const { *valueInOut = inverseTransform(*valueInOut);
                                                                                          if (unitsOut) { *unitsOut = Terra_Unit_Raw; } }

    // Sets linear calibration curvature from two points.
    void setFromTwoPoints(float point1RawMeasuredAt,
                          float point1CalibratedTo,
                          float point2RawMeasuredAt,
                          float point2CalibratedTo);

    // Sets linear calibration curvature from two voltages.
    inline void setFromTwoVoltages(float point1VoltsAt,
                                   float point1CalibTo,
                                   float point2VoltsAt,
                                   float point2CalibTo,
                                   float analogRefVolts) {
        setFromTwoPoints(point1VoltsAt / analogRefVolts, point1CalibTo,
                         point2VoltsAt / analogRefVolts, point2CalibTo);
    }

    // Sets linear calibration curvature from known output range.
    inline void setFromRange(float minValue, float maxValue) { setFromTwoPoints(0.0f, minValue, 1.0f, maxValue); }
    // Sets linear calibration curvature from known output scale.
    inline void setFromScale(float scale) { setFromRange(0.0f, scale); }

    TerraString toJSON() const;
    bool fromJSON(const TerraString &json);
};

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
