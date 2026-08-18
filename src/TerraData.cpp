/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino System Data
*/

#include "TerraData.h"
#include "TerraStrings.h"
#include "TerraUtils.h"
#include <stdlib.h>
#include <ctype.h>

static size_t terraJsonLength(const TerraString &text) {
#if defined(ARDUINO)
    return text.length();
#else
    return text.size();
#endif
}

static long terraJsonIndexOf(const TerraString &text, const TerraString &needle, size_t start = 0) {
#if defined(ARDUINO)
    return text.indexOf(needle, (unsigned int)start);
#else
    size_t pos = text.find(needle, start);
    return pos == TerraString::npos ? -1L : (long)pos;
#endif
}

static TerraString terraJsonSubstr(const TerraString &text, size_t start, size_t length) {
#if defined(ARDUINO)
    return text.substring((unsigned int)start, (unsigned int)(start + length));
#else
    return text.substr(start, length);
#endif
}

static bool terraJsonValueStart(const TerraString &json, const char *key, size_t &start) {
    if (!key) return false;
    TerraString marker = TerraString("\"") + key + "\"";
    long keyPos = terraJsonIndexOf(json, marker);
    if (keyPos < 0) return false;
    size_t i = (size_t)keyPos + terraJsonLength(marker);
    size_t len = terraJsonLength(json);
    while (i < len && isspace((unsigned char)json[i])) ++i;
    if (i >= len || json[i] != ':') return false;
    ++i;
    while (i < len && isspace((unsigned char)json[i])) ++i;
    if (i >= len) return false;
    start = i;
    return true;
}

TerraString terraJsonEscape(const TerraString &value) {
    TerraString out;
    size_t len = terraJsonLength(value);
#if defined(ARDUINO)
    out.reserve(len + 8);
#else
    out.reserve(len + 8);
#endif
    for (size_t i = 0; i < len; ++i) {
        char c = value[i];
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out += c; break;
        }
    }
    return out;
}

bool terraJsonExtractString(const TerraString &json, const char *key, TerraString &out) {
    size_t i = 0;
    if (!terraJsonValueStart(json, key, i) || json[i] != '"') return false;
    ++i;
    size_t len = terraJsonLength(json);
    out = TerraString();
    bool escaped = false;
    for (; i < len; ++i) {
        char c = json[i];
        if (escaped) {
            switch (c) {
                case 'n': out += '\n'; break;
                case 'r': out += '\r'; break;
                case 't': out += '\t'; break;
                case '"': out += '"'; break;
                case '\\': out += '\\'; break;
                default: out += c; break;
            }
            escaped = false;
        } else if (c == '\\') {
            escaped = true;
        } else if (c == '"') {
            return true;
        } else {
            out += c;
        }
    }
    return false;
}

static bool terraJsonExtractNumberToken(const TerraString &json, const char *key, TerraString &token) {
    size_t start = 0;
    if (!terraJsonValueStart(json, key, start)) return false;
    size_t len = terraJsonLength(json);
    size_t end = start;
    while (end < len) {
        char c = json[end];
        if (c == ',' || c == '}' || isspace((unsigned char)c)) break;
        ++end;
    }
    if (end == start) return false;
    token = terraJsonSubstr(json, start, end - start);
    return true;
}

bool terraJsonExtractLong(const TerraString &json, const char *key, long &out) {
    TerraString token;
    if (!terraJsonExtractNumberToken(json, key, token)) return false;
    char *end = nullptr;
    long value = strtol(token.c_str(), &end, 10);
    if (!end || end == token.c_str() || *end != '\0') return false;
    out = value;
    return true;
}

bool terraJsonExtractFloat(const TerraString &json, const char *key, float &out) {
    TerraString token;
    if (!terraJsonExtractNumberToken(json, key, token)) return false;
    char *end = nullptr;
    double value = strtod(token.c_str(), &end);
    if (!end || end == token.c_str() || *end != '\0' || !isfinite(value)) return false;
    out = (float)value;
    return true;
}

bool terraJsonExtractBool(const TerraString &json, const char *key, bool &out) {
    TerraString token;
    if (!terraJsonExtractNumberToken(json, key, token)) return false;
    if (token == TerraString("true") || token == TerraString("1")) { out = true; return true; }
    if (token == TerraString("false") || token == TerraString("0")) { out = false; return true; }
    return false;
}

TerraString TerraSystemData::toJSON() const {
    TerraString json = "{\"systemName\":\"" + terraJsonEscape(setup.systemName) + "\",\"timeZoneHours\":";
#if defined(ARDUINO)
    json += String(setup.timeZoneHours);
    json += ",\"timeZoneMinutes\":" + String(setup.timeZoneMinutes);
    json += ",\"controlMode\":\"" + String(terraControlModeToString(setup.controlMode)) + "\"";
    json += ",\"measurementMode\":\"" + String(terraMeasurementModeToString(setup.measurementMode)) + "\"";
    json += ",\"updateIntervalMs\":" + String(setup.updateIntervalMs);
    json += ",\"loggerMinimumLevel\":\"" + String(terraLogLevelToString(setup.loggerMinimumLevel)) + "\"";
    json += ",\"publisherIntervalMs\":" + String(setup.publisherIntervalMs);
    json += ",\"sequence\":" + String(sequence) + "}";
#else
    json += std::to_string((int)setup.timeZoneHours);
    json += ",\"timeZoneMinutes\":" + std::to_string((int)setup.timeZoneMinutes);
    json += ",\"controlMode\":\"" + TerraString(terraControlModeToString(setup.controlMode)) + "\"";
    json += ",\"measurementMode\":\"" + TerraString(terraMeasurementModeToString(setup.measurementMode)) + "\"";
    json += ",\"updateIntervalMs\":" + std::to_string(setup.updateIntervalMs);
    json += ",\"loggerMinimumLevel\":\"" + TerraString(terraLogLevelToString(setup.loggerMinimumLevel)) + "\"";
    json += ",\"publisherIntervalMs\":" + std::to_string(setup.publisherIntervalMs);
    json += ",\"sequence\":" + std::to_string(sequence) + "}";
#endif
    return json;
}

bool TerraSystemData::fromJSON(const TerraString &json) {
    TerraString name, mode, measurementMode, logLevel;
    long tzh = 0, tzm = 0, interval = 0, publisherInterval = 0, seq = 0;
    if (!terraJsonExtractString(json, "systemName", name)) return false;
    if (!terraJsonExtractString(json, "controlMode", mode)) return false;
    if (!terraJsonExtractString(json, "measurementMode", measurementMode)) return false;
    if (!terraJsonExtractString(json, "loggerMinimumLevel", logLevel)) return false;
    if (!terraJsonExtractLong(json, "timeZoneHours", tzh)) return false;
    if (!terraJsonExtractLong(json, "timeZoneMinutes", tzm)) return false;
    if (!terraJsonExtractLong(json, "updateIntervalMs", interval)) return false;
    if (!terraJsonExtractLong(json, "publisherIntervalMs", publisherInterval)) return false;
    if (!terraJsonExtractLong(json, "sequence", seq)) return false;
    Terra_ControlMode parsedMode = terraControlModeFromString(mode);
    Terra_MeasurementMode parsedMeasurementMode = terraMeasurementModeFromString(measurementMode);
    Terra_LogLevel parsedLogLevel = terraLogLevelFromString(logLevel);
    if (!terraStringEqualsIgnoreCase(mode, terraControlModeToString(parsedMode))) return false;
    if (!terraStringEqualsIgnoreCase(measurementMode, terraMeasurementModeToString(parsedMeasurementMode))) return false;
    if (!terraStringEqualsIgnoreCase(logLevel, terraLogLevelToString(parsedLogLevel))) return false;
    if (tzh < -23 || tzh > 23 || tzm < -59 || tzm > 59) return false;
    if (interval <= 0 || publisherInterval <= 0 || seq < 0) return false;
    setup.systemName = name;
    setup.timeZoneHours = (int8_t)tzh;
    setup.timeZoneMinutes = (int8_t)tzm;
    setup.controlMode = parsedMode;
    setup.measurementMode = parsedMeasurementMode;
    setup.updateIntervalMs = (uint32_t)interval;
    setup.loggerMinimumLevel = parsedLogLevel;
    setup.publisherIntervalMs = (uint32_t)publisherInterval;
    sequence = (uint32_t)seq;
    return true;
}


static void terraWriteU32(uint8_t *buffer, uint32_t value) {
    buffer[0] = (uint8_t)(value & 0xff);
    buffer[1] = (uint8_t)((value >> 8) & 0xff);
    buffer[2] = (uint8_t)((value >> 16) & 0xff);
    buffer[3] = (uint8_t)((value >> 24) & 0xff);
}

static uint32_t terraReadU32(const uint8_t *buffer) {
    return (uint32_t)buffer[0] |
           ((uint32_t)buffer[1] << 8) |
           ((uint32_t)buffer[2] << 16) |
           ((uint32_t)buffer[3] << 24);
}

static uint32_t terraBinaryChecksum(const uint8_t *buffer, size_t length) {
    uint32_t hash = 2166136261UL;
    for (size_t i = 0; i < length; ++i) {
        hash ^= buffer[i];
        hash *= 16777619UL;
    }
    return hash;
}

size_t TerraSystemData::binarySize() const {
#if defined(ARDUINO)
    size_t nameLength = setup.systemName.length();
#else
    size_t nameLength = setup.systemName.size();
#endif
    if (nameLength > TERRA_NAME_MAXSIZE) nameLength = TERRA_NAME_MAXSIZE;
    return 23 + nameLength + 4;
}

size_t TerraSystemData::toBinary(uint8_t *buffer, size_t capacity) const {
    if (!buffer) return 0;
#if defined(ARDUINO)
    size_t nameLength = setup.systemName.length();
#else
    size_t nameLength = setup.systemName.size();
#endif
    if (nameLength > TERRA_NAME_MAXSIZE) nameLength = TERRA_NAME_MAXSIZE;
    const size_t required = 23 + nameLength + 4;
    if (capacity < required) return 0;

    size_t pos = 0;
    buffer[pos++] = 'T'; buffer[pos++] = 'R'; buffer[pos++] = 'D'; buffer[pos++] = 'U';
    buffer[pos++] = 2; // binary format version
    buffer[pos++] = (uint8_t)setup.timeZoneHours;
    buffer[pos++] = (uint8_t)setup.timeZoneMinutes;
    buffer[pos++] = (uint8_t)setup.controlMode;
    buffer[pos++] = (uint8_t)setup.measurementMode;
    buffer[pos++] = (uint8_t)setup.loggerMinimumLevel;
    terraWriteU32(buffer + pos, setup.updateIntervalMs); pos += 4;
    terraWriteU32(buffer + pos, setup.publisherIntervalMs); pos += 4;
    terraWriteU32(buffer + pos, sequence); pos += 4;
    buffer[pos++] = (uint8_t)nameLength;
    for (size_t i = 0; i < nameLength; ++i) buffer[pos++] = (uint8_t)setup.systemName[i];
    uint32_t checksum = terraBinaryChecksum(buffer, pos);
    terraWriteU32(buffer + pos, checksum); pos += 4;
    return pos;
}

bool TerraSystemData::fromBinary(const uint8_t *buffer, size_t length) {
    if (!buffer || length < 27) return false;
    if (buffer[0] != 'T' || buffer[1] != 'R' || buffer[2] != 'D' || buffer[3] != 'U') return false;
    if (buffer[4] != 2) return false;
    uint8_t nameLength = buffer[22];
    const size_t required = 23 + (size_t)nameLength + 4;
    if (nameLength > TERRA_NAME_MAXSIZE || length != required) return false;
    uint32_t expected = terraReadU32(buffer + required - 4);
    if (terraBinaryChecksum(buffer, required - 4) != expected) return false;

    TerraSystemData parsed;
    parsed.setup.timeZoneHours = (int8_t)buffer[5];
    parsed.setup.timeZoneMinutes = (int8_t)buffer[6];
    parsed.setup.controlMode = (Terra_ControlMode)buffer[7];
    parsed.setup.measurementMode = (Terra_MeasurementMode)buffer[8];
    parsed.setup.loggerMinimumLevel = (Terra_LogLevel)buffer[9];
    if (parsed.setup.controlMode < Terra_ControlMode_Manual || parsed.setup.controlMode > Terra_ControlMode_Disabled) return false;
    if (parsed.setup.measurementMode < Terra_MeasurementMode_Imperial || parsed.setup.measurementMode >= Terra_MeasurementMode_Count) return false;
    if (parsed.setup.loggerMinimumLevel < Terra_LogLevel_Debug || parsed.setup.loggerMinimumLevel > Terra_LogLevel_Error) return false;
    parsed.setup.updateIntervalMs = terraReadU32(buffer + 10);
    parsed.setup.publisherIntervalMs = terraReadU32(buffer + 14);
    parsed.sequence = terraReadU32(buffer + 18);
    if (!parsed.setup.updateIntervalMs || !parsed.setup.publisherIntervalMs) return false;
    parsed.setup.systemName = TerraString();
#if defined(ARDUINO)
    parsed.setup.systemName.reserve(nameLength);
#else
    parsed.setup.systemName.reserve(nameLength);
#endif
    for (uint8_t i = 0; i < nameLength; ++i) parsed.setup.systemName += (char)buffer[23 + i];
    *this = parsed;
    return true;
}

