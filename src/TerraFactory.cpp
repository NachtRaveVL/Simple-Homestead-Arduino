/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Factory
*/

#include "Terraduino.h"
#include "TerraUtils.h"
#include "TerraSensors.h"
#include "TerraActuators.h"
#include "TerraReservoir.h"
#include "TerraWater.h"
#include "TerraThermal.h"
#include "TerraEnvironment.h"
#include "TerraRails.h"

template<class T>
static SharedPtr<T> terraRegisterObject(T *object, const TerraString &name)
{
    SharedPtr<T> shared(object);
    if (!shared) { return SharedPtr<T>(); }
    if (name.length()) { shared->setName(name); }
    if (getController() && getController()->registerObject(static_pointer_cast<TerraObject>(shared))) { return shared; }
    return SharedPtr<T>();
}

static tposi_t terraFirstOpen(TerraIdentity id)
{
    return getController() ? getController()->firstPositionOpen(id) : tposi_none;
}

static bool terraValidPosition(tposi_t positionIndex)
{
    return positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE;
}

SharedPtr<TerraSensor> TerraFactory::addSensor(Terra_SensorType sensorType, Terra_Unit unit, const TerraString &name)
{
    tposi_t positionIndex = terraFirstOpen(TerraIdentity(sensorType));
    if (!terraValidPosition(positionIndex)) { return SharedPtr<TerraSensor>(); }
    return terraRegisterObject(new TerraSensor(sensorType, positionIndex, unit), name);
}

SharedPtr<TerraRemoteSensor> TerraFactory::addRemoteSensor(Terra_SensorType reportedType, Terra_Unit unit, const TerraString &name)
{
    tposi_t positionIndex = terraFirstOpen(TerraIdentity(Terra_SensorType_Remote));
    if (!terraValidPosition(positionIndex)) { return SharedPtr<TerraRemoteSensor>(); }
    return terraRegisterObject(new TerraRemoteSensor(reportedType, positionIndex, unit), name);
}

SharedPtr<TerraLeakSensor> TerraFactory::addLeakIndicator(uint8_t inputPin, bool activeLow, const TerraString &name)
{
    if (inputPin == TERRA_INVALID_PIN) { return SharedPtr<TerraLeakSensor>(); }
    tposi_t positionIndex = terraFirstOpen(TerraIdentity(Terra_SensorType_Leak));
    if (!terraValidPosition(positionIndex)) { return SharedPtr<TerraLeakSensor>(); }
    return terraRegisterObject(new TerraLeakSensor(positionIndex,
                               TerraDigitalPin(inputPin, Terra_PinMode_Digital_Input, activeLow)), name);
}

SharedPtr<TerraLevelSensor> TerraFactory::addAnalogLevelSensor(uint8_t inputPin,
                                                               float rawMinimum, float rawMaximum,
                                                               float levelMinimum, float levelMaximum,
                                                               const TerraString &name)
{
    if (inputPin == TERRA_INVALID_PIN || isFPEqual(rawMinimum, rawMaximum)) { return SharedPtr<TerraLevelSensor>(); }
    tposi_t positionIndex = terraFirstOpen(TerraIdentity(Terra_SensorType_Level));
    if (!terraValidPosition(positionIndex)) { return SharedPtr<TerraLevelSensor>(); }
    auto sensor = terraRegisterObject(new TerraLevelSensor(positionIndex,
                                      TerraAnalogPin(inputPin, Terra_PinMode_Analog_Input)), name);
    if (sensor) {
        TerraCalibrationData calibration(sensor->getId(), Terra_Unit_Percent);
        calibration.setFromTwoPoints(rawMinimum, levelMinimum, rawMaximum, levelMaximum);
        sensor->setUserCalibrationData(&calibration);
    }
    return sensor;
}

SharedPtr<TerraTemperatureSensor> TerraFactory::addAnalogTemperatureSensor(uint8_t inputPin,
                                                                           float rawMinimum, float rawMaximum,
                                                                           float temperatureMinimum, float temperatureMaximum,
                                                                           Terra_Unit unit, const TerraString &name)
{
    if (inputPin == TERRA_INVALID_PIN || isFPEqual(rawMinimum, rawMaximum)) { return SharedPtr<TerraTemperatureSensor>(); }
    tposi_t positionIndex = terraFirstOpen(TerraIdentity(Terra_SensorType_Temperature));
    if (!terraValidPosition(positionIndex)) { return SharedPtr<TerraTemperatureSensor>(); }
    auto sensor = terraRegisterObject(new TerraTemperatureSensor(positionIndex,
                                      TerraAnalogPin(inputPin, Terra_PinMode_Analog_Input)), name);
    if (sensor) {
        TerraCalibrationData calibration(sensor->getId(), unit);
        calibration.setFromTwoPoints(rawMinimum, temperatureMinimum, rawMaximum, temperatureMaximum);
        sensor->setUserCalibrationData(&calibration);
        sensor->setMeasurementUnits(unit);
    }
    return sensor;
}

SharedPtr<TerraPressureSensor> TerraFactory::addAnalogPressureSensor(uint8_t inputPin,
                                                                     float rawMinimum, float rawMaximum,
                                                                     float pressureMinimum, float pressureMaximum,
                                                                     Terra_Unit unit, const TerraString &name)
{
    if (inputPin == TERRA_INVALID_PIN || isFPEqual(rawMinimum, rawMaximum)) { return SharedPtr<TerraPressureSensor>(); }
    tposi_t positionIndex = terraFirstOpen(TerraIdentity(Terra_SensorType_Pressure));
    if (!terraValidPosition(positionIndex)) { return SharedPtr<TerraPressureSensor>(); }
    auto sensor = terraRegisterObject(new TerraPressureSensor(positionIndex,
                                      TerraAnalogPin(inputPin, Terra_PinMode_Analog_Input), unit), name);
    if (sensor) {
        TerraCalibrationData calibration(sensor->getId(), unit);
        calibration.setFromTwoPoints(rawMinimum, pressureMinimum, rawMaximum, pressureMaximum);
        sensor->setUserCalibrationData(&calibration);
    }
    return sensor;
}

SharedPtr<TerraRelayActuator> TerraFactory::addRelayActuator(Terra_ActuatorType actuatorType,
                                                             uint8_t outputPin, bool activeLow,
                                                             const TerraString &name)
{
    if (outputPin == TERRA_INVALID_PIN) { return SharedPtr<TerraRelayActuator>(); }
    tposi_t positionIndex = terraFirstOpen(TerraIdentity(actuatorType));
    if (!terraValidPosition(positionIndex)) { return SharedPtr<TerraRelayActuator>(); }
    return terraRegisterObject(new TerraRelayActuator(actuatorType, positionIndex,
                               TerraDigitalPin(outputPin, Terra_PinMode_Digital_Output, activeLow)), name);
}

SharedPtr<TerraVariableActuator> TerraFactory::addVariableActuator(Terra_ActuatorType actuatorType,
                                                                   uint8_t outputPin, uint8_t outputBitRes,
                                                                   const TerraString &name)
{
    if (outputPin == TERRA_INVALID_PIN) { return SharedPtr<TerraVariableActuator>(); }
    tposi_t positionIndex = terraFirstOpen(TerraIdentity(actuatorType));
    if (!terraValidPosition(positionIndex)) { return SharedPtr<TerraVariableActuator>(); }
    return terraRegisterObject(new TerraVariableActuator(actuatorType, positionIndex,
                               TerraAnalogPin(outputPin, Terra_PinMode_Analog_Output, outputBitRes)), name);
}

SharedPtr<TerraPump> TerraFactory::addPumpRelay(uint8_t outputPin, bool activeLow, const TerraString &name)
{
    if (outputPin == TERRA_INVALID_PIN) { return SharedPtr<TerraPump>(); }
    tposi_t positionIndex = terraFirstOpen(TerraIdentity(Terra_ActuatorType_Pump));
    if (!terraValidPosition(positionIndex)) { return SharedPtr<TerraPump>(); }
    return terraRegisterObject(new TerraPump(positionIndex,
                               TerraDigitalPin(outputPin, Terra_PinMode_Digital_Output, activeLow)), name);
}

SharedPtr<TerraSumpPump> TerraFactory::addSumpPumpRelay(uint8_t outputPin, bool activeLow, const TerraString &name)
{
    if (outputPin == TERRA_INVALID_PIN) { return SharedPtr<TerraSumpPump>(); }
    tposi_t positionIndex = terraFirstOpen(TerraIdentity(Terra_ActuatorType_SumpPump));
    if (!terraValidPosition(positionIndex)) { return SharedPtr<TerraSumpPump>(); }
    return terraRegisterObject(new TerraSumpPump(positionIndex,
                               TerraDigitalPin(outputPin, Terra_PinMode_Digital_Output, activeLow)), name);
}

SharedPtr<TerraCirculator> TerraFactory::addCirculatorRelay(uint8_t outputPin, bool activeLow, const TerraString &name)
{
    if (outputPin == TERRA_INVALID_PIN) { return SharedPtr<TerraCirculator>(); }
    tposi_t positionIndex = terraFirstOpen(TerraIdentity(Terra_ActuatorType_Circulator));
    if (!terraValidPosition(positionIndex)) { return SharedPtr<TerraCirculator>(); }
    return terraRegisterObject(new TerraCirculator(positionIndex,
                               TerraDigitalPin(outputPin, Terra_PinMode_Digital_Output, activeLow)), name);
}

SharedPtr<TerraValve> TerraFactory::addValveRelay(uint8_t outputPin, bool activeLow, const TerraString &name)
{
    if (outputPin == TERRA_INVALID_PIN) { return SharedPtr<TerraValve>(); }
    tposi_t positionIndex = terraFirstOpen(TerraIdentity(Terra_ActuatorType_Valve));
    if (!terraValidPosition(positionIndex)) { return SharedPtr<TerraValve>(); }
    return terraRegisterObject(new TerraValve(positionIndex,
                               TerraDigitalPin(outputPin, Terra_PinMode_Digital_Output, activeLow)), name);
}

SharedPtr<TerraHeater> TerraFactory::addHeaterRelay(uint8_t outputPin, bool activeLow, const TerraString &name)
{
    if (outputPin == TERRA_INVALID_PIN) { return SharedPtr<TerraHeater>(); }
    tposi_t positionIndex = terraFirstOpen(TerraIdentity(Terra_ActuatorType_Heater));
    if (!terraValidPosition(positionIndex)) { return SharedPtr<TerraHeater>(); }
    return terraRegisterObject(new TerraHeater(positionIndex,
                               TerraDigitalPin(outputPin, Terra_PinMode_Digital_Output, activeLow)), name);
}

SharedPtr<TerraReservoir> TerraFactory::addResource(Terra_ReservoirType ReservoirType, const TerraString &name)
{
    tposi_t positionIndex = terraFirstOpen(TerraIdentity(ReservoirType));
    if (!terraValidPosition(positionIndex)) { return SharedPtr<TerraReservoir>(); }
    return terraRegisterObject(new TerraReservoir(ReservoirType, positionIndex, name), TerraString());
}

SharedPtr<TerraWaterStorage> TerraFactory::addWaterStorage(Terra_WaterStorageType storageType,
                                                           float capacityLiters, const TerraString &name)
{
    tposi_t positionIndex = terraFirstOpen(TerraIdentity(storageType));
    if (!terraValidPosition(positionIndex)) { return SharedPtr<TerraWaterStorage>(); }
    return terraRegisterObject(new TerraWaterStorage(capacityLiters, positionIndex, name, storageType), TerraString());
}

SharedPtr<TerraCistern> TerraFactory::addCistern(float capacityLiters, const TerraString &name)
{
    tposi_t positionIndex = terraFirstOpen(TerraIdentity(Terra_WaterStorageType_Cistern));
    if (!terraValidPosition(positionIndex)) { return SharedPtr<TerraCistern>(); }
    return terraRegisterObject(new TerraCistern(capacityLiters, positionIndex, name), TerraString());
}

SharedPtr<TerraWaterSource> TerraFactory::addWaterSource(Terra_WaterSourceType sourceType,
                                                         uint8_t priority, const TerraString &name)
{
    tposi_t positionIndex = terraFirstOpen(TerraIdentity(sourceType));
    if (!terraValidPosition(positionIndex)) { return SharedPtr<TerraWaterSource>(); }
    return terraRegisterObject(new TerraWaterSource(sourceType, priority, positionIndex, name), TerraString());
}

SharedPtr<TerraWaterRoute> TerraFactory::addWaterRoute(const TerraString &name)
{
    tposi_t positionIndex = terraFirstOpen(TerraIdentity(Terra_ObjectType_WaterRoute));
    if (!terraValidPosition(positionIndex)) { return SharedPtr<TerraWaterRoute>(); }
    return terraRegisterObject(new TerraWaterRoute(positionIndex, name), TerraString());
}

SharedPtr<TerraRainCatchment> TerraFactory::addRainCatchment(float areaSquareMeters,
                                                             float collectionEfficiency,
                                                             const TerraString &name)
{
    tposi_t positionIndex = terraFirstOpen(TerraIdentity(Terra_ObjectType_RainCatchment));
    if (!terraValidPosition(positionIndex)) { return SharedPtr<TerraRainCatchment>(); }
    return terraRegisterObject(new TerraRainCatchment(areaSquareMeters, collectionEfficiency, positionIndex, name), TerraString());
}

SharedPtr<TerraThermalStore> TerraFactory::addThermalStore(const TerraString &name)
{
    tposi_t positionIndex = terraFirstOpen(TerraIdentity(Terra_ObjectType_ThermalStore));
    if (!terraValidPosition(positionIndex)) { return SharedPtr<TerraThermalStore>(); }
    return terraRegisterObject(new TerraThermalStore(positionIndex, name), TerraString());
}

SharedPtr<TerraThermalLoop> TerraFactory::addThermalLoop(const TerraString &name)
{
    tposi_t positionIndex = terraFirstOpen(TerraIdentity(Terra_ObjectType_ThermalLoop));
    if (!terraValidPosition(positionIndex)) { return SharedPtr<TerraThermalLoop>(); }
    return terraRegisterObject(new TerraThermalLoop(positionIndex, name), TerraString());
}

SharedPtr<TerraEnvironment> TerraFactory::addEnvironment(const TerraString &name)
{
    tposi_t positionIndex = terraFirstOpen(TerraIdentity(Terra_ObjectType_Environment));
    if (!terraValidPosition(positionIndex)) { return SharedPtr<TerraEnvironment>(); }
    return terraRegisterObject(new TerraEnvironment(positionIndex, name), TerraString());
}

SharedPtr<TerraPowerRail> TerraFactory::addPowerRail(Terra_RailType railType, const TerraString &name)
{
    tposi_t positionIndex = terraFirstOpen(TerraIdentity(railType));
    if (!terraValidPosition(positionIndex)) { return SharedPtr<TerraPowerRail>(); }
    float nominalVoltage = railType == Terra_RailType_DC3V3 ? 3.3f :
                           railType == Terra_RailType_DC5V ? 5.0f :
                           railType == Terra_RailType_DC12V ? 12.0f :
                           railType == Terra_RailType_DC24V ? 24.0f : 0.0f;
    return terraRegisterObject(new TerraPowerRail(nominalVoltage, positionIndex, name, railType), TerraString());
}

TerraObject *TerraFactory::newObjectFromData(const TerraObjectData *dataIn)
{
    if (!dataIn || !dataIn->isObjectData()) { return nullptr; }

    switch ((Terra_ObjectType)dataIn->id.object.idType) {
        case Terra_ObjectType_Sensor: {
            const auto data = static_cast<const TerraSensorData *>(dataIn);
            switch (dataIn->id.object.classType) {
                case TerraSensor::Binary: return new TerraBinarySensor(data);
                case TerraSensor::Analog: return new TerraAnalogSensor(data);
                case TerraSensor::Remote: return new TerraRemoteSensor(data);
                case TerraSensor::Value: return new TerraSensor(data);
                default: return nullptr;
            }
        }
        case Terra_ObjectType_Actuator: {
            const auto data = static_cast<const TerraActuatorData *>(dataIn);
            if ((Terra_ActuatorType)dataIn->id.object.objType == Terra_ActuatorType_SumpPump) { return new TerraSumpPump(data); }
            switch (dataIn->id.object.classType) {
                case TerraActuator::RelayPump: return new TerraPump(data);
                case TerraActuator::Relay: return new TerraRelayActuator(data);
                case TerraActuator::Variable: return new TerraVariableActuator(data);
                default: return nullptr;
            }
        }
        case Terra_ObjectType_Resource:
            return new TerraReservoir(static_cast<const TerraReservoirData *>(dataIn));
        case Terra_ObjectType_WaterStorage:
            if ((Terra_WaterStorageType)dataIn->id.object.objType == Terra_WaterStorageType_Cistern) {
                return new TerraCistern(static_cast<const TerraCisternData *>(dataIn));
            }
            return new TerraWaterStorage(static_cast<const TerraWaterStorageData *>(dataIn));
        case Terra_ObjectType_WaterSource:
            return new TerraWaterSource(static_cast<const TerraWaterSourceData *>(dataIn));
        case Terra_ObjectType_WaterRoute:
            return new TerraWaterRoute(static_cast<const TerraWaterRouteData *>(dataIn));
        case Terra_ObjectType_RainCatchment:
            return new TerraRainCatchment(static_cast<const TerraRainCatchmentData *>(dataIn));
        case Terra_ObjectType_ThermalStore:
            return new TerraThermalStore(static_cast<const TerraThermalStoreData *>(dataIn));
        case Terra_ObjectType_ThermalLoop:
            return new TerraThermalLoop(static_cast<const TerraThermalLoopData *>(dataIn));
        case Terra_ObjectType_Environment:
            return new TerraEnvironment(static_cast<const TerraEnvironmentData *>(dataIn));
        case Terra_ObjectType_PowerRail:
            return new TerraPowerRail(static_cast<const TerraPowerRailData *>(dataIn));
        default:
            return nullptr;
    }
}

TerraObjectData *TerraFactory::newDataFromObject(const TerraObject *objectIn)
{
    return objectIn ? objectIn->newSaveData() : nullptr;
}
