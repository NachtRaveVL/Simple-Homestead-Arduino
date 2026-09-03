#include <Terraduino.h>

static String unitsTypeToStringForTest(Terra_UnitsType value) { return String(unitsTypeToSymbol(value)); }
static Terra_UnitsType unitsTypeFromStringForTest(const String &value) { return unitsTypeFromSymbol(value.c_str()); }
static String unitsCategoryToStringForTest(Terra_UnitsCategory value) { return String(unitsCategoryToString(value)); }
static Terra_UnitsCategory unitsCategoryFromStringForTest(const String &value) { return unitsCategoryFromString(value.c_str()); }

template <typename EnumType>
bool checkEnumRange(const char *label,
                    int first,
                    int last,
                    String (*toStringFn)(EnumType),
                    EnumType (*fromStringFn)(const String &)) {
    bool pass = true;
    for (int i = first; i <= last; ++i) {
        EnumType value = (EnumType)i;
        String encoded = toStringFn(value);
        if (fromStringFn(encoded) != value) {
            Serial.print(F("FAIL "));
            Serial.print(label);
            Serial.print(F(" value "));
            Serial.print(i);
            Serial.print(F(" encoded as "));
            Serial.println(encoded.c_str());
            pass = false;
        }

        String lower(encoded.c_str());
        lower.toLowerCase();
        if (fromStringFn(String(lower.c_str())) != value) {
            Serial.print(F("FAIL lower-case "));
            Serial.print(label);
            Serial.print(F(" value "));
            Serial.println(i);
            pass = false;
        }

        String upper(encoded.c_str());
        upper.toUpperCase();
        if (fromStringFn(String(upper.c_str())) != value) {
            Serial.print(F("FAIL upper-case "));
            Serial.print(label);
            Serial.print(F(" value "));
            Serial.println(i);
            pass = false;
        }
    }
    return pass;
}

static bool nearValue(float value, float expected, float tolerance = 0.0001f)
{
    return fabsf(value - expected) <= tolerance;
}

void setup() {
    Serial.begin(115200);
    while (!Serial) { }

    bool pass = true;
    pass &= checkEnumRange<Terra_ObjectType>("ObjectType", Terra_ObjectType_Undefined, Terra_ObjectType_Count, terraObjectTypeToString, terraObjectTypeFromString);
    pass &= checkEnumRange<Terra_ReservoirType>("ReservoirType", Terra_ReservoirType_Undefined, Terra_ReservoirType_Count, TerraReservoirTypeToString, TerraReservoirTypeFromString);
    pass &= checkEnumRange<Terra_ResourceState>("ResourceState", Terra_ResourceState_Unknown, Terra_ResourceState_Count, TerraReservoirStateToString, TerraReservoirStateFromString);
    pass &= checkEnumRange<Terra_SensorType>("SensorType", Terra_SensorType_Undefined, Terra_SensorType_Count, terraSensorTypeToString, terraSensorTypeFromString);
    pass &= checkEnumRange<Terra_ActuatorType>("ActuatorType", Terra_ActuatorType_Undefined, Terra_ActuatorType_Count, terraActuatorTypeToString, terraActuatorTypeFromString);
    pass &= checkEnumRange<Terra_MeasurementMode>("MeasurementMode", Terra_MeasurementMode_Undefined, Terra_MeasurementMode_Count, terraMeasurementModeToString, terraMeasurementModeFromString);
    pass &= checkEnumRange<Terra_EnableMode>("EnableMode", Terra_EnableMode_Undefined, Terra_EnableMode_Count, terraEnableModeToString, terraEnableModeFromString);
    pass &= checkEnumRange<Terra_PinMode>("PinMode", Terra_PinMode_Undefined, Terra_PinMode_Count, terraPinModeToString, terraPinModeFromString);
    pass &= checkEnumRange<Terra_RailType>("RailType", Terra_RailType_Undefined, Terra_RailType_Count, terraRailTypeToString, terraRailTypeFromString);
    pass &= checkEnumRange<Terra_TriggerState>("TriggerState", Terra_TriggerState_Undefined, Terra_TriggerState_Count, terraTriggerStateToString, terraTriggerStateFromString);
    pass &= checkEnumRange<Terra_SystemMode>("SystemMode", Terra_SystemMode_Undefined, Terra_SystemMode_Count, terraSystemModeToString, terraSystemModeFromString);
    pass &= checkEnumRange<Terra_UnitsType>("UnitsType", Terra_UnitsType_Undefined, Terra_UnitsType_Count, unitsTypeToStringForTest, unitsTypeFromStringForTest);
    pass &= checkEnumRange<Terra_UnitsCategory>("UnitsCategory", Terra_UnitsCategory_Undefined, Terra_UnitsCategory_Count, unitsCategoryToStringForTest, unitsCategoryFromStringForTest);

    // Invalid paths through nested trie switches must fall back rather than falling through to another enum value.
    pass &= terraObjectTypeFromString(String("xxix")) == Terra_ObjectType_Undefined;
    pass &= terraSensorTypeFromString(String("cx")) == Terra_SensorType_Undefined;
    pass &= terraRailTypeFromString(String("zz1")) == Terra_RailType_Undefined;
    pass &= sensorTypeFromString(String("cx")) == Terra_SensorType_Undefined;
    pass &= railTypeFromString(String("zz1")) == Terra_RailType_Undefined;

    float converted = 0.0f;
    pass &= tryConvertUnits(32.0f, Terra_UnitsType_Temperature_Fahrenheit, &converted, Terra_UnitsType_Temperature_Celsius) && nearValue(converted, 0.0f);
    pass &= tryConvertUnits(1.0f, Terra_UnitsType_LiqVolume_Gallons, &converted, Terra_UnitsType_LiqVolume_Liters) && nearValue(converted, 3.78541f);
    pass &= tryConvertUnits(1.0f, Terra_UnitsType_LiqFlowRate_GallonsPerMin, &converted, Terra_UnitsType_LiqFlowRate_LitersPerMin) && nearValue(converted, 3.78541f);
    pass &= tryConvertUnits(101.325f, Terra_UnitsType_Pressure_Kilopascals, &converted, Terra_UnitsType_Pressure_Hectopascals) && nearValue(converted, 1013.25f, 0.001f);
    pass &= tryConvertUnits(25.4f, Terra_UnitsType_Distance_Millimeters, &converted, Terra_UnitsType_Distance_Inches) && nearValue(converted, 1.0f);

    pass &= tryConvertUnits(180.0f, Terra_UnitsType_Angle_Degrees_360, &converted, Terra_UnitsType_Angle_Radians_2pi) && nearValue(converted, 3.14159265f);
    pass &= tryConvertUnits(3.14159265f, Terra_UnitsType_Angle_Radians_2pi, &converted, Terra_UnitsType_Angle_Minutes_24hr) && nearValue(converted, 720.0f, 0.001f);
    pass &= tryConvertUnits(720.0f, Terra_UnitsType_Angle_Minutes_24hr, &converted, Terra_UnitsType_Angle_Degrees_360) && nearValue(converted, 180.0f);

    pass &= tryConvertUnits(25.4f, Terra_UnitsType_Speed_MillimetersPerHour, &converted, Terra_UnitsType_Speed_InchesPerHour) && nearValue(converted, 1.0f);
    pass &= tryConvertUnits(3600000.0f, Terra_UnitsType_Speed_MillimetersPerHour, &converted, Terra_UnitsType_Speed_MetersPerSecond) && nearValue(converted, 1.0f);
    pass &= tryConvertUnits(1.0f, Terra_UnitsType_Speed_MetersPerSecond, &converted, Terra_UnitsType_Speed_KilometersPerHour) && nearValue(converted, 3.6f);
    pass &= tryConvertUnits(1.0f, Terra_UnitsType_Speed_MetersPerSecond, &converted, Terra_UnitsType_Speed_MilesPerHour) && nearValue(converted, 2.236936f);
    pass &= tryConvertUnits(1.0f, Terra_UnitsType_Speed_MilesPerHour, &converted, Terra_UnitsType_Speed_InchesPerHour) && nearValue(converted, 63360.0f, 0.01f);

    pass &= tryConvertUnits(1.0f, Terra_UnitsType_Power_Amperage, &converted, Terra_UnitsType_Power_Wattage, 12.0f) && nearValue(converted, 12.0f);
    pass &= tryConvertUnits(24.0f, Terra_UnitsType_Power_Wattage, &converted, Terra_UnitsType_Power_Amperage, 12.0f) && nearValue(converted, 2.0f);

    pass &= baseUnits(Terra_UnitsType_LiqFlowRate_LitersPerMin) == Terra_UnitsType_LiqVolume_Liters;
    pass &= rateUnits(Terra_UnitsType_LiqVolume_Liters) == Terra_UnitsType_LiqFlowRate_LitersPerMin;
    pass &= baseUnits(Terra_UnitsType_Speed_MillimetersPerHour) == Terra_UnitsType_Distance_Millimeters;
    pass &= baseUnits(Terra_UnitsType_Speed_InchesPerHour) == Terra_UnitsType_Distance_Inches;
    pass &= baseUnits(Terra_UnitsType_Speed_MetersPerSecond) == Terra_UnitsType_Distance_Meters;
    pass &= baseUnits(Terra_UnitsType_Speed_KilometersPerHour) == Terra_UnitsType_Undefined;
    pass &= rateUnits(Terra_UnitsType_Distance_Meters) == Terra_UnitsType_Speed_MetersPerSecond;

    pass &= defaultPercentileUnits() == Terra_UnitsType_Percentile_100;
    pass &= defaultRainRateUnits(Terra_MeasurementMode_Imperial) == Terra_UnitsType_Speed_InchesPerHour;
    pass &= defaultRainRateUnits(Terra_MeasurementMode_Metric) == Terra_UnitsType_Speed_MillimetersPerHour;
    pass &= defaultVoltageUnits() == Terra_UnitsType_Power_Volts;
    pass &= defaultCurrentUnits() == Terra_UnitsType_Power_Amperage;
    pass &= defaultUnits(Terra_UnitsCategory_Irradiance, Terra_MeasurementMode_Metric) == Terra_UnitsType_Irradiance_WattsPerSquareMeter;
    pass &= defaultUnits(Terra_UnitsCategory_Speed, Terra_MeasurementMode_Imperial) == Terra_UnitsType_Speed_MilesPerHour;
    pass &= defaultUnits(Terra_UnitsCategory_Angle, Terra_MeasurementMode_Scientific) == Terra_UnitsType_Angle_Degrees_360;

    Serial.println(pass ? F("PASS") : F("FAIL"));
}

void loop() { }
