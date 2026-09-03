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
    TERRA_SOFT_ASSERT((int)reportedType >= 0 && reportedType < Terra_SensorType_Count, SFP(TStr_Err_InvalidParameter));
    TERRA_SOFT_ASSERT(isValidIndex(positionIndex), SFP(TStr_Err_NoPositionsAvailable));

    if ((int)reportedType >= 0 && reportedType < Terra_SensorType_Count && isValidIndex(positionIndex)) {
        auto sensor = SharedPtr<TerraRemoteSensor>(new TerraRemoteSensor(reportedType, positionIndex, unit));
        if (getController()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

SharedPtr<TerraLeakSensor> TerraFactory::addLeakIndicator(pintype_t inputPin, bool activeLow)
{
    if (!getController()) { return nullptr; }
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_SensorType_Leak));
    TERRA_HARD_ASSERT(inputPinIsDigital, SFP(TStr_Err_InvalidPinOrType));
    TERRA_SOFT_ASSERT(isValidIndex(positionIndex), SFP(TStr_Err_NoPositionsAvailable));

    if (inputPinIsDigital && isValidIndex(positionIndex)) {
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
    if (!getController()) { return nullptr; }
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_SensorType_Level));
    TERRA_HARD_ASSERT(inputPinIsAnalog, SFP(TStr_Err_InvalidPinOrType));
    TERRA_SOFT_ASSERT(!isFPEqual(rawMinimum, rawMaximum), SFP(TStr_Err_InvalidParameter));
    TERRA_SOFT_ASSERT(isValidIndex(positionIndex), SFP(TStr_Err_NoPositionsAvailable));

    if (inputPinIsAnalog && !isFPEqual(rawMinimum, rawMaximum) && isValidIndex(positionIndex)) {
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
    if (!getController()) { return nullptr; }
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_SensorType_Temperature));
    TERRA_HARD_ASSERT(inputPinIsAnalog, SFP(TStr_Err_InvalidPinOrType));
    TERRA_SOFT_ASSERT(!isFPEqual(rawMinimum, rawMaximum), SFP(TStr_Err_InvalidParameter));
    TERRA_SOFT_ASSERT(isValidIndex(positionIndex), SFP(TStr_Err_NoPositionsAvailable));

    if (inputPinIsAnalog && !isFPEqual(rawMinimum, rawMaximum) && isValidIndex(positionIndex)) {
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
    if (!getController()) { return nullptr; }
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_SensorType_Pressure));
    TERRA_HARD_ASSERT(inputPinIsAnalog, SFP(TStr_Err_InvalidPinOrType));
    TERRA_SOFT_ASSERT(!isFPEqual(rawMinimum, rawMaximum), SFP(TStr_Err_InvalidParameter));
    TERRA_SOFT_ASSERT(isValidIndex(positionIndex), SFP(TStr_Err_NoPositionsAvailable));

    if (inputPinIsAnalog && !isFPEqual(rawMinimum, rawMaximum) && isValidIndex(positionIndex)) {
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
    if (!getController()) { return nullptr; }
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ActuatorType_Pump));
    TERRA_HARD_ASSERT(outputPinIsDigital, SFP(TStr_Err_InvalidPinOrType));
    TERRA_SOFT_ASSERT(isValidIndex(positionIndex), SFP(TStr_Err_NoPositionsAvailable));

    if (outputPinIsDigital && isValidIndex(positionIndex)) {
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
    if (!getController()) { return nullptr; }
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ActuatorType_Circulator));
    TERRA_HARD_ASSERT(outputPinIsDigital, SFP(TStr_Err_InvalidPinOrType));
    TERRA_SOFT_ASSERT(isValidIndex(positionIndex), SFP(TStr_Err_NoPositionsAvailable));

    if (outputPinIsDigital && isValidIndex(positionIndex)) {
        auto actuator = SharedPtr<TerraRelayPumpActuator>(new TerraRelayPumpActuator(
            Terra_ActuatorType_Circulator, positionIndex, TerraDigitalPin(outputPin, Terra_PinMode_Digital_Output, activeLow)));
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<TerraRelayPumpActuator> TerraFactory::addValveRelay(pintype_t outputPin, bool activeLow)
{
    if (!getController()) { return nullptr; }
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ActuatorType_Valve));
    TERRA_HARD_ASSERT(outputPinIsDigital, SFP(TStr_Err_InvalidPinOrType));
    TERRA_SOFT_ASSERT(isValidIndex(positionIndex), SFP(TStr_Err_NoPositionsAvailable));

    if (outputPinIsDigital && isValidIndex(positionIndex)) {
        auto actuator = SharedPtr<TerraRelayPumpActuator>(new TerraRelayPumpActuator(
            Terra_ActuatorType_Valve, positionIndex, TerraDigitalPin(outputPin, Terra_PinMode_Digital_Output, activeLow)));
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<TerraRelayActuator> TerraFactory::addFanRelay(pintype_t outputPin, bool activeLow)
{
    if (!getController()) { return nullptr; }
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ActuatorType_Fan));
    TERRA_HARD_ASSERT(outputPinIsDigital, SFP(TStr_Err_InvalidPinOrType));
    TERRA_SOFT_ASSERT(isValidIndex(positionIndex), SFP(TStr_Err_NoPositionsAvailable));

    if (outputPinIsDigital && isValidIndex(positionIndex)) {
        auto actuator = SharedPtr<TerraRelayActuator>(new TerraRelayActuator(
            Terra_ActuatorType_Fan, positionIndex, TerraDigitalPin(outputPin, Terra_PinMode_Digital_Output, activeLow)));
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<TerraRelayActuator> TerraFactory::addHeaterRelay(pintype_t outputPin, bool activeLow)
{
    if (!getController()) { return nullptr; }
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ActuatorType_Heater));
    TERRA_HARD_ASSERT(outputPinIsDigital, SFP(TStr_Err_InvalidPinOrType));
    TERRA_SOFT_ASSERT(isValidIndex(positionIndex), SFP(TStr_Err_NoPositionsAvailable));

    if (outputPinIsDigital && isValidIndex(positionIndex)) {
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
    TERRA_SOFT_ASSERT(maxVolume > FLT_EPSILON, SFP(TStr_Err_InvalidParameter));
    TERRA_SOFT_ASSERT(isValidIndex(positionIndex), SFP(TStr_Err_NoPositionsAvailable));

    if (maxVolume > FLT_EPSILON && isValidIndex(positionIndex)) {
        auto reservoir = SharedPtr<TerraWaterReservoir>(new TerraWaterReservoir(positionIndex, maxVolume));
        if (getController()->registerObject(reservoir)) { return reservoir; }
    }

    return nullptr;
}

SharedPtr<TerraThermalReservoir> TerraFactory::addThermalReservoir(float maxTemperature)
{
    if (!getController()) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ReservoirType_Thermal));
    TERRA_SOFT_ASSERT(maxTemperature > FLT_EPSILON, SFP(TStr_Err_InvalidParameter));
    TERRA_SOFT_ASSERT(isValidIndex(positionIndex), SFP(TStr_Err_NoPositionsAvailable));

    if (maxTemperature > FLT_EPSILON && isValidIndex(positionIndex)) {
        auto reservoir = SharedPtr<TerraThermalReservoir>(new TerraThermalReservoir(positionIndex, maxTemperature));
        if (getController()->registerObject(reservoir)) { return reservoir; }
    }

    return nullptr;
}

SharedPtr<TerraInfiniteWaterReservoir> TerraFactory::addInfiniteWaterReservoir(bool alwaysFilled)
{
    if (!getController()) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ReservoirType_Water));
    TERRA_SOFT_ASSERT(isValidIndex(positionIndex), SFP(TStr_Err_NoPositionsAvailable));

    if (isValidIndex(positionIndex)) {
        auto reservoir = SharedPtr<TerraInfiniteWaterReservoir>(new TerraInfiniteWaterReservoir(positionIndex, alwaysFilled));
        if (getController()->registerObject(reservoir)) { return reservoir; }
    }

    return nullptr;
}

SharedPtr<TerraInfiniteThermalReservoir> TerraFactory::addInfiniteThermalReservoir(bool alwaysFilled)
{
    if (!getController()) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(Terra_ReservoirType_Thermal));
    TERRA_SOFT_ASSERT(isValidIndex(positionIndex), SFP(TStr_Err_NoPositionsAvailable));

    if (isValidIndex(positionIndex)) {
        auto reservoir = SharedPtr<TerraInfiniteThermalReservoir>(new TerraInfiniteThermalReservoir(positionIndex, alwaysFilled));
        if (getController()->registerObject(reservoir)) { return reservoir; }
    }

    return nullptr;
}

SharedPtr<TerraSimpleRail> TerraFactory::addSimplePowerRail(Terra_RailType railType, int maxActiveAtOnce)
{
    if (!getController()) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(railType));
    TERRA_SOFT_ASSERT((int)railType >= 0 && railType < Terra_RailType_Count, SFP(TStr_Err_InvalidParameter));
    TERRA_SOFT_ASSERT(maxActiveAtOnce > 0, SFP(TStr_Err_InvalidParameter));
    TERRA_SOFT_ASSERT(isValidIndex(positionIndex), SFP(TStr_Err_NoPositionsAvailable));

    if ((int)railType >= 0 && railType < Terra_RailType_Count && maxActiveAtOnce > 0 && isValidIndex(positionIndex)) {
        auto rail = SharedPtr<TerraSimpleRail>(new TerraSimpleRail(railType, positionIndex, maxActiveAtOnce));
        if (getController()->registerObject(rail)) { return rail; }
    }

    return nullptr;
}

SharedPtr<TerraRegulatedRail> TerraFactory::addRegulatedPowerRail(Terra_RailType railType, float maxPower)
{
    if (!getController()) { return nullptr; }
    tposi_t positionIndex = getController()->firstPositionOpen(TerraIdentity(railType));
    TERRA_SOFT_ASSERT((int)railType >= 0 && railType < Terra_RailType_Count, SFP(TStr_Err_InvalidParameter));
    TERRA_SOFT_ASSERT(maxPower > FLT_EPSILON, SFP(TStr_Err_InvalidParameter));
    TERRA_SOFT_ASSERT(isValidIndex(positionIndex), SFP(TStr_Err_NoPositionsAvailable));

    if ((int)railType >= 0 && railType < Terra_RailType_Count && maxPower > FLT_EPSILON && isValidIndex(positionIndex)) {
        auto rail = SharedPtr<TerraRegulatedRail>(new TerraRegulatedRail(railType, positionIndex, maxPower));
        if (getController()->registerObject(rail)) { return rail; }
    }

    return nullptr;
}
