#include <Terraduino.h>

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
    pass &= checkEnumRange<Terra_UnitsType>("Unit", Terra_UnitsType_Undefined, Terra_UnitsType_Amps, terraUnitToString, unitsTypeFromSymbol);
    pass &= checkEnumRange<Terra_UnitsCategory>("UnitsCategory", Terra_UnitsCategory_Undefined, Terra_UnitsCategory_Count, terraUnitsCategoryToString, terraUnitsCategoryFromString);
    pass &= checkEnumRange<Terra_RailType>("RailType", Terra_RailType_Undefined, Terra_RailType_Count, terraRailTypeToString, terraRailTypeFromString);
    pass &= checkEnumRange<Terra_Comparison>("Comparison", Terra_Comparison_LessThan, Terra_Comparison_NotEqual, terraComparisonToString, terraComparisonFromString);
    pass &= checkEnumRange<Terra_TriggerState>("TriggerState", Terra_TriggerState_Inactive, Terra_TriggerState_Fault, terraTriggerStateToString, terraTriggerStateFromString);
    pass &= checkEnumRange<Terra_LogLevel>("LogLevel", Terra_LogLevel_Debug, Terra_LogLevel_Error, terraLogLevelToString, terraLogLevelFromString);
    pass &= checkEnumRange<Terra_ModuleType>("ModuleType", Terra_ModuleType_Undefined, Terra_ModuleType_IOExpander, terraModuleTypeToString, terraModuleTypeFromString);
    pass &= checkEnumRange<Terra_ControlMode>("ControlMode", Terra_ControlMode_Manual, Terra_ControlMode_Disabled, terraControlModeToString, terraControlModeFromString);
    pass &= checkEnumRange<Terra_RouteState>("RouteState", Terra_RouteState_Idle, Terra_RouteState_Fault, terraRouteStateToString, terraRouteStateFromString);
    pass &= checkEnumRange<Terra_AttachmentRole>("AttachmentRole", Terra_AttachmentRole_Undefined, Terra_AttachmentRole_Circulator, terraAttachmentRoleToString, terraAttachmentRoleFromString);


    Serial.println(pass ? F("PASS") : F("FAIL"));
}

void loop() { }
