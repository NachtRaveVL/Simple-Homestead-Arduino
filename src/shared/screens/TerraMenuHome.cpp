/*  Terraduino: Home menu screen.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Home Menu Screen
*/

#include "../TerraduinoUI.h"
#ifdef TERRA_USE_GUI

void CALLBACK_FUNCTION gotoScreen(int id)
{
    switch (id) {
        case 7: // BackToOverview
            taskManager.scheduleOnce(0, []{
                if (getBaseUI()) { getBaseUI()->reset(); }
            });
            break;
        case 5: // Information
            // todo
            break;
        case 42: // Calibrations
            // todo
            break;
        case 3: // Settings
            // todo
            break;
        case 25: // Scheduling
            // todo
            break;
        case 24: // PowerRails
            // todo
            break;
        case 22: // Water
            // todo
            break;
        case 23: // Environment
            // todo
            break;
        case 26: // Thermal
            // todo
            break;
        case 21: // Sensors
            // todo
            break;
        case 20: // Actuators
            // todo
            break;
        case 1: // Alerts
            // todo
            break;
        default: // Home
            // todo
            break;
    }
}

#ifdef TERRA_UI_ENABLE_DEBUG_MENU

void CALLBACK_FUNCTION debugAction(int id)
{
    switch (id) {
        case 65: // TriggerSigLoc
            if (getController()) { getController()->setSystemLocation(getController()->getSystemLocation(), true); }
            break;
        case 64: // TriggerSigTime
            setUnixTime(unixNow(), true);
            break;
        case 63: // TriggerSDCleanup
            if (getLogger()) { getLogger()->cleanupOldestLogs(true); }
            if (getPublisher()) { getPublisher()->cleanupOldestData(true); }
            break;
        case 62: // TriggerLowMem
            if (getController()) { getController()->broadcastLowMemory(); }
            break;
        case 61: // TriggerAutosave
            if (getController()) { getController()->performAutosave(); }
            break;
        default: break;
    }
}

#else

void CALLBACK_FUNCTION debugAction(int id) { ; }

#endif // /ifdef TERRA_UI_ENABLE_DEBUG_MENU


TerraHomeMenu::TerraHomeMenu()
    : TerraMenu(), _items(nullptr)
{ ; }

TerraHomeMenu::~TerraHomeMenu()
{
    if (_items) { delete _items; }
}

void TerraHomeMenu::loadMenu(MenuItem *addFrom)
{
    if (!_items) {
        _loaded = (bool)(_items = new TerraHomeMenuItems());
        TERRA_SOFT_ASSERT(_items, SFP(TStr_Err_AllocationFailure));
    }
}

MenuItem *TerraHomeMenu::getRootItem()
{
    if (!_loaded) { loadMenu(); }
    return _loaded && _items ? &_items->menuAlerts : nullptr;
}

void TerraHomeMenu::unloadSubMenus()
{
    // todo
}

#ifdef TERRA_DISABLE_BUILTIN_DATA

TerraHomeMenuInfo::TerraHomeMenuInfo()
{
    InitAnyMenuInfo(minfoBackToOverview, TUIStr_Item_BackToOverview, 7, NO_ADDRESS, 0, gotoScreen);
    #ifdef TERRA_UI_ENABLE_DEBUG_MENU
        InitAnyMenuInfo(minfoTriggerSigLocation, TUIStr_Item_TriggerSigLocation, 65, NO_ADDRESS, 0, debugAction);
        InitAnyMenuInfo(minfoTriggerSigTime, TUIStr_Item_TriggerSigTime, 64, NO_ADDRESS, 0, debugAction);
        InitAnyMenuInfo(minfoTriggerSDCleanup, TUIStr_Item_TriggerSDCleanup, 63, NO_ADDRESS, 0, debugAction);
        InitAnyMenuInfo(minfoTriggerLowMem, TUIStr_Item_TriggerLowMem, 62, NO_ADDRESS, 0, debugAction);
        InitAnyMenuInfo(minfoTriggerAutosave, TUIStr_Item_TriggerAutosave, 61, NO_ADDRESS, 0, debugAction);
        InitBooleanMenuInfo(minfoSimhubConnected, TUIStr_Item_SimhubConnected, 60, NO_ADDRESS, 1, NO_CALLBACK, NAMING_CHECKBOX);
        InitSubMenuInfo(minfoDebug, TUIStr_Item_Debug, 6, NO_ADDRESS, 0, debugAction);
    #endif
    InitAnyMenuInfo(minfoInformation, TUIStr_Item_Information, 5, NO_ADDRESS, 0, gotoScreen);
    InitAnyMenuInfo(minfoCalibrations, TUIStr_Item_Calibrations, 42, NO_ADDRESS, 0, gotoScreen);
    InitSubMenuInfo(minfoLibrary, TUIStr_Item_Library, 4, NO_ADDRESS, 0, NO_CALLBACK);
    InitAnyMenuInfo(minfoSettings, TUIStr_Item_Settings, 3, NO_ADDRESS, 0, gotoScreen);
    InitAnyMenuInfo(minfoScheduling, TUIStr_Item_Scheduling, 25, NO_ADDRESS, 0, gotoScreen);
    InitAnyMenuInfo(minfoPowerRails, TUIStr_Item_PowerRails, 24, NO_ADDRESS, 0, gotoScreen);
    InitAnyMenuInfo(minfoWater, TUIStr_Item_Water, 22, NO_ADDRESS, 0, gotoScreen);
    InitAnyMenuInfo(minfoEnvironment, TUIStr_Item_Environment, 23, NO_ADDRESS, 0, gotoScreen);
    InitAnyMenuInfo(minfoThermal, TUIStr_Item_Thermal, 26, NO_ADDRESS, 0, gotoScreen);
    InitAnyMenuInfo(minfoSensors, TUIStr_Item_Sensors, 21, NO_ADDRESS, 0, gotoScreen);
    InitAnyMenuInfo(minfoActuators, TUIStr_Item_Actuators, 20, NO_ADDRESS, 0, gotoScreen);
    InitSubMenuInfo(minfoSystem, TUIStr_Item_System, 2, NO_ADDRESS, 0, NO_CALLBACK);
    InitAnyMenuInfo(minfoAlerts, TUIStr_Item_Alerts, 1, NO_ADDRESS, 0, gotoScreen);
}

#endif // /ifdef TERRA_DISABLE_BUILTIN_DATA

TerraHomeMenuItems::TerraHomeMenuItems() :
    #ifdef TERRA_DISABLE_BUILTIN_DATA
        init(),
    #endif
    menuBackToOverview(InfoPtrForItem(BackToOverview, AnyMenuInfo), nullptr, InfoLocation),
    #ifdef TERRA_UI_ENABLE_DEBUG_MENU
        menuTriggerSigLocation(InfoPtrForItem(TriggerSigLocation, AnyMenuInfo), nullptr, InfoLocation),
        menuTriggerSigTime(InfoPtrForItem(TriggerSigTime, AnyMenuInfo), &menuTriggerSigLocation, InfoLocation),
        menuTriggerSDCleanup(InfoPtrForItem(TriggerSDCleanup, AnyMenuInfo), &menuTriggerSigTime, InfoLocation),
        menuTriggerLowMem(InfoPtrForItem(TriggerLowMem, AnyMenuInfo), &menuTriggerSDCleanup, InfoLocation),
        menuTriggerAutosave(InfoPtrForItem(TriggerAutosave, AnyMenuInfo), &menuTriggerLowMem, InfoLocation),
        menuSimhubConnected(InfoPtrForItem(SimhubConnected, BooleanMenuInfo), false, &menuTriggerAutosave, InfoLocation),
        menuBackDebug(InfoPtrForItem(Debug, SubMenuInfo), &menuSimhubConnected, InfoLocation),
        menuDebug(InfoPtrForItem(Debug, SubMenuInfo), &menuBackDebug, &menuBackToOverview, InfoLocation),
        menuInformation(InfoPtrForItem(Information, AnyMenuInfo), &menuDebug, InfoLocation),
    #else
        menuInformation(InfoPtrForItem(Information, AnyMenuInfo), &menuBackToOverview, InfoLocation),
    #endif
    menuCalibrations(InfoPtrForItem(Calibrations, AnyMenuInfo), nullptr, InfoLocation),
    menuBackLibrary(InfoPtrForItem(Library, SubMenuInfo), &menuCalibrations, InfoLocation),
    menuLibrary(InfoPtrForItem(Library, SubMenuInfo), &menuBackLibrary, &menuInformation, InfoLocation),
    menuSettings(InfoPtrForItem(Settings, AnyMenuInfo), &menuLibrary, InfoLocation),
    menuScheduling(InfoPtrForItem(Scheduling, AnyMenuInfo), nullptr, InfoLocation),
    menuPowerRails(InfoPtrForItem(PowerRails, AnyMenuInfo), &menuScheduling, InfoLocation),
    menuWater(InfoPtrForItem(Water, AnyMenuInfo), &menuPowerRails, InfoLocation),
    menuEnvironment(InfoPtrForItem(Environment, AnyMenuInfo), &menuWater, InfoLocation),
    menuThermal(InfoPtrForItem(Thermal, AnyMenuInfo), &menuEnvironment, InfoLocation),
    menuSensors(InfoPtrForItem(Sensors, AnyMenuInfo), &menuThermal, InfoLocation),
    menuActuators(InfoPtrForItem(Actuators, AnyMenuInfo), &menuSensors, InfoLocation),
    menuBackSystem(InfoPtrForItem(System, SubMenuInfo), &menuActuators, InfoLocation),
    menuSystem(InfoPtrForItem(System, SubMenuInfo), &menuBackSystem, &menuSettings, InfoLocation),
    menuAlerts(InfoPtrForItem(Alerts, AnyMenuInfo), &menuSystem, InfoLocation)
{
    menuBackToOverview.setReadOnly(true);
    #ifdef TERRA_UI_ENABLE_DEBUG_MENU
        menuTriggerSigLocation.setReadOnly(true);
        menuTriggerSigTime.setReadOnly(true);
        menuTriggerSDCleanup.setReadOnly(true);
        menuTriggerLowMem.setReadOnly(true);
        menuTriggerAutosave.setReadOnly(true);
        menuSimhubConnected.setReadOnly(true);
    #endif
    menuInformation.setReadOnly(true);
    menuCalibrations.setReadOnly(true);
    menuSettings.setReadOnly(true);
    menuScheduling.setReadOnly(true);
    menuPowerRails.setReadOnly(true);
    menuWater.setReadOnly(true);
    menuEnvironment.setReadOnly(true);
    menuThermal.setReadOnly(true);
    menuSensors.setReadOnly(true);
    menuActuators.setReadOnly(true);
    menuAlerts.setReadOnly(true);
}

#endif
