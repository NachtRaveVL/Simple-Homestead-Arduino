/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Resources
*/

#ifndef TerraResource_H
#define TerraResource_H

#include "TerraObject.h"
#include "TerraCoreLogic.h"

// Resource Base
// Normalized resource level with reserve, low, high, and fault states.
class TerraResource : public TerraObject {
public:
    TerraResource(Terra_ResourceType type = Terra_ResourceType_Undefined,
                  uint32_t key = TERRA_INVALID_KEY,
                  const TerraString &name = TerraString());

    Terra_ResourceType getType() const { return _type; }
    Terra_ResourceState getState() const { return _state; }
    float getLevel() const { return _level; }
    float getReserveLevel() const { return _reserveLevel; }
    float getLowLevel() const { return _lowLevel; }
    float getHighLevel() const { return _highLevel; }

    bool setThresholds(float reserveLevel, float lowLevel, float highLevel);
    void setLevel(float level);
    void setFault(const TerraString &message = TerraString());
    void clearFault();

protected:
    void updateState();
    Terra_ResourceType _type;                               // Resource/source type
    Terra_ResourceState _state;                             // Current trigger/resource state
    float _level;                                           // Normalized level, percent
    float _reserveLevel;                                    // Protected reserve level, percent
    float _lowLevel;                                        // Low threshold, percent
    float _highLevel;                                       // High threshold, percent
};

#endif
