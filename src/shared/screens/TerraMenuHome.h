/*  Terraduino: Home menu screen.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Home Menu Screen
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraMenuHome_H
#define TerraMenuHome_H

class TerraHomeMenu;
#ifdef TERRA_DISABLE_BUILTIN_DATA
struct TerraHomeMenuInfo;
#endif
struct TerraHomeMenuItems;

#include "../TerraduinoUI.h"

class TerraHomeMenu : public TerraMenu
{
public:
    TerraHomeMenu();
    virtual ~TerraHomeMenu();

    virtual void loadMenu(MenuItem *addFrom = nullptr) override;
    virtual MenuItem *getRootItem() override;

    void unloadSubMenus();

    inline TerraHomeMenuItems &getItems() { return *_items; }

protected:
    TerraHomeMenuItems *_items;
};

#ifdef TERRA_DISABLE_BUILTIN_DATA
struct TerraHomeMenuInfo {
    TerraHomeMenuInfo();

    AnyMenuInfo minfoBackToOverview;
#ifdef TERRA_UI_ENABLE_DEBUG_MENU
    AnyMenuInfo minfoTriggerSigLocation;
    AnyMenuInfo minfoTriggerSigTime;
    AnyMenuInfo minfoTriggerSDCleanup;
    AnyMenuInfo minfoTriggerLowMem;
    AnyMenuInfo minfoTriggerAutosave;
    BooleanMenuInfo minfoSimhubConnected;
    SubMenuInfo minfoDebug;
#endif
    AnyMenuInfo minfoInformation;
    AnyMenuInfo minfoCalibrations;
    SubMenuInfo minfoLibrary;
    AnyMenuInfo minfoSettings;
    AnyMenuInfo minfoScheduling;
    AnyMenuInfo minfoPowerRails;
    AnyMenuInfo minfoWater;
    AnyMenuInfo minfoEnvironment;
    AnyMenuInfo minfoThermal;
    AnyMenuInfo minfoSensors;
    AnyMenuInfo minfoActuators;
    SubMenuInfo minfoSystem;
    AnyMenuInfo minfoAlerts;
};
#endif

struct TerraHomeMenuItems {
    TerraHomeMenuItems();

#ifdef TERRA_DISABLE_BUILTIN_DATA
    TerraHomeMenuInfo init;
#endif

    ActionMenuItem menuBackToOverview;
#ifdef TERRA_UI_ENABLE_DEBUG_MENU
    ActionMenuItem menuTriggerSigLocation;
    ActionMenuItem menuTriggerSigTime;
    ActionMenuItem menuTriggerSDCleanup;
    ActionMenuItem menuTriggerLowMem;
    ActionMenuItem menuTriggerAutosave;
    BooleanMenuItem menuSimhubConnected;
    BackMenuItem menuBackDebug;
    SubMenuItem menuDebug;
#endif
    ActionMenuItem menuInformation;
    ActionMenuItem menuCalibrations;
    BackMenuItem menuBackLibrary;
    SubMenuItem menuLibrary;
    ActionMenuItem menuSettings;
    ActionMenuItem menuScheduling;
    ActionMenuItem menuPowerRails;
    ActionMenuItem menuWater;
    ActionMenuItem menuEnvironment;
    ActionMenuItem menuThermal;
    ActionMenuItem menuSensors;
    ActionMenuItem menuActuators;
    BackMenuItem menuBackSystem;
    SubMenuItem menuSystem;
    ActionMenuItem menuAlerts;
};

#endif // /ifndef TerraMenuHome_H
#endif
