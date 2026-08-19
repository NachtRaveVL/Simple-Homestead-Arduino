/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Factory
*/

#include "TerraFactory.h"
#include "TerraUtils.h"
#include "TerraSensors.h"
#include "TerraActuators.h"
#include "TerraResource.h"
#include "TerraWater.h"
#include "TerraThermal.h"
#include "TerraEnvironment.h"
#include "TerraRails.h"

static TerraAttachmentSet *terraAttachmentSetForObject(TerraObject *object)
{
    if (!object) return nullptr;
    switch (object->getObjectType()) {
        case Terra_ObjectType_WaterStorage: return &static_cast<TerraWaterStorage *>(object)->attachments();
        case Terra_ObjectType_WaterRoute: return &static_cast<TerraWaterRoute *>(object)->attachments();
        case Terra_ObjectType_RainCatchment: return &static_cast<TerraRainCatchment *>(object)->attachments();
        case Terra_ObjectType_ThermalStore: return &static_cast<TerraThermalStore *>(object)->attachments();
        case Terra_ObjectType_ThermalLoop: return &static_cast<TerraThermalLoop *>(object)->attachments();
        default: return nullptr;
    }
}

static const TerraAttachmentSet *terraAttachmentSetForObject(const TerraObject *object)
{
    return terraAttachmentSetForObject(const_cast<TerraObject *>(object));
}

static void terraApplyObjectData(TerraObject *object, const TerraObjectData *data)
{
    if (!object || !data) return;
    object->setKey(data->key);
    object->setName(data->name);
    object->setEnabled(data->enabled);
    TerraAttachmentSet *attachments = terraAttachmentSetForObject(object);
    if (attachments) {
        for (uint8_t i = 0; i < data->attachmentCount; ++i)
            attachments->attach(data->attachments[i].objectKey, data->attachments[i].role);
    }
}

static void terraFillObjectData(const TerraObject *object, TerraObjectData *data)
{
    if (!object || !data) return;
    data->key = object->getKey();
    data->objectType = object->getObjectType();
    data->name = object->getName();
    data->enabled = object->isEnabled();
    const TerraAttachmentSet *attachments = terraAttachmentSetForObject(object);
    data->attachmentCount = attachments ? attachments->size() : 0;
    for (uint8_t i = 0; i < data->attachmentCount; ++i) {
        const TerraAttachment *attachment = attachments->at(i);
        if (attachment) data->attachments[i] = *attachment;
    }
}

TerraFactory::TerraFactory() : _count(0), _nextKey(1) {
    for (uint8_t i = 0; i < TERRA_MAX_OBJECTS; ++i) _objects[i] = nullptr;
}

bool TerraFactory::registerObject(TerraObject *object) {
    if (!object || _count >= TERRA_MAX_OBJECTS) return false;
    for (uint8_t i = 0; i < _count; ++i) if (_objects[i] == object) return true;
    if (object->getKey() == TERRA_INVALID_KEY) object->setKey(allocateKey(object->getName()));
    if (findObjectByKey(object->getKey())) return false;
    _objects[_count++] = object;
    return true;
}

bool TerraFactory::unregisterObject(TerraObject *object) {
    for (uint8_t i = 0; i < _count; ++i) {
        if (_objects[i] == object) {
            for (uint8_t j = i + 1; j < _count; ++j) _objects[j - 1] = _objects[j];
            _objects[--_count] = nullptr;
            return true;
        }
    }
    return false;
}

TerraObject *TerraFactory::findObjectByKey(uint32_t key) const {
    if (!key) return nullptr;
    for (uint8_t i = 0; i < _count; ++i) if (_objects[i] && _objects[i]->getKey() == key) return _objects[i];
    return nullptr;
}

TerraObject *TerraFactory::findObjectByName(const TerraString &name) const {
    for (uint8_t i = 0; i < _count; ++i) if (_objects[i] && _objects[i]->getName() == name) return _objects[i];
    return nullptr;
}

TerraObject *TerraFactory::findFirstByType(Terra_ObjectType type) const {
    for (uint8_t i = 0; i < _count; ++i) {
        if (_objects[i] && _objects[i]->getObjectType() == type) return _objects[i];
    }
    return nullptr;
}

uint8_t TerraFactory::findByType(Terra_ObjectType type, TerraObject **output, uint8_t capacity) const {
    if (!output || !capacity) return 0;
    uint8_t found = 0;
    for (uint8_t i = 0; i < _count && found < capacity; ++i) {
        if (_objects[i] && _objects[i]->getObjectType() == type) output[found++] = _objects[i];
    }
    return found;
}

uint32_t TerraFactory::allocateKey(const TerraString &name) {
    uint32_t candidate = 0;
#if defined(ARDUINO)
    if (name.length()) candidate = terraHashString(name.c_str());
#else
    if (!name.empty()) candidate = terraHashString(name.c_str());
#endif
    if (candidate && !findObjectByKey(candidate)) return candidate;
    while (!_nextKey || findObjectByKey(_nextKey)) ++_nextKey;
    return _nextKey++;
}

void TerraFactory::updateObjects(uint32_t now) {
    for (uint8_t i = 0; i < _count; ++i) if (_objects[i] && _objects[i]->isEnabled()) _objects[i]->update(now);
}

TerraSensor *TerraFactory::newSensor(Terra_SensorType sensorType, Terra_Unit unit, uint32_t key, const TerraString &name)
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

TerraActuator *TerraFactory::newActuator(Terra_ActuatorType actuatorType, uint32_t key, const TerraString &name)
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

TerraResource *TerraFactory::newResource(Terra_ResourceType resourceType, uint32_t key, const TerraString &name)
{
    if (resourceType == Terra_ResourceType_Undefined) return nullptr;
    return new TerraResource(resourceType, key, name);
}

TerraWaterStorage *TerraFactory::newWaterStorage(Terra_WaterStorageType storageType, float capacityLiters,
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

TerraWaterSource *TerraFactory::newWaterSource(Terra_WaterSourceType sourceType, uint8_t priority,
                                               uint32_t key, const TerraString &name)
{
    if (sourceType == Terra_WaterSourceType_Undefined) return nullptr;
    return new TerraWaterSource(sourceType, priority, key, name);
}

TerraPowerRail *TerraFactory::newPowerRail(Terra_RailType railType, uint32_t key, const TerraString &name)
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
                sensor = newSensor(data->sensorType, data->unit, data->key, data->name);
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
                    TerraInputDriver *driver = nullptr;
                    if (data->pinSetup.mode == Terra_PinMode_Analog_Input) {
                        TerraAnalogInputDriver *analogDriver = new TerraAnalogInputDriver(data->pinSetup.pin, data->unit);
                        if (data->driverCalibrated && !analogDriver->setCalibration(data->driverRawMinimum, data->driverRawMaximum,
                                                                                   data->driverValueMinimum, data->driverValueMaximum)) {
                            delete analogDriver;
                            delete sensor;
                            return nullptr;
                        }
                        driver = analogDriver;
                    } else if (data->pinSetup.mode == Terra_PinMode_Digital_Input) {
                        driver = new TerraDigitalInputDriver(data->pinSetup, data->unit);
                    } else {
                        delete sensor;
                        return nullptr;
                    }
                    sensor->setDriver(driver, true);
                }
            }
            object = sensor;
        } break;

        case Terra_ObjectType_Actuator: {
            const TerraActuatorData *data = static_cast<const TerraActuatorData *>(dataIn);
            TerraActuator *actuator = newActuator(data->actuatorType, data->key, data->name);
            if (actuator) {
                actuator->setEnableMode(data->enableMode);
                if (data->actuatorType == Terra_ActuatorType_Pump)
                    static_cast<TerraPump *>(actuator)->setMaxContinuousRuntime(data->maxContinuousMs);
                else if (data->actuatorType == Terra_ActuatorType_Circulator)
                    static_cast<TerraCirculator *>(actuator)->setMaxContinuousRuntime(data->maxContinuousMs);
                else if (data->actuatorType == Terra_ActuatorType_SumpPump) {
                    TerraSumpPump *sump = static_cast<TerraSumpPump *>(actuator);
                    sump->setMaxContinuousRuntime(data->maxContinuousMs);
                    if (!sump->configureLevels(data->sumpStartPercent, data->sumpStopPercent, data->sumpAlarmPercent)) {
                        delete actuator;
                        return nullptr;
                    }
                }
                if (data->hasPinDriver) {
                    TerraOutputDriver *driver = nullptr;
                    if (data->pinSetup.mode == Terra_PinMode_Digital_Output)
                        driver = new TerraDigitalOutputDriver(data->pinSetup);
                    else if (data->pinSetup.mode == Terra_PinMode_Analog_Output)
                        driver = new TerraAnalogOutputDriver(data->pinSetup.pin, data->maximumRaw);
                    else {
                        delete actuator;
                        return nullptr;
                    }
                    actuator->setDriver(driver, true);
                }
            }
            object = actuator;
        } break;

        case Terra_ObjectType_Resource: {
            const TerraResourceData *data = static_cast<const TerraResourceData *>(dataIn);
            TerraResource *resource = newResource(data->resourceType, data->key, data->name);
            if (resource) {
                resource->setThresholds(data->reserveLevel, data->lowLevel, data->highLevel);
                resource->setLevel(data->level);
            }
            object = resource;
        } break;

        case Terra_ObjectType_WaterStorage: {
            const TerraWaterStorageData *data = static_cast<const TerraWaterStorageData *>(dataIn);
            TerraWaterStorage *storage = newWaterStorage(data->storageType, data->capacityLiters, data->key, data->name);
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
            TerraWaterSource *source = newWaterSource(data->sourceType, data->priority, data->key, data->name);
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
            route->configure(data->sourceKey, data->destinationKey, data->destinationStartPercent, data->destinationStopPercent);
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
            TerraEnvironment *environment = new TerraEnvironment(data->key, data->name);
            environment->setSnapshot(data->weather);
            object = environment;
        } break;

        case Terra_ObjectType_PowerRail: {
            const TerraPowerRailData *data = static_cast<const TerraPowerRailData *>(dataIn);
            object = new TerraPowerRail(data->nominalVoltage, data->key, data->name, data->railType);
        } break;

        case Terra_ObjectType_Undefined:
        default: return nullptr;
    }

    terraApplyObjectData(object, dataIn);
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
            TerraInputDriver *inputDriver = sensor->getDriver();
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
            TerraOutputDriver *outputDriver = actuator->getDriver();
            if (outputDriver) {
                out->hasPinDriver = outputDriver->getPinSetup(out->pinSetup);
                if (out->pinSetup.mode == Terra_PinMode_Analog_Output) {
                    const int maximumRaw = outputDriver->getMaximumRaw();
                    if (maximumRaw > 0) out->maximumRaw = maximumRaw;
                }
            }
            if (actuator->getActuatorType() == Terra_ActuatorType_Pump)
                out->maxContinuousMs = static_cast<const TerraPump *>(actuator)->getMaxContinuousRuntime();
            else if (actuator->getActuatorType() == Terra_ActuatorType_Circulator)
                out->maxContinuousMs = static_cast<const TerraCirculator *>(actuator)->getMaxContinuousRuntime();
            else if (actuator->getActuatorType() == Terra_ActuatorType_SumpPump) {
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

        case Terra_ObjectType_Environment: {
            const TerraEnvironment *environment = static_cast<const TerraEnvironment *>(objectIn);
            TerraEnvironmentData *out = new TerraEnvironmentData();
            out->weather = environment->getSnapshot();
            data = out;
        } break;

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
