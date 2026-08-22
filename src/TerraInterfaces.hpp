/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Interfaces
*/

#ifndef TerraInterfaces_HPP
#define TerraInterfaces_HPP

#include "Terraduino.h"

inline void TerraDistanceUnitsInterfaceStorage::setSpeedUnits(Terra_UnitsType speedUnits)
{
    setDistanceUnits(baseUnits(speedUnits));
}

inline Terra_UnitsType TerraDistanceUnitsInterfaceStorage::getSpeedUnits() const
{
    return rateUnits(getDistanceUnits());
}

inline Terra_UnitsType TerraMeasurementUnitsInterface::getRateUnits(uint8_t measurementRow) const
{
    return rateUnits(getMeasurementUnits(measurementRow));
}

inline Terra_UnitsType TerraMeasurementUnitsInterface::getBaseUnits(uint8_t measurementRow) const
{
    return baseUnits(getMeasurementUnits(measurementRow));
}


inline void TerraActuatorObjectInterface::setContinuousPowerUsage(float contPowerUsage, Terra_UnitsType contPowerUsageUnits)
{
    setContinuousPowerUsage(TerraSingleMeasurement(contPowerUsage, contPowerUsageUnits));
}


inline void TerraMotorObjectInterface::setContinuousSpeed(float contSpeed, Terra_UnitsType contSpeedUnits)
{
    setContinuousSpeed(TerraSingleMeasurement(contSpeed, contSpeedUnits));
}


template <class U>
inline void TerraParentActuatorAttachmentInterface::setParentActuator(U actuator)
{
    getParentActuatorAttachment().setObject(actuator);
}

template <class U>
inline SharedPtr<U> TerraParentActuatorAttachmentInterface::getParentActuator()
{
    return getParentActuatorAttachment().TerraAttachment::getObject<U>();
}

template <class U>
inline void TerraParentSensorAttachmentInterface::setParentSensor(U sensor)
{
    getParentSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> TerraParentSensorAttachmentInterface::getParentSensor()
{
    return getParentSensorAttachment().TerraAttachment::getObject<U>();
}

template <class U>
inline void TerraParentReservoirAttachmentInterface::setParentReservoir(U panel, tposi_t axisIndex)
{
    getParentReservoirAttachment().setObject(panel);
    getParentReservoirAttachment().setParentSubIndex(axisIndex);
}

template <class U>
inline SharedPtr<U> TerraParentReservoirAttachmentInterface::getParentReservoir()
{
    return getParentReservoirAttachment().TerraAttachment::getObject<U>();
}

inline tposi_t TerraParentReservoirAttachmentInterface::getParentReservoirAxisIndex()
{
    getParentReservoirAttachment().getParentSubIndex();
}

template <class U>
inline void TerraParentRailAttachmentInterface::setParentRail(U rail)
{
    getParentRailAttachment().setObject(rail);
}

template <class U>
inline SharedPtr<U> TerraParentRailAttachmentInterface::getParentRail()
{
    return getParentRailAttachment().TerraAttachment::getObject<U>();
}


template <class U>
inline void TerraSensorAttachmentInterface::setSensor(U sensor)
{
    getSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> TerraSensorAttachmentInterface::getSensor(bool poll)
{
    getSensorAttachment().updateIfNeeded(poll);
    return getSensorAttachment().TerraAttachment::getObject<U>();
}

template <class U>
inline void TerraAngleSensorAttachmentInterface::setAngleSensor(U sensor)
{
    getAngleSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> TerraAngleSensorAttachmentInterface::getAngleSensor(bool poll)
{
    getAngleSensorAttachment().updateIfNeeded(poll);
    return getAngleSensorAttachment().TerraAttachment::getObject<U>();
}

template <class U>
inline void TerraPositionSensorAttachmentInterface::setPositionSensor(U sensor)
{
    getPositionSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> TerraPositionSensorAttachmentInterface::getPositionSensor(bool poll)
{
    getPositionSensorAttachment().updateIfNeeded(poll);
    return getPositionSensorAttachment().TerraAttachment::getObject<U>();
}

template <class U>
inline void TerraPowerProductionSensorAttachmentInterface::setPowerProductionSensor(U sensor)
{
    getPowerProductionSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> TerraPowerProductionSensorAttachmentInterface::getPowerProductionSensor(bool poll)
{
    getPowerProductionSensorAttachment().updateIfNeeded(poll);
    return getPowerProductionSensorAttachment().TerraAttachment::getObject<U>();
}

template <class U>
inline void TerraPowerUsageSensorAttachmentInterface::setPowerUsageSensor(U sensor)
{
    getPowerUsageSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> TerraPowerUsageSensorAttachmentInterface::getPowerUsageSensor(bool poll)
{
    getPowerUsageSensorAttachment().updateIfNeeded(poll);
    return getPowerUsageSensorAttachment().TerraAttachment::getObject<U>();
}

template <class U>
inline void TerraSpeedSensorAttachmentInterface::setSpeedSensor(U sensor)
{
    getSpeedSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> TerraSpeedSensorAttachmentInterface::getSpeedSensor(bool poll)
{
    getSpeedSensorAttachment().updateIfNeeded(poll);
    return getSpeedSensorAttachment().TerraAttachment::getObject<U>();
}

template <class U>
inline void TerraTemperatureSensorAttachmentInterface::setTemperatureSensor(U sensor)
{
    getTemperatureSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> TerraTemperatureSensorAttachmentInterface::getTemperatureSensor(bool poll)
{
    getTemperatureSensorAttachment().updateIfNeeded(poll);
    return getTemperatureSensorAttachment().TerraAttachment::getObject<U>();
}

template <class U>
inline void TerraWindSpeedSensorAttachmentInterface::setWindSpeedSensor(U sensor)
{
    getWindSpeedSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> TerraWindSpeedSensorAttachmentInterface::getWindSpeedSensor(bool poll)
{
    getWindSpeedSensorAttachment().updateIfNeeded(poll);
    return getWindSpeedSensorAttachment().TerraAttachment::getObject<U>();
}


template <class U>
inline void TerraTriggerAttachmentInterface::setTrigger(U trigger)
{
    getTriggerAttachment().setObject(trigger);
}

template <class U>
inline SharedPtr<U> TerraTriggerAttachmentInterface::getTrigger(bool poll)
{
    getTriggerAttachment().updateIfNeeded(poll);
    return getTriggerAttachment().TerraAttachment::getObject<U>();
}

template <class U>
inline void TerraMinimumTriggerAttachmentInterface::setMinimumTrigger(U trigger)
{
    getMinimumTriggerAttachment().setObject(trigger);
}

template <class U>
inline SharedPtr<U> TerraMinimumTriggerAttachmentInterface::getMinimumTrigger(bool poll)
{
    getMinimumTriggerAttachment().updateIfNeeded(poll);
    return getMinimumTriggerAttachment().TerraAttachment::getObject<U>();
}

template <class U>
inline void TerraMaximumTriggerAttachmentInterface::setMaximumTrigger(U trigger)
{
    getMaximumTriggerAttachment().setObject(trigger);
}

template <class U>
inline SharedPtr<U> TerraMaximumTriggerAttachmentInterface::getMaximumTrigger(bool poll)
{
    getMaximumTriggerAttachment().updateIfNeeded(poll);
    return getMaximumTriggerAttachment().TerraAttachment::getObject<U>();
}

template <class U>
inline void TerraLimitTriggerAttachmentInterface::setLimitTrigger(U trigger)
{
    getLimitTriggerAttachment().setObject(trigger);
}

template <class U>
inline SharedPtr<U> TerraLimitTriggerAttachmentInterface::getLimitTrigger(bool poll)
{
    getLimitTriggerAttachment().updateIfNeeded(poll);
    return getLimitTriggerAttachment().TerraAttachment::getObject<U>();
}

#endif // /ifndef TerraInterfaces_HPP
