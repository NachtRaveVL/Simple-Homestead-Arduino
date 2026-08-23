// Enum trie to CPP export script - mainly for dev purposes.
// Generates minimum-position decision trees for Terraduino's known enum vocabulary.

#include <Terraduino.h>
#include <ctype.h>
#include <string.h>

struct EnumValue {
    TerraString text;
    int typeIndex;
};

static void printSpacer(int level)
{
    for (int index = 0; index < (level << 2); ++index) { Serial.print(' '); }
}

static char charAt(const TerraString &text, int index)
{
    char value = index >= 0 && index < (int)text.length() ? text[index] : '\0';
    return value ? (char)tolower((unsigned char)value) : '\0';
}

static void printChar(char value)
{
    if (!value) { Serial.print(F("'\\0'")); }
    else if (value == '\\') { Serial.print(F("'\\\\'")); }
    else if (value == '\'') { Serial.print(F("'\\\''")); }
    else {
        Serial.print('\'');
        Serial.print(value);
        Serial.print('\'');
    }
}

static int selectPosition(EnumValue *values, int valueCount, bool *usedPositions, int maxLength)
{
    int bestPosition = -1;
    int bestLargestGroup = valueCount + 1;
    int bestGroupCount = 0;

    for (int position = 0; position <= maxLength; ++position) {
        if (usedPositions[position]) { continue; }

        char groups[64];
        uint8_t groupSizes[64];
        int groupCount = 0;
        memset(groupSizes, 0, sizeof(groupSizes));

        for (int valueIndex = 0; valueIndex < valueCount; ++valueIndex) {
            char value = charAt(values[valueIndex].text, position);
            int groupIndex = 0;
            while (groupIndex < groupCount && groups[groupIndex] != value) { ++groupIndex; }
            if (groupIndex == groupCount) {
                groups[groupCount] = value;
                ++groupCount;
            }
            ++groupSizes[groupIndex];
        }

        if (groupCount <= 1) { continue; }

        int largestGroup = 0;
        for (int groupIndex = 0; groupIndex < groupCount; ++groupIndex) {
            largestGroup = largestGroup > (int)groupSizes[groupIndex] ? largestGroup : (int)groupSizes[groupIndex];
        }

        if (largestGroup < bestLargestGroup ||
            (largestGroup == bestLargestGroup && groupCount > bestGroupCount)) {
            bestPosition = position;
            bestLargestGroup = largestGroup;
            bestGroupCount = groupCount;
        }
    }

    return bestPosition;
}

static void printTree(EnumValue *values, int valueCount, bool *usedPositions, int maxLength,
                      int level, const TerraString &varName, const TerraString &typeCast)
{
    if (valueCount <= 0) { return; }
    if (valueCount == 1) {
        printSpacer(level);
        Serial.print(F("return "));
        Serial.print(typeCast.c_str());
        Serial.print(values[0].typeIndex);
        Serial.println(';');
        return;
    }

    int position = selectPosition(values, valueCount, usedPositions, maxLength);
    if (position < 0) { return; }
    usedPositions[position] = true;

    char groups[64];
    int groupCount = 0;
    for (int valueIndex = 0; valueIndex < valueCount; ++valueIndex) {
        char value = charAt(values[valueIndex].text, position);
        int groupIndex = 0;
        while (groupIndex < groupCount && groups[groupIndex] != value) { ++groupIndex; }
        if (groupIndex == groupCount) { groups[groupCount++] = value; }
    }

    printSpacer(level);
    Serial.print(F("switch (terraTrieChar("));
    Serial.print(varName.c_str());
    Serial.print(F(", "));
    Serial.print(position);
    Serial.println(F(")) {"));

    for (int groupIndex = 0; groupIndex < groupCount; ++groupIndex) {
        char groupChar = groups[groupIndex];
        int subsetCount = 0;
        for (int valueIndex = 0; valueIndex < valueCount; ++valueIndex) {
            if (charAt(values[valueIndex].text, position) == groupChar) { ++subsetCount; }
        }

        EnumValue *subset = new EnumValue[subsetCount];
        int subsetIndex = 0;
        for (int valueIndex = 0; valueIndex < valueCount; ++valueIndex) {
            if (charAt(values[valueIndex].text, position) == groupChar) {
                subset[subsetIndex++] = values[valueIndex];
            }
        }

        printSpacer(level + 1);
        Serial.print(F("case "));
        printChar(groupChar);
        Serial.println(':');
        printTree(subset, subsetCount, usedPositions, maxLength, level + 2, varName, typeCast);
        delete [] subset;
    }

    printSpacer(level);
    Serial.println('}');
    usedPositions[position] = false;
}

template<class T>
static void buildEnumTree(int firstValue, int lastValue, T fallback,
                          TerraString (*toStringFn)(T), const char *varName, const char *typeCast)
{
    const int valueCount = lastValue - firstValue + 1;
    EnumValue *values = new EnumValue[valueCount];
    int maxLength = 0;

    for (int valueIndex = 0; valueIndex < valueCount; ++valueIndex) {
        const int typeIndex = firstValue + valueIndex;
        values[valueIndex].text = toStringFn((T)typeIndex);
        values[valueIndex].typeIndex = typeIndex;
        maxLength = maxLength > (int)values[valueIndex].text.length() ? maxLength : (int)values[valueIndex].text.length();
    }

    bool *usedPositions = new bool[maxLength + 1];
    memset(usedPositions, 0, sizeof(bool) * (maxLength + 1));

    Serial.print(typeCast);
    Serial.print(' ');
    Serial.print(F("decode(const "));
    Serial.print(F("TerraString &"));
    Serial.print(varName);
    Serial.println(F(") {"));
    printTree(values, valueCount, usedPositions, maxLength, 1, TerraString(varName), TerraString("(" ) + typeCast + ")");
    printSpacer(1);
    Serial.print(F("return ("));
    Serial.print(typeCast);
    Serial.print(F(")"));
    Serial.print((int)fallback);
    Serial.println(';');
    Serial.println('}');
    Serial.println();

    delete [] usedPositions;
    delete [] values;
}

void setup()
{
    Serial.begin(115200);
    while (!Serial) { ; }

    buildEnumTree<Terra_ObjectType>(Terra_ObjectType_Undefined, Terra_ObjectType_PowerRail, Terra_ObjectType_Undefined, terraObjectTypeToString, "objectTypeStr", "Terra_ObjectType");
    buildEnumTree<Terra_ReservoirType>(Terra_ReservoirType_Undefined, Terra_ReservoirType_Thermal, Terra_ReservoirType_Undefined, TerraReservoirTypeToString, "ReservoirTypeStr", "Terra_ReservoirType");
    buildEnumTree<Terra_ResourceState>(Terra_ResourceState_Unknown, Terra_ResourceState_Fault, Terra_ResourceState_Unknown, TerraReservoirStateToString, "resourceStateStr", "Terra_ResourceState");
    buildEnumTree<Terra_WaterSourceType>(Terra_WaterSourceType_Undefined, Terra_WaterSourceType_Stored, Terra_WaterSourceType_Undefined, terraWaterSourceTypeToString, "waterSourceTypeStr", "Terra_WaterSourceType");
    buildEnumTree<Terra_WaterStorageType>(Terra_WaterStorageType_Undefined, Terra_WaterStorageType_Reservoir, Terra_WaterStorageType_Undefined, terraWaterStorageTypeToString, "waterStorageTypeStr", "Terra_WaterStorageType");
    buildEnumTree<Terra_SensorType>(Terra_SensorType_Undefined, Terra_SensorType_Remote, Terra_SensorType_Undefined, terraSensorTypeToString, "sensorTypeStr", "Terra_SensorType");
    buildEnumTree<Terra_ActuatorType>(Terra_ActuatorType_Undefined, Terra_ActuatorType_SumpPump, Terra_ActuatorType_Undefined, terraActuatorTypeToString, "actuatorTypeStr", "Terra_ActuatorType");
    buildEnumTree<Terra_MeasurementMode>(Terra_MeasurementMode_Undefined, Terra_MeasurementMode_Count, Terra_MeasurementMode_Undefined, terraMeasurementModeToString, "measurementModeStr", "Terra_MeasurementMode");
    buildEnumTree<Terra_EnableMode>(Terra_EnableMode_Undefined, Terra_EnableMode_Count, Terra_EnableMode_Undefined, terraEnableModeToString, "enableModeStr", "Terra_EnableMode");
    buildEnumTree<Terra_PinMode>(Terra_PinMode_Undefined, Terra_PinMode_Count, Terra_PinMode_Undefined, terraPinModeToString, "pinModeStr", "Terra_PinMode");
    buildEnumTree<Terra_UnitsType>(Terra_UnitsType_Undefined, Terra_UnitsType_Amps, Terra_UnitsType_Undefined, terraUnitToString, "unitStr", "Terra_UnitsType");
    buildEnumTree<Terra_UnitsCategory>(Terra_UnitsCategory_Undefined, Terra_UnitsCategory_Count, Terra_UnitsCategory_Undefined, terraUnitsCategoryToString, "unitsCategoryStr", "Terra_UnitsCategory");
    buildEnumTree<Terra_RailType>(Terra_RailType_Undefined, Terra_RailType_Count, Terra_RailType_Undefined, terraRailTypeToString, "railTypeStr", "Terra_RailType");
    buildEnumTree<Terra_Comparison>(Terra_Comparison_LessThan, Terra_Comparison_NotEqual, Terra_Comparison_GreaterOrEqual, terraComparisonToString, "comparisonStr", "Terra_Comparison");
    buildEnumTree<Terra_TriggerState>(Terra_TriggerState_Inactive, Terra_TriggerState_Fault, Terra_TriggerState_Fault, terraTriggerStateToString, "triggerStateStr", "Terra_TriggerState");
    buildEnumTree<Terra_LogLevel>(Terra_LogLevel_Debug, Terra_LogLevel_Error, Terra_LogLevel_Info, terraLogLevelToString, "logLevelStr", "Terra_LogLevel");
    buildEnumTree<Terra_ModuleType>(Terra_ModuleType_Undefined, Terra_ModuleType_IOExpander, Terra_ModuleType_Undefined, terraModuleTypeToString, "moduleTypeStr", "Terra_ModuleType");
    buildEnumTree<Terra_ControlMode>(Terra_ControlMode_Manual, Terra_ControlMode_Disabled, Terra_ControlMode_Disabled, terraControlModeToString, "controlModeStr", "Terra_ControlMode");
    buildEnumTree<Terra_RouteState>(Terra_RouteState_Idle, Terra_RouteState_Fault, Terra_RouteState_Idle, terraRouteStateToString, "routeStateStr", "Terra_RouteState");
    buildEnumTree<Terra_AttachmentRole>(Terra_AttachmentRole_Undefined, Terra_AttachmentRole_Circulator, Terra_AttachmentRole_Undefined, terraAttachmentRoleToString, "attachmentRoleStr", "Terra_AttachmentRole");

    Serial.println(F("Done!"));
}

void loop() { ; }
