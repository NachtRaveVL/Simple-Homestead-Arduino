/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Measurements
*/

#ifndef TerraMeasurements_H
#define TerraMeasurements_H

#include "TerraTypes.h"

struct TerraMeasurement {
    float value;                              ///< Measurement value
    Terra_Unit unit;                          ///< Unit associated with value
    uint32_t timestamp;                       ///< Local/report timestamp in milliseconds
    bool valid;                               ///< Measurement validity

    TerraMeasurement(float valueIn = 0.0f,
                     Terra_Unit unitIn = Terra_Unit_Undefined,
                     uint32_t timestampIn = 0,
                     bool validIn = false)
        : value(valueIn), unit(unitIn), timestamp(timestampIn), valid(validIn) { }
};

Terra_UnitsCategory terraUnitsCategory(Terra_Unit unit);
Terra_Unit terraDefaultUnit(Terra_UnitsCategory category, Terra_MeasurementMode mode = Terra_MeasurementMode_Metric);
bool terraCanConvertUnits(Terra_Unit from, Terra_Unit to);
float terraConvertUnits(float value, Terra_Unit from, Terra_Unit to);
TerraMeasurement terraConvertMeasurement(const TerraMeasurement &measurement, Terra_Unit to);

#endif // /ifndef TerraMeasurements_H
