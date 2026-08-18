#include <Terraduino.h>

void setup() {
    Serial.begin(115200);
    while (!Serial) { }

    bool pass = true;

    Terraduino source;
    TerraSystemSetup setup;
    setup.systemName = F("Test Homestead");
    setup.timeZoneHours = -7;
    setup.timeZoneMinutes = 30;
    setup.controlMode = Terra_ControlMode_Automatic;
    setup.measurementMode = Terra_MeasurementMode_Imperial;
    setup.updateIntervalMs = 500;
    setup.loggerMinimumLevel = Terra_LogLevel_Debug;
    setup.publisherIntervalMs = 5000;
    source.init(setup);

    String systemJSON = source.exportSystemJSON();
    Terraduino systemCopy;
    pass &= systemCopy.importSystemJSON(systemJSON);
    pass &= systemCopy.getSetup().systemName == setup.systemName;
    pass &= systemCopy.getSetup().timeZoneHours == setup.timeZoneHours;
    pass &= systemCopy.getSetup().timeZoneMinutes == setup.timeZoneMinutes;
    pass &= systemCopy.getSetup().controlMode == setup.controlMode;
    pass &= systemCopy.getSetup().measurementMode == setup.measurementMode;
    pass &= systemCopy.getSetup().updateIntervalMs == setup.updateIntervalMs;
    pass &= systemCopy.getSetup().loggerMinimumLevel == setup.loggerMinimumLevel;
    pass &= systemCopy.getSetup().publisherIntervalMs == setup.publisherIntervalMs;

    uint8_t binary[128];
    size_t binaryLength = source.exportSystemBinary(binary, sizeof(binary));
    Terraduino binaryCopy;
    pass &= binaryLength > 0;
    pass &= binaryCopy.importSystemBinary(binary, binaryLength);
    pass &= binaryCopy.getSetup().systemName == setup.systemName;
    pass &= binaryCopy.getSetup().measurementMode == setup.measurementMode;
    pass &= binaryCopy.getSetup().loggerMinimumLevel == setup.loggerMinimumLevel;
    pass &= binaryCopy.getSetup().publisherIntervalMs == setup.publisherIntervalMs;
    if (binaryLength > 10) {
        binary[10] ^= 1;
        Terraduino damagedCopy;
        pass &= !damagedCopy.importSystemBinary(binary, binaryLength);
        binary[10] ^= 1;
    }

    TerraCisternData cistern;
    cistern.key = 1001;
    cistern.name = F("Main Cistern");
    cistern.capacityLiters = 5000.0f;
    cistern.level = 63.5f;
    cistern.reserveLevel = 15.0f;
    cistern.lowLevel = 30.0f;
    cistern.highLevel = 95.0f;
    cistern.fillStartPercent = 35.0f;
    cistern.fillStopPercent = 90.0f;
    cistern.overflowPercent = 99.0f;
    cistern.attachmentCount = 2;
    cistern.attachments[0] = TerraAttachment(4001, Terra_AttachmentRole_LevelSensor);
    cistern.attachments[1] = TerraAttachment(4002, Terra_AttachmentRole_Pump);
    TerraCisternData cisternCopy;
    String cisternJSON = cistern.toJSON();
    pass &= cisternCopy.fromJSON(cisternJSON);
    pass &= cisternCopy.key == cistern.key;
    pass &= cisternCopy.name == cistern.name;
    pass &= isFPEqual(cisternCopy.capacityLiters, cistern.capacityLiters);
    pass &= isFPEqual(cisternCopy.level, cistern.level);
    pass &= cisternCopy.storageType == Terra_WaterStorageType_Cistern;
    pass &= isFPEqual(cisternCopy.fillStartPercent, cistern.fillStartPercent);
    pass &= isFPEqual(cisternCopy.fillStopPercent, cistern.fillStopPercent);
    pass &= isFPEqual(cisternCopy.overflowPercent, cistern.overflowPercent);
    pass &= cisternCopy.attachmentCount == 2;
    pass &= cisternCopy.attachments[0].objectKey == 4001;
    pass &= cisternCopy.attachments[0].role == Terra_AttachmentRole_LevelSensor;

    TerraWaterSourceData well;
    well.key = 1002;
    well.name = F("Well");
    well.sourceType = Terra_WaterSourceType_Well;
    well.priority = 1;
    well.level = 80.0f;
    well.reserveLevel = 10.0f;
    well.maximumFlowLpm = 12.0f;
    TerraWaterSourceData wellCopy;
    pass &= wellCopy.fromJSON(well.toJSON());
    pass &= wellCopy.sourceType == Terra_WaterSourceType_Well;
    pass &= wellCopy.priority == 1;

    TerraWaterRouteData route;
    route.key = 1003;
    route.name = F("Fill Cistern");
    route.sourceKey = well.key;
    route.destinationKey = cistern.key;
    route.destinationStartPercent = 30.0f;
    route.destinationStopPercent = 95.0f;
    route.minimumFlowLpm = 0.5f;
    route.maximumFlowLpm = 20.0f;
    TerraWaterRouteData routeCopy;
    pass &= routeCopy.fromJSON(route.toJSON());
    pass &= routeCopy.sourceKey == well.key;
    pass &= routeCopy.destinationKey == cistern.key;

    TerraSensorData levelSensor;
    levelSensor.key = 1004;
    levelSensor.name = F("Analog Level");
    levelSensor.sensorType = Terra_SensorType_Analog;
    levelSensor.reportedType = Terra_SensorType_Analog;
    levelSensor.unit = Terra_Unit_Percent;
    levelSensor.hasPinDriver = true;
    levelSensor.pinSetup = TerraPinSetup(7, Terra_PinMode_Analog_Input, false);
    levelSensor.driverCalibrated = true;
    levelSensor.driverRawMinimum = 100.0f;
    levelSensor.driverRawMaximum = 900.0f;
    levelSensor.driverValueMinimum = 0.0f;
    levelSensor.driverValueMaximum = 100.0f;
    TerraSensorData levelSensorCopy;
    pass &= levelSensorCopy.fromJSON(levelSensor.toJSON());
    pass &= levelSensorCopy.hasPinDriver && levelSensorCopy.pinSetup.pin == 7;

    TerraActuatorData pumpData;
    pumpData.key = 1005;
    pumpData.name = F("Fill Pump");
    pumpData.actuatorType = Terra_ActuatorType_Pump;
    pumpData.enableMode = Terra_EnableMode_Lowest;
    pumpData.maxContinuousMs = 45000;
    pumpData.hasPinDriver = true;
    pumpData.pinSetup = TerraPinSetup(9, Terra_PinMode_Digital_Output, true);
    TerraActuatorData pumpDataCopy;
    pass &= pumpDataCopy.fromJSON(pumpData.toJSON());
    pass &= pumpDataCopy.hasPinDriver && pumpDataCopy.pinSetup.pin == 9 && pumpDataCopy.pinSetup.activeLow;

    TerraThermalStoreData thermal;
    thermal.key = 2001;
    thermal.name = F("Thermal Store");
    thermal.temperatureC = 58.0f;
    thermal.minimumTargetC = 45.0f;
    thermal.maximumTargetC = 65.0f;
    thermal.absoluteMaximumC = 90.0f;
    TerraThermalStoreData thermalCopy;
    pass &= thermalCopy.fromJSON(thermal.toJSON());
    pass &= isFPEqual(thermalCopy.temperatureC, thermal.temperatureC);
    pass &= isFPEqual(thermalCopy.absoluteMaximumC, thermal.absoluteMaximumC);

    TerraEnvironmentData weather;
    weather.key = 3001;
    weather.name = F("Outside");
    weather.weather.airTemperatureC = -2.5f;
    weather.weather.relativeHumidity = 85.0f;
    weather.weather.validFields = TERRA_WEATHER_AIR_TEMPERATURE | TERRA_WEATHER_HUMIDITY;
    weather.weather.valid = true;
    TerraEnvironmentData weatherCopy;
    pass &= weatherCopy.fromJSON(weather.toJSON());
    pass &= weatherCopy.weather.valid;
    pass &= weatherCopy.weather.validFields == weather.weather.validFields;
    pass &= isFPEqual(weatherCopy.weather.airTemperatureC, weather.weather.airTemperatureC);

    Serial.println(systemJSON);
    Serial.println(cisternJSON);
    Serial.println(pass ? F("PASS") : F("FAIL"));
}

void loop() { }
