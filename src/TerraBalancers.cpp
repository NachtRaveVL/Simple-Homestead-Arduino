/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Balancers
*/

#include "Terraduino.h"
#include "TerraCoreLogic.h"

TerraBalancer::TerraBalancer(SharedPtr<TerraSensor> sensor, float targetSetpoint, float targetRange, uint8_t measurementRow, int typeIn)
    : type(static_cast<decltype(LinearEdge)>(typeIn)), _sensor(this), _balancingState(Terra_BalancingState_Undefined),
      _targetSetpoint(targetSetpoint), _targetRange(targetRange), _enabled(false)
{
    _sensor.setMeasurementRow(measurementRow);
    _sensor.setHandleMethod(&TerraBalancer::handleMeasurement, this);
    _sensor.initObject(sensor);
}

TerraBalancer::~TerraBalancer()
{
    _enabled = false;
    disableAllActivations();
}

void TerraBalancer::update()
{
    _sensor.updateIfNeeded(true);

    if (_enabled && getController() && getController()->isPollingFrameOld(_sensor.getMeasurementFrame(), TERRA_BALANCER_STALE_FRAMES)) {
        _balancingState = Terra_BalancingState_Undefined;
        disableAllActivations();
    }
}

void TerraBalancer::setTargetSetpoint(float targetSetpoint)
{
    if (!isFPEqual(_targetSetpoint, targetSetpoint)) {
        _targetSetpoint = targetSetpoint;

        _sensor.setNeedsMeasurement();
        bumpRevisionIfNeeded();
    }
}

Terra_BalancingState TerraBalancer::getBalancingState(bool poll)
{
    _sensor.updateIfNeeded(poll);
    return _balancingState;
}

void TerraBalancer::setEnabled(bool enabled)
{
    if (_enabled != enabled) {
        _enabled = enabled;
        if (_enabled) {
            _sensor.setNeedsMeasurement();
        } else {
            _balancingState = Terra_BalancingState_Undefined;
            disableAllActivations();
        }
    }
}

void TerraBalancer::setIncrementActuators(const Vector<TerraActuatorAttachment, TERRA_BAL_ACTUATORS_MAXSIZE> &incActuators)
{
    for (auto attachIter = _incActuators.begin(); attachIter != _incActuators.end(); ++attachIter) {
        bool found = false;
        auto key = attachIter->getKey();

        for (auto attachInIter = incActuators.begin(); attachInIter != incActuators.end(); ++attachInIter) {
            if (key == attachInIter->getKey()) {
                auto activation = *attachInIter;
                activation.setupActivation(attachIter->getActivationSetup());
                if (attachIter->getUpdateSlot()) { activation.setUpdateSlot(*attachIter->getUpdateSlot()); }
                found = true;
                break;
            }
        }

        if (!found) { // disables activations not found in new list, prevents same used actuators from prev cycle from turning off/on on cycle switch
            attachIter->disableActivation();
        }
    }

    {   _incActuators.clear();
        for (auto attachInIter = incActuators.begin(); attachInIter != incActuators.end(); ++attachInIter) {
            _incActuators.push_back(*attachInIter);
            _incActuators.back().setParent(this);
        }
    }
}

void TerraBalancer::setDecrementActuators(const Vector<TerraActuatorAttachment, TERRA_BAL_ACTUATORS_MAXSIZE> &decActuators)
{
    for (auto attachIter = _decActuators.begin(); attachIter != _decActuators.end(); ++attachIter) {
        bool found = false;
        auto key = attachIter->getKey();

        for (auto attachInIter = decActuators.begin(); attachInIter != decActuators.end(); ++attachInIter) {
            if (key == attachInIter->getKey()) {
                auto activation = *attachInIter;
                activation.setupActivation(attachIter->getActivationSetup());
                if (attachIter->getUpdateSlot()) { activation.setUpdateSlot(*attachIter->getUpdateSlot()); }
                found = true;
                break;
            }
        }

        if (!found) { // disables activations not found in new list
            attachIter->disableActivation();
        }
    }

    {   _decActuators.clear();
        for (auto attachInIter = decActuators.begin(); attachInIter != decActuators.end(); ++attachInIter) {
            _decActuators.push_back(*attachInIter);
            _decActuators.back().setParent(this);
        }
    }
}

void TerraBalancer::setMeasurementUnits(Terra_UnitsType measurementUnits, uint8_t)
{
    if (_measurementUnits[0] != measurementUnits) {
        _measurementUnits[0] = measurementUnits;
        //bumpRevisionIfNeeded();
    }
}

Terra_UnitsType TerraBalancer::getMeasurementUnits(uint8_t) const
{
    return definedUnitsElse(_measurementUnits[0], _sensor.getMeasurementUnits());
}

TerraSensorAttachment &TerraBalancer::getSensorAttachment()
{
    return _sensor;
}

Signal<Terra_BalancingState, TERRA_BALANCER_SIGNAL_SLOTS> &TerraBalancer::getBalancingSignal()
{
    return _balancingSignal;
}

void TerraBalancer::disableAllActivations()
{
    for (auto attachIter = _incActuators.begin(); attachIter != _incActuators.end(); ++attachIter) {
        attachIter->disableActivation();
    }
    for (auto attachIter = _decActuators.begin(); attachIter != _decActuators.end(); ++attachIter) {
        attachIter->disableActivation();
    }
}

void TerraBalancer::handleMeasurement(const TerraMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        auto balancingStateBefore = _balancingState;

        auto measure = getAsSingleMeasurement(measurement, getMeasurementRow());
        convertUnits(&measure, getMeasurementUnits(), getMeasurementConvertParam());
        _sensor.setMeasurement(measure);

        if (_enabled) {
            _balancingState = (Terra_BalancingState)terraBalancingStateForValue(measure.value, _targetSetpoint, _targetRange);

            if (_balancingState != balancingStateBefore) {
                #ifdef TERRA_USE_MULTITASKING
                    scheduleSignalFireOnce<Terra_BalancingState>(_balancingSignal, _balancingState);
                #else
                    _balancingSignal.fire(_balancingState);
                #endif
            }
        }
    }
}


TerraLinearEdgeBalancer::TerraLinearEdgeBalancer(SharedPtr<TerraSensor> sensor, float targetSetpoint, float targetRange, float edgeOffset, float edgeLength, uint8_t measurementRow)
    : TerraBalancer(sensor, targetSetpoint, targetRange, measurementRow, LinearEdge), _edgeOffset(edgeOffset), _edgeLength(edgeLength)
{ ; }

void TerraLinearEdgeBalancer::update()
{
    TerraBalancer::update();
    if (!_enabled || !_sensor) {
        disableAllActivations();
        return;
    }

    int correction = terraBalancingCorrectionForState(_balancingState);
    if (correction) {
        auto measure = _sensor.getMeasurement(true);

        float x = fabsf(measure.value - _targetSetpoint);
        float val = _edgeLength > FLT_EPSILON ? mapValue<float>(x, _edgeOffset, _edgeOffset + _edgeLength, 0.0f, 1.0f)
                                              : (x >= _edgeOffset - FLT_EPSILON ? 1.0 : 0.0f);
        val = constrain(val, 0.0f, 1.0f);

        if (correction > 0) {
            for (auto attachIter = _decActuators.begin(); attachIter != _decActuators.end(); ++attachIter) {
                attachIter->disableActivation();
            }
            for (auto attachIter = _incActuators.begin(); attachIter != _incActuators.end(); ++attachIter) {
                attachIter->setupActivation(val * attachIter->getRateMultiplier());
                attachIter->enableActivation();
            }
        } else {
            for (auto attachIter = _incActuators.begin(); attachIter != _incActuators.end(); ++attachIter) {
                attachIter->disableActivation();
            }
            for (auto attachIter = _decActuators.begin(); attachIter != _decActuators.end(); ++attachIter) {
                attachIter->setupActivation(val * attachIter->getRateMultiplier());
                attachIter->enableActivation();
            }
        }
    } else {
        disableAllActivations();
    }
}
