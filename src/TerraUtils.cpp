/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Utilities
*/

#include "Terraduino.h"
#include <pins_arduino.h>

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


#ifdef TERRA_USE_MULTITASKING

BasicArduinoInterruptAbstraction interruptImpl;

#endif // /ifdef TERRA_USE_MULTITASKING


#ifdef TERRA_USE_DEBUG_ASSERTIONS

static String fileFromFullPath(String fullPath)
{
    int index = fullPath.lastIndexOf(TERRA_BLDPATH_SEPARATOR);
    return index != -1 ? fullPath.substring(index+1) : fullPath;
}

static String makeAssertMsg(const char *file, const char *func, int line)
{
    String retVal;

    retVal.concat(SFP(TStr_ColonSpace));
    retVal.concat(fileFromFullPath(String(file)));
    retVal.concat(':');
    retVal.concat(line);
    retVal.concat(F(" in "));
    retVal.concat(func);
    retVal.concat(SFP(TStr_ColonSpace));

    return retVal;
}

void softAssert(bool cond, String msg, const char *file, const char *func, int line)
{
    if (!cond) {
        if (getLogger()) {
            getLogger()->logWarning(SFP(TStr_Err_AssertionFailure), makeAssertMsg(file, func, line), msg);
            getLogger()->flush();
        }
        #ifdef TERRA_ENABLE_DEBUG_OUTPUT
            else if (Serial) {
                Serial.print(localNow().timestamp(DateTime::TIMESTAMP_FULL));
                Serial.print(' ');
                Serial.print(SFP(TStr_Log_Prefix_Warning));
                Serial.print(SFP(TStr_Err_AssertionFailure));
                Serial.print(makeAssertMsg(file, func, line));
                Serial.println(msg);
                Serial.flush(); yield();
            }
        #endif
    }
}

void hardAssert(bool cond, String msg, const char *file, const char *func, int line)
{
    if (!cond) {
        String assertFail = SFP(TStr_Err_AssertionFailure);
        assertFail.concat(F(" HARD"));
        if (getLogger()) {
            getLogger()->logError(assertFail, makeAssertMsg(file, func, line), msg);
            getLogger()->flush();
        }
        #ifdef TERRA_ENABLE_DEBUG_OUTPUT
            else if (Serial) {
                Serial.print(localNow().timestamp(DateTime::TIMESTAMP_FULL));
                Serial.print(' ');
                Serial.print(SFP(TStr_Log_Prefix_Error));
                Serial.print(assertFail);
                Serial.print(makeAssertMsg(file, func, line));
                Serial.println(msg);
                Serial.flush(); yield();
            }
        #endif

        if (getController()) { getController()->suspend(); }
        yield(); delay(10);
        abort();
    }
}

#endif // /ifdef TERRA_USE_DEBUG_ASSERTIONS


void publishData(TerraSensor *sensor)
{
    TERRA_HARD_ASSERT(sensor, SFP(TStr_Err_InvalidParameter));

    if (getPublisher()) {
        auto measurement = sensor->getMeasurement();
        tposi_t rows = getMeasurementRowCount(measurement);
        tposi_t columnIndexStart = getPublisher()->getColumnIndexStart(sensor->getKey());

        if (columnIndexStart >= 0) {
            for (uint8_t measurementRow = 0; measurementRow < rows; ++measurementRow) {
                getPublisher()->publishData(columnIndexStart + measurementRow, getAsSingleMeasurement(measurement, measurementRow));
            }
        }
    }
}

void _setUnixTime(DateTime unixTime, bool isSigTime)
{
    time_t prevTime = unixNow();

    auto rtc = getController() ? getController()->getRTC() : nullptr;
    if (rtc) {
        rtc->adjust(unixTime);
        getController()->notifyRTCTimeUpdated();
    } else {
        setTime(unixTime.unixtime());
    }

    if (getController() && (isSigTime ||
        getLogger()->getSystemInit() <= SECS_YR_2000 ||
        abs(prevTime - unixTime.unixtime()) >= SECS_PER_DAY)) {
        getController()->notifySignificantTime(unixTime.unixtime());
    }
}

String getYYMMDDFilename(String prefix, String ext)
{
    DateTime currTime = localNow();
    uint8_t yy = currTime.year() % 100;
    uint8_t mm = currTime.month();
    uint8_t dd = currTime.day();

    String retVal; retVal.reserve(prefix.length() + 10 + 1);

    retVal.concat(prefix);
    if (yy < 10) { retVal.concat('0'); }
    retVal.concat(yy);
    if (mm < 10) { retVal.concat('0'); }
    retVal.concat(mm);
    if (dd < 10) { retVal.concat('0'); }
    retVal.concat(dd);
    retVal.concat('.');
    retVal.concat(ext);

    return retVal;
}

String getNNFilename(String prefix, unsigned int value, String ext)
{
    String retVal; retVal.reserve(prefix.length() + 6 + 1);

    retVal.concat(prefix);
    if (value < 10) { retVal.concat('0'); }
    retVal.concat(value);
    retVal.concat('.');
    retVal.concat(ext);

    return retVal;
}

void createDirectoryFor(SDClass *sd, String filename)
{
    auto slashIndex = filename.indexOf(TERRA_FSPATH_SEPARATOR);
    String directory = slashIndex != -1 ? filename.substring(0, slashIndex) : String();
    String dirWithSep = directory + String(TERRA_FSPATH_SEPARATOR);
    if (directory.length() && !sd->exists(dirWithSep.c_str())) {
        sd->mkdir(directory.c_str());
    }
}

tkey_t stringHash(String string)
{
    tkey_t hash = 5381;
    for(int index = 0; index < string.length(); ++index) {
        hash = ((hash << 5) + hash) + (tkey_t)string[index]; // Good 'ol DJB2
    }
    return hash != tkey_none ? hash : 5381;
}

String addressToString(uintptr_t addr)
{
    String retVal; retVal.reserve((2 * sizeof(void*)) + 2 + 1);
    if (addr == (uintptr_t)-1) { addr = 0; }
    retVal.concat('0'); retVal.concat('x');

    if (sizeof(void*) >= 4) {
        if (addr < 0x10000000) { retVal.concat('0'); }
        if (addr <  0x1000000) { retVal.concat('0'); }
        if (addr <   0x100000) { retVal.concat('0'); }
        if (addr <    0x10000) { retVal.concat('0'); }
    }
    if (sizeof(void*) >= 2) {
        if (addr <     0x1000) { retVal.concat('0'); }
        if (addr <      0x100) { retVal.concat('0'); }
    }
    if (sizeof(void*) >= 1) {
        if (addr <       0x10) { retVal.concat('0'); }
    }

    retVal.concat(String((unsigned long)addr, 16));

    return retVal;
}

String charsToString(const char *charsIn, size_t length)
{
    if (!charsIn || !length) { return String(SFP(TStr_null)); }
    String retVal; retVal.reserve(length + 1);
    for (size_t index = 0; index < length && charsIn[index] != '\000'; ++index) {
        retVal.concat(charsIn[index]);
    }
    return retVal.length() ? retVal : String(SFP(TStr_null));
}

String timeSpanToString(const TimeSpan &span)
{
    String retVal; retVal.reserve(15 + 1);

    if (span.days()) {
        retVal.concat(span.days());
        retVal.concat('d');
    }
    if (span.hours()) {
        if (retVal.length()) { retVal.concat(' '); }
        retVal.concat(span.hours());
        retVal.concat('h');
    }
    if (span.minutes()) {
        if (retVal.length()) { retVal.concat(' '); }
        retVal.concat(span.minutes());
        retVal.concat('m');
    }
    if (span.seconds()) {
        if (retVal.length()) { retVal.concat(' '); }
        retVal.concat(span.seconds());
        retVal.concat('s');
    }

    return retVal;
}

extern String measurementToString(float value, Terra_UnitsType units, unsigned int additionalDecPlaces)
{
    String retVal; retVal.reserve(15 + 1);
    retVal.concat(roundToString(value, additionalDecPlaces));

    String unitsSym = unitsTypeToSymbol(units, true); // also excludes dimensionless, e.g. pH
    if (unitsSym.length()) {
        retVal.concat(' ');
        retVal.concat(unitsSym);
    }

    return retVal;
}

template<>
String commaStringFromArray<float>(const float *arrayIn, size_t length)
{
    if (!arrayIn || !length) { return String(SFP(TStr_null)); }
    String retVal; retVal.reserve(length << 1 + length >> 1 + 1);
    for (size_t index = 0; index < length; ++index) {
        if (retVal.length()) { retVal.concat(','); }

        String floatString = String(arrayIn[index], 6);
        int trimIndex = floatString.length() - 1;

        while (floatString[trimIndex] == '0' && trimIndex > 0) { trimIndex--; }
        if (floatString[trimIndex] == '.' && trimIndex > 0) { trimIndex--; }
        if (trimIndex < floatString.length() - 1) {
            floatString = floatString.substring(0, trimIndex+1);
        }

        retVal += floatString;
    }
    return retVal.length() ? retVal : String(SFP(TStr_null));
}

template<>
String commaStringFromArray<double>(const double *arrayIn, size_t length)
{
    if (!arrayIn || !length) { return String(SFP(TStr_null)); }
    String retVal; retVal.reserve(length << 1 + length >> 1 + 1);
    for (size_t index = 0; index < length; ++index) {
        if (retVal.length()) { retVal.concat(','); }

        String doubleString = String(arrayIn[index], 14);
        int trimIndex = doubleString.length() - 1;

        while (doubleString[trimIndex] == '0' && trimIndex > 0) { trimIndex--; }
        if (doubleString[trimIndex] == '.' && trimIndex > 0) { trimIndex--; }
        if (trimIndex < doubleString.length() - 1) {
            doubleString = doubleString.substring(0, trimIndex+1);
        }

        retVal += doubleString;
    }
    return retVal.length() ? retVal : String(SFP(TStr_null));
}

template<>
void commaStringToArray<float>(String stringIn, float *arrayOut, size_t length)
{
    if (!stringIn.length() || !length || stringIn.equalsIgnoreCase(SFP(TStr_null))) { return; }
    int lastSepPos = -1;
    for (size_t index = 0; index < length; ++index) {
        int nextSepPos = stringIn.indexOf(',', lastSepPos+1);
        if (nextSepPos == -1) { nextSepPos = stringIn.length(); }
        String subString = stringIn.substring(lastSepPos+1, nextSepPos);
        if (nextSepPos < stringIn.length()) { lastSepPos = nextSepPos; }

        arrayOut[index] = subString.toFloat();
    }
}

template<>
void commaStringToArray<double>(String stringIn, double *arrayOut, size_t length)
{
    if (!stringIn.length() || !length || stringIn.equalsIgnoreCase(SFP(TStr_null))) { return; }
    int lastSepPos = -1;
    for (size_t index = 0; index < length; ++index) {
        int nextSepPos = stringIn.indexOf(',', lastSepPos+1);
        if (nextSepPos == -1) { nextSepPos = stringIn.length(); }
        String subString = stringIn.substring(lastSepPos+1, nextSepPos);
        if (nextSepPos < stringIn.length()) { lastSepPos = nextSepPos; }

        #if !defined(CORE_TEENSY)
            arrayOut[index] = subString.toDouble();
        #else
            arrayOut[index] = subString.toFloat();
        #endif
    }
}

template<>
void commaStringToArray<String>(String stringIn, String *arrayOut, size_t length)
{
    if (!stringIn.length() || !length || stringIn.equalsIgnoreCase(SFP(TStr_null))) { return; }
    int lastSepPos = -1;
    for (size_t index = 0; index < length; ++index) {
        int nextSepPos = stringIn.indexOf(',', lastSepPos+1);
        if (nextSepPos == -1) { nextSepPos = stringIn.length(); }
        String subString = stringIn.substring(lastSepPos+1, nextSepPos);
        if (nextSepPos < stringIn.length()) { lastSepPos = nextSepPos; }
        arrayOut[index] = subString;
    }
}

String hexStringFromBytes(const uint8_t *bytesIn, size_t length)
{
    if (!bytesIn || !length) { return String(SFP(TStr_null)); }
    String retVal; retVal.reserve((length << 1) + 1);
    for (size_t index = 0; index < length; ++index) {
        String valStr = String(bytesIn[index], 16);
        if (valStr.length() == 1) { retVal.concat('0'); }

        retVal.concat(valStr);
    }
    return retVal.length() ? retVal : String(SFP(TStr_null));
}

void hexStringToBytes(String stringIn, uint8_t *bytesOut, size_t length)
{
    if (!stringIn.length() || !length || stringIn.equalsIgnoreCase(SFP(TStr_null))) { return; }
    for (size_t index = 0; index < length; ++index) {
        String valStr = stringIn.substring(index << 1,(index+1) << 1);
        if (valStr.length() == 2) { bytesOut[index] = strtoul(valStr.c_str(), nullptr, 16); }
        else { bytesOut[index] = 0; }
    }
}

void hexStringToBytes(JsonVariantConst &variantIn, uint8_t *bytesOut, size_t length)
{
    if (variantIn.isNull() || variantIn.is<JsonObjectConst>() || variantIn.is<JsonArrayConst>()) { return; }
    hexStringToBytes(variantIn.as<String>(), bytesOut, length);
}

int occurrencesInString(String string, char singleChar)
{
    int retVal = 0;
    int posIndex = string.indexOf(singleChar);
    while (posIndex != -1) {
        retVal++;
        posIndex = string.indexOf(singleChar, posIndex+1);
    }
    return retVal;
}

int occurrencesInString(String string, String subString)
{
    int retVal = 0;
    int posIndex = string.indexOf(subString[0]);
    while (posIndex != -1) {
        if (subString.equals(string.substring(posIndex, posIndex + subString.length()))) {
            retVal++;
            posIndex += subString.length();
        }
        posIndex = string.indexOf(subString[0], posIndex+1);
    }
    return retVal;
}

int occurrencesInStringIgnoreCase(String string, char singleChar)
{
    int retVal = 0;
    int posIndex = min(string.indexOf(tolower(singleChar)), string.indexOf(toupper(singleChar)));
    while (posIndex != -1) {
        retVal++;
        posIndex = min(string.indexOf(tolower(singleChar), posIndex+1), string.indexOf(toupper(singleChar), posIndex+1));
    }
    return retVal;
}

int occurrencesInStringIgnoreCase(String string, String subString)
{
    int retVal = 0;
    int posIndex = min(string.indexOf(tolower(subString[0])), string.indexOf(toupper(subString[0])));
    while (posIndex != -1) {
        if (subString.equalsIgnoreCase(string.substring(posIndex, posIndex + subString.length()))) {
            retVal++;
            posIndex += subString.length();
        }
        posIndex = min(string.indexOf(tolower(subString[0]), posIndex+1), string.indexOf(toupper(subString[0]), posIndex+1));
    }
    return retVal;
}

template<>
bool arrayElementsEqual<float>(const float *arrayIn, size_t length, float value)
{
    for (size_t index = 0; index < length; ++index) {
        if (!isFPEqual(arrayIn[index], value)) {
            return false;
        }
    }
    return true;
}

template<>
bool arrayElementsEqual<double>(const double *arrayIn, size_t length, double value)
{
    for (size_t index = 0; index < length; ++index) {
        if (!isFPEqual(arrayIn[index], value)) {
            return false;
        }
    }
    return true;
}

// See: https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#elif !defined(ESP_PLATFORM)
extern char *__brkval;
#elif defined(ESP8266)
extern "C" {
#include "user_interface.h"
}
#endif

unsigned int freeMemory() {
    #if defined(ESP32)
        return esp_get_free_heap_size();
    #elif defined(ESP8266)
        return system_get_free_heap_size();
    #else
        char top;
        #ifdef __arm__
            return &top - reinterpret_cast<char*>(sbrk(0));
        #elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
            return &top - __brkval;
        #else
            return __brkval ? &top - __brkval : &top - __malloc_heap_start;
        #endif
        return 0;
    #endif
}

void delayFine(millis_t duration) {
    millis_t start = millis();
    millis_t end = start + duration;

    {   millis_t left = max(0, duration - TERRA_SYS_DELAYFINE_SPINMILLIS);
        if (left > 0) { delay(left); }
    }

    {   millis_t time = millis();
        while ((end >= start && (time < end)) ||
               (end < start && (time >= start || time < end))) {
            time = millis();
        }
    }
}

bool tryConvertUnits(float valueIn, Terra_UnitsType unitsIn, float *valueOut, Terra_UnitsType unitsOut, float convertParam)
{
    if (!valueOut || unitsOut == Terra_UnitsType_Undefined || unitsIn == unitsOut) return false;

    switch (unitsIn) {
        case Terra_UnitsType_Raw_1:
            switch (unitsOut) {
                // Known extents

                case Terra_UnitsType_Percentile_100:
                    *valueOut = valueIn * 100.0;
                    return true;

                case Terra_UnitsType_Angle_Degrees_360:
                    *valueOut = wrapBy360(valueIn * 360.0);
                    return true;

                case Terra_UnitsType_Angle_Radians_2pi:
                    *valueOut = wrapBy2Pi(valueIn * TWO_PI);
                    return true;

                case Terra_UnitsType_Angle_Minutes_24hr:
                    *valueOut = wrapBy24Hr(valueIn * MIN_PER_DAY);
                    return true;

                default:
                    if (convertParam != FLT_UNDEF) {
                        *valueOut = valueIn * convertParam;
                        return true;
                    }
                    break;
            }
            break;

        case Terra_UnitsType_Percentile_100:
            switch (unitsOut) {
                case Terra_UnitsType_Raw_1:
                    *valueOut = valueIn / 100.0;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Angle_Degrees_360:
            switch (unitsOut) {
                case Terra_UnitsType_Angle_Radians_2pi:
                    *valueOut = wrapBy2Pi(valueIn * (TWO_PI / 360.0));
                    return true;

                case Terra_UnitsType_Angle_Minutes_24hr:
                    *valueOut = wrapBy24Hr(valueIn * (MIN_PER_DAY / 360.0));
                    return true;

                case Terra_UnitsType_Raw_1:
                    *valueOut = valueIn / 360.0;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Angle_Radians_2pi:
            switch (unitsOut) {
                case Terra_UnitsType_Angle_Degrees_360:
                    *valueOut = wrapBy360(valueIn * (360.0 / TWO_PI));
                    return true;

                case Terra_UnitsType_Angle_Minutes_24hr:
                    *valueOut = wrapBy24Hr(valueIn * (MIN_PER_DAY / TWO_PI));
                    return true;

                case Terra_UnitsType_Raw_1:
                    *valueOut = valueIn / TWO_PI;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Angle_Minutes_24hr:
            switch (unitsOut) {
                case Terra_UnitsType_Angle_Degrees_360:
                    *valueOut = wrapBy360(valueIn * (360.0 / MIN_PER_DAY));
                    return true;

                case Terra_UnitsType_Angle_Radians_2pi:
                    *valueOut = wrapBy2Pi(valueIn * (TWO_PI / MIN_PER_DAY));
                    return true;

                case Terra_UnitsType_Raw_1:
                    *valueOut = valueIn / TWO_PI;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Distance_Millimeters:
            switch (unitsOut) {
                case Terra_UnitsType_Distance_Inches:
                    *valueOut = valueIn / 25.4f;
                    return true;

                case Terra_UnitsType_Distance_Feet:
                    *valueOut = valueIn / 304.8f;
                    return true;

                case Terra_UnitsType_Distance_Meters:
                    *valueOut = valueIn / 1000.0f;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Distance_Inches:
            switch (unitsOut) {
                case Terra_UnitsType_Distance_Millimeters:
                    *valueOut = valueIn * 25.4f;
                    return true;

                case Terra_UnitsType_Distance_Feet:
                    *valueOut = valueIn / 12.0f;
                    return true;

                    case Terra_UnitsType_Distance_Meters:
                    *valueOut = valueIn * 0.0254f;
                    return true;

                default:
                    break;
            }
            break;
        case Terra_UnitsType_Distance_Feet:
            switch (unitsOut) {
                case Terra_UnitsType_Distance_Millimeters:
                    *valueOut = valueIn * 304.8f;
                    return true;

                case Terra_UnitsType_Distance_Inches:
                    *valueOut = valueIn * 12.0f;
                    return true;

                case Terra_UnitsType_Distance_Meters:
                    *valueOut = valueIn * 0.3048f;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Distance_Meters:
            switch (unitsOut) {
                case Terra_UnitsType_Distance_Millimeters:
                    *valueOut = valueIn * 1000.0f;
                    return true;

                case Terra_UnitsType_Distance_Inches:
                    *valueOut = valueIn / 0.0254f;
                    return true;

                case Terra_UnitsType_Distance_Feet:
                    *valueOut = valueIn / 0.3048f;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_LiqVolume_Gallons:
            if (unitsOut == Terra_UnitsType_LiqVolume_Liters) {
                *valueOut = valueIn * 3.78541f;
                return true;
            }
            break;

        case Terra_UnitsType_LiqVolume_Liters:
            if (unitsOut == Terra_UnitsType_LiqVolume_Gallons) {
                *valueOut = valueIn / 3.78541f;
                return true;
            }
            break;

        case Terra_UnitsType_LiqFlowRate_GallonsPerMin:
            if (unitsOut == Terra_UnitsType_LiqFlowRate_LitersPerMin) {
                *valueOut = valueIn * 3.78541f;
                return true;
            }
            break;

        case Terra_UnitsType_LiqFlowRate_LitersPerMin:
            if (unitsOut == Terra_UnitsType_LiqFlowRate_GallonsPerMin) {
                *valueOut = valueIn / 3.78541f;
                return true;
            }
            break;

        case Terra_UnitsType_Power_Amperage:
            if (unitsOut == Terra_UnitsType_Power_Wattage && convertParam != FLT_UNDEF) {
                *valueOut = valueIn * convertParam;
                return true;
            }
            break;
        case Terra_UnitsType_Power_Wattage:
            if (unitsOut == Terra_UnitsType_Power_Amperage && convertParam != FLT_UNDEF && abs(convertParam) > FLT_EPSILON) {
                *valueOut = valueIn / convertParam;
                return true;
            }
            break;

        case Terra_UnitsType_Pressure_Kilopascals:
            switch (unitsOut) {
                case Terra_UnitsType_Pressure_PSI:
                    *valueOut = valueIn / 6.894757293f;
                    return true;

                case Terra_UnitsType_Pressure_Hectopascals:
                    *valueOut = valueIn * 10.0f;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Pressure_PSI:
            switch (unitsOut) {
                case Terra_UnitsType_Pressure_Kilopascals:
                    *valueOut = valueIn * 6.894757293f;
                    return true;

                case Terra_UnitsType_Pressure_Hectopascals:
                    *valueOut = valueIn * 68.94757293f;
                    return true;

                default:
                    break;
            }
            break;
        case Terra_UnitsType_Pressure_Hectopascals:
            switch (unitsOut) {
                case Terra_UnitsType_Pressure_Kilopascals:
                    *valueOut = valueIn / 10.0f; return true;

                case Terra_UnitsType_Pressure_PSI:
                    *valueOut = valueIn / 68.94757293f; return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Speed_MillimetersPerHour:
            switch (unitsOut) {
                case Terra_UnitsType_Speed_InchesPerHour:
                    *valueOut = valueIn / 25.4f;
                    return true;

                case Terra_UnitsType_Speed_MetersPerSecond:
                    *valueOut = valueIn / 3600000.0f;
                    return true;

                case Terra_UnitsType_Speed_KilometersPerHour:
                    *valueOut = valueIn / 1000000.0f;
                    return true;

                case Terra_UnitsType_Speed_MilesPerHour:
                    *valueOut = valueIn / 1609344.0f;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Speed_InchesPerHour:
            switch (unitsOut) {
                case Terra_UnitsType_Speed_MillimetersPerHour:
                    *valueOut = valueIn * 25.4f;
                    return true;

                case Terra_UnitsType_Speed_MetersPerSecond:
                    *valueOut = valueIn * 0.0254f / 3600.0f;
                    return true;

                case Terra_UnitsType_Speed_KilometersPerHour:
                    *valueOut = valueIn * 0.0000254f;
                    return true;

                case Terra_UnitsType_Speed_MilesPerHour:
                    *valueOut = valueIn / 63360.0f;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Speed_MetersPerSecond:
            switch (unitsOut) {
                case Terra_UnitsType_Speed_MillimetersPerHour:
                    *valueOut = valueIn * 3600000.0f;
                    return true;

                case Terra_UnitsType_Speed_InchesPerHour:
                    *valueOut = valueIn * 3600.0f / 0.0254f;
                    return true;

                case Terra_UnitsType_Speed_KilometersPerHour:
                    *valueOut = valueIn * 3.6f;
                    return true;

                case Terra_UnitsType_Speed_MilesPerHour:
                    *valueOut = valueIn / 0.44704f;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Speed_KilometersPerHour:
            switch (unitsOut) {
                case Terra_UnitsType_Speed_MillimetersPerHour:
                    *valueOut = valueIn * 1000000.0f;
                    return true;

                case Terra_UnitsType_Speed_InchesPerHour:
                    *valueOut = valueIn / 0.0000254f;
                    return true;

                case Terra_UnitsType_Speed_MetersPerSecond:
                    *valueOut = valueIn / 3.6f;
                    return true;

                case Terra_UnitsType_Speed_MilesPerHour:
                    *valueOut = valueIn / 1.609344f;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Speed_MilesPerHour:
            switch (unitsOut) {
                case Terra_UnitsType_Speed_MillimetersPerHour:
                    *valueOut = valueIn * 1609344.0f;
                    return true;

                case Terra_UnitsType_Speed_InchesPerHour:
                    *valueOut = valueIn * 63360.0f;
                    return true;

                case Terra_UnitsType_Speed_MetersPerSecond:
                    *valueOut = valueIn * 0.44704f;
                    return true;

                case Terra_UnitsType_Speed_KilometersPerHour:
                    *valueOut = valueIn * 1.609344f;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Temperature_Celsius:
            switch (unitsOut) {
                case Terra_UnitsType_Temperature_Fahrenheit:
                    *valueOut = valueIn * 1.8f + 32.0f;
                    return true;

                case Terra_UnitsType_Temperature_Kelvin:
                    *valueOut = valueIn + 273.15f;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Temperature_Fahrenheit:
            switch (unitsOut) {
                case Terra_UnitsType_Temperature_Celsius:
                    *valueOut = (valueIn - 32.0f) / 1.8f;
                    return true;

                case Terra_UnitsType_Temperature_Kelvin:
                    *valueOut = (valueIn + 459.67f) * 5.0f / 9.0f;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Temperature_Kelvin:
            switch (unitsOut) {
                case Terra_UnitsType_Temperature_Celsius:
                    *valueOut = valueIn - 273.15f;
                    return true;

                case Terra_UnitsType_Temperature_Fahrenheit: 
                    *valueOut = valueIn * 9.0f / 5.0f - 459.67f; 
                    return true;

                default: 
                    break;
            }
            break;

        case Terra_UnitsType_Undefined:
            *valueOut = valueIn;
            return true;

        default:
            break;
    }

    return false;
}

Terra_UnitsType baseUnits(Terra_UnitsType units)
{
    switch (units) {
        case Terra_UnitsType_LiqFlowRate_LitersPerMin:
            return Terra_UnitsType_LiqVolume_Liters;
        case Terra_UnitsType_LiqFlowRate_GallonsPerMin:
            return Terra_UnitsType_LiqVolume_Gallons;
        case Terra_UnitsType_Speed_MillimetersPerHour:
            return Terra_UnitsType_Distance_Millimeters;
        case Terra_UnitsType_Speed_InchesPerHour:
            return Terra_UnitsType_Distance_Inches;
        case Terra_UnitsType_Speed_MetersPerSecond:
            return Terra_UnitsType_Distance_Meters;
        default:
            return Terra_UnitsType_Undefined;
    }
}

Terra_UnitsType rateUnits(Terra_UnitsType units)
{
    switch (units) {
        case Terra_UnitsType_LiqVolume_Liters:
            return Terra_UnitsType_LiqFlowRate_LitersPerMin;
        case Terra_UnitsType_LiqVolume_Gallons:
            return Terra_UnitsType_LiqFlowRate_GallonsPerMin;
        case Terra_UnitsType_Distance_Millimeters:
            return Terra_UnitsType_Speed_MillimetersPerHour;
        case Terra_UnitsType_Distance_Inches:
            return Terra_UnitsType_Speed_InchesPerHour;
        case Terra_UnitsType_Distance_Meters:
            return Terra_UnitsType_Speed_MetersPerSecond;
        default: return Terra_UnitsType_Undefined;
    }
}

Terra_UnitsType defaultUnits(Terra_UnitsCategory unitsCategory, Terra_MeasurementMode measureMode)
{
    measureMode = (measureMode == Terra_MeasurementMode_Undefined && getController() ? getController()->getMeasurementMode() : measureMode);
    if (measureMode == Terra_MeasurementMode_Undefined) { measureMode = Terra_MeasurementMode_Default; }

    switch (unitsCategory) {
        case Terra_UnitsCategory_Angle:
            switch (measureMode) {
                case Terra_MeasurementMode_Imperial:
                case Terra_MeasurementMode_Metric:
                    return Terra_UnitsType_Angle_Degrees_360;
                case Terra_MeasurementMode_Scientific:
                    return Terra_UnitsType_Angle_Radians_2pi;
                default:
                    return Terra_UnitsType_Undefined;
            }

        case Terra_UnitsCategory_Distance:
            switch (measureMode) {
                case Terra_MeasurementMode_Imperial:
                    return Terra_UnitsType_Distance_Feet;
                case Terra_MeasurementMode_Metric:
                case Terra_MeasurementMode_Scientific:
                    return Terra_UnitsType_Distance_Meters;
                default:
                    return Terra_UnitsType_Undefined;
            }

        case Terra_UnitsCategory_Energy:
            return Terra_UnitsType_Energy_KilowattHours;

        case Terra_UnitsCategory_Irradiance:
            return Terra_UnitsType_Irradiance_WattsPerSquareMeter;

        case Terra_UnitsCategory_LiqVolume:
            switch (measureMode) {
                case Terra_MeasurementMode_Imperial:
                    return Terra_UnitsType_LiqVolume_Gallons;
                case Terra_MeasurementMode_Metric:
                case Terra_MeasurementMode_Scientific:
                    return Terra_UnitsType_LiqVolume_Liters;
                default:
                    return Terra_UnitsType_Undefined;
            }

        case Terra_UnitsCategory_LiqFlowRate:
            switch (measureMode) {
                case Terra_MeasurementMode_Imperial:
                    return Terra_UnitsType_LiqFlowRate_GallonsPerMin;
                case Terra_MeasurementMode_Metric:
                case Terra_MeasurementMode_Scientific:
                    return Terra_UnitsType_LiqFlowRate_LitersPerMin;
                default:
                    return Terra_UnitsType_Undefined;
            }

        case Terra_UnitsCategory_Power:
            return Terra_UnitsType_Power_Wattage;

        case Terra_UnitsCategory_Pressure:
            switch (measureMode) {
                case Terra_MeasurementMode_Imperial:
                    return Terra_UnitsType_Pressure_PSI;
                case Terra_MeasurementMode_Metric:
                case Terra_MeasurementMode_Scientific:
                    return Terra_UnitsType_Pressure_Kilopascals;
                default:
                    return Terra_UnitsType_Undefined;
            }

        case Terra_UnitsCategory_Speed:
            switch (measureMode) {
                case Terra_MeasurementMode_Imperial:
                    return Terra_UnitsType_Speed_MilesPerHour;
                case Terra_MeasurementMode_Metric:
                case Terra_MeasurementMode_Scientific:
                    return Terra_UnitsType_Speed_MetersPerSecond;
                default:
                    return Terra_UnitsType_Undefined;
            }

        case Terra_UnitsCategory_Temperature:
            switch (measureMode) {
                case Terra_MeasurementMode_Imperial:
                    return Terra_UnitsType_Temperature_Fahrenheit;
                case Terra_MeasurementMode_Metric:
                    return Terra_UnitsType_Temperature_Celsius;
                case Terra_MeasurementMode_Scientific:
                    return Terra_UnitsType_Temperature_Kelvin;
                default:
                    return Terra_UnitsType_Undefined;
            }

        case Terra_UnitsCategory_Count:
            switch (measureMode) {
                case Terra_MeasurementMode_Scientific:
                    return (Terra_UnitsType)2;
                default:
                    return (Terra_UnitsType)1;
            }

        case Terra_UnitsCategory_Undefined:
            return Terra_UnitsType_Undefined;
    }
    return Terra_UnitsType_Undefined;
}


int linksCountActuatorsByReservoirAndType(Pair<uint8_t, Pair<TerraObject *, int8_t> *> links, TerraReservoir *srcReservoir, Terra_ActuatorType actuatorType)
{
    int retVal = 0;

    for (tposi_t linksIndex = 0; linksIndex < links.first && links.second[linksIndex].first; ++linksIndex) {
        if (links.second[linksIndex].first->isActuatorType()) {
            auto actuator = static_cast<TerraActuator *>(links.second[linksIndex].first);

            if (actuator->getActuatorType() == actuatorType && actuator->getParentReservoir().get() == srcReservoir) {
                retVal++;
            }
        }
    }

    return retVal;
}


bool checkPinIsAnalogInput(pintype_t pin)
{
    #if !defined(NUM_ANALOG_INPUTS) || NUM_ANALOG_INPUTS == 0
        return false;
    #elif defined(ESP32)
        return checkPinIsDigital(pin); // all digital pins are ADC capable
    #else
        switch (pin) {
            #if NUM_ANALOG_INPUTS > 0
                case (pintype_t)A0:
            #endif
            #if NUM_ANALOG_INPUTS > 1 && !(defined(PIN_A0) && !defined(PIN_A1))
                case (pintype_t)A1:
            #endif
            #if NUM_ANALOG_INPUTS > 2 && !(defined(PIN_A0) && !defined(PIN_A2))
                case (pintype_t)A2:
            #endif
            #if NUM_ANALOG_INPUTS > 3 && !(defined(PIN_A0) && !defined(PIN_A3))
                case (pintype_t)A3:
            #endif
            #if NUM_ANALOG_INPUTS > 4 && !(defined(PIN_A0) && !defined(PIN_A4))
                case (pintype_t)A4:
            #endif
            #if NUM_ANALOG_INPUTS > 5 && !(defined(PIN_A0) && !defined(PIN_A5))
                case (pintype_t)A5:
            #endif
            #if NUM_ANALOG_INPUTS > 6 && !(defined(PIN_A0) && !defined(PIN_A6))
                case (pintype_t)A6:
            #endif
            #if NUM_ANALOG_INPUTS > 7 && !(defined(PIN_A0) && !defined(PIN_A7))
                case (pintype_t)A7:
            #endif
            #if NUM_ANALOG_INPUTS > 8 && !(defined(PIN_A0) && !defined(PIN_A8))
                case (pintype_t)A8:
            #endif
            #if NUM_ANALOG_INPUTS > 9 && !(defined(PIN_A0) && !defined(PIN_A9))
                case (pintype_t)A9:
            #endif
            #if NUM_ANALOG_INPUTS > 10 && !(defined(PIN_A0) && !defined(PIN_A10))
                case (pintype_t)A10:
            #endif
            #if NUM_ANALOG_INPUTS > 11 && !(defined(PIN_A0) && !defined(PIN_A11))
                case (pintype_t)A11:
            #endif
            #if NUM_ANALOG_INPUTS > 12 && !(defined(PIN_A0) && !defined(PIN_A12))
                case (pintype_t)A12:
            #endif
            #if NUM_ANALOG_INPUTS > 13 && !(defined(PIN_A0) && !defined(PIN_A13))
                case (pintype_t)A13:
            #endif
            #if NUM_ANALOG_INPUTS > 14 && !(defined(PIN_A0) && !defined(PIN_A14))
                case (pintype_t)A14:
            #endif
            #if NUM_ANALOG_INPUTS > 15 && !(defined(PIN_A0) && !defined(PIN_A15))
                case (pintype_t)A15:
            #endif
            #if NUM_ANALOG_INPUTS > 16 && !(defined(PIN_A0) && !defined(PIN_A16))
                case (pintype_t)A16:
            #endif
            #if NUM_ANALOG_INPUTS > 17 && !(defined(PIN_A0) && !defined(PIN_A17))
                case (pintype_t)A17:
            #endif
            #if NUM_ANALOG_INPUTS > 18 && !(defined(PIN_A0) && !defined(PIN_A18))
                case (pintype_t)A18:
            #endif
            #if NUM_ANALOG_INPUTS > 19 && !(defined(PIN_A0) && !defined(PIN_A19))
                case (pintype_t)A19:
            #endif
            #if NUM_ANALOG_INPUTS > 20 && !(defined(PIN_A0) && !defined(PIN_A20))
                case (pintype_t)A20:
            #endif
            #if NUM_ANALOG_INPUTS > 21 && !(defined(PIN_A0) && !defined(PIN_A21))
                case (pintype_t)A21:
            #endif
                return true;

            default:
                return false;
        }
    #endif
}

bool checkPinIsAnalogOutput(pintype_t pin)
{
    #if !defined(NUM_ANALOG_OUTPUTS) || NUM_ANALOG_OUTPUTS == 0
        return false;
    #elif defined(ESP32)
        return checkPinIsDigital(pin); // all digital pins are ADC capable
    #else
        switch (pin) {
            #if NUM_ANALOG_OUTPUTS > 0
                #ifndef PIN_DAC0
                    case (pintype_t)A0:
                #else
                    case (pintype_t)DAC0:
                #endif
            #endif
            #if NUM_ANALOG_OUTPUTS > 1
                #ifndef PIN_DAC1
                    case (pintype_t)A1:
                #else
                    case (pintype_t)DAC1:
                #endif
            #endif
            #if NUM_ANALOG_OUTPUTS > 2
                #ifndef PIN_DAC2
                    case (pintype_t)A2:
                #else
                    case (pintype_t)DAC2:
                #endif
            #endif
            #if NUM_ANALOG_OUTPUTS > 3
                #ifndef PIN_DAC3
                    case (pintype_t)A3:
                #else
                    case (pintype_t)DAC3:
                #endif
            #endif
            #if NUM_ANALOG_OUTPUTS > 4
                #ifndef PIN_DAC4
                    case (pintype_t)A4:
                #else
                    case (pintype_t)DAC4:
                #endif
            #endif
            #if NUM_ANALOG_OUTPUTS > 5
                #ifndef PIN_DAC5
                    case (pintype_t)A5:
                #else
                    case (pintype_t)DAC5:
                #endif
            #endif
            #if NUM_ANALOG_OUTPUTS > 6
                #ifndef PIN_DAC6
                    case (pintype_t)A6:
                #else
                    case (pintype_t)DAC6:
                #endif
            #endif
            #if NUM_ANALOG_OUTPUTS > 7
                #ifndef PIN_DAC7
                    case (pintype_t)A7:
                #else
                    case (pintype_t)DAC7:
                #endif
            #endif
                return true;

            default:
                return false;
        }
    #endif
}


String systemModeToString(Terra_SystemMode systemMode, bool excludeSpecial)
{
    switch (systemMode) {
        case Terra_SystemMode_Manual:
            return SFP(TStr_Enum_Manual);
        case Terra_SystemMode_Automatic:
            return SFP(TStr_Enum_Automatic);
        case Terra_SystemMode_Disabled:
            return SFP(TStr_Disabled);
        case Terra_SystemMode_Count:
           return !excludeSpecial ? SFP(TStr_Enum_Count) : String();
        case Terra_SystemMode_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(TStr_Undefined) : String();
}

String measurementModeToString(Terra_MeasurementMode measurementMode, bool excludeSpecial)
{
    switch (measurementMode) {
        case Terra_MeasurementMode_Imperial:
            return SFP(TStr_Enum_Imperial);
        case Terra_MeasurementMode_Metric:
            return SFP(TStr_Enum_Metric);
        case Terra_MeasurementMode_Scientific:
            return SFP(TStr_Enum_Scientific);
        case Terra_MeasurementMode_Count:
            return !excludeSpecial ? SFP(TStr_Enum_Count) : String();
        case Terra_MeasurementMode_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(TStr_Undefined) : String();
}

String displayOutputModeToString(Terra_DisplayOutputMode displayOutMode, bool excludeSpecial)
{
    switch (displayOutMode) {
        case Terra_DisplayOutputMode_Disabled:
            return SFP(TStr_Disabled);
        case Terra_DisplayOutputMode_LCD16x2_EN: {
            String retVal(SFP(TStr_Enum_LCD16x2));
            retVal.reserve(retVal.length() + 2 + 1);
            retVal.concat('E'); retVal.concat('N');
            return retVal;
        }
        case Terra_DisplayOutputMode_LCD16x2_RS: {
            String retVal(SFP(TStr_Enum_LCD16x2));
            retVal.reserve(retVal.length() + 2 + 1);
            retVal.concat('R'); retVal.concat('S');
            return retVal;
        }
        case Terra_DisplayOutputMode_LCD20x4_EN: {
            String retVal(SFP(TStr_Enum_LCD20x4));
            retVal.reserve(retVal.length() + 2 + 1);
            retVal.concat('E'); retVal.concat('N');
            return retVal;
        }
        case Terra_DisplayOutputMode_LCD20x4_RS: {
            String retVal(SFP(TStr_Enum_LCD20x4));
            retVal.reserve(retVal.length() + 2 + 1);
            retVal.concat('R'); retVal.concat('S');
            return retVal;
        }
        case Terra_DisplayOutputMode_SSD1305:
            return SFP(TStr_Enum_SSD1305);
        case Terra_DisplayOutputMode_SSD1305_x32Ada:
            return SFP(TStr_Enum_SSD1305x32Ada);
        case Terra_DisplayOutputMode_SSD1305_x64Ada:
            return SFP(TStr_Enum_SSD1305x64Ada);
        case Terra_DisplayOutputMode_SSD1306:
            return SFP(TStr_Enum_SSD1306);
        case Terra_DisplayOutputMode_SH1106:
            return SFP(TStr_Enum_SH1106);
        case Terra_DisplayOutputMode_CustomOLED:
            return SFP(TStr_Enum_CustomOLED);
        case Terra_DisplayOutputMode_SSD1607:
            return SFP(TStr_Enum_SSD1607);
        case Terra_DisplayOutputMode_IL3820:
            return SFP(TStr_Enum_IL3820);
        case Terra_DisplayOutputMode_IL3820_V2:
            return SFP(TStr_Enum_IL3820V2);
        case Terra_DisplayOutputMode_ST7735:
            return SFP(TStr_Enum_ST7735);
        case Terra_DisplayOutputMode_ST7789:
            return SFP(TStr_Enum_ST7789);
        case Terra_DisplayOutputMode_ILI9341:
            return SFP(TStr_Enum_ILI9341);
        case Terra_DisplayOutputMode_TFT: {
            String retVal; retVal.reserve(3 + 1);
            retVal.concat('T'); retVal.concat('F'); retVal.concat('T');
            return retVal;
        }
        case Terra_DisplayOutputMode_Count:
            return !excludeSpecial ? SFP(TStr_Enum_Count) : String();
        case Terra_DisplayOutputMode_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(TStr_Undefined) : String();
}

String controlInputModeToString(Terra_ControlInputMode controlInMode, bool excludeSpecial)
{
    switch (controlInMode) {
        case Terra_ControlInputMode_Disabled:
            return SFP(TStr_Disabled);
        case Terra_ControlInputMode_RotaryEncoderOk: {
            String retVal(SFP(TStr_Enum_RotaryEncoder));
            retVal.reserve(retVal.length() + 2 + 1);
            retVal.concat('O'); retVal.concat('k');
            return retVal;
        }
        case Terra_ControlInputMode_RotaryEncoderOkLR: {
            String retVal(SFP(TStr_Enum_RotaryEncoder));
            retVal.reserve(retVal.length() + 4 + 1);
            retVal.concat('O'); retVal.concat('k');
            retVal.concat('L'); retVal.concat('R');
            return retVal;
        }
        case Terra_ControlInputMode_UpDownButtonsOk: {
            String retVal(SFP(TStr_Enum_UpDownButtons));
            retVal.reserve(retVal.length() + 2 + 1);
            retVal.concat('O'); retVal.concat('k');
            return retVal;
        }
        case Terra_ControlInputMode_UpDownButtonsOkLR: {
            String retVal(SFP(TStr_Enum_UpDownButtons));
            retVal.reserve(retVal.length() + 4 + 1);
            retVal.concat('O'); retVal.concat('k');
            retVal.concat('L'); retVal.concat('R');
            return retVal;
        }
        case Terra_ControlInputMode_UpDownESP32TouchOk: {
            String retVal(SFP(TStr_Enum_UpDownESP32Touch));
            retVal.reserve(retVal.length() + 2 + 1);
            retVal.concat('O'); retVal.concat('k');
            return retVal;
        }
        case Terra_ControlInputMode_UpDownESP32TouchOkLR: {
            String retVal(SFP(TStr_Enum_UpDownESP32Touch));
            retVal.reserve(retVal.length() + 4 + 1);
            retVal.concat('O'); retVal.concat('k');
            retVal.concat('L'); retVal.concat('R');
            return retVal;
        }
        case Terra_ControlInputMode_AnalogJoystickOk: {
            String retVal(SFP(TStr_Enum_AnalogJoystick));
            retVal.reserve(retVal.length() + 2 + 1);
            retVal.concat('O'); retVal.concat('k');
            return retVal;
        }
        case Terra_ControlInputMode_Matrix2x2UpDownButtonsOkL: {
            String retVal(SFP(TStr_Enum_Matrix2x2));
            String concat(SFP(TStr_Enum_UpDownButtons));
            retVal.reserve(retVal.length() + concat.length() + 3 + 1);
            retVal.concat(concat);
            retVal.concat('O'); retVal.concat('k');
            retVal.concat('L');
            return retVal;
        }
        case Terra_ControlInputMode_Matrix3x4Keyboard_OptRotEncOk: {
            String retVal(SFP(TStr_Enum_Matrix3x4));
            retVal.reserve(retVal.length() + 2 + 1);
            retVal.concat('O'); retVal.concat('k');
            return retVal;
        }
        case Terra_ControlInputMode_Matrix3x4Keyboard_OptRotEncOkLR: {
            String retVal(SFP(TStr_Enum_Matrix3x4));
            retVal.reserve(retVal.length() + 4 + 1);
            retVal.concat('O'); retVal.concat('k');
            retVal.concat('L'); retVal.concat('R');
            return retVal;
        }
        case Terra_ControlInputMode_Matrix4x4Keyboard_OptRotEncOk: {
            String retVal(SFP(TStr_Enum_Matrix4x4));
            retVal.reserve(retVal.length() + 2 + 1);
            retVal.concat('O'); retVal.concat('k');
            return retVal;
        }
        case Terra_ControlInputMode_Matrix4x4Keyboard_OptRotEncOkLR: {
            String retVal(SFP(TStr_Enum_Matrix4x4));
            retVal.reserve(retVal.length() + 4 + 1);
            retVal.concat('O'); retVal.concat('k');
            retVal.concat('L'); retVal.concat('R');
            return retVal;
        }
        case Terra_ControlInputMode_ResistiveTouch:
            return SFP(TStr_Enum_ResistiveTouch);
        case Terra_ControlInputMode_TouchScreen:
            return SFP(TStr_Enum_TouchScreen);
        case Terra_ControlInputMode_TFTTouch:
            return SFP(TStr_Enum_TFTTouch);
        case Terra_ControlInputMode_RemoteControl:
            return SFP(TStr_Enum_RemoteControl);
        case Terra_ControlInputMode_Count:
            return !excludeSpecial ? SFP(TStr_Enum_Count) : String();
        case Terra_ControlInputMode_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(TStr_Undefined) : String();
}

String actuatorTypeToString(Terra_ActuatorType actuatorType, bool excludeSpecial)
{
    switch (actuatorType) {
        case Terra_ActuatorType_Pump:
            return SFP(TStr_Enum_Pump);
        case Terra_ActuatorType_Valve:
            return SFP(TStr_Enum_Valve);
        case Terra_ActuatorType_Fan:
            return SFP(TStr_Enum_Fan);
        case Terra_ActuatorType_Heater:
            return SFP(TStr_Enum_Heater);
        case Terra_ActuatorType_Circulator:
            return SFP(TStr_Enum_Circulator);
        case Terra_ActuatorType_Count:
            return !excludeSpecial ? SFP(TStr_Enum_Count) : String();
        case Terra_ActuatorType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(TStr_Undefined) : String();
}

String sensorTypeToString(Terra_SensorType sensorType, bool excludeSpecial)
{
    switch (sensorType) {
        case Terra_SensorType_Temperature:
            return SFP(TStr_Enum_Temperature);
        case Terra_SensorType_Humidity:
            return SFP(TStr_Enum_Humidity);
        case Terra_SensorType_Pressure:
            return SFP(TStr_Enum_Pressure);
        case Terra_SensorType_Rainfall:
            return SFP(TStr_Enum_Rainfall);
        case Terra_SensorType_Flow:
            return SFP(TStr_Enum_Flow);
        case Terra_SensorType_Level:
            return SFP(TStr_Enum_Level);
        case Terra_SensorType_WindSpeed:
            return SFP(TStr_Enum_WindSpeed);
        case Terra_SensorType_WindDirection:
            return SFP(TStr_Enum_WindDirection);
        case Terra_SensorType_SolarRadiation:
            return SFP(TStr_Enum_SolarRadiation);
        case Terra_SensorType_Voltage:
            return SFP(TStr_Enum_Voltage);
        case Terra_SensorType_Current:
            return SFP(TStr_Enum_Current);
        case Terra_SensorType_Leak:
            return SFP(TStr_Enum_Leak);
        case Terra_SensorType_Count:
            return !excludeSpecial ? SFP(TStr_Enum_Count) : String();
        case Terra_SensorType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(TStr_Undefined) : String();
}

String reservoirTypeToString(Terra_ReservoirType reservoirType, bool excludeSpecial)
{
    switch (reservoirType) {
        case Terra_ReservoirType_Water:
            return SFP(TStr_Enum_Water);
        case Terra_ReservoirType_Thermal:
            return SFP(TStr_Enum_Thermal);
        case Terra_ReservoirType_Count:
            return !excludeSpecial ? SFP(TStr_Enum_Count) : String();
        case Terra_ReservoirType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(TStr_Undefined) : String();
}

float getRailVoltageFromType(Terra_RailType railType)
{
    switch (railType) {
        case Terra_RailType_AC110V:
            return 110.0f;
        case Terra_RailType_AC220V:
            return 220.0f;
        case Terra_RailType_DC3V3:
            return 3.3f;
        case Terra_RailType_DC5V:
            return 5.0f;
        case Terra_RailType_DC12V:
            return 12.0f;
        case Terra_RailType_DC24V:
            return 24.0f;
        case Terra_RailType_DC48V:
            return 48.0f;
        default:
            return 0.0f;
    }
}

String railTypeToString(Terra_RailType railType, bool excludeSpecial)
{
    switch (railType) {
        case Terra_RailType_AC110V:
            return SFP(TStr_Enum_AC110V);
        case Terra_RailType_AC220V:
            return SFP(TStr_Enum_AC220V);
        case Terra_RailType_DC3V3:
            return SFP(TStr_Enum_DC3V3);
        case Terra_RailType_DC5V:
            return SFP(TStr_Enum_DC5V);
        case Terra_RailType_DC12V:
            return SFP(TStr_Enum_DC12V);
        case Terra_RailType_DC24V:
            return SFP(TStr_Enum_DC24V);
        case Terra_RailType_DC48V:
            return SFP(TStr_Enum_DC48V);
        case Terra_RailType_Count:
            return !excludeSpecial ? SFP(TStr_Enum_Count) : String();
        case Terra_RailType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(TStr_Undefined) : String();
}

String pinModeToString(Terra_PinMode pinMode, bool excludeSpecial)
{
    switch (pinMode) {
        case Terra_PinMode_Digital_Input:
            return SFP(TStr_Enum_DigitalInput);
        case Terra_PinMode_Digital_Input_PullUp:
            return SFP(TStr_Enum_DigitalInputPullUp);
        case Terra_PinMode_Digital_Input_PullDown:
            return SFP(TStr_Enum_DigitalInputPullDown);
        case Terra_PinMode_Digital_Output:
            return SFP(TStr_Enum_DigitalOutput);
        case Terra_PinMode_Digital_Output_PushPull:
            return SFP(TStr_Enum_DigitalOutputPushPull);
        case Terra_PinMode_Analog_Input:
            return SFP(TStr_Enum_AnalogInput);
        case Terra_PinMode_Analog_Output:
            return SFP(TStr_Enum_AnalogOutput);
        case Terra_PinMode_Count:
            return !excludeSpecial ? SFP(TStr_Enum_Count) : String();
        case Terra_PinMode_Undefined:
            break;
        default:
            return String((int)pinMode);
    }
    return !excludeSpecial ? SFP(TStr_Undefined) : String();
}

String enableModeToString(Terra_EnableMode enableMode, bool excludeSpecial)
{
    switch (enableMode) {
        case Terra_EnableMode_Highest:
            return SFP(TStr_Enum_Highest);
        case Terra_EnableMode_Lowest:
            return SFP(TStr_Enum_Lowest);
        case Terra_EnableMode_Average:
            return SFP(TStr_Enum_Average);
        case Terra_EnableMode_Multiply:
            return SFP(TStr_Enum_Multiply);
        case Terra_EnableMode_InOrder:
            return SFP(TStr_Enum_InOrder);
        case Terra_EnableMode_RevOrder:
            return SFP(TStr_Enum_RevOrder);
        case Terra_EnableMode_DescOrder:
            return SFP(TStr_Enum_DescOrder);
        case Terra_EnableMode_AscOrder:
            return SFP(TStr_Enum_AscOrder);
        case Terra_EnableMode_Count:
            return !excludeSpecial ? SFP(TStr_Enum_Count) : String();
        case Terra_EnableMode_Undefined:
            break;
        default:
            return String((int)enableMode);
    }
    return !excludeSpecial ? SFP(TStr_Undefined) : String();
}

String unitsCategoryToString(Terra_UnitsCategory unitsCategory, bool excludeSpecial)
{
    switch (unitsCategory) {
        case Terra_UnitsCategory_Angle:
            return SFP(TStr_Enum_Angle);
        case Terra_UnitsCategory_Distance:
            return SFP(TStr_Enum_Distance);
        case Terra_UnitsCategory_Energy:
            return SFP(TStr_Enum_Energy);
        case Terra_UnitsCategory_Irradiance:
            return SFP(TStr_Enum_Irradiance);
        case Terra_UnitsCategory_LiqVolume:
            return SFP(TStr_Enum_LiquidVolume);
        case Terra_UnitsCategory_LiqFlowRate:
            return SFP(TStr_Enum_LiquidFlowRate);
        case Terra_UnitsCategory_Power:
            return SFP(TStr_Enum_Power);
        case Terra_UnitsCategory_Pressure:
            return SFP(TStr_Enum_Pressure);
        case Terra_UnitsCategory_Speed:
            return SFP(TStr_Enum_Speed);
        case Terra_UnitsCategory_Temperature:
            return SFP(TStr_Enum_Temperature);
        case Terra_UnitsCategory_Count:
            return !excludeSpecial ? SFP(TStr_Enum_Count) : String();
        case Terra_UnitsCategory_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(TStr_Undefined) : String();
}

String unitsTypeToSymbol(Terra_UnitsType unitsType, bool excludeSpecial)
{
    switch (unitsType) {
        case Terra_UnitsType_Raw_1:
            return SFP(TStr_raw);
        case Terra_UnitsType_Percentile_100:
            return SFP(TStr_Unit_Percent);
        case Terra_UnitsType_Angle_Degrees_360:
            return SFP(TStr_Unit_deg);
        case Terra_UnitsType_Angle_Radians_2pi:
            return SFP(TStr_Unit_rad);
        case Terra_UnitsType_Angle_Minutes_24hr:
            return SFP(TStr_Unit_min);
        case Terra_UnitsType_Distance_Millimeters:
            return SFP(TStr_Unit_mm);
        case Terra_UnitsType_Distance_Inches:
            return SFP(TStr_Unit_in);
        case Terra_UnitsType_Distance_Feet:
            return SFP(TStr_Unit_ft);
        case Terra_UnitsType_Distance_Meters:
            return SFP(TStr_Unit_m);
        case Terra_UnitsType_LiqVolume_Gallons:
            return SFP(TStr_Unit_gal);
        case Terra_UnitsType_LiqVolume_Liters:
            return SFP(TStr_Unit_L);
        case Terra_UnitsType_LiqFlowRate_GallonsPerMin:
            return SFP(TStr_Unit_galPerMin);
        case Terra_UnitsType_LiqFlowRate_LitersPerMin:
            return SFP(TStr_Unit_LPerMin);
        case Terra_UnitsType_Irradiance_WattsPerSquareMeter:
            return SFP(TStr_Unit_WPerM2);
        case Terra_UnitsType_Energy_KilowattHours:
            return SFP(TStr_Unit_kWh);
        case Terra_UnitsType_Power_Amperage:
            return SFP(TStr_Unit_A);
        case Terra_UnitsType_Power_Wattage:
            return SFP(TStr_Unit_W);
        case Terra_UnitsType_Power_Volts:
            return SFP(TStr_Unit_V);
        case Terra_UnitsType_Pressure_Kilopascals:
            return SFP(TStr_Unit_kPa);
        case Terra_UnitsType_Pressure_PSI:
            return SFP(TStr_Unit_psi);
        case Terra_UnitsType_Pressure_Hectopascals:
            return SFP(TStr_Unit_hPa);
        case Terra_UnitsType_Speed_MillimetersPerHour:
            return SFP(TStr_Unit_mmPerH);
        case Terra_UnitsType_Speed_InchesPerHour:
            return SFP(TStr_Unit_inPerH);
        case Terra_UnitsType_Speed_MetersPerSecond:
            return SFP(TStr_Unit_mPerS);
        case Terra_UnitsType_Speed_KilometersPerHour:
            return SFP(TStr_Unit_kmPerH);
        case Terra_UnitsType_Speed_MilesPerHour:
            return SFP(TStr_Unit_mph);
        case Terra_UnitsType_Temperature_Celsius:
            return SFP(TStr_Unit_C);
        case Terra_UnitsType_Temperature_Fahrenheit:
            return SFP(TStr_Unit_F);
        case Terra_UnitsType_Temperature_Kelvin:
            return SFP(TStr_Unit_K);
        case Terra_UnitsType_Count:
            return !excludeSpecial ? SFP(TStr_Enum_Count) : String();
        case Terra_UnitsType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(TStr_Undefined) : String();
}


String positionIndexToString(tposi_t positionIndex, bool excludeSpecial)
{
    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        return String(positionIndex + TERRA_POS_EXPORT_BEGFROM);
    } else if (!excludeSpecial) {
        if (positionIndex == TERRA_POS_MAXSIZE) {
            return SFP(TStr_Enum_Count);
        } else {
            return SFP(TStr_Undefined);
        }
    }
    return String();
}

tposi_t positionIndexFromString(String positionIndexStr)
{
    if (positionIndexStr == positionIndexToString(TERRA_POS_MAXSIZE)) {
        return TERRA_POS_MAXSIZE;
    } else if (positionIndexStr == positionIndexToString(-1)) {
        return -1;
    } else {
        int8_t decode = positionIndexStr.toInt();
        return decode >= 0 && decode < TERRA_POS_MAXSIZE ? decode : -1;
    }
}


// All remaining methods generated from minimum spanning trie

Terra_SystemMode systemModeFromString(String systemModeStr)
{
    systemModeStr.toLowerCase();
    switch (systemModeStr.length() > 0 ? systemModeStr[0] : '\000') {
        case 'a':
            return Terra_SystemMode_Automatic;
        case 'c':
            return Terra_SystemMode_Count;
        case 'd':
            return Terra_SystemMode_Disabled;
        case 'm':
            return Terra_SystemMode_Manual;
        case 'u':
            return Terra_SystemMode_Undefined;
    }
    return Terra_SystemMode_Undefined;
}

Terra_MeasurementMode measurementModeFromString(String measurementModeStr)
{
    measurementModeStr.toLowerCase();
    switch (measurementModeStr.length() > 0 ? measurementModeStr[0] : '\000') {
        case 'c':
            return Terra_MeasurementMode_Count;
        case 'i':
            return Terra_MeasurementMode_Imperial;
        case 'm':
            return Terra_MeasurementMode_Metric;
        case 's':
            return Terra_MeasurementMode_Scientific;
        case 'u':
            return Terra_MeasurementMode_Undefined;
    }
    return Terra_MeasurementMode_Undefined;
}

Terra_DisplayOutputMode displayOutputModeFromString(String displayOutModeStr)
{
    displayOutModeStr.toLowerCase();
    switch (displayOutModeStr.length() > 6 ? displayOutModeStr[6] : '\000') {
        case '\000':
            switch (displayOutModeStr.length() > 4 ? displayOutModeStr[4] : '\000') {
                case '\000':
                    return Terra_DisplayOutputMode_TFT;
                case '0':
                    return Terra_DisplayOutputMode_SH1106;
                case '2':
                    return Terra_DisplayOutputMode_IL3820;
                case '3':
                    return Terra_DisplayOutputMode_ST7735;
                case '8':
                    return Terra_DisplayOutputMode_ST7789;
                case 't':
                    return Terra_DisplayOutputMode_Count;
            }
            break;
        case '1':
            return Terra_DisplayOutputMode_ILI9341;
        case '2':
            switch (displayOutModeStr.length() > 7 ? displayOutModeStr[7] : '\000') {
                case 'e':
                    return Terra_DisplayOutputMode_LCD16x2_EN;
                case 'r':
                    return Terra_DisplayOutputMode_LCD16x2_RS;
            }
            break;
        case '4':
            switch (displayOutModeStr.length() > 7 ? displayOutModeStr[7] : '\000') {
                case 'e':
                    return Terra_DisplayOutputMode_LCD20x4_EN;
                case 'r':
                    return Terra_DisplayOutputMode_LCD20x4_RS;
            }
            break;
        case '5':
            switch (displayOutModeStr.length() > 8 ? displayOutModeStr[8] : '\000') {
                case '\000':
                    return Terra_DisplayOutputMode_SSD1305;
                case '3':
                    return Terra_DisplayOutputMode_SSD1305_x32Ada;
                case '6':
                    return Terra_DisplayOutputMode_SSD1305_x64Ada;
            }
            break;
        case '6':
            return Terra_DisplayOutputMode_SSD1306;
        case '7':
            return Terra_DisplayOutputMode_SSD1607;
        case 'e':
            return Terra_DisplayOutputMode_Disabled;
        case 'n':
            return Terra_DisplayOutputMode_Undefined;
        case 'o':
            return Terra_DisplayOutputMode_CustomOLED;
        case 'v':
            return Terra_DisplayOutputMode_IL3820_V2;
    }
    return Terra_DisplayOutputMode_Undefined;
}

Terra_ControlInputMode controlInputModeFromString(String controlInModeStr)
{
    controlInModeStr.toLowerCase();
    switch (controlInModeStr.length() > 5 ? controlInModeStr[5] : '\000') {
        case '\000':
            return Terra_ControlInputMode_Count;
        case 'e':
            return Terra_ControlInputMode_RemoteControl;
        case 'g':
            return Terra_ControlInputMode_AnalogJoystickOk;
        case 'i':
            return Terra_ControlInputMode_Undefined;
        case 'l':
            return Terra_ControlInputMode_Disabled;
        case 'n':
            switch (controlInModeStr.length() > 15 ? controlInModeStr[15] : '\000') {
                case '\000':
                    return Terra_ControlInputMode_UpDownButtonsOk;
                case 'h':
                    switch (controlInModeStr.length() > 18 ? controlInModeStr[18] : '\000') {
                        case '\000':
                            return Terra_ControlInputMode_UpDownESP32TouchOk;
                        case 'l':
                            return Terra_ControlInputMode_UpDownESP32TouchOkLR;
                    }
                    break;
                case 'l':
                    return Terra_ControlInputMode_UpDownButtonsOkLR;
            }
            break;
        case 's':
            return Terra_ControlInputMode_TouchScreen;
        case 't':
            return Terra_ControlInputMode_ResistiveTouch;
        case 'u':
            return Terra_ControlInputMode_TFTTouch;
        case 'x':
            switch (controlInModeStr.length() > 6 ? controlInModeStr[6] : '\000') {
                case '2':
                    return Terra_ControlInputMode_Matrix2x2UpDownButtonsOkL;
                case '3':
                    switch (controlInModeStr.length() > 11 ? controlInModeStr[11] : '\000') {
                        case '\000':
                            return Terra_ControlInputMode_Matrix3x4Keyboard_OptRotEncOk;
                        case 'l':
                            return Terra_ControlInputMode_Matrix3x4Keyboard_OptRotEncOkLR;
                    }
                    break;
                case '4':
                    switch (controlInModeStr.length() > 11 ? controlInModeStr[11] : '\000') {
                        case '\000':
                            return Terra_ControlInputMode_Matrix4x4Keyboard_OptRotEncOk;
                        case 'l':
                            return Terra_ControlInputMode_Matrix4x4Keyboard_OptRotEncOkLR;
                    }
            }
            break;
        case 'y':
            switch (controlInModeStr.length() > 15 ? controlInModeStr[15] : '\000') {
                case '\000':
                    return Terra_ControlInputMode_RotaryEncoderOk;
                case 'l':
                    return Terra_ControlInputMode_RotaryEncoderOkLR;
            }
    }
    return Terra_ControlInputMode_Undefined;
}

Terra_ActuatorType actuatorTypeFromString(String actuatorTypeStr)
{
    actuatorTypeStr.toLowerCase();
    switch (actuatorTypeStr.length() > 2 ? actuatorTypeStr[2] : '\000') {
        case 'a':
            return Terra_ActuatorType_Heater;
        case 'd':
            return Terra_ActuatorType_Undefined;
        case 'l':
            return Terra_ActuatorType_Valve;
        case 'm':
            return Terra_ActuatorType_Pump;
        case 'n':
            return Terra_ActuatorType_Fan;
        case 'r':
            return Terra_ActuatorType_Circulator;
        case 'u':
            return Terra_ActuatorType_Count;
    }
    return Terra_ActuatorType_Undefined;
}

Terra_SensorType sensorTypeFromString(String sensorTypeStr)
{
    sensorTypeStr.toLowerCase();
    switch (sensorTypeStr.length() > 0 ? sensorTypeStr[0] : '\000') {
        case 'c':
            switch (sensorTypeStr.length() > 1 ? sensorTypeStr[1] : '\000') {
                case 'o':
                    return Terra_SensorType_Count;
                case 'u':
                    return Terra_SensorType_Current;
            }
            break;
        case 'f':
            return Terra_SensorType_Flow;
        case 'h':
            return Terra_SensorType_Humidity;
        case 'l':
            switch (sensorTypeStr.length() > 2 ? sensorTypeStr[2] : '\000') {
                case 'a':
                    return Terra_SensorType_Leak;
                case 'v':
                    return Terra_SensorType_Level;
            }
            break;
        case 'p':
            return Terra_SensorType_Pressure;
        case 'r':
            return Terra_SensorType_Rainfall;
        case 's':
            return Terra_SensorType_SolarRadiation;
        case 't':
            return Terra_SensorType_Temperature;
        case 'u':
            return Terra_SensorType_Undefined;
        case 'v':
            return Terra_SensorType_Voltage;
        case 'w':
            switch (sensorTypeStr.length() > 4 ? sensorTypeStr[4] : '\000') {
                case 'd':
                    return Terra_SensorType_WindDirection;
                case 's':
                    return Terra_SensorType_WindSpeed;
            }
    }
    return Terra_SensorType_Undefined;
}

Terra_ReservoirType reservoirTypeFromString(String reservoirTypeStr)
{
    reservoirTypeStr.toLowerCase();
    switch (reservoirTypeStr.length() > 0 ? reservoirTypeStr[0] : '\000') {
        case 'c':
            return Terra_ReservoirType_Count;
        case 't':
            return Terra_ReservoirType_Thermal;
        case 'u':
            return Terra_ReservoirType_Undefined;
        case 'w':
            return Terra_ReservoirType_Water;
    }
    return Terra_ReservoirType_Undefined;
}

Terra_RailType railTypeFromString(String railTypeStr)
{
    railTypeStr.toLowerCase();
    switch (railTypeStr.length() > 2 ? railTypeStr[2] : '\000') {
        case '1':
            switch (railTypeStr.length() > 0 ? railTypeStr[0] : '\000') {
                case 'a':
                    return Terra_RailType_AC110V;
                case 'd':
                    return Terra_RailType_DC12V;
            }
            break;
        case '2':
            switch (railTypeStr.length() > 0 ? railTypeStr[0] : '\000') {
                case 'a':
                    return Terra_RailType_AC220V;
                case 'd':
                    return Terra_RailType_DC24V;
            }
            break;
        case '3':
            return Terra_RailType_DC3V3;
        case '4':
            return Terra_RailType_DC48V;
        case '5':
            return Terra_RailType_DC5V;
        case 'd':
            return Terra_RailType_Undefined;
        case 'u':
            return Terra_RailType_Count;
    }
    return Terra_RailType_Undefined;
}

Terra_PinMode pinModeFromString(String pinModeStr)
{
    pinModeStr.toLowerCase();
    switch (pinModeStr.length() > 7 ? pinModeStr[7] : '\000') {
        case '\000':
            return Terra_PinMode_Count;
        case 'e':
            return Terra_PinMode_Undefined;
        case 'i':
            switch (pinModeStr.length() > 12 ? pinModeStr[12] : '\000') {
                case '\000':
                    return Terra_PinMode_Digital_Input;
                case 'p':
                    switch (pinModeStr.length() > 16 ? pinModeStr[16] : '\000') {
                        case 'd':
                            return Terra_PinMode_Digital_Input_PullDown;
                        case 'u':
                            return Terra_PinMode_Digital_Input_PullUp;
                    }
                    break;
            }
            break;
        case 'n':
            return Terra_PinMode_Analog_Input;
        case 'o':
            switch (pinModeStr.length() > 13 ? pinModeStr[13] : '\000') {
                case '\000':
                    return Terra_PinMode_Digital_Output;
                case 'p':
                    return Terra_PinMode_Digital_Output_PushPull;
            }
            break;
        case 'u':
            return Terra_PinMode_Analog_Output;
    }
    return Terra_PinMode_Undefined;
}

Terra_EnableMode enableModeFromString(String enableModeStr)
{
    enableModeStr.toLowerCase();
    switch (enableModeStr.length() > 2 ? enableModeStr[2] : '\000') {
        case 'c':
            return Terra_EnableMode_AscOrder;
        case 'd':
            return Terra_EnableMode_Undefined;
        case 'e':
            return Terra_EnableMode_Average;
        case 'g':
            return Terra_EnableMode_Highest;
        case 'l':
            return Terra_EnableMode_Multiply;
        case 'o':
            return Terra_EnableMode_InOrder;
        case 's':
            return Terra_EnableMode_DescOrder;
        case 'u':
            return Terra_EnableMode_Count;
        case 'v':
            return Terra_EnableMode_RevOrder;
        case 'w':
            return Terra_EnableMode_Lowest;
    }
    return Terra_EnableMode_Undefined;
}

Terra_UnitsCategory unitsCategoryFromString(String unitsCategoryStr)
{
    unitsCategoryStr.toLowerCase();
    switch (unitsCategoryStr.length() > 0 ? unitsCategoryStr[0] : '\000') {
        case 'a':
            return Terra_UnitsCategory_Angle;
        case 'c':
            return Terra_UnitsCategory_Count;
        case 'd':
            return Terra_UnitsCategory_Distance;
        case 'e':
            return Terra_UnitsCategory_Energy;
        case 'i':
            return Terra_UnitsCategory_Irradiance;
        case 'l':
            switch (unitsCategoryStr.length() > 6 ? unitsCategoryStr[6] : '\000') {
                case 'f':
                    return Terra_UnitsCategory_LiqFlowRate;
                case 'v':
                    return Terra_UnitsCategory_LiqVolume;
            }
            break;
        case 'p':
            switch (unitsCategoryStr.length() > 1 ? unitsCategoryStr[1] : '\000') {
                case 'o':
                    return Terra_UnitsCategory_Power;
                case 'r':
                    return Terra_UnitsCategory_Pressure;
            }
            break;
        case 's':
            return Terra_UnitsCategory_Speed;
        case 't':
            return Terra_UnitsCategory_Temperature;
        case 'u':
            return Terra_UnitsCategory_Undefined;
    }
    return Terra_UnitsCategory_Undefined;
}

Terra_UnitsType unitsTypeFromSymbol(String unitsSymbolStr)
{
    unitsSymbolStr.toLowerCase();
    switch (unitsSymbolStr.length() > 0 ? unitsSymbolStr[0] : '\000') {
        case '%':
            return Terra_UnitsType_Percentile_100;
        case 'a':
            return Terra_UnitsType_Power_Amperage;
        case 'c':
            switch (unitsSymbolStr.length() > 1 ? unitsSymbolStr[1] : '\000') {
                case '\000':
                    return Terra_UnitsType_Temperature_Celsius;
                case 'o':
                    return Terra_UnitsType_Count;
            }
            break;
        case 'd':
            return Terra_UnitsType_Angle_Degrees_360;
        case 'f':
            switch (unitsSymbolStr.length() > 1 ? unitsSymbolStr[1] : '\000') {
                case '\000':
                    return Terra_UnitsType_Temperature_Fahrenheit;
                case 't':
                    return Terra_UnitsType_Distance_Feet;
            }
            break;
        case 'g':
            switch (unitsSymbolStr.length() > 3 ? unitsSymbolStr[3] : '\000') {
                case '\000':
                    return Terra_UnitsType_LiqVolume_Gallons;
                case '/':
                    return Terra_UnitsType_LiqFlowRate_GallonsPerMin;
            }
            break;
        case 'h':
            return Terra_UnitsType_Pressure_Hectopascals;
        case 'i':
            switch (unitsSymbolStr.length() > 2 ? unitsSymbolStr[2] : '\000') {
                case '\000':
                    return Terra_UnitsType_Distance_Inches;
                case '/':
                    return Terra_UnitsType_Speed_InchesPerHour;
            }
            break;
        case 'k':
            switch (unitsSymbolStr.length() > 1 ? unitsSymbolStr[1] : '\000') {
                case '\000':
                    return Terra_UnitsType_Temperature_Kelvin;
                case 'm':
                    return Terra_UnitsType_Speed_KilometersPerHour;
                case 'p':
                    return Terra_UnitsType_Pressure_Kilopascals;
                case 'w':
                    return Terra_UnitsType_Energy_KilowattHours;
            }
            break;
        case 'l':
            switch (unitsSymbolStr.length() > 1 ? unitsSymbolStr[1] : '\000') {
                case '\000':
                    return Terra_UnitsType_LiqVolume_Liters;
                case '/':
                    return Terra_UnitsType_LiqFlowRate_LitersPerMin;
            }
            break;
        case 'm':
            switch (unitsSymbolStr.length() > 1 ? unitsSymbolStr[1] : '\000') {
                case '\000':
                    return Terra_UnitsType_Distance_Meters;
                case '/':
                    return Terra_UnitsType_Speed_MetersPerSecond;
                case 'i':
                    return Terra_UnitsType_Angle_Minutes_24hr;
                case 'm':
                    switch (unitsSymbolStr.length() > 2 ? unitsSymbolStr[2] : '\000') {
                        case '\000':
                            return Terra_UnitsType_Distance_Millimeters;
                        case '/':
                            return Terra_UnitsType_Speed_MillimetersPerHour;
                    }
                    break;
                case 'p':
                    return Terra_UnitsType_Speed_MilesPerHour;
            }
            break;
        case 'p':
            return Terra_UnitsType_Pressure_PSI;
        case 'r':
            switch (unitsSymbolStr.length() > 2 ? unitsSymbolStr[2] : '\000') {
                case 'd':
                    return Terra_UnitsType_Angle_Radians_2pi;
                case 'w':
                    return Terra_UnitsType_Raw_1;
            }
            break;
        case 'u':
            return Terra_UnitsType_Undefined;
        case 'v':
            return Terra_UnitsType_Power_Volts;
        case 'w':
            switch (unitsSymbolStr.length() > 1 ? unitsSymbolStr[1] : '\000') {
                case '\000':
                    return Terra_UnitsType_Power_Wattage;
                case '/':
                    return Terra_UnitsType_Irradiance_WattsPerSquareMeter;
            }
    }
    return Terra_UnitsType_Undefined;
}

