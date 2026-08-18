/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino System Data
*/

#ifndef TerraData_H
#define TerraData_H

#include "TerraSetup.h"
#include "TerraInterfaces.h"

TerraString terraJsonEscape(const TerraString &value);
bool terraJsonExtractString(const TerraString &json, const char *key, TerraString &out);
bool terraJsonExtractLong(const TerraString &json, const char *key, long &out);
bool terraJsonExtractFloat(const TerraString &json, const char *key, float &out);
bool terraJsonExtractBool(const TerraString &json, const char *key, bool &out);

struct TerraSystemData : public TerraSerializable {
    TerraSystemSetup setup;                                 // Saved setup data
    uint32_t sequence;                                      // Saved system-data sequence

    TerraSystemData() : setup(), sequence(0) { }
    TerraString toJSON() const override;
    bool fromJSON(const TerraString &json);
    size_t binarySize() const;
    size_t toBinary(uint8_t *buffer, size_t capacity) const;
    bool fromBinary(const uint8_t *buffer, size_t length);
};

#endif
