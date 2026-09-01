/*  Terraduino: UI strings.
    Copyright (C) 2026 NachtRaveVL
    Terraduino UI Strings
*/

#include "TerraduinoUI.h"
#include <string.h>
#ifdef TERRA_USE_GUI

// UI stub: Domain-specific UI strings are populated when the corresponding menus are implemented.
static uint16_t _uiStringDataAddress = (uint16_t)-1;
static String _uiStringDataFilePrefix;

String stringFromPGM(TerraUI_String strNum)
{
#ifndef TERRA_DISABLE_BUILTIN_DATA
    return stringFromPGMAddr(pgmAddrForStr(strNum));
#else
    (void)strNum;
    return String();
#endif
}

void beginUIStringsFromEEPROM(uint16_t uiDataAddress)
{
    _uiStringDataAddress = uiDataAddress;
}

void beginUIStringsFromSDCard(String uiDataFilePrefix)
{
    _uiStringDataFilePrefix = uiDataFilePrefix;
}

#ifndef TERRA_DISABLE_BUILTIN_DATA
const char *pgmAddrForStr(TerraUI_String strNum)
{
    (void)_uiStringDataAddress;
    (void)_uiStringDataFilePrefix;

    switch (strNum) {
        case TUIStr_Keys_MatrixActions: {
            static const char flashUIStr_Keys_MatrixActions[] PROGMEM = {TERRA_UI_MATRIX_ACTIONS};
            return flashUIStr_Keys_MatrixActions;
        } break;
        default: break;
    }
    return nullptr;
}
#endif

size_t enumListPitch(const char *enumData)
{
    if (!enumData) { return 0; }
    size_t size = 0;
    while (get_info_char(enumData) != '\000' && size < 64) {
        ++size; ++enumData;
    }
    while (get_info_char(enumData) == '\000' && size < 64) {
        ++size; ++enumData;
    }
    return size;
}

#endif
