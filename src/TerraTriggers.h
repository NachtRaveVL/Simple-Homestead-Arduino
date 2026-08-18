/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Triggers
*/

#ifndef TerraTriggers_H
#define TerraTriggers_H

#include "TerraMeasurements.h"
#include "TerraTypes.h"

class TerraThresholdTrigger {
public:
    TerraThresholdTrigger(float threshold = 0.0f,
                          Terra_Comparison comparison = Terra_Comparison_GreaterOrEqual,
                          float hysteresis = 0.0f);         // Hysteresis

    Terra_TriggerState evaluate(const TerraMeasurement &measurement);
    Terra_TriggerState getState() const { return _state; }
    void setThreshold(float threshold) { _threshold = threshold; }
    void setHysteresis(float hysteresis) { _hysteresis = hysteresis < 0.0f ? 0.0f : hysteresis; }

protected:
    bool compare(float value, float threshold) const;
    float _threshold;                                       // Trigger threshold
    Terra_Comparison _comparison;                           // Threshold comparison mode
    float _hysteresis;                                      // Trigger hysteresis
    Terra_TriggerState _state;                              // Current trigger/resource state
};

class TerraRangeTrigger {
public:
    TerraRangeTrigger(float low = 0.0f, float high = 1.0f, float hysteresis = 0.0f);
    Terra_TriggerState evaluate(const TerraMeasurement &measurement);
    Terra_TriggerState getState() const { return _state; }

protected:
    float _low;                                             // Low range threshold
    float _high;                                            // High range threshold
    float _hysteresis;                                      // Trigger hysteresis
    Terra_TriggerState _state;                              // Current trigger/resource state
};

#endif
