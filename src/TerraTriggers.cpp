/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Triggers
*/

#include "TerraTriggers.h"
#include "TerraUtils.h"

TerraThresholdTrigger::TerraThresholdTrigger(float threshold, Terra_Comparison comparison, float hysteresis)
    : _threshold(threshold), _comparison(comparison), _hysteresis(hysteresis < 0.0f ? 0.0f : hysteresis), _state(Terra_TriggerState_Inactive) { }

bool TerraThresholdTrigger::compare(float value, float threshold) const {
    switch (_comparison) {
        case Terra_Comparison_LessThan: return value < threshold;
        case Terra_Comparison_LessOrEqual: return value <= threshold;
        case Terra_Comparison_GreaterThan: return value > threshold;
        case Terra_Comparison_GreaterOrEqual: return value >= threshold;
        case Terra_Comparison_Equal: return isFPEqual(value, threshold);
        case Terra_Comparison_NotEqual: return !isFPEqual(value, threshold);
        default: return false;
    }
}

Terra_TriggerState TerraThresholdTrigger::evaluate(const TerraMeasurement &measurement) {
    if (!measurement.valid || isnan(measurement.value)) return _state = Terra_TriggerState_Fault;
    float threshold = _threshold;
    if (_state == Terra_TriggerState_Active && _hysteresis > 0.0f) {
        if (_comparison == Terra_Comparison_GreaterThan || _comparison == Terra_Comparison_GreaterOrEqual) threshold -= _hysteresis;
        if (_comparison == Terra_Comparison_LessThan || _comparison == Terra_Comparison_LessOrEqual) threshold += _hysteresis;
    }
    _state = compare(measurement.value, threshold) ? Terra_TriggerState_Active : Terra_TriggerState_Inactive;
    return _state;
}

TerraRangeTrigger::TerraRangeTrigger(float low, float high, float hysteresis)
    : _low(low), _high(high), _hysteresis(hysteresis < 0.0f ? 0.0f : hysteresis), _state(Terra_TriggerState_Inactive) { }

Terra_TriggerState TerraRangeTrigger::evaluate(const TerraMeasurement &measurement) {
    if (!measurement.valid || isnan(measurement.value)) return _state = Terra_TriggerState_Fault;
    float low = _low, high = _high;
    if (_state == Terra_TriggerState_Active) { low += _hysteresis; high -= _hysteresis; }
    _state = (measurement.value < low || measurement.value > high) ? Terra_TriggerState_Active : Terra_TriggerState_Inactive;
    return _state;
}
