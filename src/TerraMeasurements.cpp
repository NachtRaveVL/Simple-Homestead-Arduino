/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Measurements
*/

#include "TerraMeasurements.h"
#include <math.h>

static bool terraSameFamily(Terra_Unit from, Terra_Unit to,
                            Terra_Unit a, Terra_Unit b,
                            Terra_Unit c = Terra_Unit_Undefined)
{
    const bool fromMatches = from == a || from == b || (c != Terra_Unit_Undefined && from == c);
    const bool toMatches = to == a || to == b || (c != Terra_Unit_Undefined && to == c);
    return fromMatches && toMatches;
}

Terra_UnitsCategory terraUnitsCategory(Terra_Unit unit)
{
    switch (unit) {
        case Terra_Unit_Raw: return Terra_UnitsCategory_Raw;
        case Terra_Unit_Percent: return Terra_UnitsCategory_Percentile;
        case Terra_Unit_Celsius:
        case Terra_Unit_Fahrenheit:
        case Terra_Unit_Kelvin: return Terra_UnitsCategory_Temperature;
        case Terra_Unit_Liters:
        case Terra_Unit_GallonsUS: return Terra_UnitsCategory_LiquidVolume;
        case Terra_Unit_LitersPerMinute:
        case Terra_Unit_GallonsPerMinute: return Terra_UnitsCategory_LiquidFlowRate;
        case Terra_Unit_Kilopascals:
        case Terra_Unit_PSI:
        case Terra_Unit_Hectopascals: return Terra_UnitsCategory_Pressure;
        case Terra_Unit_Millimeters:
        case Terra_Unit_Inches: return Terra_UnitsCategory_Distance;
        case Terra_Unit_MillimetersPerHour:
        case Terra_Unit_InchesPerHour: return Terra_UnitsCategory_RainRate;
        case Terra_Unit_Watts: return Terra_UnitsCategory_Power;
        case Terra_Unit_WattsPerSquareMeter: return Terra_UnitsCategory_Irradiance;
        case Terra_Unit_KilowattHours: return Terra_UnitsCategory_Energy;
        case Terra_Unit_MetersPerSecond:
        case Terra_Unit_KilometersPerHour:
        case Terra_Unit_MilesPerHour: return Terra_UnitsCategory_Speed;
        case Terra_Unit_Degrees: return Terra_UnitsCategory_Angle;
        case Terra_Unit_Volts: return Terra_UnitsCategory_Voltage;
        case Terra_Unit_Amps: return Terra_UnitsCategory_Current;
        case Terra_Unit_Undefined: break;
    }
    return Terra_UnitsCategory_Undefined;
}

Terra_Unit terraDefaultUnit(Terra_UnitsCategory category, Terra_MeasurementMode mode)
{
    switch (category) {
        case Terra_UnitsCategory_Raw: return Terra_Unit_Raw;
        case Terra_UnitsCategory_Percentile: return Terra_Unit_Percent;
        case Terra_UnitsCategory_Temperature:
            if (mode == Terra_MeasurementMode_Imperial) return Terra_Unit_Fahrenheit;
            if (mode == Terra_MeasurementMode_Scientific) return Terra_Unit_Kelvin;
            return Terra_Unit_Celsius;
        case Terra_UnitsCategory_LiquidVolume:
            return mode == Terra_MeasurementMode_Imperial ? Terra_Unit_GallonsUS : Terra_Unit_Liters;
        case Terra_UnitsCategory_LiquidFlowRate:
            return mode == Terra_MeasurementMode_Imperial ? Terra_Unit_GallonsPerMinute : Terra_Unit_LitersPerMinute;
        case Terra_UnitsCategory_Pressure:
            return mode == Terra_MeasurementMode_Imperial ? Terra_Unit_PSI : Terra_Unit_Kilopascals;
        case Terra_UnitsCategory_Distance:
            return mode == Terra_MeasurementMode_Imperial ? Terra_Unit_Inches : Terra_Unit_Millimeters;
        case Terra_UnitsCategory_RainRate:
            return mode == Terra_MeasurementMode_Imperial ? Terra_Unit_InchesPerHour : Terra_Unit_MillimetersPerHour;
        case Terra_UnitsCategory_Power: return Terra_Unit_Watts;
        case Terra_UnitsCategory_Irradiance: return Terra_Unit_WattsPerSquareMeter;
        case Terra_UnitsCategory_Energy: return Terra_Unit_KilowattHours;
        case Terra_UnitsCategory_Speed:
            if (mode == Terra_MeasurementMode_Imperial) return Terra_Unit_MilesPerHour;
            if (mode == Terra_MeasurementMode_Scientific) return Terra_Unit_MetersPerSecond;
            return Terra_Unit_KilometersPerHour;
        case Terra_UnitsCategory_Angle: return Terra_Unit_Degrees;
        case Terra_UnitsCategory_Voltage: return Terra_Unit_Volts;
        case Terra_UnitsCategory_Current: return Terra_Unit_Amps;
        case Terra_UnitsCategory_Count:
        case Terra_UnitsCategory_Undefined: break;
    }
    return Terra_Unit_Undefined;
}

bool terraCanConvertUnits(Terra_Unit from, Terra_Unit to)
{
    if (from == to) return true;
    if (from == Terra_Unit_Undefined || to == Terra_Unit_Undefined) return false;
    if (terraSameFamily(from, to, Terra_Unit_Celsius, Terra_Unit_Fahrenheit, Terra_Unit_Kelvin)) return true;
    if (terraSameFamily(from, to, Terra_Unit_Liters, Terra_Unit_GallonsUS)) return true;
    if (terraSameFamily(from, to, Terra_Unit_LitersPerMinute, Terra_Unit_GallonsPerMinute)) return true;
    if (terraSameFamily(from, to, Terra_Unit_Kilopascals, Terra_Unit_PSI, Terra_Unit_Hectopascals)) return true;
    if (terraSameFamily(from, to, Terra_Unit_Millimeters, Terra_Unit_Inches)) return true;
    if (terraSameFamily(from, to, Terra_Unit_MillimetersPerHour, Terra_Unit_InchesPerHour)) return true;
    if (terraSameFamily(from, to, Terra_Unit_MetersPerSecond, Terra_Unit_KilometersPerHour, Terra_Unit_MilesPerHour)) return true;
    return false;
}

float terraConvertUnits(float value, Terra_Unit from, Terra_Unit to)
{
    if (from == to) return value;
    if (!terraCanConvertUnits(from, to)) return NAN;

    if (from == Terra_Unit_Celsius) {
        if (to == Terra_Unit_Fahrenheit) return value * 9.0f / 5.0f + 32.0f;
        if (to == Terra_Unit_Kelvin) return value + 273.15f;
    } else if (from == Terra_Unit_Fahrenheit) {
        const float celsius = (value - 32.0f) * 5.0f / 9.0f;
        if (to == Terra_Unit_Celsius) return celsius;
        if (to == Terra_Unit_Kelvin) return celsius + 273.15f;
    } else if (from == Terra_Unit_Kelvin) {
        const float celsius = value - 273.15f;
        if (to == Terra_Unit_Celsius) return celsius;
        if (to == Terra_Unit_Fahrenheit) return celsius * 9.0f / 5.0f + 32.0f;
    }

    if (from == Terra_Unit_Liters) return value / 3.785411784f;
    if (from == Terra_Unit_GallonsUS) return value * 3.785411784f;
    if (from == Terra_Unit_LitersPerMinute) return value / 3.785411784f;
    if (from == Terra_Unit_GallonsPerMinute) return value * 3.785411784f;

    if (from == Terra_Unit_Kilopascals) {
        if (to == Terra_Unit_PSI) return value * 0.145037738f;
        if (to == Terra_Unit_Hectopascals) return value * 10.0f;
    } else if (from == Terra_Unit_PSI) {
        const float kpa = value / 0.145037738f;
        if (to == Terra_Unit_Kilopascals) return kpa;
        if (to == Terra_Unit_Hectopascals) return kpa * 10.0f;
    } else if (from == Terra_Unit_Hectopascals) {
        const float kpa = value / 10.0f;
        if (to == Terra_Unit_Kilopascals) return kpa;
        if (to == Terra_Unit_PSI) return kpa * 0.145037738f;
    }

    if (from == Terra_Unit_Millimeters) return value / 25.4f;
    if (from == Terra_Unit_Inches) return value * 25.4f;
    if (from == Terra_Unit_MillimetersPerHour) return value / 25.4f;
    if (from == Terra_Unit_InchesPerHour) return value * 25.4f;

    if (from == Terra_Unit_MetersPerSecond) {
        if (to == Terra_Unit_KilometersPerHour) return value * 3.6f;
        if (to == Terra_Unit_MilesPerHour) return value * 2.236936292f;
    } else if (from == Terra_Unit_KilometersPerHour) {
        const float mps = value / 3.6f;
        if (to == Terra_Unit_MetersPerSecond) return mps;
        if (to == Terra_Unit_MilesPerHour) return mps * 2.236936292f;
    } else if (from == Terra_Unit_MilesPerHour) {
        const float mps = value / 2.236936292f;
        if (to == Terra_Unit_MetersPerSecond) return mps;
        if (to == Terra_Unit_KilometersPerHour) return mps * 3.6f;
    }

    return NAN;
}

TerraMeasurement terraConvertMeasurement(const TerraMeasurement &measurement, Terra_Unit to)
{
    if (!measurement.valid) return TerraMeasurement(measurement.value, to, measurement.timestamp, false);
    const float converted = terraConvertUnits(measurement.value, measurement.unit, to);
    return TerraMeasurement(converted, to, measurement.timestamp, !isnan(converted));
}
