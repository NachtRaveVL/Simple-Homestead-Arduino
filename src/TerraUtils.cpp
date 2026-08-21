/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Utilities
*/

#include "TerraUtils.h"
#include "TerraLogger.h"
#include <cstring>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>


#ifdef ARDUINO
bool TerraRTCWrapper<RTC_DS1307>::begin(TwoWire *wireInstance)
{
    return _rtc.begin(wireInstance);
}

void TerraRTCWrapper<RTC_DS1307>::adjust(const DateTime &dt)
{
    _rtc.adjust(dt);
}

bool TerraRTCWrapper<RTC_DS1307>::lostPower(void)
{
    return false; // not implemented
}

DateTime TerraRTCWrapper<RTC_DS1307>::now()
{
    return _rtc.now();
}
#endif

extern TerraLogger *getLogger();

static TerraString terraAssertLocation(const char *file, const char *function, int line)
{
    char buffer[160];
    snprintf(buffer, sizeof(buffer), "%s:%d in %s", file ? file : "?", line, function ? function : "?");
    return TerraString(buffer);
}

void terraSoftAssert(bool condition, const TerraString &message, const char *file, const char *function, int line)
{
    if (condition) { return; }
    TerraString output = TerraString("Assertion failure: ") + terraAssertLocation(file, function, line);
#if defined(ARDUINO)
    if (message.length()) { output += TerraString(": ") + message; }
#else
    if (!message.empty()) { output += TerraString(": ") + message; }
#endif
    if (getLogger()) { getLogger()->warning("assert", output); }
}

void terraHardAssert(bool condition, const TerraString &message, const char *file, const char *function, int line)
{
    if (condition) { return; }
    TerraString output = TerraString("Assertion failure HARD: ") + terraAssertLocation(file, function, line);
#if defined(ARDUINO)
    if (message.length()) { output += TerraString(": ") + message; }
#else
    if (!message.empty()) { output += TerraString(": ") + message; }
#endif
    if (getLogger()) { getLogger()->error("assert", output); }
    abort();
}


float terraMapFloat(float value, float inMin, float inMax, float outMin, float outMax)
{
    if (isFPEqual(inMin, inMax)) return outMin;
    float ratio = (value - inMin) / (inMax - inMin);
    return outMin + ratio * (outMax - outMin);
}

uint32_t terraHashString(const char *text)
{
    uint32_t hash = 2166136261UL;
    if (!text) return hash;
    while (*text) {
        hash ^= (uint8_t)*text++;
        hash *= 16777619UL;
    }
    return hash ? hash : 1UL;
}

bool terraStringEqualsIgnoreCase(const TerraString &a, const char *b)
{
    if (!b) return false;
#if defined(ARDUINO)
    size_t len = a.length();
    size_t blen = strlen(b);
    if (len != blen) return false;
    for (size_t i = 0; i < len; ++i) {
        if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i])) return false;
    }
    return true;
#else
    if (a.size() != strlen(b)) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i])) return false;
    }
    return true;
#endif
}

bool terraStringEqualsIgnoreCase(const TerraString &a, const TerraString &b)
{
#if defined(ARDUINO)
    size_t len = a.length();
    if (len != b.length()) return false;
#else
    size_t len = a.size();
    if (len != b.size()) return false;
#endif
    for (size_t i = 0; i < len; ++i) {
        if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i])) return false;
    }
    return true;
}

bool terraStringStartsWithIgnoreCase(const TerraString &text, const TerraString &prefix)
{
#if defined(ARDUINO)
    const size_t textLen = text.length(), prefixLen = prefix.length();
#else
    const size_t textLen = text.size(), prefixLen = prefix.size();
#endif
    if (prefixLen > textLen) return false;
    for (size_t i = 0; i < prefixLen; ++i) {
        if (tolower((unsigned char)text[i]) != tolower((unsigned char)prefix[i])) return false;
    }
    return true;
}

TerraString terraTrim(const TerraString &text)
{
#if defined(ARDUINO)
    TerraString result(text);
    result.trim();
    return result;
#else
    size_t begin = 0, end = text.size();
    while (begin < end && isspace((unsigned char)text[begin])) ++begin;
    while (end > begin && isspace((unsigned char)text[end - 1])) --end;
    return text.substr(begin, end - begin);
#endif
}

TerraString terraSubstring(const TerraString &text, size_t offset)
{
#if defined(ARDUINO)
    return offset < text.length() ? text.substring(offset) : TerraString();
#else
    return offset < text.size() ? text.substr(offset) : TerraString();
#endif
}

bool terraParseLong(const TerraString &text, long &valueOut)
{
    const char *chars = text.c_str();
    if (!chars || !*chars) return false;
    char *end = nullptr;
    long value = strtol(chars, &end, 10);
    if (end == chars || *end != '\0') return false;
    valueOut = value;
    return true;
}

TerraString terraFloatToString(float value, uint8_t decimals)
{
#if defined(ARDUINO)
    return String(value, decimals);
#else
    char buffer[40];
    snprintf(buffer, sizeof(buffer), "%.*f", (int)decimals, value);
    return TerraString(buffer);
#endif
}
