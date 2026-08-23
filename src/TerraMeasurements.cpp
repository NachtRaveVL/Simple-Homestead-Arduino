/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Sensor Measurements
*/

#include "Terraduino.h"
#include <math.h>

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

static Terra_UnitsCategory unitsCategory(Terra_UnitsType units)
{
    switch (units) {
        case Terra_UnitsType_Raw: return Terra_UnitsCategory_Raw;
        case Terra_UnitsType_Percent: return Terra_UnitsCategory_Percentile;
        case Terra_UnitsType_Celsius:
        case Terra_UnitsType_Fahrenheit:
        case Terra_UnitsType_Kelvin: return Terra_UnitsCategory_Temperature;
        case Terra_UnitsType_Liters:
        case Terra_UnitsType_GallonsUS: return Terra_UnitsCategory_LiquidVolume;
        case Terra_UnitsType_LitersPerMinute:
        case Terra_UnitsType_GallonsPerMinute: return Terra_UnitsCategory_LiquidFlowRate;
        case Terra_UnitsType_Kilopascals:
        case Terra_UnitsType_PSI:
        case Terra_UnitsType_Hectopascals: return Terra_UnitsCategory_Pressure;
        case Terra_UnitsType_Millimeters:
        case Terra_UnitsType_Inches: return Terra_UnitsCategory_Distance;
        case Terra_UnitsType_MillimetersPerHour:
        case Terra_UnitsType_InchesPerHour: return Terra_UnitsCategory_RainRate;
        case Terra_UnitsType_Watts: return Terra_UnitsCategory_Power;
        case Terra_UnitsType_WattsPerSquareMeter: return Terra_UnitsCategory_Irradiance;
        case Terra_UnitsType_KilowattHours: return Terra_UnitsCategory_Energy;
        case Terra_UnitsType_MetersPerSecond:
        case Terra_UnitsType_KilometersPerHour:
        case Terra_UnitsType_MilesPerHour: return Terra_UnitsCategory_Speed;
        case Terra_UnitsType_Degrees: return Terra_UnitsCategory_Angle;
        case Terra_UnitsType_Volts: return Terra_UnitsCategory_Voltage;
        case Terra_UnitsType_Amps: return Terra_UnitsCategory_Current;
        default: break;
    }
    return Terra_UnitsCategory_Undefined;
}

bool canConvertUnits(Terra_UnitsType fromUnits, Terra_UnitsType toUnits)
{
    return fromUnits == toUnits || (fromUnits != Terra_UnitsType_Undefined && toUnits != Terra_UnitsType_Undefined &&
                                    unitsCategory(fromUnits) == unitsCategory(toUnits));
}

float convertUnits(float value, Terra_UnitsType fromUnits, Terra_UnitsType toUnits)
{
    if (fromUnits == toUnits) { return value; }
    if (!canConvertUnits(fromUnits, toUnits)) { return NAN; }

    switch (unitsCategory(fromUnits)) {
        case Terra_UnitsCategory_Temperature: {
            float celsius = fromUnits == Terra_UnitsType_Celsius ? value :
                            fromUnits == Terra_UnitsType_Fahrenheit ? (value - 32.0f) * (5.0f / 9.0f) : value - 273.15f;
            return toUnits == Terra_UnitsType_Celsius ? celsius :
                   toUnits == Terra_UnitsType_Fahrenheit ? celsius * (9.0f / 5.0f) + 32.0f : celsius + 273.15f;
        }
        case Terra_UnitsCategory_LiquidVolume: {
            float liters = fromUnits == Terra_UnitsType_Liters ? value : value * 3.785411784f;
            return toUnits == Terra_UnitsType_Liters ? liters : liters / 3.785411784f;
        }
        case Terra_UnitsCategory_LiquidFlowRate: {
            float lpm = fromUnits == Terra_UnitsType_LitersPerMinute ? value : value * 3.785411784f;
            return toUnits == Terra_UnitsType_LitersPerMinute ? lpm : lpm / 3.785411784f;
        }
        case Terra_UnitsCategory_Pressure: {
            float kPa = fromUnits == Terra_UnitsType_Kilopascals ? value :
                        fromUnits == Terra_UnitsType_Hectopascals ? value / 10.0f : value * 6.894757293f;
            return toUnits == Terra_UnitsType_Kilopascals ? kPa :
                   toUnits == Terra_UnitsType_Hectopascals ? kPa * 10.0f : kPa / 6.894757293f;
        }
        case Terra_UnitsCategory_Distance: {
            float mm = fromUnits == Terra_UnitsType_Millimeters ? value : value * 25.4f;
            return toUnits == Terra_UnitsType_Millimeters ? mm : mm / 25.4f;
        }
        case Terra_UnitsCategory_RainRate: {
            float mmph = fromUnits == Terra_UnitsType_MillimetersPerHour ? value : value * 25.4f;
            return toUnits == Terra_UnitsType_MillimetersPerHour ? mmph : mmph / 25.4f;
        }
        case Terra_UnitsCategory_Speed: {
            float mps = fromUnits == Terra_UnitsType_MetersPerSecond ? value :
                        fromUnits == Terra_UnitsType_KilometersPerHour ? value / 3.6f : value * 0.44704f;
            return toUnits == Terra_UnitsType_MetersPerSecond ? mps :
                   toUnits == Terra_UnitsType_KilometersPerHour ? mps * 3.6f : mps / 0.44704f;
        }
        default: return value;
    }
}

TerraSingleMeasurement &TerraSingleMeasurement::toUnits(Terra_UnitsType outUnits)
{
    if (units != outUnits) {
        float converted = convertUnits(value, units, outUnits);
        if (!isnan(converted)) {
            value = converted;
            units = outUnits;
        } else {
            frame = tframe_none;
        }
    }
    return *this;
}

TerraSingleMeasurement TerraSingleMeasurement::asUnits(Terra_UnitsType outUnits) const
{
    TerraSingleMeasurement measurement(*this);
    return measurement.toUnits(outUnits);
}

TerraMeasurementData::TerraMeasurementData()
    : TerraSubData(TerraMeasurement::Unknown), measurementRow(0), value(0.0f),
      units(Terra_UnitsType_Undefined), timestamp(0)
{ ; }

void TerraMeasurementData::toJSONObject(JsonObject &objectOut) const
{
    objectOut["measurementRow"] = measurementRow;
    objectOut["value"] = value;
    objectOut["units"] = terraUnitToString(units);
    objectOut["timestamp"] = timestamp;
}

void TerraMeasurementData::fromJSONObject(JsonObjectConst &objectIn)
{
    measurementRow = objectIn["measurementRow"] | measurementRow;
    value = objectIn["value"] | value;
    const char *unitsString = objectIn["units"] | nullptr;
    if (unitsString) { units = unitsTypeFromSymbol(TerraString(unitsString)); }
    timestamp = objectIn["timestamp"] | timestamp;
}

void TerraMeasurementData::fromJSONVariant(JsonVariantConst &variantIn)
{
    if (variantIn.is<JsonObjectConst>()) {
        JsonObjectConst objectIn = variantIn.as<JsonObjectConst>();
        fromJSONObject(objectIn);
    } else if (variantIn.is<float>() || variantIn.is<int>()) {
        value = variantIn.as<float>();
    } else {
        TERRA_SOFT_ASSERT(false, TerraString("Unsupported operation"));
    }
}
