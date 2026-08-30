/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Sensor Measurements
*/

#include "Terraduino.h"

TerraSingleMeasurement TerraBinaryMeasurement::getAsSingleMeasurement(float trueScale, Terra_UnitsType unitsIn) const
{
    return TerraSingleMeasurement(state ? trueScale : 0.0f, unitsIn, timestamp, frame);
}

TerraDoubleMeasurement::TerraDoubleMeasurement(float value1, Terra_UnitsType units1,
                                               float value2, Terra_UnitsType units2,
                                               uint32_t timestampIn, tframe_t frameIn)
    : TerraMeasurement(Double, timestampIn, frameIn), value{value1, value2}, units{units1, units2}
{ ; }

TerraSingleMeasurement TerraDoubleMeasurement::getAsSingleMeasurement(uint8_t row) const
{
    return row < 2 ? TerraSingleMeasurement(value[row], units[row], timestamp, frame) : TerraSingleMeasurement();
}

TerraTripleMeasurement::TerraTripleMeasurement(float value1, Terra_UnitsType units1,
                                               float value2, Terra_UnitsType units2,
                                               float value3, Terra_UnitsType units3,
                                               uint32_t timestampIn, tframe_t frameIn)
    : TerraMeasurement(Triple, timestampIn, frameIn),
      value{value1, value2, value3}, units{units1, units2, units3}
{ ; }

TerraSingleMeasurement TerraTripleMeasurement::getAsSingleMeasurement(uint8_t row) const
{
    return row < 3 ? TerraSingleMeasurement(value[row], units[row], timestamp, frame) : TerraSingleMeasurement();
}

TerraDoubleMeasurement TerraTripleMeasurement::getAsDoubleMeasurement(uint8_t row1, uint8_t row2) const
{
    return row1 < 3 && row2 < 3
        ? TerraDoubleMeasurement(value[row1], units[row1], value[row2], units[row2], timestamp, frame)
        : TerraDoubleMeasurement();
}

float getMeasurementValue(const TerraMeasurement *measurement, uint8_t row, float trueScale)
{
    if (!measurement) { return 0.0f; }
    switch (measurement->type) {
        case TerraMeasurement::Binary: return static_cast<const TerraBinaryMeasurement *>(measurement)->state ? trueScale : 0.0f;
        case TerraMeasurement::Single: return static_cast<const TerraSingleMeasurement *>(measurement)->value;
        case TerraMeasurement::Double: return row < 2 ? static_cast<const TerraDoubleMeasurement *>(measurement)->value[row] : 0.0f;
        case TerraMeasurement::Triple: return row < 3 ? static_cast<const TerraTripleMeasurement *>(measurement)->value[row] : 0.0f;
        default: break;
    }
    return 0.0f;
}

Terra_UnitsType getMeasurementUnits(const TerraMeasurement *measurement, uint8_t row, Terra_UnitsType binaryUnits)
{
    if (!measurement) { return Terra_UnitsType_Undefined; }
    switch (measurement->type) {
        case TerraMeasurement::Binary: return binaryUnits;
        case TerraMeasurement::Single: return static_cast<const TerraSingleMeasurement *>(measurement)->units;
        case TerraMeasurement::Double: return row < 2 ? static_cast<const TerraDoubleMeasurement *>(measurement)->units[row] : Terra_UnitsType_Undefined;
        case TerraMeasurement::Triple: return row < 3 ? static_cast<const TerraTripleMeasurement *>(measurement)->units[row] : Terra_UnitsType_Undefined;
        default: break;
    }
    return Terra_UnitsType_Undefined;
}

uint8_t getMeasurementRowCount(const TerraMeasurement *measurement)
{
    if (!measurement) { return 0; }
    switch (measurement->type) {
        case TerraMeasurement::Binary:
        case TerraMeasurement::Single: return 1;
        case TerraMeasurement::Double: return 2;
        case TerraMeasurement::Triple: return 3;
        default: break;
    }
    return 0;
}

TerraSingleMeasurement getAsSingleMeasurement(const TerraMeasurement *measurement, uint8_t row,
                                              float trueScale, Terra_UnitsType binaryUnits)
{
    if (!measurement) { return TerraSingleMeasurement(); }
    switch (measurement->type) {
        case TerraMeasurement::Binary: return static_cast<const TerraBinaryMeasurement *>(measurement)->getAsSingleMeasurement(trueScale, binaryUnits);
        case TerraMeasurement::Single: return *static_cast<const TerraSingleMeasurement *>(measurement);
        case TerraMeasurement::Double: return static_cast<const TerraDoubleMeasurement *>(measurement)->getAsSingleMeasurement(row);
        case TerraMeasurement::Triple: return static_cast<const TerraTripleMeasurement *>(measurement)->getAsSingleMeasurement(row);
        default: break;
    }
    return TerraSingleMeasurement();
}

TerraMeasurementData::TerraMeasurementData()
    : TerraSubData(TerraMeasurement::Unknown), measurementRow(0), value(0.0f),
      units(Terra_UnitsType_Undefined), timestamp(0)
{ ; }

void TerraMeasurementData::toJSONObject(JsonObject &objectOut) const
{
    objectOut[SFP(TStr_Key_MeasurementRow)] = measurementRow;
    objectOut[SFP(TStr_Key_Value)] = value;
    objectOut[SFP(TStr_Key_Units)] = unitsTypeToSymbol(units);
    objectOut[SFP(TStr_Key_Timestamp)] = timestamp;
}

void TerraMeasurementData::fromJSONObject(JsonObjectConst &objectIn)
{
    measurementRow = objectIn[SFP(TStr_Key_MeasurementRow)] | measurementRow;
    value = objectIn[SFP(TStr_Key_Value)] | value;
    const char *unitsString = objectIn[SFP(TStr_Key_Units)] | nullptr;
    if (unitsString) { units = unitsTypeFromSymbol(unitsString); }
    timestamp = objectIn[SFP(TStr_Key_Timestamp)] | timestamp;
}

void TerraMeasurementData::fromJSONVariant(JsonVariantConst &variantIn)
{
    if (variantIn.is<JsonObjectConst>()) {
        JsonObjectConst objectIn = variantIn.as<JsonObjectConst>();
        fromJSONObject(objectIn);
    } else if (variantIn.is<float>() || variantIn.is<int>()) {
        value = variantIn.as<float>();
    } else {
        TERRA_SOFT_ASSERT(false, SFP(TStr_Err_UnsupportedOperation));
    }
}
