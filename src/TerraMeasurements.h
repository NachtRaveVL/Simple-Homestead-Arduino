/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2023 NachtRaveVL          <nachtravevl@gmail.com>
    Terraduino Sensor Measurements
*/

#ifndef TerraMeasurements_H
#define TerraMeasurements_H

struct TerraMeasurement;
struct TerraSingleMeasurement;
struct TerraBinaryMeasurement;
struct TerraDoubleMeasurement;
struct TerraTripleMeasurement;

struct TerraMeasurementData;

#include "Terraduino.h"
#include "TerraData.h"

// Creates measurement object from passed trigger sub data (return ownership transfer - user code *must* delete returned object)
extern TerraMeasurement *newMeasurementObjectFromSubData(const TerraMeasurementData *dataIn);

// Gets the value of a measurement at a specified row (with optional binary true scaling value).
extern float getMeasurementValue(const TerraMeasurement *measurement, uint8_t measurementRow = 0, float binScale = 1.0f);
// Gets the units of a measurement at a specified row (with optional binary units).
extern Terra_UnitsType getMeasurementUnits(const TerraMeasurement *measurement, uint8_t measurementRow = 0, Terra_UnitsType binUnits = Terra_UnitsType_Raw_1);
// Gets the number of rows of data that a measurement holds.
extern uint8_t getMeasurementRowCount(const TerraMeasurement *measurement);
// Gets the single measurement of a measurement (with optional binary true value / units).
extern TerraSingleMeasurement getAsSingleMeasurement(const TerraMeasurement *measurement, uint8_t measurementRow = 0, float binScale = 1.0f, Terra_UnitsType binUnits = Terra_UnitsType_Raw_1);

// Sensor Data Measurement Base
struct TerraMeasurement {
    enum : signed char { Binary, Single, Double, Triple, Unknown = -1 } type; // Measurement type (custom RTTI)
    inline bool isBinaryType() const { return type == Binary; }
    inline bool isSingleType() const { return type == Single; }
    inline bool isDoubleType() const { return type == Double; }
    inline bool isTripleType() const { return type == Triple; }
    inline bool isUnknownType() const { return type <= Unknown; }

    time_t timestamp;                                       // Time event recorded (UTC)
    hframe_t frame;                                         // Polling frame # measurement taken on, or 0 if not-set else 1 if user-set

    inline TerraMeasurement() : type(Unknown), timestamp(unixNow()), frame(0) { ; }
    inline TerraMeasurement(int classType, time_t timestampIn, hframe_t frameIn) : type((typeof(type))classType), timestamp(timestampIn), frame(frameIn) { ; }
    TerraMeasurement(int classType, time_t timestamp = 0);
    TerraMeasurement(const TerraMeasurementData *dataIn);

    void saveToData(TerraMeasurementData *dataOut, uint8_t measurementRow = 0, unsigned int additionalDecPlaces = 0) const;

    inline void updateTimestamp() { timestamp = unixNow(); }
    void updateFrame(hframe_t minFrame = 0);
    inline void setMinFrame(hframe_t minFrame = 0) { frame = max(minFrame, frame); }
    inline bool isSet() const { return frame != hframe_none; }
};

// Single Value Sensor Data Measurement
struct TerraSingleMeasurement : public TerraMeasurement {
    float value;                                            // Polled value
    Terra_UnitsType units;                                  // Units of value

    TerraSingleMeasurement();
    TerraSingleMeasurement(float value, Terra_UnitsType units, time_t timestamp = unixNow());
    TerraSingleMeasurement(float value, Terra_UnitsType units, time_t timestamp, hframe_t frame);
    TerraSingleMeasurement(const TerraMeasurementData *dataIn);

    void saveToData(TerraMeasurementData *dataOut, uint8_t measurementRow = 0, unsigned int additionalDecPlaces = 0) const;

    // Modifiers (in utils)

    inline TerraSingleMeasurement &toUnits(Terra_UnitsType outUnits, float convertParam = FLT_UNDEF);

    inline TerraSingleMeasurement &wrapBy(float range);
    inline TerraSingleMeasurement &wrapBySplit(float range);
    inline TerraSingleMeasurement &wrapBy360() { return wrapBy(360); }
    inline TerraSingleMeasurement &wrapBy180Neg180() { return wrapBySplit(360); }
    inline TerraSingleMeasurement &wrapBy2Pi() { return wrapBy(TWO_PI); }
    inline TerraSingleMeasurement &wrapByPiNegPi() { return wrapBySplit(TWO_PI); }
    inline TerraSingleMeasurement &wrapBy24Hr() { return wrapBy(MIN_PER_DAY); }
    inline TerraSingleMeasurement &wrapBy12HrNeg12Hr() { return wrapBySplit(MIN_PER_DAY); }

    // Copiers (in utils)

    inline TerraSingleMeasurement asUnits(Terra_UnitsType outUnits, float convertParam = FLT_UNDEF) const;

    inline TerraSingleMeasurement wrappedBy(float range) const;
    inline TerraSingleMeasurement wrappedBySplit(float range) const;
    inline TerraSingleMeasurement wrappedBy360() const { return wrappedBy(360); }
    inline TerraSingleMeasurement wrappedBy180Neg180() const { return wrappedBySplit(360); }
    inline TerraSingleMeasurement wrappedBy2Pi() const { return wrappedBy(TWO_PI); }
    inline TerraSingleMeasurement wrappedByPiNegPi() const { return wrappedBySplit(TWO_PI); }
    inline TerraSingleMeasurement wrappedBy24Hr() const { return wrappedBy(MIN_PER_DAY); }
    inline TerraSingleMeasurement wrappedBy12HrNeg12Hr() const { return wrappedBySplit(MIN_PER_DAY); }
};

// Binary Value Sensor Data Measurement
struct TerraBinaryMeasurement : public TerraMeasurement {
    bool state;                                             // Polled state

    TerraBinaryMeasurement();
    TerraBinaryMeasurement(bool state, time_t timestamp = unixNow());
    TerraBinaryMeasurement(bool state, time_t timestamp, hframe_t frame);
    TerraBinaryMeasurement(const TerraMeasurementData *dataIn);

    void saveToData(TerraMeasurementData *dataOut, uint8_t measurementRow = 0, unsigned int additionalDecPlaces = 0) const;

    inline TerraSingleMeasurement getAsSingleMeasurement(float binScale = 1.0f, Terra_UnitsType binUnits = Terra_UnitsType_Raw_1) { return TerraSingleMeasurement(state ? binScale : 0.0f, binUnits, timestamp, frame); }
};

// Double Value Sensor Data Measurement
struct TerraDoubleMeasurement : public TerraMeasurement {
    float value[2];                                         // Polled values
    Terra_UnitsType units[2];                               // Units of values

    TerraDoubleMeasurement();
    TerraDoubleMeasurement(float value1, Terra_UnitsType units1, 
                           float value2, Terra_UnitsType units2, 
                           time_t timestamp = unixNow());
    TerraDoubleMeasurement(float value1, Terra_UnitsType units1, 
                           float value2, Terra_UnitsType units2, 
                           time_t timestamp, hframe_t frame);
    TerraDoubleMeasurement(const TerraMeasurementData *dataIn);

    void saveToData(TerraMeasurementData *dataOut, uint8_t measurementRow = 0, unsigned int additionalDecPlaces = 0) const;

    inline TerraSingleMeasurement getAsSingleMeasurement(uint8_t measurementRow) { return TerraSingleMeasurement(value[measurementRow], units[measurementRow], timestamp, frame); }
};

// Triple Value Sensor Data Measurement
struct TerraTripleMeasurement : public TerraMeasurement {
    float value[3];                                         // Polled values
    Terra_UnitsType units[3];                               // Units of values

    TerraTripleMeasurement();
    TerraTripleMeasurement(float value1, Terra_UnitsType units1, 
                           float value2, Terra_UnitsType units2, 
                           float value3, Terra_UnitsType units3,
                           time_t timestamp = unixNow());
    TerraTripleMeasurement(float value1, Terra_UnitsType units1, 
                           float value2, Terra_UnitsType units2, 
                           float value3, Terra_UnitsType units3,
                           time_t timestamp, hframe_t frame);
    TerraTripleMeasurement(const TerraMeasurementData *dataIn);

    void saveToData(TerraMeasurementData *dataOut, uint8_t measurementRow = 0, unsigned int additionalDecPlaces = 0) const;

    inline TerraSingleMeasurement getAsSingleMeasurement(uint8_t measurementRow) { return TerraSingleMeasurement(value[measurementRow], units[measurementRow], timestamp, frame); }
    inline TerraDoubleMeasurement getAsDoubleMeasurement(uint8_t measurementRow1, uint8_t measurementRow2) { return TerraDoubleMeasurement(value[measurementRow1], units[measurementRow1], value[measurementRow2], units[measurementRow2], timestamp, frame); }
};


// Combined Measurement Serialization Sub Data
struct TerraMeasurementData : public TerraSubData {
    uint8_t measurementRow;                                 // Source measurement row index that data is from
    float value;                                            // Value
    Terra_UnitsType units;                                  // Units of value
    time_t timestamp;                                       // Timestamp

    TerraMeasurementData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
    void fromJSONVariant(JsonVariantConst &variantIn);
};

#endif // /ifndef TerraMeasurements_H
