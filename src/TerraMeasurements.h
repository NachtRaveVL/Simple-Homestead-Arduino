/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Sensor Measurements
*/

#ifndef TerraMeasurements_H
#define TerraMeasurements_H

#include "TerraData.h"

struct TerraSingleMeasurement;

// Sensor Measurement Base
struct TerraMeasurement {
    enum : signed char { Binary, Single, Double, Triple, Unknown = -1 } type; // Measurement class type

    uint32_t timestamp;                                     // Measurement timestamp
    tframe_t frame;                                         // Polling frame number

    TerraMeasurement(int classType = Unknown,
                     uint32_t timestampIn = 0,
                     tframe_t frameIn = tframe_none)
        : type((decltype(type))classType), timestamp(timestampIn), frame(frameIn) { ; }

    inline bool isBinaryType() const { return type == Binary; }
    inline bool isSingleType() const { return type == Single; }
    inline bool isDoubleType() const { return type == Double; }
    inline bool isTripleType() const { return type == Triple; }
    inline bool isUnknownType() const { return type <= Unknown; }
    inline bool isSet() const { return frame != tframe_none; }

    inline void updateFrame(tframe_t minFrame = 1) { frame = frame < minFrame ? minFrame : frame; }
};

// Binary Measurement
struct TerraBinaryMeasurement : public TerraMeasurement {
    bool state;                                             // Binary state

    TerraBinaryMeasurement(bool stateIn = false,
                           uint32_t timestampIn = 0,
                           tframe_t frameIn = tframe_none)
        : TerraMeasurement(Binary, timestampIn, frameIn), state(stateIn) { ; }

    TerraSingleMeasurement getAsSingleMeasurement(float trueScale = 1.0f,
                                                   Terra_UnitsType unitsIn = Terra_UnitsType_Raw_1) const;
};

// Single Value Measurement
struct TerraSingleMeasurement : public TerraMeasurement {
    float value;                                            // Measured value
    Terra_UnitsType units;                                       // Measurement units

    TerraSingleMeasurement(float valueIn = 0.0f,
                           Terra_UnitsType unitsIn = Terra_UnitsType_Undefined,
                           uint32_t timestampIn = 0,
                           tframe_t frameIn = tframe_none)
        : TerraMeasurement(Single, timestampIn, frameIn), value(valueIn), units(unitsIn) { ; }

    // Modifiers (in utils)

    inline TerraSingleMeasurement &toUnits(Terra_UnitsType outUnits, float convertParam = FLT_UNDEF);
    inline TerraSingleMeasurement &wrapBy(float range);
    inline TerraSingleMeasurement &wrapBySplit(float range);

    // Copiers (in utils)

    inline TerraSingleMeasurement asUnits(Terra_UnitsType outUnits, float convertParam = FLT_UNDEF) const;
    inline TerraSingleMeasurement wrappedBy(float range) const;
    inline TerraSingleMeasurement wrappedBySplit(float range) const;
};

// Double Value Measurement
struct TerraDoubleMeasurement : public TerraMeasurement {
    float value[2];                                         // Measured values
    Terra_UnitsType units[2];                                    // Measurement units

    TerraDoubleMeasurement(float value1 = 0.0f, Terra_UnitsType units1 = Terra_UnitsType_Undefined,
                           float value2 = 0.0f, Terra_UnitsType units2 = Terra_UnitsType_Undefined,
                           uint32_t timestampIn = 0, tframe_t frameIn = tframe_none);

    TerraSingleMeasurement getAsSingleMeasurement(uint8_t row) const;
};

// Triple Value Measurement
struct TerraTripleMeasurement : public TerraMeasurement {
    float value[3];                                         // Measured values
    Terra_UnitsType units[3];                                    // Measurement units

    TerraTripleMeasurement(float value1 = 0.0f, Terra_UnitsType units1 = Terra_UnitsType_Undefined,
                           float value2 = 0.0f, Terra_UnitsType units2 = Terra_UnitsType_Undefined,
                           float value3 = 0.0f, Terra_UnitsType units3 = Terra_UnitsType_Undefined,
                           uint32_t timestampIn = 0, tframe_t frameIn = tframe_none);

    TerraSingleMeasurement getAsSingleMeasurement(uint8_t row) const;
    TerraDoubleMeasurement getAsDoubleMeasurement(uint8_t row1, uint8_t row2) const;
};

extern float getMeasurementValue(const TerraMeasurement *measurement, uint8_t row = 0, float trueScale = 1.0f);
extern Terra_UnitsType getMeasurementUnits(const TerraMeasurement *measurement, uint8_t row = 0,
                                      Terra_UnitsType binaryUnits = Terra_UnitsType_Raw_1);
extern uint8_t getMeasurementRowCount(const TerraMeasurement *measurement);
extern TerraSingleMeasurement getAsSingleMeasurement(const TerraMeasurement *measurement, uint8_t row = 0,
                                                      float trueScale = 1.0f,
                                                      Terra_UnitsType binaryUnits = Terra_UnitsType_Raw_1);

// Measurement Serialization Data
struct TerraMeasurementData : public TerraSubData {
    uint8_t measurementRow;                                // Measurement row
    float value;                                            // Measured value
    Terra_UnitsType units;                                       // Measurement units
    uint32_t timestamp;                                     // Measurement timestamp

    TerraMeasurementData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
    void fromJSONVariant(JsonVariantConst &variantIn);
};

#endif // /ifndef TerraMeasurements_H
