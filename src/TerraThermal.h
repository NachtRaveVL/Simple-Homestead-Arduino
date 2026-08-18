/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Thermal Management
*/

#ifndef TerraThermal_H
#define TerraThermal_H

#include "TerraResource.h"
#include "TerraAttachments.h"

// Thermal Store
// Tracks stored heat temperature and normal/safety target ranges.
class TerraThermalStore : public TerraResource {
public:
    TerraThermalStore(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString());

    void setTemperature(float celsius) { _temperatureC = celsius; }
    float getTemperature() const { return _temperatureC; }
    bool setTargetRange(float minimumC, float maximumC);
    bool setAbsoluteMaximum(float maximumC);
    float getMinimumTarget() const { return _minimumTargetC; }
    float getMaximumTarget() const { return _maximumTargetC; }
    float getAbsoluteMaximum() const { return _absoluteMaximumC; }
    bool needsHeat() const { return _temperatureC < _minimumTargetC; }
    bool isOverTemperature() const { return _temperatureC > _maximumTargetC; }
    bool isSafetyLimitExceeded() const { return _temperatureC >= _absoluteMaximumC; }
    TerraAttachmentSet &attachments() { return _attachments; }
    const TerraAttachmentSet &attachments() const { return _attachments; }

protected:
    float _temperatureC;                                    // Current temperature, degrees Celsius
    float _minimumTargetC;                                  // Minimum target temperature
    float _maximumTargetC;                                  // Maximum target temperature
    float _absoluteMaximumC;                                // Absolute temperature safety limit
    TerraAttachmentSet _attachments;                        // Object attachment relationships
};

// Thermal Loop
// Differential circulation state for moving heat between a source and storage.
class TerraThermalLoop : public TerraObject {
public:
    TerraThermalLoop(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString());

    bool configure(float onDifferentialC, float offDifferentialC, float maxStoreTempC);
    bool shouldCirculate(float sourceTempC, float storeTempC) const;
    void setRunning(bool running) { _running = running && _enabled && !_fault; }
    void setEnabled(bool enabled) override { if (!enabled) _running = false; TerraObject::setEnabled(enabled); }
    bool isRunning() const { return _running; }
    float getOnDifferential() const { return _onDifferentialC; }
    float getOffDifferential() const { return _offDifferentialC; }
    float getMaxStoreTemperature() const { return _maxStoreTempC; }
    TerraAttachmentSet &attachments() { return _attachments; }
    const TerraAttachmentSet &attachments() const { return _attachments; }

protected:
    float _onDifferentialC;                                 // Circulation-on temperature differential
    float _offDifferentialC;                                // Circulation-off temperature differential
    float _maxStoreTempC;                                   // Maximum allowed storage temperature
    bool _running;                                          // Current circulation state
    TerraAttachmentSet _attachments;                        // Object attachment relationships
};

#endif
