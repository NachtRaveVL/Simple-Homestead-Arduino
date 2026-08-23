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

                case Terra_UnitsType_Alkalinity_pH_14:
                    *valueOut = valueIn * 14.0;
                    return true;

                case Terra_UnitsType_Concentration_EC_5:
                    *valueOut = valueIn * 5.0;
                    return true;

                case Terra_UnitsType_Concentration_PPM_500:
                    *valueOut = valueIn * (5.0 * 500.0);
                    return true;

                case Terra_UnitsType_Concentration_PPM_640:
                    *valueOut = valueIn * (5.0 * 640.0);
                    return true;

                case Terra_UnitsType_Concentration_PPM_700:
                    *valueOut = valueIn * (5.0 * 700.0);
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

        case Terra_UnitsType_Alkalinity_pH_14:
            switch (unitsOut) {
                case Terra_UnitsType_Raw_1:
                    *valueOut = valueIn / 14.0;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Concentration_EC_5:
            switch (unitsOut) {
                case Terra_UnitsType_Raw_1:
                    *valueOut = valueIn / 5.0;
                    return true;

                case Terra_UnitsType_Concentration_PPM_500:
                    *valueOut = valueIn * 500.0;
                    return true;

                case Terra_UnitsType_Concentration_PPM_640:
                    *valueOut = valueIn * 640.0;
                    return true;

                case Terra_UnitsType_Concentration_PPM_700:
                    *valueOut = valueIn * 700.0;
                    return true;

                default:
                    break;
            }
            break;
        
        case Terra_UnitsType_Concentration_PPM_500:
            switch (unitsOut) {
                case Terra_UnitsType_Raw_1:
                    *valueOut = valueIn / (5.0 * 500.0);
                    return true;

                case Terra_UnitsType_Concentration_EC_5:
                    *valueOut = valueIn / 500.0;
                    return true;

                case Terra_UnitsType_Concentration_PPM_640:
                    *valueOut = valueIn / 500.0 * 640.0;
                    return true;

                case Terra_UnitsType_Concentration_PPM_700:
                    *valueOut = valueIn / 500.0 * 700.0;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Concentration_PPM_640:
            switch (unitsOut) {
                case Terra_UnitsType_Raw_1:
                    *valueOut = valueIn / (5.0 * 640.0);
                    return true;

                case Terra_UnitsType_Concentration_EC_5:
                    *valueOut = valueIn / 640.0;
                    return true;

                case Terra_UnitsType_Concentration_PPM_500:
                    *valueOut = valueIn / 640.0 * 500.0;
                    return true;

                case Terra_UnitsType_Concentration_PPM_700:
                    *valueOut = valueIn / 640.0 * 700.0;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Concentration_PPM_700:
            switch (unitsOut) {
                case Terra_UnitsType_Raw_1:
                    *valueOut = valueIn / (5.0 * 700.0);
                    return true;

                case Terra_UnitsType_Concentration_EC_5:
                    *valueOut = valueIn / 700.0;
                    return true;

                case Terra_UnitsType_Concentration_PPM_500:
                    *valueOut = valueIn / 700.0 * 500.0;
                    return true;

                case Terra_UnitsType_Concentration_PPM_640:
                    *valueOut = valueIn / 700.0 * 640.0;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Distance_Feet:
            switch (unitsOut) {
                case Terra_UnitsType_Distance_Meters:
                    *valueOut = valueIn * 0.3048;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Distance_Meters:
            switch (unitsOut) {
                case Terra_UnitsType_Distance_Feet:
                    *valueOut = valueIn * 3.28084;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_LiqVolume_Gallons:
            switch (unitsOut) {
                case Terra_UnitsType_LiqVolume_Liters:
                    *valueOut = valueIn * 3.78541;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_LiqVolume_Liters:
            switch (unitsOut) {
                case Terra_UnitsType_LiqVolume_Gallons:
                    *valueOut = valueIn * 0.264172;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_LiqFlowRate_GallonsPerMin:
            switch (unitsOut) {
                case Terra_UnitsType_LiqFlowRate_LitersPerMin:
                    *valueOut = valueIn * 3.78541;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_LiqFlowRate_LitersPerMin:
            switch (unitsOut) {
                case Terra_UnitsType_LiqFlowRate_GallonsPerMin:
                    *valueOut = valueIn * 0.264172;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_LiqDilution_MilliLiterPerGallon:
            switch (unitsOut) {
                case Terra_UnitsType_LiqDilution_MilliLiterPerLiter:
                    *valueOut = valueIn * 0.264172;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_LiqDilution_MilliLiterPerLiter:
            switch (unitsOut) {
                case Terra_UnitsType_LiqDilution_MilliLiterPerGallon:
                    *valueOut = valueIn * 3.78541;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Power_Amperage:
            switch (unitsOut) {
                case Terra_UnitsType_Power_Wattage:
                    if (convertParam != FLT_UNDEF) { // convertParam = rail voltage
                        *valueOut = valueIn * convertParam;
                        return true;
                    }
                break;
            }
            break;

        case Terra_UnitsType_Power_Wattage:
            switch (unitsOut) {
                case Terra_UnitsType_Power_Amperage:
                    if (convertParam != FLT_UNDEF) { // convertParam = rail voltage
                        *valueOut = valueIn / convertParam;
                        return true;
                    }
                break;
            }
            break;

        case Terra_UnitsType_Temperature_Celsius:
            switch (unitsOut) {
                case Terra_UnitsType_Temperature_Fahrenheit:
                    *valueOut = valueIn * 1.8 + 32.0;
                    return true;

                case Terra_UnitsType_Temperature_Kelvin:
                    *valueOut = valueIn + 273.15;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Temperature_Fahrenheit:
            switch (unitsOut) {
                case Terra_UnitsType_Temperature_Celsius:
                    *valueOut = (valueIn - 32.0) / 1.8;
                    return true;

                case Terra_UnitsType_Temperature_Kelvin:
                    *valueOut = ((valueIn + 459.67) * 5.0) / 9.0;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Temperature_Kelvin:
            switch (unitsOut) {
                case Terra_UnitsType_Temperature_Celsius:
                    *valueOut = valueIn - 273.15;
                    return true;

                case Terra_UnitsType_Temperature_Fahrenheit:
                    *valueOut = ((valueIn * 9.0) / 5.0) - 459.67;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Weight_Kilograms:
            switch (unitsOut) {
                case Terra_UnitsType_Weight_Pounds:
                    *valueOut = valueIn * 2.20462;
                    return true;

                default:
                    break;
            }
            break;

        case Terra_UnitsType_Weight_Pounds:
            switch (unitsOut) {
                case Terra_UnitsType_Weight_Kilograms:
                    *valueOut = valueIn * 0.453592;
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
        case Terra_UnitsType_LiqDilution_MilliLiterPerLiter:
            return Terra_UnitsType_LiqVolume_Liters;
        case Terra_UnitsType_LiqFlowRate_GallonsPerMin:
        case Terra_UnitsType_LiqDilution_MilliLiterPerGallon:
            return Terra_UnitsType_LiqVolume_Gallons;
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
        default:
            return Terra_UnitsType_Undefined;
    }
}

Terra_UnitsType volumeUnits(Terra_UnitsType units)
{
    switch (units) {
        case Terra_UnitsType_LiqDilution_MilliLiterPerLiter:
            return Terra_UnitsType_LiqVolume_Liters;
        case Terra_UnitsType_LiqDilution_MilliLiterPerGallon:
            return Terra_UnitsType_LiqVolume_Gallons;
        default:
            return Terra_UnitsType_Undefined;
    }
}

Terra_UnitsType dilutionUnits(Terra_UnitsType units)
{
    switch (units) {
        case Terra_UnitsType_LiqDilution_MilliLiterPerLiter:
            return Terra_UnitsType_LiqVolume_Liters;
        case Terra_UnitsType_LiqDilution_MilliLiterPerGallon:
            return Terra_UnitsType_LiqVolume_Gallons;
        default:
            return Terra_UnitsType_Undefined;
    }
}

Terra_UnitsType defaultUnits(Terra_UnitsCategory unitsCategory, Terra_MeasurementMode measureMode)
{
    measureMode = (measureMode == Terra_MeasurementMode_Undefined && getController() ? getController()->getMeasurementMode() : measureMode);

    switch (unitsCategory) {
        case Terra_UnitsCategory_Alkalinity:
            return Terra_UnitsType_Alkalinity_pH_14;

        case Terra_UnitsCategory_Concentration:
            return Terra_UnitsType_Concentration_EC_5;

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

        case Terra_UnitsCategory_LiqDilution:
            switch (measureMode) {
                case Terra_MeasurementMode_Imperial:
                    return Terra_UnitsType_LiqDilution_MilliLiterPerGallon;
                case Terra_MeasurementMode_Metric:
                case Terra_MeasurementMode_Scientific:
                    return Terra_UnitsType_LiqDilution_MilliLiterPerLiter;
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

        case Terra_UnitsCategory_Percentile:
            return Terra_UnitsType_Percentile_100;

        case Terra_UnitsCategory_Power:
            return Terra_UnitsType_Power_Wattage;

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

        case Terra_UnitsCategory_Weight:
            switch (measureMode) {
                case Terra_MeasurementMode_Imperial:
                    return Terra_UnitsType_Weight_Pounds;
                case Terra_MeasurementMode_Metric:
                case Terra_MeasurementMode_Scientific:
                    return Terra_UnitsType_Weight_Kilograms;
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


int linksCountTrackableTargets(Pair<uint8_t, Pair<TerraObject *, int8_t> *> links)
{
    int retVal = 0;

    for (tposi_t linksIndex = 0; linksIndex < links.first && links.second[linksIndex].first; ++linksIndex) {
        if (links.second[linksIndex].first->isTargetType()) {
            auto target = static_cast<TerraTarget *>(links.second[linksIndex].first);

            // TODO: See if object is trackable.
        }
    }

    return retVal;
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
        case Terra_SystemMode_Recycling:
            return SFP(TStr_Enum_Recycling);
        case Terra_SystemMode_DrainToWaste:
            return SFP(TStr_Enum_DrainToWaste);
        case Terra_SystemMode_Count:
            return !excludeSpecial ? SFP(TStr_Count) : String();
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
            return !excludeSpecial ? SFP(TStr_Count) : String();
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
            return !excludeSpecial ? SFP(TStr_Count) : String();
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
            return !excludeSpecial ? SFP(TStr_Count) : String();
        case Terra_ControlInputMode_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(TStr_Undefined) : String();
}

String actuatorTypeToString(Terra_ActuatorType actuatorType, bool excludeSpecial)
{
    switch (actuatorType) {
        case Terra_ActuatorType_FanExhaust:
            return SFP(TStr_Enum_FanExhaust);
        case Terra_ActuatorType_GrowLights:
            return SFP(TStr_Enum_GrowLights);
        case Terra_ActuatorType_PeristalticPump:
            return SFP(TStr_Enum_PeristalticPump);
        case Terra_ActuatorType_WaterAerator:
            return SFP(TStr_Enum_WaterAerator);
        case Terra_ActuatorType_WaterHeater:
            return SFP(TStr_Enum_WaterHeater);
        case Terra_ActuatorType_WaterPump:
            return SFP(TStr_Enum_WaterPump);
        case Terra_ActuatorType_WaterSprayer:
            return SFP(TStr_Enum_WaterSprayer);
        case Terra_ActuatorType_Count:
            return !excludeSpecial ? SFP(TStr_Count) : String();
        case Terra_ActuatorType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(TStr_Undefined) : String();
}

String sensorTypeToString(Terra_SensorType sensorType, bool excludeSpecial)
{
    switch (sensorType) {
        case Terra_SensorType_AirCarbonDioxide:
            return SFP(TStr_Enum_AirCarbonDioxide);
        case Terra_SensorType_AirTempHumidity:
            return SFP(TStr_Enum_AirTemperatureHumidity);
        case Terra_SensorType_PotentialTerragen:
            return SFP(TStr_Enum_WaterPH);
        case Terra_SensorType_PowerLevel:
            return SFP(TStr_Enum_PowerLevel);
        case Terra_SensorType_PumpFlow:
            return SFP(TStr_Enum_PumpFlow);
        case Terra_SensorType_SoilMoisture:
            return SFP(TStr_Enum_SoilMoisture);
        case Terra_SensorType_TotalDissolvedSolids:
            return SFP(TStr_Enum_WaterTDS);
        case Terra_SensorType_WaterHeight:
            return SFP(TStr_Enum_WaterHeight);
        case Terra_SensorType_WaterLevel:
            return SFP(TStr_Enum_WaterLevel);
        case Terra_SensorType_WaterTemperature:
            return SFP(TStr_Enum_WaterTemperature);
        case Terra_SensorType_Count:
            return !excludeSpecial ? SFP(TStr_Count) : String();
        case Terra_SensorType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(TStr_Undefined) : String();
}

String targetTypeToString(Terra_TargetType targetType, bool excludeSpecial)
{
    switch (targetType) {
        case Terra_TargetType_AloeVera:
            return SFP(TStr_Enum_AloeVera);
        case Terra_TargetType_Anise:
            return SFP(TStr_Enum_Anise);
        case Terra_TargetType_Artichoke:
            return SFP(TStr_Enum_Artichoke);
        case Terra_TargetType_Arugula:
            return SFP(TStr_Enum_Arugula);
        case Terra_TargetType_Asparagus:
            return SFP(TStr_Enum_Asparagus);
        case Terra_TargetType_Basil:
            return SFP(TStr_Enum_Basil);
        case Terra_TargetType_Bean:
            return SFP(TStr_Enum_Bean);
        case Terra_TargetType_BeanBroad:
            return SFP(TStr_Enum_BeanBroad);
        case Terra_TargetType_Beetroot:
            return SFP(TStr_Enum_Beetroot);
        case Terra_TargetType_BlackCurrant:
            return SFP(TStr_Enum_BlackCurrant);
        case Terra_TargetType_Blueberry:
            return SFP(TStr_Enum_Blueberry);
        case Terra_TargetType_BokChoi:
            return SFP(TStr_Enum_BokChoi);
        case Terra_TargetType_Broccoli:
            return SFP(TStr_Enum_Broccoli);
        case Terra_TargetType_BrusselsSprout:
            return SFP(TStr_Enum_BrusselsSprout);
        case Terra_TargetType_Cabbage:
            return SFP(TStr_Enum_Cabbage);
        case Terra_TargetType_Cannabis:
            return SFP(TStr_Enum_Cannabis);
        case Terra_TargetType_Capsicum:
            return SFP(TStr_Enum_Capsicum);
        case Terra_TargetType_Carrots:
            return SFP(TStr_Enum_Carrots);
        case Terra_TargetType_Catnip:
            return SFP(TStr_Enum_Catnip);
        case Terra_TargetType_Cauliflower:
            return SFP(TStr_Enum_Cauliflower);
        case Terra_TargetType_Celery:
            return SFP(TStr_Enum_Celery);
        case Terra_TargetType_Chamomile:
            return SFP(TStr_Enum_Chamomile);
        case Terra_TargetType_Chicory:
            return SFP(TStr_Enum_Chicory);
        case Terra_TargetType_Chives:
            return SFP(TStr_Enum_Chives);
        case Terra_TargetType_Cilantro:
            return SFP(TStr_Enum_Cilantro);
        case Terra_TargetType_Coriander:
            return SFP(TStr_Enum_Coriander);
        case Terra_TargetType_CornSweet:
            return SFP(TStr_Enum_CornSweet);
        case Terra_TargetType_Cucumber:
            return SFP(TStr_Enum_Cucumber);
        case Terra_TargetType_Dill:
            return SFP(TStr_Enum_Dill);
        case Terra_TargetType_Eggplant:
            return SFP(TStr_Enum_Eggplant);
        case Terra_TargetType_Endive:
            return SFP(TStr_Enum_Endive);
        case Terra_TargetType_Fennel:
            return SFP(TStr_Enum_Fennel);
        case Terra_TargetType_Fodder:
            return SFP(TStr_Enum_Fodder);
        case Terra_TargetType_Flowers:
            return SFP(TStr_Enum_Flowers);
        case Terra_TargetType_Garlic:
            return SFP(TStr_Enum_Garlic);
        case Terra_TargetType_Ginger:
            return SFP(TStr_Enum_Ginger);
        case Terra_TargetType_Kale:
            return SFP(TStr_Enum_Kale);
        case Terra_TargetType_Lavender:
            return SFP(TStr_Enum_Lavender);
        case Terra_TargetType_Leek:
            return SFP(TStr_Enum_Leek);
        case Terra_TargetType_LemonBalm:
            return SFP(TStr_Enum_LemonBalm);
        case Terra_TargetType_Lettuce:
            return SFP(TStr_Enum_Lettuce);
        case Terra_TargetType_Marrow:
            return SFP(TStr_Enum_Marrow);
        case Terra_TargetType_Melon:
            return SFP(TStr_Enum_Melon);
        case Terra_TargetType_Mint:
            return SFP(TStr_Enum_Mint);
        case Terra_TargetType_MustardCress:
            return SFP(TStr_Enum_MustardCress);
        case Terra_TargetType_Okra:
            return SFP(TStr_Enum_Okra);
        case Terra_TargetType_Onions:
            return SFP(TStr_Enum_Onions);
        case Terra_TargetType_Oregano:
            return SFP(TStr_Enum_Oregano);
        case Terra_TargetType_PakChoi:
            return SFP(TStr_Enum_PakChoi);
        case Terra_TargetType_Parsley:
            return SFP(TStr_Enum_Parsley);
        case Terra_TargetType_Parsnip:
            return SFP(TStr_Enum_Parsnip);
        case Terra_TargetType_Pea:
            return SFP(TStr_Enum_Pea);
        case Terra_TargetType_PeaSugar:
            return SFP(TStr_Enum_PeaSugar);
        case Terra_TargetType_Pepino:
            return SFP(TStr_Enum_Pepino);
        case Terra_TargetType_PeppersBell:
            return SFP(TStr_Enum_PeppersBell);
        case Terra_TargetType_PeppersHot:
            return SFP(TStr_Enum_PeppersHot);
        case Terra_TargetType_Potato:
            return SFP(TStr_Enum_Potato);
        case Terra_TargetType_PotatoSweet:
            return SFP(TStr_Enum_PotatoSweet);
        case Terra_TargetType_Pumpkin:
            return SFP(TStr_Enum_Pumpkin);
        case Terra_TargetType_Radish:
            return SFP(TStr_Enum_Radish);
        case Terra_TargetType_Rhubarb:
            return SFP(TStr_Enum_Rhubarb);
        case Terra_TargetType_Rosemary:
            return SFP(TStr_Enum_Rosemary);
        case Terra_TargetType_Sage:
            return SFP(TStr_Enum_Sage);
        case Terra_TargetType_Silverbeet:
            return SFP(TStr_Enum_Silverbeet);
        case Terra_TargetType_Spinach:
            return SFP(TStr_Enum_Spinach);
        case Terra_TargetType_Squash:
            return SFP(TStr_Enum_Squash);
        case Terra_TargetType_Sunflower:
            return SFP(TStr_Enum_Sunflower);
        case Terra_TargetType_Strawberries:
            return SFP(TStr_Enum_Strawberries);
        case Terra_TargetType_SwissChard:
            return SFP(TStr_Enum_SwissChard);
        case Terra_TargetType_Taro:
            return SFP(TStr_Enum_Taro);
        case Terra_TargetType_Tarragon:
            return SFP(TStr_Enum_Tarragon);
        case Terra_TargetType_Thyme:
            return SFP(TStr_Enum_Thyme);
        case Terra_TargetType_Tomato:
            return SFP(TStr_Enum_Tomato);
        case Terra_TargetType_Turnip:
            return SFP(TStr_Enum_Turnip);
        case Terra_TargetType_Watercress:
            return SFP(TStr_Enum_Watercress);
        case Terra_TargetType_Watermelon:
            return SFP(TStr_Enum_Watermelon);
        case Terra_TargetType_Zucchini:
            return SFP(TStr_Enum_Zucchini);
        case Terra_TargetType_CustomTarget1:
            return SFP(TStr_Enum_CustomTarget1);
        case Terra_TargetType_CustomTarget2:
            return SFP(TStr_Enum_CustomTarget2);
        case Terra_TargetType_CustomTarget3:
            return SFP(TStr_Enum_CustomTarget3);
        case Terra_TargetType_CustomTarget4:
            return SFP(TStr_Enum_CustomTarget4);
        case Terra_TargetType_CustomTarget5:
            return SFP(TStr_Enum_CustomTarget5);
        case Terra_TargetType_CustomTarget6:
            return SFP(TStr_Enum_CustomTarget6);
        case Terra_TargetType_CustomTarget7:
            return SFP(TStr_Enum_CustomTarget7);
        case Terra_TargetType_CustomTarget8:
            return SFP(TStr_Enum_CustomTarget8);
        case Terra_TargetType_Count:
            return !excludeSpecial ? SFP(TStr_Count) : String();
        case Terra_TargetType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(TStr_Undefined) : String();
}

String substrateTypeToString(Terra_SubstrateType substrateType, bool excludeSpecial)
{
    switch (substrateType) {
        case Terra_SubstrateType_ClayPebbles:
            return SFP(TStr_Enum_ClayPebbles);
        case Terra_SubstrateType_CoconutCoir:
            return SFP(TStr_Enum_CoconutCoir);
        case Terra_SubstrateType_Rockwool:
            return SFP(TStr_Enum_Rockwool);
        case Terra_SubstrateType_Count:
            return !excludeSpecial ? SFP(TStr_Count) : String();
        case Terra_SubstrateType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(TStr_Undefined) : String();
}

String reservoirTypeToString(Terra_ReservoirType reservoirType, bool excludeSpecial)
{
    switch (reservoirType) {
        case Terra_ReservoirType_FeedWater:
            return SFP(TStr_Enum_FeedWater);
        case Terra_ReservoirType_DrainageWater:
            return SFP(TStr_Enum_DrainageWater);
        case Terra_ReservoirType_NutrientPremix:
            return SFP(TStr_Enum_NutrientPremix);
        case Terra_ReservoirType_FreshWater:
            return SFP(TStr_Enum_FreshWater);
        case Terra_ReservoirType_PhUpSolution:
            return SFP(TStr_Enum_PhUpSolution);
        case Terra_ReservoirType_PhDownSolution:
            return SFP(TStr_Enum_PhDownSolution);
        case Terra_ReservoirType_CustomAdditive1:
            return SFP(TStr_Enum_CustomAdditive1);
        case Terra_ReservoirType_CustomAdditive2:
            return SFP(TStr_Enum_CustomAdditive2);
        case Terra_ReservoirType_CustomAdditive3:
            return SFP(TStr_Enum_CustomAdditive3);
        case Terra_ReservoirType_CustomAdditive4:
            return SFP(TStr_Enum_CustomAdditive4);
        case Terra_ReservoirType_CustomAdditive5:
            return SFP(TStr_Enum_CustomAdditive5);
        case Terra_ReservoirType_CustomAdditive6:
            return SFP(TStr_Enum_CustomAdditive6);
        case Terra_ReservoirType_CustomAdditive7:
            return SFP(TStr_Enum_CustomAdditive7);
        case Terra_ReservoirType_CustomAdditive8:
            return SFP(TStr_Enum_CustomAdditive8);
        case Terra_ReservoirType_CustomAdditive9:
            return SFP(TStr_Enum_CustomAdditive9);
        case Terra_ReservoirType_CustomAdditive10:
            return SFP(TStr_Enum_CustomAdditive10);
        case Terra_ReservoirType_CustomAdditive11:
            return SFP(TStr_Enum_CustomAdditive11);
        case Terra_ReservoirType_CustomAdditive12:
            return SFP(TStr_Enum_CustomAdditive12);
        case Terra_ReservoirType_CustomAdditive13:
            return SFP(TStr_Enum_CustomAdditive13);
        case Terra_ReservoirType_CustomAdditive14:
            return SFP(TStr_Enum_CustomAdditive14);
        case Terra_ReservoirType_CustomAdditive15:
            return SFP(TStr_Enum_CustomAdditive15);
        case Terra_ReservoirType_CustomAdditive16:
            return SFP(TStr_Enum_CustomAdditive16);
        case Terra_ReservoirType_Count:
            return !excludeSpecial ? SFP(TStr_Count) : String();
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
            return !excludeSpecial ? SFP(TStr_Count) : String();
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
            return !excludeSpecial ? SFP(TStr_Count) : String();
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
            return !excludeSpecial ? SFP(TStr_Count) : String();
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
        case Terra_UnitsCategory_Alkalinity:
            return SFP(TStr_Enum_Alkalinity);
        case Terra_UnitsCategory_Concentration:
            return SFP(TStr_Enum_Concentration);
        case Terra_UnitsCategory_Distance:
            return SFP(TStr_Enum_Distance);
        case Terra_UnitsCategory_LiqDilution:
            return SFP(TStr_Enum_LiqDilution);
        case Terra_UnitsCategory_LiqFlowRate:
            return SFP(TStr_Enum_LiqFlowRate);
        case Terra_UnitsCategory_LiqVolume:
            return SFP(TStr_Enum_LiqVolume);
        case Terra_UnitsCategory_Percentile:
            return SFP(TStr_Enum_Percentile);
        case Terra_UnitsCategory_Power:
            return SFP(TStr_Enum_Power);
        case Terra_UnitsCategory_Temperature:
            return SFP(TStr_Enum_Temperature);
        case Terra_UnitsCategory_Weight:
            return SFP(TStr_Enum_Weight);
        case Terra_UnitsCategory_Count:
            return !excludeSpecial ? SFP(TStr_Count) : String();
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
            return String('%');
        case Terra_UnitsType_Alkalinity_pH_14:
            return !excludeSpecial ? SFP(TStr_Unit_pH14) : String(); // technically unitless
        case Terra_UnitsType_Concentration_EC_5:
            return SFP(TStr_Unit_EC5); // alt: mS/cm, TDS
        case Terra_UnitsType_Concentration_PPM_500:
            return SFP(TStr_Unit_PPM500);
        case Terra_UnitsType_Concentration_PPM_640:
            return SFP(TStr_Unit_PPM640);
        case Terra_UnitsType_Concentration_PPM_700:
            return SFP(TStr_Unit_PPM700);
        case Terra_UnitsType_Distance_Feet:
            return SFP(TStr_Unit_Feet);
        case Terra_UnitsType_Distance_Meters:
            return String('m');
        case Terra_UnitsType_LiqDilution_MilliLiterPerGallon: {
            String retVal(SFP(TStr_Unit_MilliLiterPer));
            String concat(SFP(TStr_Unit_Gallons));
            retVal.reserve(retVal.length() + concat.length() + 1);
            retVal.concat(concat);
            return retVal;
        }
        case Terra_UnitsType_LiqDilution_MilliLiterPerLiter: {
            String retVal(SFP(TStr_Unit_MilliLiterPer));
            retVal.reserve(retVal.length() + 1 + 1);
            retVal.concat('L');
            return retVal;
        }
        case Terra_UnitsType_LiqFlowRate_GallonsPerMin: {
            String retVal(SFP(TStr_Unit_Gallons));
            String concat(SFP(TStr_Unit_PerMinute));
            retVal.reserve(retVal.length() + concat.length() + 1);
            retVal.concat(concat);
            return retVal;
        }
        case Terra_UnitsType_LiqFlowRate_LitersPerMin: {
            String retVal('L');
            String concat(SFP(TStr_Unit_PerMinute));
            retVal.reserve(retVal.length() + concat.length() + 1);
            retVal.concat(concat);
            return retVal;
        }
        case Terra_UnitsType_LiqVolume_Gallons:
            return SFP(TStr_Unit_Gallons);
        case Terra_UnitsType_LiqVolume_Liters:
            return String('L');
        case Terra_UnitsType_Power_Amperage:
            return String('A');
        case Terra_UnitsType_Power_Wattage:
            return String('W'); // alt: J/s
        case Terra_UnitsType_Temperature_Celsius: {
            String retVal(SFP(TStr_Unit_Degree));
            retVal.reserve(retVal.length() + 1 + 1);
            retVal.concat('C');
            return retVal;
        }
        case Terra_UnitsType_Temperature_Fahrenheit: {
            String retVal(SFP(TStr_Unit_Degree));
            retVal.reserve(retVal.length() + 1 + 1);
            retVal.concat('F');
            return retVal;
        }
        case Terra_UnitsType_Temperature_Kelvin: {
            String retVal(SFP(TStr_Unit_Degree));
            retVal.reserve(retVal.length() + 1 + 1);
            retVal.concat('K');
            return retVal;
        }
        case Terra_UnitsType_Weight_Kilograms:
            return SFP(TStr_Unit_Kilograms);
        case Terra_UnitsType_Weight_Pounds:
            return SFP(TStr_Unit_Pounds);
        case Terra_UnitsType_Count:
            return !excludeSpecial ? SFP(TStr_Unit_Count) : String();
        case Terra_UnitsType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(TStr_Unit_Undefined) : String();
}

String positionIndexToString(tposi_t positionIndex, bool excludeSpecial)
{
    if (positionIndex >= 0 && positionIndex < TERRA_POS_MAXSIZE) {
        return String(positionIndex + TERRA_POS_EXPORT_BEGFROM);
    } else if (!excludeSpecial) {
        if (positionIndex == TERRA_POS_MAXSIZE) {
            return SFP(TStr_Count);
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
        switch (systemModeStr.length() >= 1 ? systemModeStr[0] : '\000') {
        case 'C':
            return (Terra_SystemMode)2;
        case 'D':
            return (Terra_SystemMode)1;
        case 'R':
            return (Terra_SystemMode)0;
        case 'U':
            return (Terra_SystemMode)-1;
    }
    return Terra_SystemMode_Undefined;
}

Terra_MeasurementMode measurementModeFromString(String measurementModeStr)
{
    switch (measurementModeStr.length() >= 1 ? measurementModeStr[0] : '\000') {
        case 'C':
            return (Terra_MeasurementMode)3;
        case 'I':
            return (Terra_MeasurementMode)0;
        case 'M':
            return (Terra_MeasurementMode)1;
        case 'S':
            return (Terra_MeasurementMode)2;
        case 'U':
            return (Terra_MeasurementMode)-1;
    }
    return Terra_MeasurementMode_Undefined;
}

Terra_DisplayOutputMode displayOutputModeFromString(String displayOutModeStr)
{
    switch (displayOutModeStr.length() >= 1 ? displayOutModeStr[0] : '\000') {
        case 'C':
            switch (displayOutModeStr.length() >= 2 ? displayOutModeStr[1] : '\000') {
                case 'o':
                    return (Terra_DisplayOutputMode)18;
                case 'u':
                    return (Terra_DisplayOutputMode)10;
            }
            break;
        case 'D':
            return (Terra_DisplayOutputMode)0;
        case 'I':
            switch (displayOutModeStr.length() >= 3 ? displayOutModeStr[2] : '\000') {
                case '3':
                    switch (displayOutModeStr.length() >= 7 ? displayOutModeStr[6] : '\000') {
                        case '\000':
                            return (Terra_DisplayOutputMode)12;
                        case 'V':
                            return (Terra_DisplayOutputMode)13;
                    }
                    break;
                case 'I':
                    return (Terra_DisplayOutputMode)16;
            }
            break;
        case 'L':
            switch (displayOutModeStr.length() >= 4 ? displayOutModeStr[3] : '\000') {
                case '1':
                    switch (displayOutModeStr.length() >= 8 ? displayOutModeStr[7] : '\000') {
                        case 'E':
                            return (Terra_DisplayOutputMode)1;
                        case 'R':
                            return (Terra_DisplayOutputMode)2;
                    }
                    break;
                case '2':
                    switch (displayOutModeStr.length() >= 8 ? displayOutModeStr[7] : '\000') {
                        case 'E':
                            return (Terra_DisplayOutputMode)3;
                        case 'R':
                            return (Terra_DisplayOutputMode)4;
                    }
                    break;
            }
            break;
        case 'S':
            switch (displayOutModeStr.length() >= 2 ? displayOutModeStr[1] : '\000') {
                case 'H':
                    return (Terra_DisplayOutputMode)9;
                case 'S':
                    switch (displayOutModeStr.length() >= 5 ? displayOutModeStr[4] : '\000') {
                        case '3':
                            switch (displayOutModeStr.length() >= 7 ? displayOutModeStr[6] : '\000') {
                                case '5':
                                    switch (displayOutModeStr.length() >= 8 ? displayOutModeStr[7] : '\000') {
                                        case '\000':
                                            return (Terra_DisplayOutputMode)5;
                                        case 'x':
                                            switch (displayOutModeStr.length() >= 9 ? displayOutModeStr[8] : '\000') {
                                                case '3':
                                                    return (Terra_DisplayOutputMode)6;
                                                case '6':
                                                    return (Terra_DisplayOutputMode)7;
                                            }
                                            break;
                                    }
                                    break;
                                case '6':
                                    return (Terra_DisplayOutputMode)8;
                            }
                            break;
                        case '6':
                            return (Terra_DisplayOutputMode)11;
                    }
                    break;
                case 'T':
                    switch (displayOutModeStr.length() >= 5 ? displayOutModeStr[4] : '\000') {
                        case '3':
                            return (Terra_DisplayOutputMode)14;
                        case '8':
                            return (Terra_DisplayOutputMode)15;
                    }
                    break;
            }
            break;
        case 'T':
            return (Terra_DisplayOutputMode)17;
        case 'U':
            return (Terra_DisplayOutputMode)-1;
    }
    return Terra_DisplayOutputMode_Undefined;
}

Terra_ControlInputMode controlInputModeFromString(String controlInModeStr)
{
    switch (controlInModeStr.length() >= 1 ? controlInModeStr[0] : '\000') {
        case 'A':
            return (Terra_ControlInputMode)7;
        case 'C':
            return (Terra_ControlInputMode)17;
        case 'D':
            return (Terra_ControlInputMode)0;
        case 'M':
            switch (controlInModeStr.length() >= 7 ? controlInModeStr[6] : '\000') {
                case '2':
                    return (Terra_ControlInputMode)8;
                case '3':
                    switch (controlInModeStr.length() >= 12 ? controlInModeStr[11] : '\000') {
                        case '\000':
                            return (Terra_ControlInputMode)9;
                        case 'L':
                            return (Terra_ControlInputMode)10;
                    }
                    break;
                case '4':
                    switch (controlInModeStr.length() >= 12 ? controlInModeStr[11] : '\000') {
                        case '\000':
                            return (Terra_ControlInputMode)11;
                        case 'L':
                            return (Terra_ControlInputMode)12;
                    }
                    break;
            }
            break;
        case 'R':
            switch (controlInModeStr.length() >= 2 ? controlInModeStr[1] : '\000') {
                case 'e':
                    switch (controlInModeStr.length() >= 3 ? controlInModeStr[2] : '\000') {
                        case 'm':
                            return (Terra_ControlInputMode)16;
                        case 's':
                            return (Terra_ControlInputMode)13;
                    }
                    break;
                case 'o':
                    switch (controlInModeStr.length() >= 16 ? controlInModeStr[15] : '\000') {
                        case '\000':
                            return (Terra_ControlInputMode)1;
                        case 'L':
                            return (Terra_ControlInputMode)2;
                    }
                    break;
            }
            break;
        case 'T':
            switch (controlInModeStr.length() >= 2 ? controlInModeStr[1] : '\000') {
                case 'F':
                    return (Terra_ControlInputMode)15;
                case 'o':
                    return (Terra_ControlInputMode)14;
            }
            break;
        case 'U':
            switch (controlInModeStr.length() >= 2 ? controlInModeStr[1] : '\000') {
                case 'n':
                    return (Terra_ControlInputMode)-1;
                case 'p':
                    switch (controlInModeStr.length() >= 7 ? controlInModeStr[6] : '\000') {
                        case 'B':
                            switch (controlInModeStr.length() >= 16 ? controlInModeStr[15] : '\000') {
                                case '\000':
                                    return (Terra_ControlInputMode)3;
                                case 'L':
                                    return (Terra_ControlInputMode)4;
                            }
                            break;
                        case 'E':
                            switch (controlInModeStr.length() >= 19 ? controlInModeStr[18] : '\000') {
                                case '\000':
                                    return (Terra_ControlInputMode)5;
                                case 'L':
                                    return (Terra_ControlInputMode)6;
                            }
                            break;
                    }
                    break;
            }
            break;
    }
    return Terra_ControlInputMode_Undefined;
}

Terra_ActuatorType actuatorTypeFromString(String actuatorTypeStr)
{
    switch (actuatorTypeStr.length() >= 1 ? actuatorTypeStr[0] : '\000') {
        case 'C':
            return (Terra_ActuatorType)7;
        case 'F':
            return (Terra_ActuatorType)0;
        case 'G':
            return (Terra_ActuatorType)1;
        case 'P':
            return (Terra_ActuatorType)2;
        case 'U':
            return (Terra_ActuatorType)-1;
        case 'W':
            switch (actuatorTypeStr.length() >= 6 ? actuatorTypeStr[5] : '\000') {
                case 'A':
                    return (Terra_ActuatorType)3;
                case 'H':
                    return (Terra_ActuatorType)4;
                case 'P':
                    return (Terra_ActuatorType)5;
                case 'S':
                    return (Terra_ActuatorType)6;
            }
            break;
    }
    return Terra_ActuatorType_Undefined;
}

Terra_SensorType sensorTypeFromString(String sensorTypeStr)
{
    switch (sensorTypeStr.length() >= 1 ? sensorTypeStr[0] : '\000') {
        case 'A':
            switch (sensorTypeStr.length() >= 4 ? sensorTypeStr[3] : '\000') {
                case 'C':
                    return (Terra_SensorType)0;
                case 'T':
                    return (Terra_SensorType)1;
            }
            break;
        case 'C':
            return (Terra_SensorType)10;
        case 'L':
            return (Terra_SensorType)8;
        case 'P':
            switch (sensorTypeStr.length() >= 2 ? sensorTypeStr[1] : '\000') {
                case 'o':
                    return (Terra_SensorType)3;
                case 'u':
                    return (Terra_SensorType)4;
            }
            break;
        case 'S':
            return (Terra_SensorType)5;
        case 'U':
            return (Terra_SensorType)-1;
        case 'W':
            switch (sensorTypeStr.length() >= 6 ? sensorTypeStr[5] : '\000') {
                case 'H':
                    return (Terra_SensorType)7;
                case 'P':
                    return (Terra_SensorType)2;
                case 'T':
                    switch (sensorTypeStr.length() >= 7 ? sensorTypeStr[6] : '\000') {
                        case 'D':
                            return (Terra_SensorType)6;
                        case 'e':
                            return (Terra_SensorType)9;
                    }
                    break;
            }
            break;
    }
    return Terra_SensorType_Undefined;
}

Terra_TargetType targetTypeFromString(String targetTypeStr)
{
    switch (targetTypeStr.length() >= 1 ? targetTypeStr[0] : '\000') {
        case 'A':
            switch (targetTypeStr.length() >= 2 ? targetTypeStr[1] : '\000') {
                case 'l':
                    return (Terra_TargetType)0;
                case 'n':
                    return (Terra_TargetType)1;
                case 'r':
                    switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                        case 't':
                            return (Terra_TargetType)2;
                        case 'u':
                            return (Terra_TargetType)3;
                    }
                    break;
                case 's':
                    return (Terra_TargetType)4;
            }
            break;
        case 'B':
            switch (targetTypeStr.length() >= 2 ? targetTypeStr[1] : '\000') {
                case 'a':
                    return (Terra_TargetType)5;
                case 'e':
                    switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                        case 'a':
                            switch (targetTypeStr.length() >= 5 ? targetTypeStr[4] : '\000') {
                                case '\000':
                                    return (Terra_TargetType)6;
                                case 'B':
                                    return (Terra_TargetType)7;
                            }
                            break;
                        case 'e':
                            return (Terra_TargetType)8;
                    }
                    break;
                case 'l':
                    switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                        case 'a':
                            return (Terra_TargetType)9;
                        case 'u':
                            return (Terra_TargetType)10;
                    }
                    break;
                case 'o':
                    return (Terra_TargetType)11;
                case 'r':
                    switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                        case 'o':
                            return (Terra_TargetType)12;
                        case 'u':
                            return (Terra_TargetType)13;
                    }
                    break;
            }
            break;
        case 'C':
            switch (targetTypeStr.length() >= 2 ? targetTypeStr[1] : '\000') {
                case 'a':
                    switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                        case 'b':
                            return (Terra_TargetType)14;
                        case 'n':
                            return (Terra_TargetType)15;
                        case 'p':
                            return (Terra_TargetType)16;
                        case 'r':
                            return (Terra_TargetType)17;
                        case 't':
                            return (Terra_TargetType)18;
                        case 'u':
                            return (Terra_TargetType)19;
                    }
                    break;
                case 'e':
                    return (Terra_TargetType)20;
                case 'h':
                    switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                        case 'a':
                            return (Terra_TargetType)21;
                        case 'i':
                            switch (targetTypeStr.length() >= 4 ? targetTypeStr[3] : '\000') {
                                case 'c':
                                    return (Terra_TargetType)22;
                                case 'v':
                                    return (Terra_TargetType)23;
                            }
                            break;
                    }
                    break;
                case 'i':
                    return (Terra_TargetType)24;
                case 'o':
                    switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                        case 'r':
                            switch (targetTypeStr.length() >= 4 ? targetTypeStr[3] : '\000') {
                                case 'i':
                                    return (Terra_TargetType)25;
                                case 'n':
                                    return (Terra_TargetType)26;
                            }
                            break;
                        case 'u':
                            return (Terra_TargetType)85;
                    }
                    break;
                case 'u':
                    switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                        case 'c':
                            return (Terra_TargetType)27;
                        case 's':
                            switch (targetTypeStr.length() >= 11 ? targetTypeStr[10] : '\000') {
                                case '1':
                                    return (Terra_TargetType)77;
                                case '2':
                                    return (Terra_TargetType)78;
                                case '3':
                                    return (Terra_TargetType)79;
                                case '4':
                                    return (Terra_TargetType)80;
                                case '5':
                                    return (Terra_TargetType)81;
                                case '6':
                                    return (Terra_TargetType)82;
                                case '7':
                                    return (Terra_TargetType)83;
                                case '8':
                                    return (Terra_TargetType)84;
                            }
                            break;
                    }
                    break;
            }
            break;
        case 'D':
            return (Terra_TargetType)28;
        case 'E':
            switch (targetTypeStr.length() >= 2 ? targetTypeStr[1] : '\000') {
                case 'g':
                    return (Terra_TargetType)29;
                case 'n':
                    return (Terra_TargetType)30;
            }
            break;
        case 'F':
            switch (targetTypeStr.length() >= 2 ? targetTypeStr[1] : '\000') {
                case 'e':
                    return (Terra_TargetType)31;
                case 'l':
                    return (Terra_TargetType)33;
                case 'o':
                    return (Terra_TargetType)32;
            }
            break;
        case 'G':
            switch (targetTypeStr.length() >= 2 ? targetTypeStr[1] : '\000') {
                case 'a':
                    return (Terra_TargetType)34;
                case 'i':
                    return (Terra_TargetType)35;
            }
            break;
        case 'K':
            return (Terra_TargetType)36;
        case 'L':
            switch (targetTypeStr.length() >= 2 ? targetTypeStr[1] : '\000') {
                case 'a':
                    return (Terra_TargetType)37;
                case 'e':
                    switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                        case 'e':
                            return (Terra_TargetType)38;
                        case 'm':
                            return (Terra_TargetType)39;
                        case 't':
                            return (Terra_TargetType)40;
                    }
                    break;
            }
            break;
        case 'M':
            switch (targetTypeStr.length() >= 2 ? targetTypeStr[1] : '\000') {
                case 'a':
                    return (Terra_TargetType)41;
                case 'e':
                    return (Terra_TargetType)42;
                case 'i':
                    return (Terra_TargetType)43;
                case 'u':
                    return (Terra_TargetType)44;
            }
            break;
        case 'O':
            switch (targetTypeStr.length() >= 2 ? targetTypeStr[1] : '\000') {
                case 'k':
                    return (Terra_TargetType)45;
                case 'n':
                    return (Terra_TargetType)46;
                case 'r':
                    return (Terra_TargetType)47;
            }
            break;
        case 'P':
            switch (targetTypeStr.length() >= 2 ? targetTypeStr[1] : '\000') {
                case 'a':
                    switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                        case 'k':
                            return (Terra_TargetType)48;
                        case 'r':
                            switch (targetTypeStr.length() >= 5 ? targetTypeStr[4] : '\000') {
                                case 'l':
                                    return (Terra_TargetType)49;
                                case 'n':
                                    return (Terra_TargetType)50;
                            }
                            break;
                    }
                    break;
                case 'e':
                    switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                        case 'a':
                            switch (targetTypeStr.length() >= 4 ? targetTypeStr[3] : '\000') {
                                case '\000':
                                    return (Terra_TargetType)51;
                                case 'S':
                                    return (Terra_TargetType)52;
                            }
                            break;
                        case 'p':
                            switch (targetTypeStr.length() >= 4 ? targetTypeStr[3] : '\000') {
                                case 'i':
                                    return (Terra_TargetType)53;
                                case 'p':
                                    switch (targetTypeStr.length() >= 8 ? targetTypeStr[7] : '\000') {
                                        case 'B':
                                            return (Terra_TargetType)54;
                                        case 'H':
                                            return (Terra_TargetType)55;
                                    }
                                    break;
                            }
                            break;
                    }
                    break;
                case 'o':
                    switch (targetTypeStr.length() >= 7 ? targetTypeStr[6] : '\000') {
                        case '\000':
                            return (Terra_TargetType)56;
                        case 'S':
                            return (Terra_TargetType)57;
                    }
                    break;
                case 'u':
                    return (Terra_TargetType)58;
            }
            break;
        case 'R':
            switch (targetTypeStr.length() >= 2 ? targetTypeStr[1] : '\000') {
                case 'a':
                    return (Terra_TargetType)59;
                case 'h':
                    return (Terra_TargetType)60;
                case 'o':
                    return (Terra_TargetType)61;
            }
            break;
        case 'S':
            switch (targetTypeStr.length() >= 2 ? targetTypeStr[1] : '\000') {
                case 'a':
                    return (Terra_TargetType)62;
                case 'i':
                    return (Terra_TargetType)63;
                case 'p':
                    return (Terra_TargetType)64;
                case 'q':
                    return (Terra_TargetType)65;
                case 't':
                    return (Terra_TargetType)67;
                case 'u':
                    return (Terra_TargetType)66;
                case 'w':
                    return (Terra_TargetType)68;
            }
            break;
        case 'T':
            switch (targetTypeStr.length() >= 2 ? targetTypeStr[1] : '\000') {
                case 'a':
                    switch (targetTypeStr.length() >= 4 ? targetTypeStr[3] : '\000') {
                        case 'o':
                            return (Terra_TargetType)69;
                        case 'r':
                            return (Terra_TargetType)70;
                    }
                    break;
                case 'h':
                    return (Terra_TargetType)71;
                case 'o':
                    return (Terra_TargetType)72;
                case 'u':
                    return (Terra_TargetType)73;
            }
            break;
        case 'U':
            return (Terra_TargetType)-1;
        case 'W':
            switch (targetTypeStr.length() >= 6 ? targetTypeStr[5] : '\000') {
                case 'c':
                    return (Terra_TargetType)74;
                case 'm':
                    return (Terra_TargetType)75;
            }
            break;
        case 'Z':
            return (Terra_TargetType)76;
    }
    return Terra_TargetType_Undefined;
}

Terra_SubstrateType substrateTypeFromString(String substrateTypeStr)
{
        switch (substrateTypeStr.length() >= 1 ? substrateTypeStr[0] : '\000') {
        case 'C':
            switch (substrateTypeStr.length() >= 2 ? substrateTypeStr[1] : '\000') {
                case 'l':
                    return (Terra_SubstrateType)0;
                case 'o':
                    switch (substrateTypeStr.length() >= 3 ? substrateTypeStr[2] : '\000') {
                        case 'c':
                            return (Terra_SubstrateType)1;
                        case 'u':
                            return (Terra_SubstrateType)3;
                    }
                    break;
            }
            break;
        case 'R':
            return (Terra_SubstrateType)2;
        case 'U':
            return (Terra_SubstrateType)-1;
    }
    return Terra_SubstrateType_Undefined;
}

Terra_ReservoirType reservoirTypeFromString(String reservoirTypeStr)
{
     switch (reservoirTypeStr.length() >= 1 ? reservoirTypeStr[0] : '\000') {
        case 'C':
            switch (reservoirTypeStr.length() >= 2 ? reservoirTypeStr[1] : '\000') {
                case 'o':
                    return (Terra_ReservoirType)22;
                case 'u':
                    switch (reservoirTypeStr.length() >= 15 ? reservoirTypeStr[14] : '\000') {
                        case '1':
                            switch (reservoirTypeStr.length() >= 16 ? reservoirTypeStr[15] : '\000') {
                                case '\000':
                                    return (Terra_ReservoirType)6;
                                case '0':
                                    return (Terra_ReservoirType)15;
                                case '1':
                                    return (Terra_ReservoirType)16;
                                case '2':
                                    return (Terra_ReservoirType)17;
                                case '3':
                                    return (Terra_ReservoirType)18;
                                case '4':
                                    return (Terra_ReservoirType)19;
                                case '5':
                                    return (Terra_ReservoirType)20;
                                case '6':
                                    return (Terra_ReservoirType)21;
                            }
                            break;
                        case '2':
                            return (Terra_ReservoirType)7;
                        case '3':
                            return (Terra_ReservoirType)8;
                        case '4':
                            return (Terra_ReservoirType)9;
                        case '5':
                            return (Terra_ReservoirType)10;
                        case '6':
                            return (Terra_ReservoirType)11;
                        case '7':
                            return (Terra_ReservoirType)12;
                        case '8':
                            return (Terra_ReservoirType)13;
                        case '9':
                            return (Terra_ReservoirType)14;
                    }
                    break;
            }
            break;
        case 'D':
            return (Terra_ReservoirType)1;
        case 'F':
            switch (reservoirTypeStr.length() >= 2 ? reservoirTypeStr[1] : '\000') {
                case 'e':
                    return (Terra_ReservoirType)0;
                case 'r':
                    return (Terra_ReservoirType)3;
            }
            break;
        case 'N':
            return (Terra_ReservoirType)2;
        case 'P':
            switch (reservoirTypeStr.length() >= 3 ? reservoirTypeStr[2] : '\000') {
                case 'D':
                    return (Terra_ReservoirType)5;
                case 'U':
                    return (Terra_ReservoirType)4;
            }
            break;
        case 'U':
            return (Terra_ReservoirType)-1;
    }
    return Terra_ReservoirType_Undefined;
}

Terra_RailType railTypeFromString(String railTypeStr) {
    switch (railTypeStr.length() >= 1 ? railTypeStr[0] : '\000') {
        case 'A':
            switch (railTypeStr.length() >= 3 ? railTypeStr[2] : '\000') {
                case '1':
                    return (Terra_RailType)0;
                case '2':
                    return (Terra_RailType)1;
            }
            break;
        case 'C':
            return (Terra_RailType)7;
        case 'D':
            switch (railTypeStr.length() >= 3 ? railTypeStr[2] : '\000') {
                case '1':
                    return (Terra_RailType)4;
                case '2':
                    return (Terra_RailType)5;
                case '3':
                    return (Terra_RailType)2;
                case '4':
                    return (Terra_RailType)6;
                case '5':
                    return (Terra_RailType)3;
            }
            break;
        case 'U':
            return (Terra_RailType)-1;
    }
    return Terra_RailType_Undefined;
}

Terra_PinMode pinModeFromString(String pinModeStr)
{
    switch (pinModeStr.length() >= 1 ? pinModeStr[0] : '\000') {
        case 'A':
            switch (pinModeStr.length() >= 7 ? pinModeStr[6] : '\000') {
                case 'I':
                    return (Terra_PinMode)5;
                case 'O':
                    return (Terra_PinMode)6;
            }
            break;
        case 'C':
            return (Terra_PinMode)7;
        case 'D':
            switch (pinModeStr.length() >= 8 ? pinModeStr[7] : '\000') {
                case 'I':
                    switch (pinModeStr.length() >= 13 ? pinModeStr[12] : '\000') {
                        case '\000':
                            return (Terra_PinMode)0;
                        case 'P':
                            switch (pinModeStr.length() >= 17 ? pinModeStr[16] : '\000') {
                                case 'D':
                                    return (Terra_PinMode)2;
                                case 'U':
                                    return (Terra_PinMode)1;
                            }
                            break;
                    }
                    break;
                case 'O':
                    switch (pinModeStr.length() >= 14 ? pinModeStr[13] : '\000') {
                        case '\000':
                            return (Terra_PinMode)3;
                        case 'P':
                            return (Terra_PinMode)4;
                    }
                    break;
            }
            break;
        case 'U':
            return (Terra_PinMode)-1;
    }
    return Terra_PinMode_Undefined;
}

Terra_EnableMode enableModeFromString(String enableModeStr)
{
    switch (enableModeStr.length() >= 1 ? enableModeStr[0] : '\000') {
        case 'A':
            switch (enableModeStr.length() >= 2 ? enableModeStr[1] : '\000') {
                case 's':
                    return (Terra_EnableMode)7;
                case 'v':
                    return (Terra_EnableMode)2;
            }
            break;
        case 'C':
            return (Terra_EnableMode)8;
        case 'D':
            return (Terra_EnableMode)6;
        case 'H':
            return (Terra_EnableMode)0;
        case 'I':
            return (Terra_EnableMode)4;
        case 'L':
            return (Terra_EnableMode)1;
        case 'M':
            return (Terra_EnableMode)3;
        case 'R':
            return (Terra_EnableMode)5;
        case 'U':
            return (Terra_EnableMode)-1;
    }
    return Terra_EnableMode_Undefined;
}

Terra_UnitsCategory unitsCategoryFromString(String unitsCategoryStr)
{
     switch (unitsCategoryStr.length() >= 1 ? unitsCategoryStr[0] : '\000') {
        case 'A':
            return (Terra_UnitsCategory)0;
        case 'C':
            switch (unitsCategoryStr.length() >= 3 ? unitsCategoryStr[2] : '\000') {
                case 'n':
                    return (Terra_UnitsCategory)1;
                case 'u':
                    return (Terra_UnitsCategory)10;
            }
            break;
        case 'D':
            return (Terra_UnitsCategory)2;
        case 'L':
            switch (unitsCategoryStr.length() >= 4 ? unitsCategoryStr[3] : '\000') {
                case 'D':
                    return (Terra_UnitsCategory)3;
                case 'F':
                    return (Terra_UnitsCategory)4;
                case 'V':
                    return (Terra_UnitsCategory)5;
            }
            break;
        case 'P':
            switch (unitsCategoryStr.length() >= 2 ? unitsCategoryStr[1] : '\000') {
                case 'e':
                    return (Terra_UnitsCategory)7;
                case 'o':
                    return (Terra_UnitsCategory)8;
            }
            break;
        case 'T':
            return (Terra_UnitsCategory)6;
        case 'U':
            return (Terra_UnitsCategory)-1;
        case 'W':
            return (Terra_UnitsCategory)9;
    }
    return Terra_UnitsCategory_Undefined;
}

Terra_UnitsType unitsTypeFromSymbol(String unitsSymbolStr)
{
    switch (unitsSymbolStr.length() >= 1 ? unitsSymbolStr[0] : '\000') {
        case '%':
            return (Terra_UnitsType)1;
        case 'A':
            return (Terra_UnitsType)15;
        case 'E':
            return (Terra_UnitsType)3;
        case 'J':
            return (Terra_UnitsType)16;
        case 'K':
            return (Terra_UnitsType)20;
        case 'L':
            switch (unitsSymbolStr.length() >= 2 ? unitsSymbolStr[1] : '\000') {
                case '\000':
                    return (Terra_UnitsType)14;
                case '/':
                    return (Terra_UnitsType)12;
            }
            break;
        case 'T':
            return (Terra_UnitsType)3;
        case 'W':
            return (Terra_UnitsType)16;
        case '[':
            switch (unitsSymbolStr.length() >= 2 ? unitsSymbolStr[1] : '\000') {
                case 'p':
                    return (Terra_UnitsType)2;
                case 'q':
                    return (Terra_UnitsType)22;
                case 'u':
                    return (Terra_UnitsType)-1;
            }
            break;
        case 'f':
            return (Terra_UnitsType)7;
        case 'g':
            switch (unitsSymbolStr.length() >= 4 ? unitsSymbolStr[3] : '\000') {
                case '\000':
                    return (Terra_UnitsType)13;
                case '/':
                    return (Terra_UnitsType)11;
            }
            break;
        case 'l':
            return (Terra_UnitsType)21;
        case 'm':
            switch (unitsSymbolStr.length() >= 2 ? unitsSymbolStr[1] : '\000') {
                case '\000':
                    return (Terra_UnitsType)8;
                case 'L':
                    switch (unitsSymbolStr.length() >= 4 ? unitsSymbolStr[3] : '\000') {
                        case 'L':
                            return (Terra_UnitsType)10;
                        case 'g':
                            return (Terra_UnitsType)9;
                    }
                    break;
                case 'S':
                    return (Terra_UnitsType)3;
            }
            break;
        case 'p':
            switch (unitsSymbolStr.length() >= 4 ? unitsSymbolStr[3] : '\000') {
                case '\000':
                    return (Terra_UnitsType)4;
                case '(':
                    switch (unitsSymbolStr.length() >= 5 ? unitsSymbolStr[4] : '\000') {
                        case '5':
                            return (Terra_UnitsType)4;
                        case '6':
                            return (Terra_UnitsType)5;
                        case '7':
                            return (Terra_UnitsType)6;
                    }
                    break;
            }
            break;
        case 'r':
            return (Terra_UnitsType)0;
        default: // degree symbol
            switch (unitsSymbolStr.length() >= 3 ? unitsSymbolStr[2] : '\000') {
                case 'C':
                    return (Terra_UnitsType)17;
                case 'F':
                    return (Terra_UnitsType)18;
                case 'K':
                    return (Terra_UnitsType)19;
            }
            break;
    }
    return Terra_UnitsType_Undefined;
}
