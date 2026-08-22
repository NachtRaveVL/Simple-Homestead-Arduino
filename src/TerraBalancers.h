/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Balancers
*/

#ifndef TerraBalancers_H
#define TerraBalancers_H

#include "TerraAttachments.h"

class TerraWaterRoute;
class TerraWaterSource;
class TerraWaterStorage;
class TerraCistern;
class TerraThermalLoop;
class TerraThermalStore;

// Water Balancer
// Owns the source, destination, pump, and optional flow-sensor attachment points for one
// water route. Normal transfer decisions and actuator control occur during update().
class TerraWaterBalancer {
public:
    TerraWaterBalancer(TerraWaterRoute *route = nullptr);

    void setParent(TerraWaterRoute *route);
    template<class T> inline void setSource(const SharedPtr<T> &source) { _source.setObject(source); }
    template<class T> inline void setDestination(const SharedPtr<T> &destination) { _destination.setObject(destination); }
    template<class T> inline void setPump(const SharedPtr<T> &pump) { _pump.setObject(pump); }
    template<class T> inline void setFlowSensor(const SharedPtr<T> &sensor) { _flowSensor.setObject(sensor); }
    void update(uint32_t now = millis());
    void unresolveAny(TerraObject *object);

    // Selects the usable source with the lowest numeric priority value.
    // Sources at or below their reserve level are not considered usable.
    const TerraWaterSource *selectSource(const TerraWaterSource *const *sources, uint8_t count) const;
    TerraCistern *selectFillCistern(TerraCistern *const *cisterns, uint8_t count) const;
    const TerraCistern *selectSupplyCistern(const TerraCistern *const *cisterns, uint8_t count) const;
    float transferAllowance(const TerraCistern &source, const TerraCistern &destination, float requestedLiters) const;

    TerraAttachment &getSourceAttachment() { return _source; }
    const TerraAttachment &getSourceAttachment() const { return _source; }
    TerraAttachment &getDestinationAttachment() { return _destination; }
    const TerraAttachment &getDestinationAttachment() const { return _destination; }
    TerraActuatorAttachment &getPumpAttachment() { return _pump; }
    const TerraActuatorAttachment &getPumpAttachment() const { return _pump; }
    TerraSensorAttachment &getFlowSensorAttachment() { return _flowSensor; }
    const TerraSensorAttachment &getFlowSensorAttachment() const { return _flowSensor; }

protected:
    TerraWaterRoute *_route;                                // Parent water route, not owned
    TerraAttachment _source;                                // Water source attachment
    TerraAttachment _destination;                           // Destination storage attachment
    TerraActuatorAttachment _pump;                          // Transfer pump attachment
    TerraSensorAttachment _flowSensor;                      // Optional flow sensor attachment
};

// Thermal Balancer
// Owns source-temperature, storage, and circulation attachment points for one thermal
// loop. Differential-temperature circulation is evaluated during update().
class TerraThermalBalancer {
public:
    TerraThermalBalancer(TerraThermalLoop *loop = nullptr);

    void setParent(TerraThermalLoop *loop);
    template<class T> inline void setSourceTemperatureSensor(const SharedPtr<T> &sensor) { _sourceTemperature.setObject(sensor); }
    template<class T> inline void setThermalStore(const SharedPtr<T> &store) { _store.setObject(store); }
    template<class T> inline void setCirculator(const SharedPtr<T> &circulator) { _circulator.setObject(circulator); }
    void update(uint32_t now = millis());
    void unresolveAny(TerraObject *object);

    TerraSensorAttachment &getSourceTemperatureAttachment() { return _sourceTemperature; }
    const TerraSensorAttachment &getSourceTemperatureAttachment() const { return _sourceTemperature; }
    TerraAttachment &getStoreAttachment() { return _store; }
    const TerraAttachment &getStoreAttachment() const { return _store; }
    TerraActuatorAttachment &getCirculatorAttachment() { return _circulator; }
    const TerraActuatorAttachment &getCirculatorAttachment() const { return _circulator; }

protected:
    TerraThermalLoop *_loop;                                // Parent thermal loop, not owned
    TerraSensorAttachment _sourceTemperature;               // Source temperature sensor attachment
    TerraAttachment _store;                                 // Thermal storage attachment
    TerraActuatorAttachment _circulator;                    // Circulation pump attachment
};

#endif
