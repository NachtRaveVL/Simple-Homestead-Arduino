#include <Terraduino.h>

static TerraString unitsTypeToStringForTest(Terra_UnitsType value) { return unitsTypeToSymbol(value); }
static Terra_UnitsType unitsTypeFromStringForTest(const TerraString &value) { return unitsTypeFromSymbol(String(value.c_str())); }
static TerraString unitsCategoryToStringForTest(Terra_UnitsCategory value) { return unitsCategoryToString(value); }
static Terra_UnitsCategory unitsCategoryFromStringForTest(const TerraString &value) { return unitsCategoryFromString(String(value.c_str())); }

template <typename EnumType>
bool checkEnumRange(const char *label,
                    int first,
                    int last,
                    TerraString (*toStringFn)(EnumType),
                    EnumType (*fromStringFn)(const TerraString &)) {
    bool pass = true;
    for (int i = first; i <= last; ++i) {
        EnumType value = (EnumType)i;
        TerraString encoded = toStringFn(value);
        EnumType decoded = fromStringFn(encoded);
        if (decoded != value) {
            Serial.print(F("FAIL "));
            Serial.print(label);
            Serial.print(F(" value "));
            Serial.print(i);
            Serial.print(F(" encoded as "));
            Serial.println(encoded);
            pass = false;
        }

        String lower(encoded.c_str());
        lower.toLowerCase();
        if (fromStringFn(lower) != value) {
            Serial.print(F("FAIL case-insensitive "));
            Serial.print(label);
            Serial.print(F(" value "));
            Serial.println(i);
            pass = false;
        }
    }
    return pass;
}

void setup() {
    Serial.begin(115200);
    while (!Serial) { }

    bool pass = true;
    pass &= checkEnumRange<Terra_ObjectType>("ObjectType", Terra_ObjectType_Undefined, Terra_ObjectType_PowerRail, terraObjectTypeToString, terraObjectTypeFromString);
    pass &= checkEnumRange<Terra_ReservoirType>("ReservoirType", Terra_ReservoirType_Undefined, Terra_ReservoirType_Thermal, TerraReservoirTypeToString, TerraReservoirTypeFromString);
    pass &= checkEnumRange<Terra_ResourceState>("ResourceState", Terra_ResourceState_Unknown, Terra_ResourceState_Fault, TerraReservoirStateToString, TerraReservoirStateFromString);
    pass &= checkEnumRange<Terra_WaterSourceType>("WaterSourceType", Terra_WaterSourceType_Undefined, Terra_WaterSourceType_Stored, terraWaterSourceTypeToString, terraWaterSourceTypeFromString);
    pass &= checkEnumRange<Terra_WaterStorageType>("WaterStorageType", Terra_WaterStorageType_Undefined, Terra_WaterStorageType_Reservoir, terraWaterStorageTypeToString, terraWaterStorageTypeFromString);
    pass &= checkEnumRange<Terra_SensorType>("SensorType", Terra_SensorType_Undefined, Terra_SensorType_Remote, terraSensorTypeToString, terraSensorTypeFromString);
    pass &= checkEnumRange<Terra_ActuatorType>("ActuatorType", Terra_ActuatorType_Undefined, Terra_ActuatorType_SumpPump, terraActuatorTypeToString, terraActuatorTypeFromString);
    pass &= checkEnumRange<Terra_MeasurementMode>("MeasurementMode", Terra_MeasurementMode_Undefined, Terra_MeasurementMode_Count, terraMeasurementModeToString, terraMeasurementModeFromString);
    pass &= checkEnumRange<Terra_EnableMode>("EnableMode", Terra_EnableMode_Undefined, Terra_EnableMode_Count, terraEnableModeToString, terraEnableModeFromString);
    pass &= checkEnumRange<Terra_PinMode>("PinMode", Terra_PinMode_Undefined, Terra_PinMode_Count, terraPinModeToString, terraPinModeFromString);
    pass &= checkEnumRange<Terra_UnitsType>("Unit", Terra_UnitsType_Undefined, Terra_UnitsType_Count, unitsTypeToStringForTest, unitsTypeFromStringForTest);
    pass &= checkEnumRange<Terra_UnitsCategory>("UnitsCategory", Terra_UnitsCategory_Undefined, Terra_UnitsCategory_Count, unitsCategoryToStringForTest, unitsCategoryFromStringForTest);
    pass &= checkEnumRange<Terra_RailType>("RailType", Terra_RailType_Undefined, Terra_RailType_Count, terraRailTypeToString, terraRailTypeFromString);
    pass &= checkEnumRange<Terra_Comparison>("Comparison", Terra_Comparison_LessThan, Terra_Comparison_NotEqual, terraComparisonToString, terraComparisonFromString);
    pass &= checkEnumRange<Terra_TriggerState>("TriggerState", Terra_TriggerState_Inactive, Terra_TriggerState_Fault, terraTriggerStateToString, terraTriggerStateFromString);
    pass &= checkEnumRange<Terra_LogLevel>("LogLevel", Terra_LogLevel_Debug, Terra_LogLevel_Error, terraLogLevelToString, terraLogLevelFromString);
    pass &= checkEnumRange<Terra_ModuleType>("ModuleType", Terra_ModuleType_Undefined, Terra_ModuleType_IOExpander, terraModuleTypeToString, terraModuleTypeFromString);
    pass &= checkEnumRange<Terra_ControlMode>("ControlMode", Terra_ControlMode_Manual, Terra_ControlMode_Disabled, terraControlModeToString, terraControlModeFromString);
    pass &= checkEnumRange<Terra_RouteState>("RouteState", Terra_RouteState_Idle, Terra_RouteState_Fault, terraRouteStateToString, terraRouteStateFromString);
    pass &= checkEnumRange<Terra_AttachmentRole>("AttachmentRole", Terra_AttachmentRole_Undefined, Terra_AttachmentRole_Circulator, terraAttachmentRoleToString, terraAttachmentRoleFromString);


    float converted = 0.0f;
    pass &= tryConvertUnits(32.0f, Terra_UnitsType_Temperature_Fahrenheit, &converted, Terra_UnitsType_Temperature_Celsius) && isFPEqual(converted, 0.0f);
    pass &= tryConvertUnits(1.0f, Terra_UnitsType_LiqVolume_Gallons, &converted, Terra_UnitsType_LiqVolume_Liters) && fabsf(converted - 3.78541f) <= 0.0001f;
    pass &= tryConvertUnits(1.0f, Terra_UnitsType_LiqFlowRate_GallonsPerMin, &converted, Terra_UnitsType_LiqFlowRate_LitersPerMin) && fabsf(converted - 3.78541f) <= 0.0001f;
    pass &= tryConvertUnits(101.325f, Terra_UnitsType_Pressure_Kilopascals, &converted, Terra_UnitsType_Pressure_Hectopascals) && fabsf(converted - 1013.25f) <= 0.001f;
    pass &= tryConvertUnits(25.4f, Terra_UnitsType_Distance_Millimeters, &converted, Terra_UnitsType_Distance_Inches) && fabsf(converted - 1.0f) <= 0.0001f;
    pass &= tryConvertUnits(25.4f, Terra_UnitsType_Speed_MillimetersPerHour, &converted, Terra_UnitsType_Speed_InchesPerHour) && fabsf(converted - 1.0f) <= 0.0001f;
    pass &= tryConvertUnits(1.0f, Terra_UnitsType_Speed_MetersPerSecond, &converted, Terra_UnitsType_Speed_MilesPerHour) && fabsf(converted - 2.23694f) <= 0.0001f;
    pass &= tryConvertUnits(1.0f, Terra_UnitsType_Current_Amperage, &converted, Terra_UnitsType_Power_Wattage, 12.0f) && isFPEqual(converted, 12.0f);
    pass &= defaultUnits(Terra_UnitsCategory_Irradiance, Terra_MeasurementMode_Metric) == Terra_UnitsType_Irradiance_WattsPerSquareMeter;

    Serial.println(pass ? F("PASS") : F("FAIL"));
}

void loop() { }
