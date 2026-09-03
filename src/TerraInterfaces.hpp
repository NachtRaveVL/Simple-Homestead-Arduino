/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Interfaces
*/

#ifndef TerraInterfaces_HPP
#define TerraInterfaces_HPP

#include "Terraduino.h"

inline Terra_UnitsType TerraFlowRateUnitsInterfaceStorage::getVolumeUnits() const
{
    return baseUnits(getFlowRateUnits());
}

inline Terra_UnitsType TerraMeasurementUnitsInterface::getRateUnits(uint8_t measurementRow) const
{
    return rateUnits(getMeasurementUnits(measurementRow));
}

inline Terra_UnitsType TerraMeasurementUnitsInterface::getBaseUnits(uint8_t measurementRow) const
{
    return baseUnits(getMeasurementUnits(measurementRow));
}

inline Terra_UnitsType TerraVolumeUnitsInterfaceStorage::getFlowRateUnits() const
{
    return rateUnits(_volumeUnits);
}

inline void TerraActuatorObjectInterface::setContinuousPowerUsage(float contPowerUsage, Terra_UnitsType contPowerUsageUnits)
{
    setContinuousPowerUsage(TerraSingleMeasurement(contPowerUsage, contPowerUsageUnits));
}


template <class U>
inline void TerraPumpObjectInterface::setSourceReservoir(U reservoir)
{
    getSourceReservoirAttachment().setObject(reservoir);
}

template <class U>
inline SharedPtr<U> TerraPumpObjectInterface::getSourceReservoir()
{
    return getSourceReservoirAttachment().TerraAttachment::getObject<U>();
}

template <class U>
inline void TerraPumpObjectInterface::setDestinationReservoir(U reservoir)
{
    getDestinationReservoirAttachment().setObject(reservoir);
}

template <class U>
inline SharedPtr<U> TerraPumpObjectInterface::getDestinationReservoir()
{
    return getDestinationReservoirAttachment().TerraAttachment::getObject<U>();
}

inline void TerraPumpObjectInterface::setContinuousFlowRate(float contFlowRate, Terra_UnitsType contFlowRateUnits)
{
    setContinuousFlowRate(TerraSingleMeasurement(contFlowRate, contFlowRateUnits));
}

inline bool TerraPumpObjectInterface::isSourceReservoirEmpty(bool poll)
{
    return getSourceReservoir() && getSourceReservoir()->isEmpty(poll);
}

inline bool TerraPumpObjectInterface::isDestinationReservoirFilled(bool poll)
{
    return getDestinationReservoir() && getDestinationReservoir()->isFilled(poll);
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
inline void TerraParentReservoirAttachmentInterface::setParentReservoir(U reservoir)
{
    getParentReservoirAttachment().setObject(reservoir);
}

template <class U>
inline SharedPtr<U> TerraParentReservoirAttachmentInterface::getParentReservoir()
{
    return getParentReservoirAttachment().TerraAttachment::getObject<U>();
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
inline void TerraWaterReservoirAttachmentInterface::setWaterReservoir(U reservoir)
{
    getWaterReservoirAttachment().setObject(reservoir);
}

template <class U>
inline SharedPtr<U> TerraWaterReservoirAttachmentInterface::getWaterReservoir(bool poll)
{
    getWaterReservoirAttachment().updateIfNeeded(poll);
    return getWaterReservoirAttachment().TerraAttachment::getObject<U>();
}

template <class U>
inline void TerraThermalReservoirAttachmentInterface::setThermalReservoir(U reservoir)
{
    getThermalReservoirAttachment().setObject(reservoir);
}

template <class U>
inline SharedPtr<U> TerraThermalReservoirAttachmentInterface::getThermalReservoir(bool poll)
{
    getThermalReservoirAttachment().updateIfNeeded(poll);
    return getThermalReservoirAttachment().TerraAttachment::getObject<U>();
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
inline void TerraAirCO2SensorAttachmentInterface::setAirCO2Sensor(U sensor)
{
    getAirCO2SensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> TerraAirCO2SensorAttachmentInterface::getAirCO2Sensor(bool poll)
{
    getAirCO2SensorAttachment().updateIfNeeded(poll);
    return getAirCO2SensorAttachment().TerraAttachment::getObject<U>();
}

template <class U>
inline void TerraAirTemperatureSensorAttachmentInterface::setAirTemperatureSensor(U sensor)
{
    getAirTemperatureSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> TerraAirTemperatureSensorAttachmentInterface::getAirTemperatureSensor(bool poll)
{
    getAirTemperatureSensorAttachment().updateIfNeeded(poll);
    return getAirTemperatureSensorAttachment().TerraAttachment::getObject<U>();
}

template <class U>
inline void TerraMediumTemperatureSensorAttachmentInterface::setMediumTemperatureSensor(U sensor)
{
    getMediumTemperatureSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> TerraMediumTemperatureSensorAttachmentInterface::getMediumTemperatureSensor(bool poll)
{
    getMediumTemperatureSensorAttachment().updateIfNeeded(poll);
    return getMediumTemperatureSensorAttachment().TerraAttachment::getObject<U>();
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
inline void TerraWaterFlowRateSensorAttachmentInterface::setFlowRateSensor(U sensor)
{
    getFlowRateSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> TerraWaterFlowRateSensorAttachmentInterface::getFlowRateSensor(bool poll)
{
    getFlowRateSensorAttachment().updateIfNeeded(poll);
    return getFlowRateSensorAttachment().TerraAttachment::getObject<U>();
}

template <class U>
inline void TerraWaterTemperatureSensorAttachmentInterface::setWaterTemperatureSensor(U sensor)
{
    getWaterTemperatureSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> TerraWaterTemperatureSensorAttachmentInterface::getWaterTemperatureSensor(bool poll)
{
    getWaterTemperatureSensorAttachment().updateIfNeeded(poll);
    return getWaterTemperatureSensorAttachment().TerraAttachment::getObject<U>();
}

template <class U>
inline void TerraWaterVolumeSensorAttachmentInterface::setWaterVolumeSensor(U sensor)
{
    getWaterVolumeSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> TerraWaterVolumeSensorAttachmentInterface::getWaterVolumeSensor(bool poll)
{
    getWaterVolumeSensorAttachment().updateIfNeeded(poll);
    return getWaterVolumeSensorAttachment().TerraAttachment::getObject<U>();
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
inline void TerraFilledTriggerAttachmentInterface::setFilledTrigger(U trigger)
{
    getFilledTriggerAttachment().setObject(trigger);
}

template <class U>
inline SharedPtr<U> TerraFilledTriggerAttachmentInterface::getFilledTrigger(bool poll)
{
    getFilledTriggerAttachment().updateIfNeeded(poll);
    return getFilledTriggerAttachment().TerraAttachment::getObject<U>();
}

template <class U>
inline void TerraHighTriggerAttachmentInterface::setHighTrigger(U trigger)
{
    getHighTriggerAttachment().setObject(trigger);
}

template <class U>
inline SharedPtr<U> TerraHighTriggerAttachmentInterface::getHighTrigger(bool poll)
{
    getHighTriggerAttachment().updateIfNeeded(poll);
    return getHighTriggerAttachment().TerraAttachment::getObject<U>();
}

template <class U>
inline void TerraLowTriggerAttachmentInterface::setLowTrigger(U trigger)
{
    getLowTriggerAttachment().setObject(trigger);
}

template <class U>
inline SharedPtr<U> TerraLowTriggerAttachmentInterface::getLowTrigger(bool poll)
{
    getLowTriggerAttachment().updateIfNeeded(poll);
    return getLowTriggerAttachment().TerraAttachment::getObject<U>();
}

template <class U>
inline void TerraEmptyTriggerAttachmentInterface::setEmptyTrigger(U trigger)
{
    getEmptyTriggerAttachment().setObject(trigger);
}

template <class U>
inline SharedPtr<U> TerraEmptyTriggerAttachmentInterface::getEmptyTrigger(bool poll)
{
    getEmptyTriggerAttachment().updateIfNeeded(poll);
    return getEmptyTriggerAttachment().TerraAttachment::getObject<U>();
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
