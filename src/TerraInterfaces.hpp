/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Interfaces
*/

#ifndef TerraInterfaces_HPP
#define TerraInterfaces_HPP

#include "Terraduino.h"


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
