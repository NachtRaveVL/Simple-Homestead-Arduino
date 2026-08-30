#include <Terraduino.h>
#include <string.h>

#define JSON_DOC_SIZE 1024

template<class T>
bool roundTripData(const T &source, T &copy)
{
    StaticJsonDocument<JSON_DOC_SIZE> doc;
    JsonObject json = doc.to<JsonObject>();
    source.toJSONObject(json);
    copy.fromJSONObject(json.as<JsonObjectConst>());
    return true;
}

void setup()
{
    Serial.begin(115200);
    while (!Serial) { ; }

    bool pass = true;

    TerraSystemData system;
    system.systemMode = Terra_SystemMode_Automatic;
    system.measurementMode = Terra_MeasurementMode_Imperial;
    system.timeZoneOffset = -7;
    system.pollingInterval = 500;
    strncpy(system.systemName, "Test Homestead", TERRA_NAME_MAXSIZE - 1);

    TerraSystemData systemCopy;
    roundTripData(system, systemCopy);
    pass &= systemCopy.systemMode == system.systemMode;
    pass &= systemCopy.measurementMode == system.measurementMode;
    pass &= systemCopy.timeZoneOffset == system.timeZoneOffset;
    pass &= systemCopy.pollingInterval == system.pollingInterval;
    pass &= !strcmp(systemCopy.systemName, system.systemName);

    TerraWaterReservoirData water;
    water.id.object.posIndex = 0;
    water.volumeUnits = Terra_UnitsType_LiqVolume_Liters;
    water.maxVolume = 5000.0f;
    strncpy(water.name, "Main Reservoir", TERRA_NAME_MAXSIZE - 1);
    strncpy(water.volumeSensor, "Level #0", TERRA_NAME_MAXSIZE - 1);

    TerraWaterReservoirData waterCopy;
    roundTripData(water, waterCopy);
    pass &= waterCopy.id.object.idType == (tid_t)Terra_ObjectType_Reservoir;
    pass &= waterCopy.id.object.objType == (tid_t)Terra_ReservoirType_Water;
    pass &= waterCopy.id.object.classType == (tid_t)TerraReservoir::Water;
    pass &= waterCopy.volumeUnits == water.volumeUnits;
    pass &= isFPEqual(waterCopy.maxVolume, water.maxVolume);
    pass &= !strcmp(waterCopy.name, water.name);
    pass &= !strcmp(waterCopy.volumeSensor, water.volumeSensor);

    TerraThermalReservoirData thermal;
    thermal.id.object.posIndex = 0;
    thermal.temperatureUnits = Terra_UnitsType_Temperature_Celsius;
    thermal.maxTemperature = 90.0f;
    strncpy(thermal.name, "Thermal Store", TERRA_NAME_MAXSIZE - 1);
    strncpy(thermal.temperatureSensor, "Temperature #0", TERRA_NAME_MAXSIZE - 1);

    TerraThermalReservoirData thermalCopy;
    roundTripData(thermal, thermalCopy);
    pass &= thermalCopy.id.object.idType == (tid_t)Terra_ObjectType_Reservoir;
    pass &= thermalCopy.id.object.objType == (tid_t)Terra_ReservoirType_Thermal;
    pass &= thermalCopy.id.object.classType == (tid_t)TerraReservoir::Thermal;
    pass &= thermalCopy.temperatureUnits == thermal.temperatureUnits;
    pass &= isFPEqual(thermalCopy.maxTemperature, thermal.maxTemperature);
    pass &= !strcmp(thermalCopy.temperatureSensor, thermal.temperatureSensor);

    TerraInfiniteWaterReservoirData waterPipe;
    waterPipe.id.object.posIndex = 1;
    waterPipe.alwaysFilled = false;

    TerraInfiniteWaterReservoirData waterPipeCopy;
    roundTripData(waterPipe, waterPipeCopy);
    pass &= waterPipeCopy.id.object.classType == (tid_t)TerraReservoir::WaterPipe;
    pass &= waterPipeCopy.alwaysFilled == waterPipe.alwaysFilled;

    TerraInfiniteThermalReservoirData thermalPipe;
    thermalPipe.id.object.posIndex = 1;
    thermalPipe.alwaysFilled = false;

    TerraInfiniteThermalReservoirData thermalPipeCopy;
    roundTripData(thermalPipe, thermalPipeCopy);
    pass &= thermalPipeCopy.id.object.classType == (tid_t)TerraReservoir::ThermalPipe;
    pass &= thermalPipeCopy.alwaysFilled == thermalPipe.alwaysFilled;

    TerraSensorData sensor;
    sensor.id.object.idType = (tid_t)Terra_ObjectType_Sensor;
    sensor.id.object.objType = (tid_t)Terra_SensorType_Level;
    sensor.id.object.posIndex = 0;
    sensor.id.object.classType = (tid_t)TerraSensor::Remote;
    sensor.measurementUnits = Terra_UnitsType_LiqVolume_Liters;
    sensor.reportedType = Terra_SensorType_Level;
    sensor.staleAfterMs = 30000;
    strncpy(sensor.name, "Remote Level", TERRA_NAME_MAXSIZE - 1);

    TerraSensorData sensorCopy;
    roundTripData(sensor, sensorCopy);
    pass &= sensorCopy.id.object.objType == (tid_t)Terra_SensorType_Level;
    pass &= sensorCopy.id.object.classType == (tid_t)TerraSensor::Remote;
    pass &= sensorCopy.measurementUnits == sensor.measurementUnits;
    pass &= sensorCopy.reportedType == sensor.reportedType;
    pass &= sensorCopy.staleAfterMs == sensor.staleAfterMs;

    TerraPumpActuatorData pump;
    pump.id.object.idType = (tid_t)Terra_ObjectType_Actuator;
    pump.id.object.objType = (tid_t)Terra_ActuatorType_Pump;
    pump.id.object.posIndex = 0;
    pump.id.object.classType = (tid_t)TerraActuator::RelayPump;
    pump.enableMode = Terra_EnableMode_Highest;
    pump.flowRateUnits = Terra_UnitsType_LiqFlowRate_LitersPerMin;
    strncpy(pump.name, "Fill Pump", TERRA_NAME_MAXSIZE - 1);
    strncpy(pump.reservoirName, "Water #0", TERRA_NAME_MAXSIZE - 1);
    strncpy(pump.destReservoir, "Water #1", TERRA_NAME_MAXSIZE - 1);

    TerraPumpActuatorData pumpCopy;
    roundTripData(pump, pumpCopy);
    pass &= pumpCopy.id.object.objType == (tid_t)Terra_ActuatorType_Pump;
    pass &= pumpCopy.id.object.classType == (tid_t)TerraActuator::RelayPump;
    pass &= pumpCopy.enableMode == pump.enableMode;
    pass &= pumpCopy.flowRateUnits == pump.flowRateUnits;
    pass &= !strcmp(pumpCopy.reservoirName, pump.reservoirName);
    pass &= !strcmp(pumpCopy.destReservoir, pump.destReservoir);

    TerraData *allocatedWater = _allocateDataForObjType((int8_t)Terra_ObjectType_Reservoir, (int8_t)TerraReservoir::Water);
    TerraData *allocatedThermal = _allocateDataForObjType((int8_t)Terra_ObjectType_Reservoir, (int8_t)TerraReservoir::Thermal);
    TerraData *allocatedWaterPipe = _allocateDataForObjType((int8_t)Terra_ObjectType_Reservoir, (int8_t)TerraReservoir::WaterPipe);
    TerraData *allocatedThermalPipe = _allocateDataForObjType((int8_t)Terra_ObjectType_Reservoir, (int8_t)TerraReservoir::ThermalPipe);
    pass &= allocatedWater && allocatedWater->id.object.classType == (tid_t)TerraReservoir::Water;
    pass &= allocatedThermal && allocatedThermal->id.object.classType == (tid_t)TerraReservoir::Thermal;
    pass &= allocatedWaterPipe && allocatedWaterPipe->id.object.classType == (tid_t)TerraReservoir::WaterPipe;
    pass &= allocatedThermalPipe && allocatedThermalPipe->id.object.classType == (tid_t)TerraReservoir::ThermalPipe;
    delete allocatedWater;
    delete allocatedThermal;
    delete allocatedWaterPipe;
    delete allocatedThermalPipe;

    Serial.println(pass ? F("PASS") : F("FAIL"));
}

void loop() { ; }
