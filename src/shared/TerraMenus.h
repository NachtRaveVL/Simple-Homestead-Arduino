/*  Terraduino: menu screen scaffolding.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Menu Screens
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraMenus_H
#define TerraMenus_H

class TerraMenu;

#include "TerraduinoUI.h"
#include "RemoteMenuItem.h"
#include "EditableLargeNumberMenuItem.h"

class TerraMenu
{
public:
    inline TerraMenu() : _loaded(false) { ; }
    virtual ~TerraMenu() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) = 0;
    virtual MenuItem *getRootItem() = 0;

    inline bool isLoaded() const { return _loaded; }

protected:
    bool _loaded;
};

#define InitAnyMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback)\
    safeProgCpy(varName.name, CFP(strNum), NAME_SIZE_T);\
    varName.id = itemId;\
    varName.eepromAddr = eepromPosition;\
    varName.maxValue = valMaximum;\
    varName.callback = fnCallback

#define InitBooleanMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback,boolNaming)\
    InitAnyMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback);\
    varName.naming = boolNaming

#define InitSubMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback)\
    InitAnyMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback)

#define InitEnumMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback,enumItems)\
    InitAnyMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback);\
    varName.menuItems = enumItems

#define InitAnalogMenuInfoUnits(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback,valOffset,valDivisor,unitsStrNum)\
    InitAnyMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback);\
    varName.offset = valOffset;\
    varName.divisor = valDivisor;\
    safeProgCpy(varName.unitName, CFP(unitsStrNum), UNIT_SIZE_T)

#define InitAnalogMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback,valOffset,valDivisor)\
    InitAnyMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback);\
    varName.offset = valOffset;\
    varName.divisor = valDivisor;\
    varName.unitName[0] = '\000'

#define T_RENDERING_CALLBACK_NAME_INVOKE(fnName, parent, strNum, eepromPosition, invoke) \
int fnName(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int buffSize) { \
    switch(mode) { \
        case RENDERFN_NAME: safeProgCpy(buffer, CFP(strNum), buffSize); return true; \
        case RENDERFN_INVOKE: invokeIfSafe(invoke, item); return true; \
        case RENDERFN_EEPROM_POS: return eepromPosition; \
        default: return parent(item, row, mode, buffer, buffSize); \
    } \
}

#ifdef TERRA_DISABLE_BUILTIN_DATA
#define InfoPtrForItem(itemName,castType) (&init.minfo##itemName)
#define InfoLocation INFO_LOCATION_RAM
#else
#define InfoPtrForItem(itemName,castType) ((const castType *)CFP(TUIStr_Item_##itemName))
#define InfoLocation INFO_LOCATION_PGM
#endif

#include "screens/TerraMenuHome.h"
#include "screens/TerraMenuAlerts.h"
#include "screens/TerraMenuActuators.h"
#include "screens/TerraMenuSensors.h"
#include "screens/TerraMenuWater.h"
#include "screens/TerraMenuEnvironment.h"
#include "screens/TerraMenuThermal.h"
#include "screens/TerraMenuPowerRails.h"
#include "screens/TerraMenuScheduling.h"
#include "screens/TerraMenuSettings.h"
#include "screens/TerraMenuCalibrations.h"
#include "screens/TerraMenuInformation.h"

#endif // /ifndef TerraMenus_H
#endif
