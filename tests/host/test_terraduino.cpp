#include "Terraduino.h"
#include <cassert>
#include <cmath>
#include <iostream>

static int taskRuns = 0;
static void taskCallback(void *) { ++taskRuns; }
static float lastActuatorValue = -1.0f;
static void actuatorCallback(void *, float value) { lastActuatorValue = value; }
static int publishCount = 0;
static float publishedValue = 0.0f;
static Terra_Unit publishedUnit = Terra_Unit_Undefined;
static void publishCallback(void *, const char *, const TerraMeasurement &m) { ++publishCount; publishedValue = m.value; publishedUnit = m.unit; }
static float driverReadValue = 0.0f;
static float driverRead(void *) { return driverReadValue; }

static void test_core_logic() {
    assert(terraClassifyResourceState(5, 10, 25, 90) == Terra_ResourceState_Reserve);
    assert(terraClassifyResourceState(20, 10, 25, 90) == Terra_ResourceState_Low);
    assert(terraClassifyResourceState(50, 10, 25, 90) == Terra_ResourceState_Normal);
    assert(terraClassifyResourceState(95, 10, 25, 90) == Terra_ResourceState_High);
    assert(terraClassifyResourceState(50, 10, 25, 90, true) == Terra_ResourceState_Fault);

    TerraTransferDecision d = terraEvaluateWaterTransfer(true, 80, 10, 20, 40, 90);
    assert(d.shouldRun && d.state == Terra_RouteState_Requested);
    d = terraEvaluateWaterTransfer(true, 80, 10, 50, 40, 90, false, true);
    assert(d.shouldRun && d.state == Terra_RouteState_Active);
    d = terraEvaluateWaterTransfer(true, 80, 10, 90, 40, 90, false, true);
    assert(!d.shouldRun && d.state == Terra_RouteState_Complete);
    d = terraEvaluateWaterTransfer(true, 5, 10, 20, 40, 90);
    assert(!d.shouldRun);
    assert(terraThermalLoopShouldRun(70, 50, 8, 3, 80, false));
    assert(!terraThermalLoopShouldRun(52, 50, 8, 3, 80, false));
    assert(terraFlowFault(true, 0.1f, 1.0f));
    assert(terraFlowFault(false, 2.0f, 1.0f));

    const float requests[] = {0.25f, 0.75f, 0.5f};
    assert(isFPEqual(terraResolveActuatorRequests(requests, 3, Terra_EnableMode_Highest), 0.75f));
    assert(isFPEqual(terraResolveActuatorRequests(requests, 3, Terra_EnableMode_Lowest), 0.25f));
    assert(isFPEqual(terraResolveActuatorRequests(requests, 3, Terra_EnableMode_Average), 0.5f));
    assert(isFPEqual(terraResolveActuatorRequests(requests, 3, Terra_EnableMode_Multiply), 0.09375f));
    assert(isFPEqual(terraResolveActuatorRequests(requests, 3, Terra_EnableMode_InOrder), 0.25f));
    assert(isFPEqual(terraResolveActuatorRequests(requests, 3, Terra_EnableMode_RevOrder), 0.5f));
}

static TerraString lowerCopy(const TerraString &text) {
    TerraString value(text);
    for (size_t i = 0; i < value.size(); ++i) value[i] = (char)tolower((unsigned char)value[i]);
    return value;
}

static void test_measurements_and_strings() {
    assert(isFPEqual(terraConvertUnits(0, Terra_Unit_Celsius, Terra_Unit_Fahrenheit), 32));
    assert(isFPEqual(terraConvertUnits(10, Terra_Unit_Liters, Terra_Unit_GallonsUS), 2.64172f));
    assert(isFPEqual(terraConvertUnits(100, Terra_Unit_Kilopascals, Terra_Unit_Hectopascals), 1000));
    assert(isFPEqual(terraConvertUnits(1, Terra_Unit_Inches, Terra_Unit_Millimeters), 25.4f));
    assert(isFPEqual(terraConvertUnits(10, Terra_Unit_MetersPerSecond, Terra_Unit_KilometersPerHour), 36));
    assert(std::isnan(terraConvertUnits(1, Terra_Unit_Watts, Terra_Unit_Liters)));

    for (int i = Terra_ObjectType_Undefined; i <= Terra_ObjectType_PowerRail; ++i) {
        Terra_ObjectType v = (Terra_ObjectType)i;
        assert(terraObjectTypeFromString(terraObjectTypeToString(v)) == v);
        assert(terraObjectTypeFromString(lowerCopy(terraObjectTypeToString(v))) == v);
    }
    for (int i = Terra_ResourceType_Undefined; i <= Terra_ResourceType_Thermal; ++i) {
        Terra_ResourceType v = (Terra_ResourceType)i;
        assert(terraResourceTypeFromString(terraResourceTypeToString(v)) == v);
    }
    for (int i = Terra_ResourceState_Unknown; i <= Terra_ResourceState_Fault; ++i) {
        Terra_ResourceState v = (Terra_ResourceState)i;
        assert(terraResourceStateFromString(terraResourceStateToString(v)) == v);
    }
    for (int i = Terra_WaterSourceType_Undefined; i <= Terra_WaterSourceType_Stored; ++i) {
        Terra_WaterSourceType v = (Terra_WaterSourceType)i;
        assert(terraWaterSourceTypeFromString(terraWaterSourceTypeToString(v)) == v);
    }
    for (int i = Terra_WaterStorageType_Undefined; i <= Terra_WaterStorageType_Reservoir; ++i) {
        Terra_WaterStorageType v = (Terra_WaterStorageType)i;
        assert(terraWaterStorageTypeFromString(terraWaterStorageTypeToString(v)) == v);
    }
    for (int i = Terra_SensorType_Undefined; i <= Terra_SensorType_Remote; ++i) {
        Terra_SensorType v = (Terra_SensorType)i;
        assert(terraSensorTypeFromString(terraSensorTypeToString(v)) == v);
    }
    for (int i = Terra_ActuatorType_Undefined; i <= Terra_ActuatorType_SumpPump; ++i) {
        Terra_ActuatorType v = (Terra_ActuatorType)i;
        assert(terraActuatorTypeFromString(terraActuatorTypeToString(v)) == v);
    }
    for (int i = Terra_MeasurementMode_Undefined; i <= Terra_MeasurementMode_Count; ++i) {
        Terra_MeasurementMode v = (Terra_MeasurementMode)i;
        assert(terraMeasurementModeFromString(terraMeasurementModeToString(v)) == v);
    }
    for (int i = Terra_EnableMode_Undefined; i <= Terra_EnableMode_Count; ++i) {
        Terra_EnableMode v = (Terra_EnableMode)i;
        assert(terraEnableModeFromString(terraEnableModeToString(v)) == v);
    }
    for (int i = Terra_PinMode_Undefined; i <= Terra_PinMode_Count; ++i) {
        Terra_PinMode v = (Terra_PinMode)i;
        assert(terraPinModeFromString(terraPinModeToString(v)) == v);
    }
    for (int i = Terra_Unit_Undefined; i <= Terra_Unit_Amps; ++i) {
        Terra_Unit v = (Terra_Unit)i;
        assert(terraUnitFromString(terraUnitToString(v)) == v);
        assert(terraUnitFromString(lowerCopy(terraUnitToString(v))) == v);
    }
    for (int i = Terra_UnitsCategory_Undefined; i <= Terra_UnitsCategory_Count; ++i) {
        Terra_UnitsCategory v = (Terra_UnitsCategory)i;
        assert(terraUnitsCategoryFromString(terraUnitsCategoryToString(v)) == v);
    }
    for (int i = Terra_RailType_Undefined; i <= Terra_RailType_Count; ++i) {
        Terra_RailType v = (Terra_RailType)i;
        assert(terraRailTypeFromString(terraRailTypeToString(v)) == v);
    }
    for (int i = Terra_Comparison_LessThan; i <= Terra_Comparison_NotEqual; ++i) {
        Terra_Comparison v = (Terra_Comparison)i;
        assert(terraComparisonFromString(terraComparisonToString(v)) == v);
    }
    for (int i = Terra_TriggerState_Inactive; i <= Terra_TriggerState_Fault; ++i) {
        Terra_TriggerState v = (Terra_TriggerState)i;
        assert(terraTriggerStateFromString(terraTriggerStateToString(v)) == v);
    }
    for (int i = Terra_LogLevel_Debug; i <= Terra_LogLevel_Error; ++i) {
        Terra_LogLevel v = (Terra_LogLevel)i;
        assert(terraLogLevelFromString(terraLogLevelToString(v)) == v);
    }
    for (int i = Terra_ModuleType_Undefined; i <= Terra_ModuleType_IOExpander; ++i) {
        Terra_ModuleType v = (Terra_ModuleType)i;
        assert(terraModuleTypeFromString(terraModuleTypeToString(v)) == v);
    }
    for (int i = Terra_ControlMode_Manual; i <= Terra_ControlMode_Disabled; ++i) {
        Terra_ControlMode v = (Terra_ControlMode)i;
        assert(terraControlModeFromString(terraControlModeToString(v)) == v);
    }
    for (int i = Terra_RouteState_Idle; i <= Terra_RouteState_Fault; ++i) {
        Terra_RouteState v = (Terra_RouteState)i;
        assert(terraRouteStateFromString(terraRouteStateToString(v)) == v);
    }
    for (int i = Terra_AttachmentRole_Undefined; i <= Terra_AttachmentRole_Circulator; ++i) {
        Terra_AttachmentRole v = (Terra_AttachmentRole)i;
        assert(terraAttachmentRoleFromString(terraAttachmentRoleToString(v)) == v);
    }

}
static void test_resource_water() {
    TerraResource r(Terra_ResourceType_Water, 1, "water");
    assert(r.setThresholds(10, 30, 90));
    assert(!r.setThresholds(40, 20, 90));
    r.setLevel(5); assert(r.getState() == Terra_ResourceState_Reserve);
    r.setLevel(25); assert(r.getState() == Terra_ResourceState_Low);
    r.setLevel(50); assert(r.getState() == Terra_ResourceState_Normal);
    r.setLevel(95); assert(r.getState() == Terra_ResourceState_High);
    r.setFault("fault"); assert(r.getState() == Terra_ResourceState_Fault);
    r.clearFault(); assert(r.getState() == Terra_ResourceState_High);

    TerraCistern cistern(1000, 2, "cistern");
    cistern.setThresholds(20, 35, 90);
    assert(cistern.configureFillBand(40, 90, 99));
    assert(!cistern.configureFillBand(40, 99, 99));
    cistern.setStoredLiters(500);
    assert(isFPEqual(cistern.getLevel(), 50));
    assert(isFPEqual(cistern.availableAboveReserveLiters(), 300));
    assert(isFPEqual(cistern.safeFillCapacityLiters(), 490));

    TerraWaterSource source(Terra_WaterSourceType_Rainwater, 1, 3, "rain");
    source.setLevel(80); source.setReserveLevel(10);
    TerraWaterRoute route(4, "fill");
    route.configure(source.getKey(), cistern.getKey(), 40, 90);
    route.setMinimumFlow(1.0f); route.setMaximumFlow(20.0f);
    cistern.setLevel(35);
    TerraTransferDecision d = route.evaluate(source, cistern);
    assert(d.shouldRun && d.state == Terra_RouteState_Requested);
    cistern.setLevel(60);
    d = route.evaluate(source, cistern);
    assert(d.shouldRun && d.state == Terra_RouteState_Active);
    cistern.setLevel(91);
    d = route.evaluate(source, cistern);
    assert(!d.shouldRun && d.state == Terra_RouteState_Complete);
    assert(!route.validateFlow(0.2f, true));
    assert(route.hasFault());
    assert(route.validateFlow(3.0f, true));
    assert(!route.hasFault());

    TerraWaterSource well(Terra_WaterSourceType_Well, 1, 5, "well");
    TerraWaterSource municipal(Terra_WaterSourceType_Municipal, 2, 6, "municipal");
    source.setPriority(0);
    const TerraWaterSource *sources[] = { &municipal, &well, &source };
    TerraWaterBalancer waterBalancer;
    assert(waterBalancer.selectSource(sources, 3) == &source);
    source.setLevel(source.getReserveLevel());
    assert(waterBalancer.selectSource(sources, 3) == &well);
    well.setAvailable(false);
    assert(waterBalancer.selectSource(sources, 3) == &municipal);

    TerraFirstFlushController flush(10);
    assert(flush.shouldDivert()); flush.recordFlow(4); assert(isFPEqual(flush.getRemainingLiters(), 6));
    flush.recordFlow(6); assert(!flush.shouldDivert());

    TerraCistern rainCistern(1000, 7, "rain cistern");
    rainCistern.setThresholds(10, 25, 90);
    assert(rainCistern.configureFillBand(35, 90, 99));
    rainCistern.setStoredLiters(980);
    assert(isFPEqual(rainCistern.receiveWater(50), 10));
    assert(isFPEqual(rainCistern.getStoredLiters(), 990));
    assert(isFPEqual(rainCistern.getOverflowLiters(), 40));

    rainCistern.setStoredLiters(500);
    rainCistern.resetFlowTotals();
    TerraRainCatchment catchment(100.0f, 0.8f, 8, "roof");
    TerraFirstFlushController rainFlush(10.0f);
    TerraRainCollectionResult rain = catchment.collectInto(rainCistern, 1.0f, &rainFlush);
    assert(isFPEqual(rain.capturedLiters, 80.0f));
    assert(isFPEqual(rain.discardedLiters, 10.0f));
    assert(isFPEqual(rain.storedLiters, 70.0f));
    assert(isFPEqual(rain.overflowLiters, 0.0f));
    assert(!rainFlush.shouldDivert());
}

static void test_thermal_environment() {
    TerraThermalStore store(10, "thermal");
    assert(store.setTargetRange(45, 65));
    assert(!store.setTargetRange(70, 60));
    store.setAbsoluteMaximum(90);
    assert(!store.setTargetRange(45, 95));
    assert(isFPEqual(store.getAbsoluteMaximum(), 90));
    store.setTemperature(40); assert(store.needsHeat());
    store.setTemperature(70); assert(store.isOverTemperature());
    store.setTemperature(90); assert(store.isSafetyLimitExceeded());

    TerraThermalLoop loop(11, "loop");
    assert(loop.configure(8, 3, 80));
    assert(!loop.configure(2, 3, 80));
    TerraThermalBalancer balancer;
    assert(balancer.evaluate(loop, 70, 50));
    assert(loop.isRunning());
    assert(balancer.evaluate(loop, 54, 50));
    assert(!balancer.evaluate(loop, 52, 50));

    TerraEnvironment env(12, "outside");
    assert(std::isnan(env.dewPointC()));
    env.setAirTemperature(-2, 100);
    assert(env.hasField(TERRA_WEATHER_AIR_TEMPERATURE));
    assert(!env.hasField(TERRA_WEATHER_HUMIDITY));
    assert(std::isnan(env.dewPointC()));
    env.setRelativeHumidity(80, 100);
    env.setWind(5, -450, 100);
    assert(env.isFreezing());
    assert(isFPEqual(env.getWindDirection(), 270));
    assert(!std::isnan(env.dewPointC()));
}

static void test_sensors_actuators_triggers() {
    TerraRemoteSensor remote(Terra_SensorType_Temperature, Terra_Unit_Celsius, 20, "barn");
    remote.setStaleAfter(1000);
    remote.receiveReport(21.5f, Terra_Unit_Celsius, 100, true);
    assert(remote.isOnline(500));
    assert(!remote.isOnline(1200));
    remote.update(1200); assert(remote.hasFault());
    assert(!remote.getMeasurement().valid);
    remote.receiveReport(22, Terra_Unit_Celsius, 1300, true); assert(!remote.hasFault());
    remote.receiveReport(0, Terra_Unit_Celsius, 1400, false); assert(remote.hasFault());

    TerraPump pump(21, "pump");
    pump.setWriteCallback(actuatorCallback);
    pump.setMaxContinuousRuntime(1000);
    pump.setOutput(1, 0, 100);
    assert(pump.isActive() && isFPEqual(lastActuatorValue, 1));
    pump.setOutput(1, 0, 900); // Repeated command must not reset continuous-runtime accounting.
    pump.update(1101); assert(!pump.isActive() && pump.hasFault() && isFPEqual(lastActuatorValue, 0));
    pump.clearFault();
    pump.setOutput(1, 0, 1200);
    pump.setEnabled(false);
    assert(!pump.isActive() && isFPEqual(lastActuatorValue, 0));

    TerraCallbackInputDriver inputDriver(driverRead, nullptr, Terra_Unit_Celsius);
    TerraTemperatureSensor localTemperature(22, "local-temp");
    localTemperature.setUpdateInterval(1);
    localTemperature.setDriver(&inputDriver);
    driverReadValue = 18.25f;
    localTemperature.update(10);
    assert(localTemperature.getMeasurement().valid && isFPEqual(localTemperature.getMeasurement().value, 18.25f));

    TerraCallbackOutputDriver outputDriver(actuatorCallback);
    TerraValve valve(23, "valve");
    valve.setDriver(&outputDriver);
    valve.open();
    assert(valve.isActive() && isFPEqual(lastActuatorValue, 1.0f));
    valve.close();
    assert(!valve.isActive() && isFPEqual(lastActuatorValue, 0.0f));

    TerraAnalogSensor mapped(Terra_Unit_Percent, 24, "mapped");
    mapped.setReadCallback(driverRead);
    assert(mapped.setCalibration(0, 1000, 0, 100));
    mapped.setUpdateInterval(1);
    driverReadValue = 250;
    mapped.update(10);
    assert(isFPEqual(mapped.getMeasurement().value, 25.0f));

    TerraThresholdTrigger t(10, Terra_Comparison_GreaterOrEqual, 2);
    assert(t.evaluate(TerraMeasurement(11, Terra_Unit_Raw, 0, true)) == Terra_TriggerState_Active);
    assert(t.evaluate(TerraMeasurement(9, Terra_Unit_Raw, 0, true)) == Terra_TriggerState_Active);
    assert(t.evaluate(TerraMeasurement(7, Terra_Unit_Raw, 0, true)) == Terra_TriggerState_Inactive);
    assert(t.evaluate(TerraMeasurement(NAN, Terra_Unit_Raw, 0, false)) == Terra_TriggerState_Fault);

    TerraSumpPump sump(26, "basement sump");
    lastActuatorValue = -1.0f;
    sump.setWriteCallback(actuatorCallback);
    sump.setMaxContinuousRuntime(1000);
    assert(sump.configureLevels(70.0f, 20.0f, 90.0f));
    assert(!sump.configureLevels(20.0f, 70.0f, 90.0f));
    assert(sump.updateLevel(75.0f, true, 100));
    assert(sump.isActive() && isFPEqual(lastActuatorValue, 1.0f));
    assert(sump.updateLevel(50.0f, true, 500));
    assert(!sump.updateLevel(15.0f, true, 600));
    assert(!sump.isActive() && isFPEqual(lastActuatorValue, 0.0f));
    assert(sump.updateLevel(95.0f, true, 700));
    assert(sump.hasHighWaterAlarm());
    assert(!sump.updateLevel(95.0f, false, 800));
    assert(sump.hasFault() && !sump.hasValidLevel() && !sump.isActive());
    assert(sump.updateLevel(75.0f, true, 900));
    assert(!sump.hasFault() && sump.hasValidLevel());
    sump.update(2000);
    assert(sump.hasFault() && !sump.isActive());
}

static void test_data_roundtrips() {
    TerraSystemData system;
    system.setup.systemName = "Cabin \"North\"";
    system.setup.timeZoneHours = -7;
    system.setup.timeZoneMinutes = 30;
    system.setup.controlMode = Terra_ControlMode_Automatic;
    system.setup.measurementMode = Terra_MeasurementMode_Scientific;
    system.setup.updateIntervalMs = 500;
    system.setup.loggerMinimumLevel = Terra_LogLevel_Warning;
    system.setup.publisherIntervalMs = 15000;
    system.sequence = 42;
    TerraSystemData parsedSystem;
    assert(parsedSystem.fromJSON(system.toJSON()));
    assert(parsedSystem.setup.systemName == system.setup.systemName);
    assert(parsedSystem.setup.timeZoneHours == -7);
    assert(parsedSystem.setup.timeZoneMinutes == 30);
    assert(parsedSystem.setup.measurementMode == Terra_MeasurementMode_Scientific);
    assert(parsedSystem.setup.loggerMinimumLevel == Terra_LogLevel_Warning);
    assert(parsedSystem.setup.publisherIntervalMs == 15000);
    assert(parsedSystem.sequence == 42);

    uint8_t binary[128] = {0};
    size_t binaryLength = system.toBinary(binary, sizeof(binary));
    assert(binaryLength == system.binarySize());
    TerraSystemData binarySystem;
    assert(binarySystem.fromBinary(binary, binaryLength));
    assert(binarySystem.setup.systemName == system.setup.systemName);
    assert(binarySystem.setup.timeZoneHours == system.setup.timeZoneHours);
    assert(binarySystem.setup.timeZoneMinutes == system.setup.timeZoneMinutes);
    assert(binarySystem.setup.controlMode == system.setup.controlMode);
    assert(binarySystem.setup.measurementMode == system.setup.measurementMode);
    assert(binarySystem.setup.updateIntervalMs == system.setup.updateIntervalMs);
    assert(binarySystem.setup.loggerMinimumLevel == system.setup.loggerMinimumLevel);
    assert(binarySystem.setup.publisherIntervalMs == system.setup.publisherIntervalMs);
    assert(binarySystem.sequence == system.sequence);
    TerraSystemData invalidJSONSystem;
    assert(!invalidJSONSystem.fromJSON("{\"systemName\":\"bad\",\"timeZoneHours\":0,\"timeZoneMinutes\":0,\"controlMode\":\"automagic\",\"updateIntervalMs\":250,\"sequence\":0}"));
    assert(!invalidJSONSystem.fromJSON("{\"systemName\":\"bad\",\"timeZoneHours\":0junk,\"timeZoneMinutes\":0,\"controlMode\":\"Automatic\",\"updateIntervalMs\":250,\"sequence\":0}"));
    binary[10] ^= 0x01;
    TerraSystemData corruptSystem;
    assert(!corruptSystem.fromBinary(binary, binaryLength));
    binary[10] ^= 0x01;

    TerraCisternData storage;
    storage.key = 101;
    storage.name = "Main Cistern";
    storage.enabled = true;
    storage.level = 64.5f;
    storage.reserveLevel = 15;
    storage.lowLevel = 30;
    storage.highLevel = 95;
    storage.capacityLiters = 5000;
    storage.fillStartPercent = 35;
    storage.fillStopPercent = 90;
    storage.overflowPercent = 99;
    TerraCisternData storage2;
    assert(storage2.fromJSON(storage.toJSON()));
    assert(storage2.key == 101 && storage2.objectType == Terra_ObjectType_WaterStorage);
    assert(storage2.resourceType == Terra_ResourceType_Water);
    assert(storage2.storageType == Terra_WaterStorageType_Cistern);
    assert(isFPEqual(storage2.level, 64.5f) && isFPEqual(storage2.capacityLiters, 5000));
    assert(isFPEqual(storage2.fillStartPercent, 35) && isFPEqual(storage2.overflowPercent, 99));

    TerraWaterSourceData source;
    source.key = 102;
    source.name = "Well";
    source.sourceType = Terra_WaterSourceType_Well;
    source.priority = 1;
    source.level = 72;
    source.reserveLevel = 10;
    source.maximumFlowLpm = 12.5f;
    TerraWaterSourceData source2;
    assert(source2.fromJSON(source.toJSON()));
    assert(source2.sourceType == Terra_WaterSourceType_Well && source2.priority == 1);
    assert(isFPEqual(source2.maximumFlowLpm, 12.5f));

    TerraWaterRouteData route;
    route.key = 103;
    route.name = "Fill";
    route.sourceKey = 102;
    route.destinationKey = 101;
    route.destinationStartPercent = 30;
    route.destinationStopPercent = 95;
    route.minimumFlowLpm = 1;
    route.maximumFlowLpm = 20;
    route.routeState = Terra_RouteState_Active;
    TerraWaterRouteData route2;
    assert(route2.fromJSON(route.toJSON()));
    assert(route2.sourceKey == 102 && route2.destinationKey == 101);
    assert(route2.routeState == Terra_RouteState_Active);
    TerraWaterRouteData invalidRoute;
    assert(!invalidRoute.fromJSON("{\"key\":103,\"objectType\":\"WaterRoute\",\"name\":\"bad\",\"enabled\":true,\"sourceKey\":1,\"destinationKey\":2,\"destinationStartPercent\":30,\"destinationStopPercent\":95,\"minimumFlowLpm\":10,\"maximumFlowLpm\":5,\"routeState\":\"Active\"}"));

    TerraThermalStoreData thermal;
    thermal.key = 104;
    thermal.name = "Heat Store";
    thermal.level = 70;
    thermal.temperatureC = 58;
    thermal.minimumTargetC = 45;
    thermal.maximumTargetC = 65;
    thermal.absoluteMaximumC = 90;
    TerraThermalStoreData thermal2;
    assert(thermal2.fromJSON(thermal.toJSON()));
    assert(thermal2.objectType == Terra_ObjectType_ThermalStore);
    assert(isFPEqual(thermal2.temperatureC, 58) && isFPEqual(thermal2.absoluteMaximumC, 90));

    TerraSensorData remoteData;
    remoteData.key = 106;
    remoteData.name = "Barn Remote";
    remoteData.sensorType = Terra_SensorType_Remote;
    remoteData.reportedType = Terra_SensorType_Temperature;
    remoteData.unit = Terra_Unit_Celsius;
    remoteData.updateIntervalMs = 5000;
    remoteData.staleAfterMs = 30000;
    TerraSensorData remoteData2;
    assert(remoteData2.fromJSON(remoteData.toJSON()));
    assert(remoteData2.sensorType == Terra_SensorType_Remote);
    assert(remoteData2.reportedType == Terra_SensorType_Temperature);
    assert(remoteData2.unit == Terra_Unit_Celsius);
    assert(remoteData2.staleAfterMs == 30000);

    TerraActuatorData pumpData;
    pumpData.key = 107;
    pumpData.name = "Transfer Pump";
    pumpData.actuatorType = Terra_ActuatorType_Pump;
    pumpData.enableMode = Terra_EnableMode_Average;
    pumpData.maxContinuousMs = 120000;
    TerraActuatorData pumpData2;
    assert(pumpData2.fromJSON(pumpData.toJSON()));
    assert(pumpData2.actuatorType == Terra_ActuatorType_Pump);
    assert(pumpData2.enableMode == Terra_EnableMode_Average);
    assert(pumpData2.maxContinuousMs == 120000);

    TerraActuatorData sumpData;
    sumpData.key = 110;
    sumpData.name = "Basement Sump";
    sumpData.actuatorType = Terra_ActuatorType_SumpPump;
    sumpData.maxContinuousMs = 45000;
    sumpData.sumpStartPercent = 72.0f;
    sumpData.sumpStopPercent = 18.0f;
    sumpData.sumpAlarmPercent = 93.0f;
    TerraActuatorData sumpData2;
    assert(sumpData2.fromJSON(sumpData.toJSON()));
    assert(sumpData2.actuatorType == Terra_ActuatorType_SumpPump);
    assert(isFPEqual(sumpData2.sumpStartPercent, 72.0f));
    assert(isFPEqual(sumpData2.sumpStopPercent, 18.0f));
    assert(isFPEqual(sumpData2.sumpAlarmPercent, 93.0f));

    TerraThermalLoopData loopData;
    loopData.key = 108;
    loopData.name = "Solar Loop";
    loopData.onDifferentialC = 9;
    loopData.offDifferentialC = 4;
    loopData.maxStoreTempC = 82;
    TerraThermalLoopData loopData2;
    assert(loopData2.fromJSON(loopData.toJSON()));
    assert(isFPEqual(loopData2.onDifferentialC, 9));
    assert(isFPEqual(loopData2.offDifferentialC, 4));

    TerraPowerRailData railData;
    railData.key = 109;
    railData.name = "12V Controls";
    railData.railType = Terra_RailType_DC12V;
    railData.nominalVoltage = 12;
    TerraPowerRailData railData2;
    assert(railData2.fromJSON(railData.toJSON()));
    assert(railData2.railType == Terra_RailType_DC12V);
    assert(isFPEqual(railData2.nominalVoltage, 12));

    TerraEnvironmentData environment;
    environment.key = 105;
    environment.name = "Outside";
    environment.weather.airTemperatureC = -2.5f;
    environment.weather.relativeHumidity = 85;
    environment.weather.windDirectionDeg = 275;
    environment.weather.timestamp = 1234;
    environment.weather.validFields = TERRA_WEATHER_AIR_TEMPERATURE | TERRA_WEATHER_HUMIDITY | TERRA_WEATHER_WIND_DIRECTION;
    environment.weather.valid = true;
    TerraEnvironmentData environment2;
    assert(environment2.fromJSON(environment.toJSON()));
    assert(environment2.weather.valid);
    assert(environment2.weather.validFields == environment.weather.validFields);
    assert(isFPEqual(environment2.weather.airTemperatureC, -2.5f));
}

static void test_factory_persistence_reconstruction() {
    TerraRemoteSensor remote(Terra_SensorType_Temperature, Terra_Unit_Celsius, 200, "remote-temp");
    remote.setUpdateInterval(5000);
    remote.setStaleAfter(30000);
    TerraObjectData *remoteData = TerraFactory::newDataFromObject(&remote);
    assert(remoteData != nullptr);
    TerraObject *remoteCopyObject = TerraFactory::newObjectFromData(remoteData);
    assert(remoteCopyObject != nullptr && remoteCopyObject->getObjectType() == Terra_ObjectType_Sensor);
    TerraRemoteSensor *remoteCopy = static_cast<TerraRemoteSensor *>(remoteCopyObject);
    assert(remoteCopy->getSensorType() == Terra_SensorType_Remote);
    assert(remoteCopy->getReportedType() == Terra_SensorType_Temperature);
    assert(remoteCopy->getStaleAfter() == 30000);
    assert(remoteCopy->getUpdateInterval() == 5000);
    delete remoteCopyObject;
    delete remoteData;

    TerraAnalogSensor analog(Terra_Unit_Percent, 206, "tank-level-raw");
    TerraAnalogInputDriver analogDriver(7, Terra_Unit_Raw);
    assert(analogDriver.setCalibration(100.0f, 900.0f, 0.0f, 100.0f));
    analog.setDriver(&analogDriver);
    assert(analog.setCalibration(0.0f, 100.0f, 5.0f, 95.0f));
    TerraObjectData *analogDataBase = TerraFactory::newDataFromObject(&analog);
    TerraSensorData *analogData = static_cast<TerraSensorData *>(analogDataBase);
    assert(analogData && analogData->hasPinDriver);
    assert(analogData->pinSetup.pin == 7 && analogData->pinSetup.mode == Terra_PinMode_Analog_Input);
    assert(analogData->driverCalibrated && analogData->sensorCalibrated);
    TerraSensorData parsedAnalogData;
    assert(parsedAnalogData.fromJSON(analogData->toJSON()));
    TerraObject *analogCopyObject = TerraFactory::newObjectFromData(&parsedAnalogData);
    TerraAnalogSensor *analogCopy = static_cast<TerraAnalogSensor *>(analogCopyObject);
    assert(analogCopy && analogCopy->getDriver());
    TerraPinSetup analogPin;
    assert(analogCopy->getDriver()->getPinSetup(analogPin));
    assert(analogPin.pin == 7 && analogPin.mode == Terra_PinMode_Analog_Input);
    float rawMin = 0.0f, rawMax = 0.0f, valMin = 0.0f, valMax = 0.0f;
    assert(analogCopy->getDriver()->getCalibration(rawMin, rawMax, valMin, valMax));
    assert(isFPEqual(rawMin, 100.0f) && isFPEqual(rawMax, 900.0f));
    assert(analogCopy->getCalibration(rawMin, rawMax, valMin, valMax));
    assert(isFPEqual(valMin, 5.0f) && isFPEqual(valMax, 95.0f));
    delete analogCopyObject;
    delete analogDataBase;

    TerraWaterSource disabledWell(Terra_WaterSourceType_Well, 2, 207, "backup-well");
    disabledWell.setAvailable(true);
    disabledWell.setEnabled(false);
    TerraObjectData *wellDataBase = TerraFactory::newDataFromObject(&disabledWell);
    TerraWaterSourceData *wellData = static_cast<TerraWaterSourceData *>(wellDataBase);
    assert(wellData && wellData->available);
    TerraObject *wellCopyObject = TerraFactory::newObjectFromData(wellData);
    TerraWaterSource *wellCopy = static_cast<TerraWaterSource *>(wellCopyObject);
    assert(wellCopy && !wellCopy->isEnabled() && wellCopy->isConfiguredAvailable());
    delete wellCopyObject;
    delete wellDataBase;

    TerraCistern cistern(4200, 201, "main-cistern");
    cistern.setThresholds(12, 25, 92);
    cistern.configureFillBand(32, 88, 98);
    cistern.setLevel(61);
    assert(cistern.attachments().attach(501, Terra_AttachmentRole_LevelSensor));
    assert(cistern.attachments().attach(502, Terra_AttachmentRole_Pump));
    TerraObjectData *cisternData = TerraFactory::newDataFromObject(&cistern);
    assert(cisternData != nullptr);
    TerraString cisternJson = cisternData->toJSON();
    TerraCisternData parsedCisternData;
    assert(parsedCisternData.fromJSON(cisternJson));
    TerraObject *cisternCopyObject = TerraFactory::newObjectFromData(&parsedCisternData);
    TerraCistern *cisternCopy = static_cast<TerraCistern *>(cisternCopyObject);
    assert(cisternCopy && cisternCopy->getStorageType() == Terra_WaterStorageType_Cistern);
    assert(isFPEqual(cisternCopy->getCapacityLiters(), 4200));
    assert(isFPEqual(cisternCopy->getLevel(), 61));
    assert(isFPEqual(cisternCopy->getFillStartPercent(), 32));
    assert(isFPEqual(cisternCopy->getFillStopPercent(), 88));
    assert(isFPEqual(cisternCopy->getOverflowPercent(), 98));
    assert(cisternCopy->attachments().size() == 2);
    assert(cisternCopy->attachments().find(Terra_AttachmentRole_LevelSensor) == 501);
    assert(cisternCopy->attachments().find(Terra_AttachmentRole_Pump) == 502);
    delete cisternCopyObject;
    delete cisternData;

    TerraPump pump(202, "transfer-pump");
    TerraDigitalOutputDriver pumpDriver(TerraPinSetup(9, Terra_PinMode_Digital_Output, true));
    pump.setDriver(&pumpDriver);
    pump.setEnableMode(Terra_EnableMode_Lowest);
    pump.setMaxContinuousRuntime(45000);
    TerraObjectData *pumpData = TerraFactory::newDataFromObject(&pump);
    assert(pumpData != nullptr);
    TerraObject *pumpCopyObject = TerraFactory::newObjectFromData(pumpData);
    TerraPump *pumpCopy = static_cast<TerraPump *>(pumpCopyObject);
    assert(pumpCopy && pumpCopy->getActuatorType() == Terra_ActuatorType_Pump);
    assert(pumpCopy->getEnableMode() == Terra_EnableMode_Lowest);
    assert(pumpCopy->getMaxContinuousRuntime() == 45000);
    assert(pumpCopy->getDriver());
    TerraPinSetup pumpPin;
    assert(pumpCopy->getDriver()->getPinSetup(pumpPin));
    assert(pumpPin.pin == 9 && pumpPin.mode == Terra_PinMode_Digital_Output && pumpPin.activeLow);
    delete pumpCopyObject;
    delete pumpData;

    TerraSumpPump sumpPersist(208, "sump-persist");
    sumpPersist.setMaxContinuousRuntime(45000);
    assert(sumpPersist.configureLevels(72.0f, 18.0f, 93.0f));
    TerraObjectData *sumpDataBase = TerraFactory::newDataFromObject(&sumpPersist);
    TerraActuatorData *sumpData = static_cast<TerraActuatorData *>(sumpDataBase);
    assert(sumpData && sumpData->actuatorType == Terra_ActuatorType_SumpPump);
    assert(isFPEqual(sumpData->sumpStartPercent, 72.0f));
    TerraString sumpJson = sumpData->toJSON();
    TerraActuatorData parsedSumpData;
    assert(parsedSumpData.fromJSON(sumpJson));
    TerraObject *sumpCopyObject = TerraFactory::newObjectFromData(&parsedSumpData);
    TerraSumpPump *sumpCopy = static_cast<TerraSumpPump *>(sumpCopyObject);
    assert(sumpCopy && sumpCopy->getActuatorType() == Terra_ActuatorType_SumpPump);
    assert(isFPEqual(sumpCopy->getStartLevelPercent(), 72.0f));
    assert(isFPEqual(sumpCopy->getStopLevelPercent(), 18.0f));
    assert(isFPEqual(sumpCopy->getAlarmLevelPercent(), 93.0f));
    assert(sumpCopy->getMaxContinuousRuntime() == 45000);
    delete sumpCopyObject;
    delete sumpDataBase;

    TerraThermalLoop loop(203, "thermal-loop");
    assert(loop.configure(10, 4, 84));
    TerraObjectData *loopData = TerraFactory::newDataFromObject(&loop);
    assert(loopData != nullptr);
    TerraObject *loopCopyObject = TerraFactory::newObjectFromData(loopData);
    TerraThermalLoop *loopCopy = static_cast<TerraThermalLoop *>(loopCopyObject);
    assert(loopCopy);
    assert(isFPEqual(loopCopy->getOnDifferential(), 10));
    assert(isFPEqual(loopCopy->getOffDifferential(), 4));
    assert(isFPEqual(loopCopy->getMaxStoreTemperature(), 84));
    delete loopCopyObject;
    delete loopData;

    TerraPowerRail *rail = TerraFactory::newPowerRail(Terra_RailType_DC24V, 204, "24v");
    assert(rail && isFPEqual(rail->getNominalVoltage(), 24));
    TerraObjectData *railData = TerraFactory::newDataFromObject(rail);
    TerraObject *railCopyObject = TerraFactory::newObjectFromData(railData);
    TerraPowerRail *railCopy = static_cast<TerraPowerRail *>(railCopyObject);
    assert(railCopy && railCopy->getRailType() == Terra_RailType_DC24V);
    assert(isFPEqual(railCopy->getNominalVoltage(), 24));
    delete railCopyObject;
    delete railData;
    delete rail;

    TerraActuator variable(Terra_ActuatorType_Variable, 205, "blend");
    variable.setWriteCallback(actuatorCallback);
    variable.setEnableMode(Terra_EnableMode_Average);
    const float requests[] = {0.2f, 0.4f, 0.6f};
    variable.setOutputRequests(requests, 3, 0, 100);
    assert(isFPEqual(variable.getOutput(), 0.4f));
    assert(isFPEqual(lastActuatorValue, 0.4f));
}

static void test_factory_scheduler_logger_publisher() {
    Terraduino terra;
    TerraSystemSetup setup; setup.systemName = "test-homestead"; setup.updateIntervalMs = 10;
    setup.measurementMode = Terra_MeasurementMode_Imperial;
    setup.loggerMinimumLevel = Terra_LogLevel_Debug;
    setup.publisherIntervalMs = 2500;
    terra.init(setup); assert(terra.isInitialized() && !terra.isRunning());
    TerraSensor sensor(Terra_SensorType_Temperature, Terra_Unit_Celsius, 0, "temperature");
    sensor.setMeasurement(20, Terra_Unit_Celsius, 100, true);
    assert(terra.registerObject(&sensor));
    assert(sensor.getKey() != 0);
    assert(terra.findObjectByName("temperature") == &sensor);
    assert(terra.objectCount() == 1);

    taskRuns = 0;
    uint32_t base = terraMillis();
    int8_t task = terra.scheduler.addIntervalTask(taskCallback, nullptr, 100, false);
    assert(task >= 0);
    terra.scheduler.update(base + 50); assert(taskRuns == 0);
    terra.scheduler.update(base + 150); assert(taskRuns == 1);
    terra.scheduler.addDailyTask(taskCallback, nullptr, 600);
    terra.scheduler.update(base + 160, 599, 1); assert(taskRuns == 1);
    terra.scheduler.update(base + 170, 600, 1); assert(taskRuns == 2);
    terra.scheduler.update(base + 180, 700, 1); assert(taskRuns == 2);
    terra.scheduler.update(base + 190, 700, 2); assert(taskRuns == 3);

    TerraBufferSink sink;
    terra.logger.setSink(&sink);
    terra.logger.info("test", "hello", 123);
    assert(terra.logger.count() >= 2); // includes init record
    assert(sink.buffer().find("hello") != TerraString::npos);

    publishCount = 0;
    terra.publisher.setCallback(publishCallback);
    assert(terra.publisher.addChannel("temp-f", &sensor, Terra_Unit_Fahrenheit));
    terra.publisher.publish(500);
    assert(publishCount == 1 && isFPEqual(publishedValue, 68) && publishedUnit == Terra_Unit_Fahrenheit);

    assert(terra.modules.add(Terra_ModuleType_RTC, "rtc") >= 0);
    assert(terra.modules.find(Terra_ModuleType_RTC) != nullptr);
    TerraPowerRail rail(12, 30, "12v");
    rail.setEnableCallback(actuatorCallback);
    rail.setEnabledState(true); assert(rail.isRailEnabled() && isFPEqual(lastActuatorValue, 1));

    TerraString json = terra.exportSystemJSON();
    Terraduino imported;
    assert(imported.importSystemJSON(json));
    assert(imported.systemData().setup.systemName == "test-homestead");
    assert(imported.systemData().setup.updateIntervalMs == 10);
    assert(imported.systemData().setup.measurementMode == Terra_MeasurementMode_Imperial);
    assert(imported.logger.getMinimumLevel() == Terra_LogLevel_Debug);
    assert(imported.publisher.getInterval() == 2500);

    uint8_t binary[128] = {0};
    size_t written = terra.exportSystemBinary(binary, sizeof(binary));
    assert(written > 0);
    Terraduino importedBinary;
    assert(importedBinary.importSystemBinary(binary, written));
    assert(importedBinary.systemData().setup.systemName == "test-homestead");
    assert(importedBinary.systemData().setup.updateIntervalMs == 10);
    assert(importedBinary.systemData().setup.measurementMode == Terra_MeasurementMode_Imperial);
    assert(importedBinary.logger.getMinimumLevel() == Terra_LogLevel_Debug);
    assert(importedBinary.publisher.getInterval() == 2500);
}

int main() {
    test_core_logic();
    test_measurements_and_strings();
    test_resource_water();
    test_thermal_environment();
    test_sensors_actuators_triggers();
    test_data_roundtrips();
    test_factory_persistence_reconstruction();
    test_factory_scheduler_logger_publisher();
    std::cout << "Terraduino host tests: PASS\n";
    return 0;
}
