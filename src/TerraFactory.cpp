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

SharedPtr<TerraRemoteSensor> TerraFactory::addRemoteSensor(Terra_SensorType reportedType, Terra_UnitsType unit, const TerraString &name)
{
    if (!getController()) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_SensorType_Remote));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto sensor = SharedPtr<TerraRemoteSensor>(new TerraRemoteSensor(reportedType, positionIndex, unit));
        if (name.length()) { sensor->setName(name); }
        if (getController()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

SharedPtr<TerraLeakSensor> TerraFactory::addLeakIndicator(uint8_t inputPin, bool activeLow, const TerraString &name)
{
    if (!getController() || inputPin == TERRA_INVALID_PIN) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_SensorType_Leak));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto sensor = SharedPtr<TerraLeakSensor>(new TerraLeakSensor(
            positionIndex, TerraDigitalPin(inputPin, Terra_PinMode_Digital_Input, activeLow)));
        if (name.length()) { sensor->setName(name); }
        if (getController()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

SharedPtr<TerraLevelSensor> TerraFactory::addAnalogLevelSensor(uint8_t inputPin,
                                                               float rawMinimum, float rawMaximum,
                                                               float levelMinimum, float levelMaximum,
                                                               const TerraString &name)
{
    if (!getController() || inputPin == TERRA_INVALID_PIN || isFPEqual(rawMinimum, rawMaximum)) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_SensorType_Level));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto sensor = SharedPtr<TerraLevelSensor>(new TerraLevelSensor(
            positionIndex, TerraAnalogPin(inputPin, Terra_PinMode_Analog_Input)));
        if (name.length()) { sensor->setName(name); }
        if (getController()->registerObject(sensor)) {
            TerraCalibrationData calibration(sensor->getId(), Terra_UnitsType_Percentile_100);
            calibration.setFromTwoPoints(rawMinimum, levelMinimum, rawMaximum, levelMaximum);
            sensor->setUserCalibrationData(&calibration);
            return sensor;
        }
    }

    return nullptr;
}

SharedPtr<TerraTemperatureSensor> TerraFactory::addAnalogTemperatureSensor(uint8_t inputPin,
                                                                           float rawMinimum, float rawMaximum,
                                                                           float temperatureMinimum, float temperatureMaximum,
                                                                           Terra_UnitsType unit, const TerraString &name)
{
    if (!getController() || inputPin == TERRA_INVALID_PIN || isFPEqual(rawMinimum, rawMaximum)) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_SensorType_Temperature));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto sensor = SharedPtr<TerraTemperatureSensor>(new TerraTemperatureSensor(
            positionIndex, TerraAnalogPin(inputPin, Terra_PinMode_Analog_Input)));
        if (name.length()) { sensor->setName(name); }
        if (getController()->registerObject(sensor)) {
            TerraCalibrationData calibration(sensor->getId(), unit);
            calibration.setFromTwoPoints(rawMinimum, temperatureMinimum, rawMaximum, temperatureMaximum);
            sensor->setUserCalibrationData(&calibration);
            sensor->setMeasurementUnits(unit);
            return sensor;
        }
    }

    return nullptr;
}

SharedPtr<TerraPressureSensor> TerraFactory::addAnalogPressureSensor(uint8_t inputPin,
                                                                     float rawMinimum, float rawMaximum,
                                                                     float pressureMinimum, float pressureMaximum,
                                                                     Terra_UnitsType unit, const TerraString &name)
{
    if (!getController() || inputPin == TERRA_INVALID_PIN || isFPEqual(rawMinimum, rawMaximum)) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_SensorType_Pressure));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto sensor = SharedPtr<TerraPressureSensor>(new TerraPressureSensor(
            positionIndex, TerraAnalogPin(inputPin, Terra_PinMode_Analog_Input), unit));
        if (name.length()) { sensor->setName(name); }
        if (getController()->registerObject(sensor)) {
            TerraCalibrationData calibration(sensor->getId(), unit);
            calibration.setFromTwoPoints(rawMinimum, pressureMinimum, rawMaximum, pressureMaximum);
            sensor->setUserCalibrationData(&calibration);
            return sensor;
        }
    }

    return nullptr;
}

SharedPtr<TerraPump> TerraFactory::addPumpRelay(uint8_t outputPin, bool activeLow, const TerraString &name)
{
    if (!getController() || outputPin == TERRA_INVALID_PIN) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ActuatorType_Pump));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto actuator = SharedPtr<TerraPump>(new TerraPump(
            positionIndex, TerraDigitalPin(outputPin, Terra_PinMode_Digital_Output, activeLow)));
        if (name.length()) { actuator->setName(name); }
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<TerraSumpPump> TerraFactory::addSumpPumpRelay(uint8_t outputPin, bool activeLow, const TerraString &name)
{
    if (!getController() || outputPin == TERRA_INVALID_PIN) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ActuatorType_SumpPump));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto actuator = SharedPtr<TerraSumpPump>(new TerraSumpPump(
            positionIndex, TerraDigitalPin(outputPin, Terra_PinMode_Digital_Output, activeLow)));
        if (name.length()) { actuator->setName(name); }
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<TerraCirculator> TerraFactory::addCirculatorRelay(uint8_t outputPin, bool activeLow, const TerraString &name)
{
    if (!getController() || outputPin == TERRA_INVALID_PIN) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ActuatorType_Circulator));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto actuator = SharedPtr<TerraCirculator>(new TerraCirculator(
            positionIndex, TerraDigitalPin(outputPin, Terra_PinMode_Digital_Output, activeLow)));
        if (name.length()) { actuator->setName(name); }
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<TerraValve> TerraFactory::addValveRelay(uint8_t outputPin, bool activeLow, const TerraString &name)
{
    if (!getController() || outputPin == TERRA_INVALID_PIN) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ActuatorType_Valve));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto actuator = SharedPtr<TerraValve>(new TerraValve(
            positionIndex, TerraDigitalPin(outputPin, Terra_PinMode_Digital_Output, activeLow)));
        if (name.length()) { actuator->setName(name); }
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<TerraHeater> TerraFactory::addHeaterRelay(uint8_t outputPin, bool activeLow, const TerraString &name)
{
    if (!getController() || outputPin == TERRA_INVALID_PIN) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ActuatorType_Heater));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto actuator = SharedPtr<TerraHeater>(new TerraHeater(
            positionIndex, TerraDigitalPin(outputPin, Terra_PinMode_Digital_Output, activeLow)));
        if (name.length()) { actuator->setName(name); }
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<TerraReservoir> TerraFactory::addReservoir(Terra_ReservoirType reservoirType, const TerraString &name)
{
    if (!getController()) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(reservoirType));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto reservoir = SharedPtr<TerraReservoir>(new TerraReservoir(reservoirType, positionIndex, name));
        if (getController()->registerObject(reservoir)) { return reservoir; }
    }

    return nullptr;
}

SharedPtr<TerraWaterStorage> TerraFactory::addWaterStorage(float capacityLiters, const TerraString &name)
{
    if (!getController()) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ReservoirType_Water));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto reservoir = SharedPtr<TerraWaterStorage>(new TerraWaterStorage(capacityLiters, positionIndex, name));
        if (getController()->registerObject(reservoir)) { return reservoir; }
    }

    return nullptr;
}

SharedPtr<TerraCistern> TerraFactory::addCistern(float capacityLiters, const TerraString &name)
{
    if (!getController()) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ReservoirType_Water));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto reservoir = SharedPtr<TerraCistern>(new TerraCistern(capacityLiters, positionIndex, name));
        if (getController()->registerObject(reservoir)) { return reservoir; }
    }

    return nullptr;
}

SharedPtr<TerraWaterSource> TerraFactory::addWaterSource(uint8_t priority, const TerraString &name)
{
    if (!getController()) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ReservoirType_Water));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto reservoir = SharedPtr<TerraWaterSource>(new TerraWaterSource(priority, positionIndex, name));
        if (getController()->registerObject(reservoir)) { return reservoir; }
    }

    return nullptr;
}

SharedPtr<TerraWaterRoute> TerraFactory::addWaterRoute(const TerraString &name)
{
    if (!getController()) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ObjectType_WaterRoute));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto route = SharedPtr<TerraWaterRoute>(new TerraWaterRoute(positionIndex, name));
        if (getController()->registerObject(route)) { return route; }
    }

    return nullptr;
}

SharedPtr<TerraRainCatchment> TerraFactory::addRainCatchment(float areaSquareMeters,
                                                             float collectionEfficiency,
                                                             const TerraString &name)
{
    if (!getController()) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ObjectType_RainCatchment));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto catchment = SharedPtr<TerraRainCatchment>(new TerraRainCatchment(
            areaSquareMeters, collectionEfficiency, positionIndex, name));
        if (getController()->registerObject(catchment)) { return catchment; }
    }

    return nullptr;
}

SharedPtr<TerraThermalStore> TerraFactory::addThermalStore(const TerraString &name)
{
    if (!getController()) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ReservoirType_Thermal));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto reservoir = SharedPtr<TerraThermalStore>(new TerraThermalStore(positionIndex, name));
        if (getController()->registerObject(reservoir)) { return reservoir; }
    }

    return nullptr;
}

SharedPtr<TerraThermalLoop> TerraFactory::addThermalLoop(const TerraString &name)
{
    if (!getController()) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ObjectType_ThermalLoop));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto loop = SharedPtr<TerraThermalLoop>(new TerraThermalLoop(positionIndex, name));
        if (getController()->registerObject(loop)) { return loop; }
    }

    return nullptr;
}

SharedPtr<TerraEnvironment> TerraFactory::addEnvironment(const TerraString &name)
{
    if (!getController()) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ObjectType_Environment));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto environment = SharedPtr<TerraEnvironment>(new TerraEnvironment(positionIndex, name));
        if (getController()->registerObject(environment)) { return environment; }
    }

    return nullptr;
}

SharedPtr<TerraSimpleRail> TerraFactory::addSimplePowerRail(Terra_RailType railType, int maxActiveAtOnce)
{
    if (!getController()) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(railType));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto rail = SharedPtr<TerraSimpleRail>(new TerraSimpleRail(railType, positionIndex, maxActiveAtOnce));
        if (getController()->registerObject(rail)) { return rail; }
    }

    return nullptr;
}

SharedPtr<TerraRegulatedRail> TerraFactory::addRegulatedPowerRail(Terra_RailType railType, float maxPower)
{
    if (!getController()) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(railType));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto rail = SharedPtr<TerraRegulatedRail>(new TerraRegulatedRail(railType, positionIndex, maxPower));
        if (getController()->registerObject(rail)) { return rail; }
    }

    return nullptr;
}

TerraObject *TerraFactory::newObjectFromData(const TerraObjectData *dataIn)
{
    if (!dataIn || !dataIn->isObjectData()) { return nullptr; }

    switch ((Terra_ObjectType)dataIn->id.object.idType) {
        case Terra_ObjectType_Actuator:
            return newActuatorObjectFromData(static_cast<const TerraActuatorData *>(dataIn));
        case Terra_ObjectType_Sensor:
            return newSensorObjectFromData(static_cast<const TerraSensorData *>(dataIn));
        case Terra_ObjectType_Reservoir:
            return newReservoirObjectFromData(static_cast<const TerraReservoirData *>(dataIn));
        case Terra_ObjectType_WaterRoute:
            return dataIn->id.object.classType == (tid_t)TerraWaterRoute::Route
                ? new TerraWaterRoute(static_cast<const TerraWaterRouteData *>(dataIn)) : nullptr;
        case Terra_ObjectType_RainCatchment:
            return dataIn->id.object.classType == (tid_t)TerraRainCatchment::Catchment
                ? new TerraRainCatchment(static_cast<const TerraRainCatchmentData *>(dataIn)) : nullptr;
        case Terra_ObjectType_ThermalLoop:
            return dataIn->id.object.classType == (tid_t)TerraThermalLoop::Loop
                ? new TerraThermalLoop(static_cast<const TerraThermalLoopData *>(dataIn)) : nullptr;
        case Terra_ObjectType_Environment:
            return dataIn->id.object.classType == (tid_t)TerraEnvironment::Standard
                ? new TerraEnvironment(static_cast<const TerraEnvironmentData *>(dataIn)) : nullptr;
        case Terra_ObjectType_Rail:
            return newRailObjectFromData(static_cast<const TerraRailData *>(dataIn));
        default:
            return nullptr;
    }
}
