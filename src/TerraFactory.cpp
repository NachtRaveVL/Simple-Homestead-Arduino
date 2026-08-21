/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Factory
*/

#include "Terraduino.h"
#include "TerraUtils.h"
#include "TerraSensors.h"
#include "TerraActuators.h"
#include "TerraResource.h"
#include "TerraWater.h"
#include "TerraThermal.h"
#include "TerraEnvironment.h"
#include "TerraRails.h"

static void terraAddAttachmentData(TerraObjectData *data, uint32_t key, Terra_AttachmentRole role)
{
    if (!data || key == TERRA_INVALID_KEY || data->attachmentCount >= TERRA_MAX_ATTACHMENTS) return;
    data->attachments[data->attachmentCount++] = TerraAttachmentData(key, role);
}

void TerraFactory::applyObjectData(TerraObject *object, const TerraObjectData *data)
{
    if (!object || !data) return;
    object->setKey(data->key);
    object->setName(data->name);
    object->setEnabled(data->enabled);

    for (uint8_t index = 0; index < data->attachmentCount; ++index) {
        const TerraAttachmentData &attachment = data->attachments[index];
        switch (object->getObjectType()) {
            case Terra_ObjectType_Actuator:
                if (static_cast<TerraActuator *>(object)->getActuatorType() == Terra_ActuatorType_SumpPump &&
                    attachment.role == Terra_AttachmentRole_LevelSensor) {
                    static_cast<TerraSumpPump *>(object)->initLevelSensorKey(attachment.objectKey);
                }
                break;
            case Terra_ObjectType_WaterStorage:
                if (attachment.role == Terra_AttachmentRole_LevelSensor) {
                    static_cast<TerraWaterStorage *>(object)->initLevelSensorKey(attachment.objectKey);
                }
                break;
            case Terra_ObjectType_WaterSource:
                if (attachment.role == Terra_AttachmentRole_LevelSensor) {
                    static_cast<TerraWaterSource *>(object)->initLevelSensorKey(attachment.objectKey);
                }
                break;
            case Terra_ObjectType_WaterRoute: {
                TerraWaterRoute *route = static_cast<TerraWaterRoute *>(object);
                if (attachment.role == Terra_AttachmentRole_Pump) {
                    route->initPumpKey(attachment.objectKey);
                } else if (attachment.role == Terra_AttachmentRole_FlowSensor) {
                    route->initFlowSensorKey(attachment.objectKey);
                }
            } break;
            case Terra_ObjectType_ThermalStore:
                if (attachment.role == Terra_AttachmentRole_TemperatureSensor) {
                    static_cast<TerraThermalStore *>(object)->initTemperatureSensorKey(attachment.objectKey);
                }
                break;
            case Terra_ObjectType_ThermalLoop: {
                TerraThermalLoop *loop = static_cast<TerraThermalLoop *>(object);
                if (attachment.role == Terra_AttachmentRole_TemperatureSensor) {
                    loop->initSourceTemperatureKey(attachment.objectKey);
                } else if (attachment.role == Terra_AttachmentRole_Storage) {
                    loop->initStoreKey(attachment.objectKey);
                } else if (attachment.role == Terra_AttachmentRole_Circulator) {
                    loop->initCirculatorKey(attachment.objectKey);
                }
            } break;
            case Terra_ObjectType_Environment:
                static_cast<TerraEnvironment *>(object)->initAttachmentKey(attachment.role, attachment.objectKey);
                break;
            default: break;
        }
    }
}

static void terraFillObjectData(const TerraObject *object, TerraObjectData *data)
{
    if (!object || !data) return;
    data->key = object->getKey();
    data->objectType = object->getObjectType();
    data->name = object->getName();
    data->enabled = object->isEnabled();
    data->attachmentCount = 0;

    switch (object->getObjectType()) {
        case Terra_ObjectType_Actuator: {
            const TerraActuator *actuator = static_cast<const TerraActuator *>(object);
            if (actuator->getActuatorType() == Terra_ActuatorType_SumpPump) {
                terraAddAttachmentData(data, static_cast<const TerraSumpPump *>(object)->getLevelSensorAttachment().getKey(), Terra_AttachmentRole_LevelSensor);
            }
        } break;
        case Terra_ObjectType_WaterStorage:
            terraAddAttachmentData(data, static_cast<const TerraWaterStorage *>(object)->getLevelSensorAttachment().getKey(), Terra_AttachmentRole_LevelSensor);
            break;
        case Terra_ObjectType_WaterSource:
            terraAddAttachmentData(data, static_cast<const TerraWaterSource *>(object)->getLevelSensorAttachment().getKey(), Terra_AttachmentRole_LevelSensor);
            break;
        case Terra_ObjectType_WaterRoute: {
            const TerraWaterRoute *route = static_cast<const TerraWaterRoute *>(object);
            terraAddAttachmentData(data, route->getPumpKey(), Terra_AttachmentRole_Pump);
            terraAddAttachmentData(data, route->getFlowSensorKey(), Terra_AttachmentRole_FlowSensor);
        } break;
        case Terra_ObjectType_ThermalStore:
            terraAddAttachmentData(data, static_cast<const TerraThermalStore *>(object)->getTemperatureSensorAttachment().getKey(), Terra_AttachmentRole_TemperatureSensor);
            break;
        case Terra_ObjectType_ThermalLoop: {
            const TerraThermalLoop *loop = static_cast<const TerraThermalLoop *>(object);
            terraAddAttachmentData(data, loop->getSourceTemperatureSensorKey(), Terra_AttachmentRole_TemperatureSensor);
            terraAddAttachmentData(data, loop->getThermalStoreKey(), Terra_AttachmentRole_Storage);
            terraAddAttachmentData(data, loop->getCirculatorKey(), Terra_AttachmentRole_Circulator);
        } break;
        case Terra_ObjectType_Environment: {
            const TerraEnvironment *environment = static_cast<const TerraEnvironment *>(object);
            terraAddAttachmentData(data, environment->getAirTemperatureSensorAttachment().getKey(), Terra_AttachmentRole_TemperatureSensor);
            terraAddAttachmentData(data, environment->getHumiditySensorAttachment().getKey(), Terra_AttachmentRole_HumiditySensor);
            terraAddAttachmentData(data, environment->getPressureSensorAttachment().getKey(), Terra_AttachmentRole_PressureSensor);
            terraAddAttachmentData(data, environment->getRainfallSensorAttachment().getKey(), Terra_AttachmentRole_RainfallSensor);
            terraAddAttachmentData(data, environment->getRainRateSensorAttachment().getKey(), Terra_AttachmentRole_RainRateSensor);
            terraAddAttachmentData(data, environment->getWindSpeedSensorAttachment().getKey(), Terra_AttachmentRole_WindSpeedSensor);
            terraAddAttachmentData(data, environment->getWindDirectionSensorAttachment().getKey(), Terra_AttachmentRole_WindDirectionSensor);
            terraAddAttachmentData(data, environment->getSolarRadiationSensorAttachment().getKey(), Terra_AttachmentRole_SolarRadiationSensor);
        } break;
        default: break;
    }
}

SharedPtr<TerraSensor> TerraFactory::addSensor(Terra_SensorType sensorType, Terra_Unit unit, uint32_t key, const TerraString &name)
{
    SharedPtr<TerraSensor> object(newSensorObject(sensorType, unit, key, name));
    if (object && getController() && getController()->registerObject(static_pointer_cast<TerraObject>(object))) return object;
    return SharedPtr<TerraSensor>();
}

SharedPtr<TerraRemoteSensor> TerraFactory::addRemoteSensor(Terra_SensorType reportedType, Terra_Unit unit, uint32_t key, const TerraString &name)
{
    SharedPtr<TerraRemoteSensor> object(new TerraRemoteSensor(reportedType, unit, key, name));
    if (object && getController() && getController()->registerObject(static_pointer_cast<TerraObject>(object))) return object;
    return SharedPtr<TerraRemoteSensor>();
}

SharedPtr<TerraActuator> TerraFactory::addActuator(Terra_ActuatorType actuatorType, uint32_t key, const TerraString &name)
{
    SharedPtr<TerraActuator> object(newActuatorObject(actuatorType, key, name));
    if (object && getController() && getController()->registerObject(static_pointer_cast<TerraObject>(object))) return object;
    return SharedPtr<TerraActuator>();
}

SharedPtr<TerraPump> TerraFactory::addPump(uint32_t key, const TerraString &name)
{
    SharedPtr<TerraPump> object(new TerraPump(key, name));
    if (object && getController() && getController()->registerObject(static_pointer_cast<TerraObject>(object))) return object;
    return SharedPtr<TerraPump>();
}

SharedPtr<TerraSumpPump> TerraFactory::addSumpPump(uint32_t key, const TerraString &name)
{
    SharedPtr<TerraSumpPump> object(new TerraSumpPump(key, name));
    if (object && getController() && getController()->registerObject(static_pointer_cast<TerraObject>(object))) return object;
    return SharedPtr<TerraSumpPump>();
}

SharedPtr<TerraCirculator> TerraFactory::addCirculator(uint32_t key, const TerraString &name)
{
    SharedPtr<TerraCirculator> object(new TerraCirculator(key, name));
    if (object && getController() && getController()->registerObject(static_pointer_cast<TerraObject>(object))) return object;
    return SharedPtr<TerraCirculator>();
}

SharedPtr<TerraResource> TerraFactory::addResource(Terra_ResourceType resourceType, uint32_t key, const TerraString &name)
{
    SharedPtr<TerraResource> object(newResourceObject(resourceType, key, name));
    if (object && getController() && getController()->registerObject(static_pointer_cast<TerraObject>(object))) return object;
    return SharedPtr<TerraResource>();
}

SharedPtr<TerraWaterStorage> TerraFactory::addWaterStorage(Terra_WaterStorageType storageType, float capacityLiters, uint32_t key, const TerraString &name)
{
    SharedPtr<TerraWaterStorage> object(newWaterStorageObject(storageType, capacityLiters, key, name));
    if (object && getController() && getController()->registerObject(static_pointer_cast<TerraObject>(object))) return object;
    return SharedPtr<TerraWaterStorage>();
}

SharedPtr<TerraCistern> TerraFactory::addCistern(float capacityLiters, uint32_t key, const TerraString &name)
{
    SharedPtr<TerraCistern> object(new TerraCistern(capacityLiters, key, name));
    if (object && getController() && getController()->registerObject(static_pointer_cast<TerraObject>(object))) return object;
    return SharedPtr<TerraCistern>();
}

SharedPtr<TerraWaterSource> TerraFactory::addWaterSource(Terra_WaterSourceType sourceType, uint8_t priority, uint32_t key, const TerraString &name)
{
    SharedPtr<TerraWaterSource> object(newWaterSourceObject(sourceType, priority, key, name));
    if (object && getController() && getController()->registerObject(static_pointer_cast<TerraObject>(object))) return object;
    return SharedPtr<TerraWaterSource>();
}

SharedPtr<TerraWaterRoute> TerraFactory::addWaterRoute(uint32_t key, const TerraString &name)
{
    SharedPtr<TerraWaterRoute> object(new TerraWaterRoute(key, name));
    if (object && getController() && getController()->registerObject(static_pointer_cast<TerraObject>(object))) return object;
    return SharedPtr<TerraWaterRoute>();
}

SharedPtr<TerraRainCatchment> TerraFactory::addRainCatchment(float areaSquareMeters, float collectionEfficiency, uint32_t key, const TerraString &name)
{
    SharedPtr<TerraRainCatchment> object(new TerraRainCatchment(areaSquareMeters, collectionEfficiency, key, name));
    if (object && getController() && getController()->registerObject(static_pointer_cast<TerraObject>(object))) return object;
    return SharedPtr<TerraRainCatchment>();
}

SharedPtr<TerraThermalStore> TerraFactory::addThermalStore(uint32_t key, const TerraString &name)
{
    SharedPtr<TerraThermalStore> object(new TerraThermalStore(key, name));
    if (object && getController() && getController()->registerObject(static_pointer_cast<TerraObject>(object))) return object;
    return SharedPtr<TerraThermalStore>();
}

SharedPtr<TerraThermalLoop> TerraFactory::addThermalLoop(uint32_t key, const TerraString &name)
{
    SharedPtr<TerraThermalLoop> object(new TerraThermalLoop(key, name));
    if (object && getController() && getController()->registerObject(static_pointer_cast<TerraObject>(object))) return object;
    return SharedPtr<TerraThermalLoop>();
}

SharedPtr<TerraEnvironment> TerraFactory::addEnvironment(uint32_t key, const TerraString &name)
{
    SharedPtr<TerraEnvironment> object(new TerraEnvironment(key, name));
    if (object && getController() && getController()->registerObject(static_pointer_cast<TerraObject>(object))) return object;
    return SharedPtr<TerraEnvironment>();
}

SharedPtr<TerraPowerRail> TerraFactory::addPowerRail(Terra_RailType railType, uint32_t key, const TerraString &name)
{
    SharedPtr<TerraPowerRail> object(newPowerRailObject(railType, key, name));
    if (object && getController() && getController()->registerObject(static_pointer_cast<TerraObject>(object))) return object;
    return SharedPtr<TerraPowerRail>();
}

TerraSensor *TerraFactory::newSensorObject(Terra_SensorType sensorType, Terra_Unit unit, uint32_t key, const TerraString &name)
{
    switch (sensorType) {
        case Terra_SensorType_Binary: return new TerraBinarySensor(key, name);
        case Terra_SensorType_Analog: return new TerraAnalogSensor(unit, key, name);
        case Terra_SensorType_Temperature: return new TerraTemperatureSensor(key, name);
        case Terra_SensorType_Humidity: return new TerraHumiditySensor(key, name);
        case Terra_SensorType_Pressure: return new TerraPressureSensor(unit, key, name);
        case Terra_SensorType_Rainfall: return new TerraRainfallSensor(unit == Terra_Unit_MillimetersPerHour || unit == Terra_Unit_InchesPerHour, key, name);
        case Terra_SensorType_Flow: return new TerraFlowSensor(key, name);
        case Terra_SensorType_Level: return new TerraLevelSensor(key, name);
        case Terra_SensorType_WindSpeed: return new TerraWindSpeedSensor(key, name);
        case Terra_SensorType_WindDirection: return new TerraWindDirectionSensor(key, name);
        case Terra_SensorType_SolarRadiation: return new TerraSolarRadiationSensor(key, name);
        case Terra_SensorType_Voltage: return new TerraVoltageSensor(key, name);
        case Terra_SensorType_Current: return new TerraCurrentSensor(key, name);
        case Terra_SensorType_Leak: return new TerraLeakSensor(key, name);
        case Terra_SensorType_Remote: return new TerraRemoteSensor(Terra_SensorType_Undefined, unit, key, name);
        case Terra_SensorType_Undefined:
        default: return nullptr;
    }
}

TerraActuator *TerraFactory::newActuatorObject(Terra_ActuatorType actuatorType, uint32_t key, const TerraString &name)
{
    switch (actuatorType) {
        case Terra_ActuatorType_Digital: return new TerraActuator(actuatorType, key, name);
        case Terra_ActuatorType_Variable: return new TerraVariableActuator(key, name);
        case Terra_ActuatorType_Pump: return new TerraPump(key, name);
        case Terra_ActuatorType_Valve: return new TerraValve(key, name);
        case Terra_ActuatorType_Diverter: return new TerraDiverter(key, name);
        case Terra_ActuatorType_Heater: return new TerraHeater(key, name);
        case Terra_ActuatorType_Circulator: return new TerraCirculator(key, name);
        case Terra_ActuatorType_SumpPump: return new TerraSumpPump(key, name);
        case Terra_ActuatorType_Undefined:
        default: return nullptr;
    }
}

TerraResource *TerraFactory::newResourceObject(Terra_ResourceType resourceType, uint32_t key, const TerraString &name)
{
    if (resourceType == Terra_ResourceType_Undefined) return nullptr;
    return new TerraResource(resourceType, key, name);
}

TerraWaterStorage *TerraFactory::newWaterStorageObject(Terra_WaterStorageType storageType, float capacityLiters,
                                                 uint32_t key, const TerraString &name)
{
    switch (storageType) {
        case Terra_WaterStorageType_Cistern: return new TerraCistern(capacityLiters, key, name);
        case Terra_WaterStorageType_Tank:
        case Terra_WaterStorageType_Reservoir:
        case Terra_WaterStorageType_Undefined:
            return new TerraWaterStorage(capacityLiters, key, name, storageType);
        default: return nullptr;
    }
}

TerraWaterSource *TerraFactory::newWaterSourceObject(Terra_WaterSourceType sourceType, uint8_t priority,
                                               uint32_t key, const TerraString &name)
{
    if (sourceType == Terra_WaterSourceType_Undefined) return nullptr;
    return new TerraWaterSource(sourceType, priority, key, name);
}

TerraPowerRail *TerraFactory::newPowerRailObject(Terra_RailType railType, uint32_t key, const TerraString &name)
{
    float voltage = 0.0f;
    switch (railType) {
        case Terra_RailType_DC3V3: voltage = 3.3f; break;
        case Terra_RailType_DC5V: voltage = 5.0f; break;
        case Terra_RailType_DC12V: voltage = 12.0f; break;
        case Terra_RailType_DC24V: voltage = 24.0f; break;
        case Terra_RailType_Custom: break;
        case Terra_RailType_Undefined:
        case Terra_RailType_Count:
        default: return nullptr;
    }
    return new TerraPowerRail(voltage, key, name, railType);
}

TerraObject *TerraFactory::newObjectFromData(const TerraObjectData *dataIn)
{
    if (!dataIn) return nullptr;
    TerraObject *object = nullptr;

    switch (dataIn->objectType) {
        case Terra_ObjectType_Sensor: {
            const TerraSensorData *data = static_cast<const TerraSensorData *>(dataIn);
            TerraSensor *sensor = nullptr;
            if (data->sensorType == Terra_SensorType_Remote) {
                TerraRemoteSensor *remote = new TerraRemoteSensor(data->reportedType, data->unit, data->key, data->name);
                remote->setStaleAfter(data->staleAfterMs ? data->staleAfterMs : TERRA_DEFAULT_REMOTE_STALE_MS);
                sensor = remote;
            } else {
                sensor = newSensorObject(data->sensorType, data->unit, data->key, data->name);
            }
            if (sensor) {
                sensor->setUpdateInterval(data->updateIntervalMs);
                sensor->setMeasurement(0.0f, data->unit, 0, false);
                if (data->sensorCalibrated && data->sensorType == Terra_SensorType_Analog) {
                    if (!static_cast<TerraAnalogSensor *>(sensor)->setCalibration(data->sensorRawMinimum, data->sensorRawMaximum,
                                                                                 data->sensorValueMinimum, data->sensorValueMaximum)) {
                        delete sensor;
                        return nullptr;
                    }
                }
                if (data->hasPinDriver) {
                    SharedPtr<TerraInputDriver> driver;
                    if (data->pinSetup.mode == Terra_PinMode_Analog_Input) {
                        SharedPtr<TerraAnalogInputDriver> analogDriver(new TerraAnalogInputDriver(data->pinSetup.pin, data->unit));
                        if (data->driverCalibrated && !analogDriver->setCalibration(data->driverRawMinimum, data->driverRawMaximum,
                                                                                   data->driverValueMinimum, data->driverValueMaximum)) {
                            delete sensor;
                            return nullptr;
                        }
                        driver = analogDriver;
                    } else if (data->pinSetup.mode == Terra_PinMode_Digital_Input) {
                        driver = SharedPtr<TerraInputDriver>(new TerraDigitalInputDriver(data->pinSetup, data->unit));
                    } else {
                        delete sensor;
                        return nullptr;
                    }
                    sensor->setDriver(driver);
                }
            }
            object = sensor;
        } break;

        case Terra_ObjectType_Actuator: {
            const TerraActuatorData *data = static_cast<const TerraActuatorData *>(dataIn);
            TerraActuator *actuator = newActuatorObject(data->actuatorType, data->key, data->name);
            if (actuator) {
                actuator->setEnableMode(data->enableMode);
                if (data->actuatorType == Terra_ActuatorType_Pump) {
                    static_cast<TerraPump *>(actuator)->setMaxContinuousRuntime(data->maxContinuousMs);
                } else if (data->actuatorType == Terra_ActuatorType_Circulator) {
                    static_cast<TerraCirculator *>(actuator)->setMaxContinuousRuntime(data->maxContinuousMs);
                }
                else if (data->actuatorType == Terra_ActuatorType_SumpPump) {
                    TerraSumpPump *sump = static_cast<TerraSumpPump *>(actuator);
                    sump->setMaxContinuousRuntime(data->maxContinuousMs);
                    if (!sump->configureLevels(data->sumpStartPercent, data->sumpStopPercent, data->sumpAlarmPercent)) {
                        delete actuator;
                        return nullptr;
                    }
                }
                if (data->hasPinDriver) {
                    SharedPtr<TerraOutputDriver> driver;
                    if (data->pinSetup.mode == Terra_PinMode_Digital_Output) {
                        driver = SharedPtr<TerraOutputDriver>(new TerraDigitalOutputDriver(data->pinSetup));
                    } else if (data->pinSetup.mode == Terra_PinMode_Analog_Output) {
                        driver = SharedPtr<TerraOutputDriver>(new TerraAnalogOutputDriver(data->pinSetup.pin, data->maximumRaw));
                    } else {
                        delete actuator;
                        return nullptr;
                    }
                    actuator->setDriver(driver);
                }
            }
            object = actuator;
        } break;

        case Terra_ObjectType_Resource: {
            const TerraResourceData *data = static_cast<const TerraResourceData *>(dataIn);
            TerraResource *resource = newResourceObject(data->resourceType, data->key, data->name);
            if (resource) {
                resource->setThresholds(data->reserveLevel, data->lowLevel, data->highLevel);
                resource->setLevel(data->level);
            }
            object = resource;
        } break;

        case Terra_ObjectType_WaterStorage: {
            const TerraWaterStorageData *data = static_cast<const TerraWaterStorageData *>(dataIn);
            TerraWaterStorage *storage = newWaterStorageObject(data->storageType, data->capacityLiters, data->key, data->name);
            if (storage) {
                storage->setThresholds(data->reserveLevel, data->lowLevel, data->highLevel);
                storage->setLevel(data->level);
                if (data->storageType == Terra_WaterStorageType_Cistern) {
                    static_cast<TerraCistern *>(storage)->configureFillBand(data->fillStartPercent,
                                                                           data->fillStopPercent,
                                                                           data->overflowPercent);
                }
            }
            object = storage;
        } break;

        case Terra_ObjectType_WaterSource: {
            const TerraWaterSourceData *data = static_cast<const TerraWaterSourceData *>(dataIn);
            TerraWaterSource *source = newWaterSourceObject(data->sourceType, data->priority, data->key, data->name);
            if (source) {
                source->setAvailable(data->available);
                source->setLevel(data->level);
                source->setReserveLevel(data->reserveLevel);
                source->setMaximumFlowLpm(data->maximumFlowLpm);
            }
            object = source;
        } break;

        case Terra_ObjectType_WaterRoute: {
            const TerraWaterRouteData *data = static_cast<const TerraWaterRouteData *>(dataIn);
            TerraWaterRoute *route = new TerraWaterRoute(data->key, data->name);
            route->initSourceKey(data->sourceKey);
            route->initDestinationKey(data->destinationKey);
            route->setDestinationBand(data->destinationStartPercent, data->destinationStopPercent);
            route->setMinimumFlow(data->minimumFlowLpm);
            route->setMaximumFlow(data->maximumFlowLpm);
            route->setRouteState(data->routeState);
            object = route;
        } break;

        case Terra_ObjectType_RainCatchment: {
            const TerraRainCatchmentData *data = static_cast<const TerraRainCatchmentData *>(dataIn);
            object = new TerraRainCatchment(data->areaSquareMeters, data->collectionEfficiency, data->key, data->name);
        } break;

        case Terra_ObjectType_ThermalStore: {
            const TerraThermalStoreData *data = static_cast<const TerraThermalStoreData *>(dataIn);
            TerraThermalStore *store = new TerraThermalStore(data->key, data->name);
            store->setThresholds(data->reserveLevel, data->lowLevel, data->highLevel);
            store->setLevel(data->level);
            store->setTargetRange(data->minimumTargetC, data->maximumTargetC);
            store->setAbsoluteMaximum(data->absoluteMaximumC);
            store->setTemperature(data->temperatureC);
            object = store;
        } break;

        case Terra_ObjectType_ThermalLoop: {
            const TerraThermalLoopData *data = static_cast<const TerraThermalLoopData *>(dataIn);
            TerraThermalLoop *loop = new TerraThermalLoop(data->key, data->name);
            loop->configure(data->onDifferentialC, data->offDifferentialC, data->maxStoreTempC);
            object = loop;
        } break;

        case Terra_ObjectType_Environment: {
            const TerraEnvironmentData *data = static_cast<const TerraEnvironmentData *>(dataIn);
            object = new TerraEnvironment(data->key, data->name);
        } break;

        case Terra_ObjectType_PowerRail: {
            const TerraPowerRailData *data = static_cast<const TerraPowerRailData *>(dataIn);
            object = new TerraPowerRail(data->nominalVoltage, data->key, data->name, data->railType);
        } break;

        case Terra_ObjectType_Undefined:
        default: return nullptr;
    }

    applyObjectData(object, dataIn);
    return object;
}

TerraObjectData *TerraFactory::newDataFromObject(const TerraObject *objectIn)
{
    if (!objectIn) return nullptr;
    TerraObjectData *data = nullptr;

    switch (objectIn->getObjectType()) {
        case Terra_ObjectType_Sensor: {
            const TerraSensor *sensor = static_cast<const TerraSensor *>(objectIn);
            TerraSensorData *out = new TerraSensorData();
            out->sensorType = sensor->getSensorType();
            out->reportedType = sensor->getSensorType();
            out->unit = sensor->getMeasurement().unit;
            out->updateIntervalMs = sensor->getUpdateInterval();
            SharedPtr<TerraInputDriver> inputDriver = sensor->getDriver();
            if (inputDriver) {
                out->hasPinDriver = inputDriver->getPinSetup(out->pinSetup);
                out->driverCalibrated = inputDriver->getCalibration(out->driverRawMinimum, out->driverRawMaximum,
                                                                    out->driverValueMinimum, out->driverValueMaximum);
            }
            if (sensor->getSensorType() == Terra_SensorType_Analog) {
                const TerraAnalogSensor *analogSensor = static_cast<const TerraAnalogSensor *>(sensor);
                out->sensorCalibrated = analogSensor->getCalibration(out->sensorRawMinimum, out->sensorRawMaximum,
                                                                      out->sensorValueMinimum, out->sensorValueMaximum);
            }
            if (sensor->getSensorType() == Terra_SensorType_Remote) {
                const TerraRemoteSensor *remote = static_cast<const TerraRemoteSensor *>(sensor);
                out->reportedType = remote->getReportedType();
                out->staleAfterMs = remote->getStaleAfter();
            }
            data = out;
        } break;

        case Terra_ObjectType_Actuator: {
            const TerraActuator *actuator = static_cast<const TerraActuator *>(objectIn);
            TerraActuatorData *out = new TerraActuatorData();
            out->actuatorType = actuator->getActuatorType();
            out->enableMode = actuator->getEnableMode();
            SharedPtr<TerraOutputDriver> outputDriver = actuator->getDriver();
            if (outputDriver) {
                out->hasPinDriver = outputDriver->getPinSetup(out->pinSetup);
                if (out->pinSetup.mode == Terra_PinMode_Analog_Output) {
                    const int maximumRaw = outputDriver->getMaximumRaw();
                    if (maximumRaw > 0) out->maximumRaw = maximumRaw;
                }
            }
            if (actuator->getActuatorType() == Terra_ActuatorType_Pump) {
                out->maxContinuousMs = static_cast<const TerraPump *>(actuator)->getMaxContinuousRuntime();
            } else if (actuator->getActuatorType() == Terra_ActuatorType_Circulator) {
                out->maxContinuousMs = static_cast<const TerraCirculator *>(actuator)->getMaxContinuousRuntime();
            } else if (actuator->getActuatorType() == Terra_ActuatorType_SumpPump) {
                const TerraSumpPump *sump = static_cast<const TerraSumpPump *>(actuator);
                out->maxContinuousMs = sump->getMaxContinuousRuntime();
                out->sumpStartPercent = sump->getStartLevelPercent();
                out->sumpStopPercent = sump->getStopLevelPercent();
                out->sumpAlarmPercent = sump->getAlarmLevelPercent();
            }
            data = out;
        } break;

        case Terra_ObjectType_Resource: {
            const TerraResource *resource = static_cast<const TerraResource *>(objectIn);
            TerraResourceData *out = new TerraResourceData();
            out->resourceType = resource->getType();
            out->level = resource->getLevel();
            out->reserveLevel = resource->getReserveLevel();
            out->lowLevel = resource->getLowLevel();
            out->highLevel = resource->getHighLevel();
            data = out;
        } break;

        case Terra_ObjectType_WaterStorage: {
            const TerraWaterStorage *storage = static_cast<const TerraWaterStorage *>(objectIn);
            TerraWaterStorageData *out;
            if (storage->getStorageType() == Terra_WaterStorageType_Cistern) {
                const TerraCistern *cistern = static_cast<const TerraCistern *>(storage);
                TerraCisternData *cisternOut = new TerraCisternData();
                cisternOut->fillStartPercent = cistern->getFillStartPercent();
                cisternOut->fillStopPercent = cistern->getFillStopPercent();
                cisternOut->overflowPercent = cistern->getOverflowPercent();
                out = cisternOut;
            } else {
                out = new TerraWaterStorageData();
            }
            out->storageType = storage->getStorageType();
            out->resourceType = Terra_ResourceType_Water;
            out->capacityLiters = storage->getCapacityLiters();
            out->level = storage->getLevel();
            out->reserveLevel = storage->getReserveLevel();
            out->lowLevel = storage->getLowLevel();
            out->highLevel = storage->getHighLevel();
            data = out;
        } break;

        case Terra_ObjectType_WaterSource: {
            const TerraWaterSource *source = static_cast<const TerraWaterSource *>(objectIn);
            TerraWaterSourceData *out = new TerraWaterSourceData();
            out->sourceType = source->getType();
            out->priority = source->getPriority();
            out->available = source->isConfiguredAvailable();
            out->level = source->getLevel();
            out->reserveLevel = source->getReserveLevel();
            out->maximumFlowLpm = source->getMaximumFlowLpm();
            data = out;
        } break;

        case Terra_ObjectType_WaterRoute: {
            const TerraWaterRoute *route = static_cast<const TerraWaterRoute *>(objectIn);
            TerraWaterRouteData *out = new TerraWaterRouteData();
            out->sourceKey = route->getSourceKey();
            out->destinationKey = route->getDestinationKey();
            out->destinationStartPercent = route->getDestinationStartPercent();
            out->destinationStopPercent = route->getDestinationStopPercent();
            out->minimumFlowLpm = route->getMinimumFlow();
            out->maximumFlowLpm = route->getMaximumFlow();
            out->routeState = route->getRouteState();
            data = out;
        } break;

        case Terra_ObjectType_RainCatchment: {
            const TerraRainCatchment *catchment = static_cast<const TerraRainCatchment *>(objectIn);
            TerraRainCatchmentData *out = new TerraRainCatchmentData();
            out->areaSquareMeters = catchment->getAreaSquareMeters();
            out->collectionEfficiency = catchment->getCollectionEfficiency();
            data = out;
        } break;

        case Terra_ObjectType_ThermalStore: {
            const TerraThermalStore *store = static_cast<const TerraThermalStore *>(objectIn);
            TerraThermalStoreData *out = new TerraThermalStoreData();
            out->resourceType = Terra_ResourceType_Thermal;
            out->level = store->getLevel();
            out->reserveLevel = store->getReserveLevel();
            out->lowLevel = store->getLowLevel();
            out->highLevel = store->getHighLevel();
            out->temperatureC = store->getTemperature();
            out->minimumTargetC = store->getMinimumTarget();
            out->maximumTargetC = store->getMaximumTarget();
            out->absoluteMaximumC = store->getAbsoluteMaximum();
            data = out;
        } break;

        case Terra_ObjectType_ThermalLoop: {
            const TerraThermalLoop *loop = static_cast<const TerraThermalLoop *>(objectIn);
            TerraThermalLoopData *out = new TerraThermalLoopData();
            out->onDifferentialC = loop->getOnDifferential();
            out->offDifferentialC = loop->getOffDifferential();
            out->maxStoreTempC = loop->getMaxStoreTemperature();
            data = out;
        } break;

        case Terra_ObjectType_Environment:
            data = new TerraEnvironmentData();
            break;

        case Terra_ObjectType_PowerRail: {
            const TerraPowerRail *rail = static_cast<const TerraPowerRail *>(objectIn);
            TerraPowerRailData *out = new TerraPowerRailData();
            out->railType = rail->getRailType();
            out->nominalVoltage = rail->getNominalVoltage();
            data = out;
        } break;

        case Terra_ObjectType_Undefined:
        default: return nullptr;
    }

    terraFillObjectData(objectIn, data);
    return data;
}
