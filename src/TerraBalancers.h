/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Balancers
*/

#ifndef TerraBalancers_H
#define TerraBalancers_H

class TerraBalancer;
class TerraLinearEdgeBalancer;

#include "Terraduino.h"
#include "TerraObject.h"
#include "TerraTriggers.h"

// Balancer Base
// This is the base class for all balancer objects, which are used to modify the external
// environment via a set of actuators that can affect a measured value. Balancers allow
// for a set-point to be used to drive such tasks, with different balancers specializing
// the manner in which they operate.
class TerraBalancer : public TerraSubObject,
                      public TerraBalancerObjectInterface,
                      public TerraMeasurementUnitsInterfaceStorageSingle,
                      public TerraSensorAttachmentInterface {
public:
    const enum : signed char { LinearEdge, Unknown = -1 } type; // Balancer type (custom RTTI)
    inline bool isLinearEdgeType() const { return type == LinearEdge; }
    inline bool isUnknownType() const { return type <= Unknown; }

    TerraBalancer(SharedPtr<TerraSensor> sensor,
                  float targetSetpoint,
                  float targetRange,
                  uint8_t measurementRow,
                  int type = Unknown);
    virtual ~TerraBalancer();

    virtual void update();

    virtual void setTargetSetpoint(float targetSetpoint) override;
    virtual Terra_BalancingState getBalancingState(bool poll = false) override;

    void setIncrementActuators(const Vector<TerraActuatorAttachment, TERRA_BAL_ACTUATORS_MAXSIZE> &incActuators);
    inline const Vector<TerraActuatorAttachment, TERRA_BAL_ACTUATORS_MAXSIZE> &getIncrementActuators() { return _incActuators; }

    void setDecrementActuators(const Vector<TerraActuatorAttachment, TERRA_BAL_ACTUATORS_MAXSIZE> &decActuators);
    inline const Vector<TerraActuatorAttachment, TERRA_BAL_ACTUATORS_MAXSIZE> &getDecrementActuators() { return _decActuators; }

    inline float getTargetSetpoint() const { return _targetSetpoint; }
    inline float getTargetRange() const { return _targetRange; }

    void setEnabled(bool enabled);
    inline bool isEnabled() const { return _enabled; }

    virtual void setMeasurementUnits(Terra_UnitsType measurementUnits, uint8_t = 0) override;
    virtual Terra_UnitsType getMeasurementUnits(uint8_t = 0) const override;

    inline uint8_t getMeasurementRow() const { return _sensor.getMeasurementRow(); }
    inline float getMeasurementConvertParam() const { return _sensor.getMeasurementConvertParam(); }

    virtual TerraSensorAttachment &getSensorAttachment() override;

    Signal<Terra_BalancingState, TERRA_BALANCER_SIGNAL_SLOTS> &getBalancingSignal();

protected:
    TerraSensorAttachment _sensor;                          // Sensor attachment
    Terra_BalancingState _balancingState;                   // Balancing state (last handled)
    float _targetSetpoint;                                  // Target set-point value
    float _targetRange;                                     // Target range value
    bool _enabled;                                          // Enabled flag

    Signal<Terra_BalancingState, TERRA_BALANCER_SIGNAL_SLOTS> _balancingSignal; // Balancing signal

    Vector<TerraActuatorAttachment, TERRA_BAL_ACTUATORS_MAXSIZE> _incActuators; // Increment actuator attachments
    Vector<TerraActuatorAttachment, TERRA_BAL_ACTUATORS_MAXSIZE> _decActuators; // Decrement actuator attachments

    void disableAllActivations();

    void handleMeasurement(const TerraMeasurement *measurement);
};


// Linear Edge Balancer
// A linear edge balancer is a balancer that provides the ability to form high and low
// areas of actuator control either by a vertical edge or a linear-gradient edge that
// interpolates along an edge's length. A vertical edge in this case can be thought of as
// an edge with zero length, which is the default. Useful for fans, heaters, and others.
class TerraLinearEdgeBalancer : public TerraBalancer {
public:
    TerraLinearEdgeBalancer(SharedPtr<TerraSensor> sensor,
                            float targetSetpoint,
                            float targetRange,
                            float edgeOffset = 0,
                            float edgeLength = 0,
                            uint8_t measurementRow = 0);

    virtual void update() override;

    inline float getEdgeOffset() const { return _edgeOffset; }
    inline float getEdgeLength() const { return _edgeLength; }

protected:
    float _edgeOffset;                                      // Edge offset
    float _edgeLength;                                      // Length of edge (0 for non-linear)
};

#endif // /ifndef TerraBalancers_H
