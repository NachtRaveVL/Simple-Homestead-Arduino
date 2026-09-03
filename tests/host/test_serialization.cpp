#include "Terraduino.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void testSystemData()
{
    TerraSystemData data;
    strncpy(data.systemName, "Test Homestead", sizeof(data.systemName) - 1);
    data.systemName[sizeof(data.systemName) - 1] = '\0';
    data.systemMode = Terra_SystemMode_Automatic;
    data.measureMode = Terra_MeasurementMode_Imperial;
    data.timeZoneOffset = -7.0f;
    data.pollingInterval = 500;
    data.latitude = 49.2827;
    data.longitude = -123.1207;
    data.altitude = 70.0;
    data.scheduler.reportInterval = 3600;
    data.logger.logLevel = Terra_LogLevel_Warnings;

    StaticJsonDocument<2048> doc;
    JsonObject object = doc.to<JsonObject>();
    data.toJSONObject(object);

    TerraSystemData decoded;
    JsonObjectConst objectConst = doc.as<JsonObjectConst>();
    decoded.fromJSONObject(objectConst);
    assert(decoded.isSystemData());
    assert(strcmp(decoded.systemName, data.systemName) == 0);
    assert(decoded.systemMode == data.systemMode);
    assert(decoded.measureMode == data.measureMode);
    assert(isFPEqual(decoded.timeZoneOffset, data.timeZoneOffset));
    assert(decoded.pollingInterval == data.pollingInterval);
    assert(isFPEqual(decoded.latitude, data.latitude));
    assert(isFPEqual(decoded.longitude, data.longitude));
    assert(isFPEqual(decoded.altitude, data.altitude));
    assert(decoded.scheduler.reportInterval == data.scheduler.reportInterval);
    assert(decoded.logger.logLevel == data.logger.logLevel);

    TerraData *allocated = newDataFromJSONObject(objectConst);
    assert(allocated && allocated->isSystemData());
    delete allocated;
}

static void testCalibrationData()
{
    TerraCalibrationData data(TerraIdentity(Terra_SensorType_Temperature, 1), Terra_UnitsType_Temperature_Celsius);
    data.setFromTwoPoints(0.1f, -10.0f, 0.9f, 30.0f);
    assert(isFPEqual(data.transform(0.5f), 10.0f));
    assert(isFPEqual(data.inverseTransform(10.0f), 0.5f));

    StaticJsonDocument<256> doc;
    JsonObject object = doc.to<JsonObject>();
    data.toJSONObject(object);
    JsonObjectConst objectConst = doc.as<JsonObjectConst>();

    TerraCalibrationData decoded;
    decoded.fromJSONObject(objectConst);
    assert(decoded.isCalibrationData());
    assert(strcmp(decoded.ownerName, data.ownerName) == 0);
    assert(decoded.calibrationUnits == data.calibrationUnits);
    assert(isFPEqual(decoded.multiplier, data.multiplier));
    assert(isFPEqual(decoded.offset, data.offset));
}

static void testActuatorData()
{
    TerraActuatorData data;
    data.id.object.idType = (tid_t)Terra_ObjectType_Actuator;
    data.id.object.objType = (tid_t)Terra_ActuatorType_Heater;
    data.id.object.posIndex = 2;
    data.id.object.classType = (tid_t)TerraActuator::Relay;
    data.enableMode = Terra_EnableMode_Highest;
    TerraDigitalPin(8, Terra_PinMode_Digital_Output_PushPull, false).saveToData(&data.outputPin);
    strncpy(data.railName, "AC110V #0", sizeof(data.railName) - 1);
    data.railName[sizeof(data.railName) - 1] = '\0';
    strncpy(data.reservoirName, "Water #0", sizeof(data.reservoirName) - 1);
    data.reservoirName[sizeof(data.reservoirName) - 1] = '\0';

    StaticJsonDocument<512> doc;
    JsonObject object = doc.to<JsonObject>();
    data.toJSONObject(object);
    JsonObjectConst objectConst = doc.as<JsonObjectConst>();

    TerraData *allocated = newDataFromJSONObject(objectConst);
    assert(allocated && allocated->isObjectData());
    TerraActuatorData *decoded = static_cast<TerraActuatorData *>(allocated);
    assert(decoded->id.object.idType == (tid_t)Terra_ObjectType_Actuator);
    assert(decoded->id.object.objType == (tid_t)Terra_ActuatorType_Heater);
    assert(decoded->id.object.posIndex == 2);
    assert(decoded->id.object.classType == (tid_t)TerraActuator::Relay);
    assert(decoded->enableMode == Terra_EnableMode_Highest);
    assert(decoded->outputPin.pin == 8);
    assert(!decoded->outputPin.dataAs.digitalPin.activeLow);
    assert(strcmp(decoded->railName, data.railName) == 0);
    assert(strcmp(decoded->reservoirName, data.reservoirName) == 0);
    delete allocated;
}

static void testBinarySensorData()
{
    TerraSensorData data;
    data.id.object.idType = (tid_t)Terra_ObjectType_Sensor;
    data.id.object.objType = (tid_t)Terra_SensorType_Leak;
    data.id.object.posIndex = 1;
    data.id.object.classType = (tid_t)TerraSensor::Binary;
    data.measurementUnits = Terra_UnitsType_Raw_1;
    TerraDigitalPin(14, Terra_PinMode_Digital_Input_PullUp, true).saveToData(&data.inputPin);

    StaticJsonDocument<512> doc;
    JsonObject object = doc.to<JsonObject>();
    data.toJSONObject(object);
    JsonObjectConst objectConst = doc.as<JsonObjectConst>();

    TerraData *allocated = newDataFromJSONObject(objectConst);
    assert(allocated && allocated->isObjectData());
    TerraSensorData *decoded = static_cast<TerraSensorData *>(allocated);
    assert(decoded->id.object.idType == (tid_t)Terra_ObjectType_Sensor);
    assert(decoded->id.object.objType == (tid_t)Terra_SensorType_Leak);
    assert(decoded->id.object.classType == (tid_t)TerraSensor::Binary);
    assert(decoded->measurementUnits == Terra_UnitsType_Raw_1);
    assert(decoded->inputPin.pin == 14);
    assert(decoded->inputPin.dataAs.digitalPin.activeLow);

    TerraSensor *sensor = newSensorObjectFromData(decoded);
    assert(sensor && sensor->isBinaryClass());
    delete sensor;
    delete allocated;
}

static void testTriggerSubData()
{
    TerraTriggerSubData data;
    data.type = TerraTrigger::MeasureValue;
    strncpy(data.sensorName, "Temperature #1", sizeof(data.sensorName) - 1);
    data.sensorName[sizeof(data.sensorName) - 1] = '\0';
    data.measurementRow = 0;
    data.measurementUnits = Terra_UnitsType_Temperature_Celsius;
    data.detriggerTol = 0.5f;
    data.detriggerDelay = 1000;
    data.dataAs.measureValue.tolerance = 5.0f;
    data.dataAs.measureValue.triggerBelow = true;

    StaticJsonDocument<384> doc;
    JsonObject object = doc.to<JsonObject>();
    data.toJSONObject(object);
    JsonObjectConst objectConst = doc.as<JsonObjectConst>();

    TerraTriggerSubData decoded;
    decoded.fromJSONObject(objectConst);
    assert(decoded.type == data.type);
    assert(strcmp(decoded.sensorName, data.sensorName) == 0);
    assert(decoded.measurementRow == data.measurementRow);
    assert(decoded.measurementUnits == data.measurementUnits);
    assert(isFPEqual(decoded.detriggerTol, data.detriggerTol));
    assert(decoded.detriggerDelay == data.detriggerDelay);
    assert(isFPEqual(decoded.dataAs.measureValue.tolerance, data.dataAs.measureValue.tolerance));
    assert(decoded.dataAs.measureValue.triggerBelow == data.dataAs.measureValue.triggerBelow);
}

int main()
{
    testSystemData();
    testCalibrationData();
    testActuatorData();
    testBinarySensorData();
    testTriggerSubData();
    puts("PASS serialization");
    return 0;
}
