/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Factory
*/

#include "Terraduino.h"
#include "TerraUtils.h"
#include "TerraSensors.h"
#include "TerraActuators.h"
#include "TerraReservoirs.h"
#include "TerraRails.h"

SharedPtr<TerraRemoteSensor> TerraFactory::addRemoteSensor(Terra_SensorType reportedType, Terra_UnitsType unit)
{
    if (!getController()) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(reportedType));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto sensor = SharedPtr<TerraRemoteSensor>(new TerraRemoteSensor(reportedType, positionIndex, unit));
        if (getController()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

SharedPtr<TerraLeakSensor> TerraFactory::addLeakIndicator(pintype_t inputPin, bool activeLow)
{
    if (!getController() || !isValidPin(inputPin)) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_SensorType_Leak));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto sensor = SharedPtr<TerraLeakSensor>(new TerraLeakSensor(
            positionIndex, TerraDigitalPin(inputPin, Terra_PinMode_Digital_Input, activeLow)));
        if (getController()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

SharedPtr<TerraLevelSensor> TerraFactory::addAnalogLevelSensor(pintype_t inputPin,
                                                               float rawMinimum, float rawMaximum,
                                                               float levelMinimum, float levelMaximum)
{
    if (!getController() || !isValidPin(inputPin) || isFPEqual(rawMinimum, rawMaximum)) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_SensorType_Level));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto sensor = SharedPtr<TerraLevelSensor>(new TerraLevelSensor(
            positionIndex, TerraAnalogPin(inputPin, Terra_PinMode_Analog_Input)));
        if (getController()->registerObject(sensor)) {
            TerraCalibrationData calibration(sensor->getId(), Terra_UnitsType_Percentile_100);
            calibration.setFromTwoPoints(rawMinimum, levelMinimum, rawMaximum, levelMaximum);
            sensor->setUserCalibrationData(&calibration);
            return sensor;
        }
    }

    return nullptr;
}

SharedPtr<TerraTemperatureSensor> TerraFactory::addAnalogTemperatureSensor(pintype_t inputPin,
                                                                           float rawMinimum, float rawMaximum,
                                                                           float temperatureMinimum, float temperatureMaximum,
                                                                           Terra_UnitsType unit)
{
    if (!getController() || !isValidPin(inputPin) || isFPEqual(rawMinimum, rawMaximum)) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_SensorType_Temperature));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto sensor = SharedPtr<TerraTemperatureSensor>(new TerraTemperatureSensor(
            positionIndex, TerraAnalogPin(inputPin, Terra_PinMode_Analog_Input)));
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

SharedPtr<TerraPressureSensor> TerraFactory::addAnalogPressureSensor(pintype_t inputPin,
                                                                     float rawMinimum, float rawMaximum,
                                                                     float pressureMinimum, float pressureMaximum,
                                                                     Terra_UnitsType unit)
{
    if (!getController() || !isValidPin(inputPin) || isFPEqual(rawMinimum, rawMaximum)) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_SensorType_Pressure));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto sensor = SharedPtr<TerraPressureSensor>(new TerraPressureSensor(
            positionIndex, TerraAnalogPin(inputPin, Terra_PinMode_Analog_Input), unit));
        if (getController()->registerObject(sensor)) {
            TerraCalibrationData calibration(sensor->getId(), unit);
            calibration.setFromTwoPoints(rawMinimum, pressureMinimum, rawMaximum, pressureMaximum);
            sensor->setUserCalibrationData(&calibration);
            return sensor;
        }
    }

    return nullptr;
}

SharedPtr<TerraRelayPumpActuator> TerraFactory::addPumpRelay(pintype_t outputPin, bool activeLow)
{
    if (!getController() || !isValidPin(outputPin)) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ActuatorType_Pump));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto actuator = SharedPtr<TerraRelayPumpActuator>(new TerraRelayPumpActuator(
            Terra_ActuatorType_Pump, positionIndex, TerraDigitalPin(outputPin, Terra_PinMode_Digital_Output, activeLow)));
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<TerraRelayPumpActuator> TerraFactory::addSumpPumpRelay(pintype_t outputPin, bool activeLow)
{
    return addPumpRelay(outputPin, activeLow);
}

SharedPtr<TerraRelayPumpActuator> TerraFactory::addCirculatorRelay(pintype_t outputPin, bool activeLow)
{
    if (!getController() || !isValidPin(outputPin)) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ActuatorType_Circulator));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto actuator = SharedPtr<TerraRelayPumpActuator>(new TerraRelayPumpActuator(
            Terra_ActuatorType_Circulator, positionIndex, TerraDigitalPin(outputPin, Terra_PinMode_Digital_Output, activeLow)));
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<TerraRelayPumpActuator> TerraFactory::addValveRelay(pintype_t outputPin, bool activeLow)
{
    if (!getController() || !isValidPin(outputPin)) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ActuatorType_Valve));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto actuator = SharedPtr<TerraRelayPumpActuator>(new TerraRelayPumpActuator(
            Terra_ActuatorType_Valve, positionIndex, TerraDigitalPin(outputPin, Terra_PinMode_Digital_Output, activeLow)));
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<TerraRelayActuator> TerraFactory::addFanRelay(pintype_t outputPin, bool activeLow)
{
    if (!getController() || !isValidPin(outputPin)) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ActuatorType_Fan));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto actuator = SharedPtr<TerraRelayActuator>(new TerraRelayActuator(
            Terra_ActuatorType_Fan, positionIndex, TerraDigitalPin(outputPin, Terra_PinMode_Digital_Output, activeLow)));
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<TerraRelayActuator> TerraFactory::addHeaterRelay(pintype_t outputPin, bool activeLow)
{
    if (!getController() || !isValidPin(outputPin)) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ActuatorType_Heater));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto actuator = SharedPtr<TerraRelayActuator>(new TerraRelayActuator(
            Terra_ActuatorType_Heater, positionIndex, TerraDigitalPin(outputPin, Terra_PinMode_Digital_Output, activeLow)));
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<TerraWaterReservoir> TerraFactory::addWaterReservoir(float maxVolume)
{
    if (!getController()) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ReservoirType_Water));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto reservoir = SharedPtr<TerraWaterReservoir>(new TerraWaterReservoir(positionIndex, maxVolume));
        if (getController()->registerObject(reservoir)) { return reservoir; }
    }

    return nullptr;
}

SharedPtr<TerraThermalReservoir> TerraFactory::addThermalReservoir(float maxTemperature)
{
    if (!getController()) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ReservoirType_Thermal));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto reservoir = SharedPtr<TerraThermalReservoir>(new TerraThermalReservoir(positionIndex, maxTemperature));
        if (getController()->registerObject(reservoir)) { return reservoir; }
    }

    return nullptr;
}

SharedPtr<TerraInfiniteWaterReservoir> TerraFactory::addInfiniteWaterReservoir(bool alwaysFilled)
{
    if (!getController()) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ReservoirType_Water));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto reservoir = SharedPtr<TerraInfiniteWaterReservoir>(new TerraInfiniteWaterReservoir(positionIndex, alwaysFilled));
        if (getController()->registerObject(reservoir)) { return reservoir; }
    }

    return nullptr;
}

SharedPtr<TerraInfiniteThermalReservoir> TerraFactory::addInfiniteThermalReservoir(bool alwaysFilled)
{
    if (!getController()) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ReservoirType_Thermal));

    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        auto reservoir = SharedPtr<TerraInfiniteThermalReservoir>(new TerraInfiniteThermalReservoir(positionIndex, alwaysFilled));
        if (getController()->registerObject(reservoir)) { return reservoir; }
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
        case Terra_ObjectType_Rail:
            return newRailObjectFromData(static_cast<const TerraRailData *>(dataIn));
        default:
            return nullptr;
    }
}
