/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Base UI
*/

#include "TerraduinoUI.h"
#ifdef TERRA_USE_GUI

TerraduinoBaseUI::TerraduinoBaseUI(String deviceUUID, UIControlSetup uiControlSetup, UIDisplaySetup uiDisplaySetup, bool isActiveLowIO, bool allowInterruptableIO, bool enableTcUnicodeFonts, bool enableBufferedVRAM)
    : _appInfo{0}, _uiCtrlSetup(uiControlSetup), _uiDispSetup(uiDisplaySetup),
      _isActiveLow(isActiveLowIO), _allowISR(allowInterruptableIO), _isTcUnicodeFonts(enableTcUnicodeFonts), _isBufferedVRAM(enableBufferedVRAM),
      _uiData(nullptr), _input(nullptr), _display(nullptr), _remoteServer(nullptr), _backlight(nullptr), _blTimeout(0),
      _overview(nullptr), _homeMenu(nullptr), _clockFont(nullptr), _detailFont(nullptr), _itemFont(nullptr), _titleFont(nullptr)
{
    if (getController()) { strncpy(_appInfo.name, getController()->getSystemNameChars(), 30); }
    strncpy(_appInfo.uuid, deviceUUID.c_str(), 38);
    pintype_t ledPin = uiDisplaySetup.getBacklightPin();
    if (uiDisplaySetup.dispCfgType != UIDisplaySetup::LCD && isValidPin(ledPin)) { // LCD has its own backlight
        switch (uiDisplaySetup.getBacklightMode()) {
            case Terra_BacklightMode_Inverted:
                _backlight = new TerraDigitalPin(ledPin, OUTPUT, ACT_LOW, tpinchnl_none);
                break;
            case Terra_BacklightMode_PWM:
                _backlight = new TerraAnalogPin(ledPin, OUTPUT, uiDisplaySetup.getBacklightBitRes(),
#ifdef ESP32
                                                uiDisplaySetup.getBacklightChannel(),
#endif
#ifdef ESP_PLATFORM
                                                uiDisplaySetup.getBacklightFrequency(),
#endif
                                                tpinchnl_none);
                break;
            default: // Normal
                _backlight = new TerraDigitalPin(ledPin, OUTPUT, ACT_HIGH, tpinchnl_none);
                break;
        }
        TERRA_SOFT_ASSERT(_backlight, SFP(TStr_Err_AllocationFailure));

        if (_backlight) { _backlight->init(); }
    }
}

TerraduinoBaseUI::~TerraduinoBaseUI()
{
    if (_overview) { delete _overview; }
    while (_remotes.size()) {
        delete (*_remotes.begin());
        _remotes.erase(_remotes.begin());
    }
    if (_input) { delete _input; }
    if (_display) { delete _display; }
    if (_remoteServer) { delete _remoteServer; }
    if (_backlight) { delete _backlight; }
}

void TerraduinoBaseUI::init(uint8_t updatesPerSec, Terra_DisplayTheme displayTheme, Terra_TitleMode titleMode, bool analogSlider, bool editingIcons)
{
    if (!_uiData) {
        _uiData = new TerraUIData();
        TERRA_SOFT_ASSERT(_uiData, SFP(TStr_Err_AllocationFailure));
    }
    if (_uiData) {
        _uiData->updatesPerSec = updatesPerSec;
        _uiData->displayTheme = displayTheme;
        _uiData->titleMode = titleMode;
        _uiData->analogSlider = analogSlider;
        _uiData->editingIcons = editingIcons;
    }

    if (!_homeMenu) { // must stay allocated while menuMgr active
        _homeMenu = new TerraHomeMenu();
        TERRA_SOFT_ASSERT(_homeMenu, SFP(TStr_Err_AllocationFailure));
    }
}

TerraUIData *TerraduinoBaseUI::init(TerraUIData *uiData)
{
    if (uiData && (_uiData = uiData)) { // Customized data
        init(_uiData->updatesPerSec,
             _uiData->displayTheme, _uiData->titleMode,
             _uiData->analogSlider, _uiData->editingIcons);
    } else if (_display) { // Display driver default
        _display->initBaseUIFromDefaults(); // calls back into above init with default settings for display
    } else { // Remote control default
        init(2, Terra_DisplayTheme_Undefined, Terra_TitleMode_Always);
    }
    return _uiData;
}

void TerraduinoBaseUI::begin()
{
    BaseMenuRenderer *baseRenderer = nullptr;

    if (_display) {
        _display->begin();

        // Base rendering setup for all displays
        baseRenderer = _display->getBaseRenderer();
        if (baseRenderer) {
            baseRenderer->setCustomDrawingHandler(this);
            baseRenderer->setUpdatesPerSecond(_uiData->updatesPerSec);
        }

        setBacklightEnable(true);
    }

    if (_input) { // Driver responsible for call to menuMgr.init[-like]()
        _input->begin(_display, _homeMenu ? _homeMenu->getRootItem() : nullptr);
    } else { // Default/remote init
        menuMgr.initWithoutInput(baseRenderer, _homeMenu ? _homeMenu->getRootItem() : nullptr);
    }

    if (_display) { // setupRendering() typically results in display driver refresh/reorient
        _display->setupRendering(_uiData->displayTheme, _uiData->titleMode,
                                 _itemFont, _titleFont,
                                 _uiData->analogSlider, _uiData->editingIcons,
                                 _isTcUnicodeFonts);
    }

    #if TERRA_UI_START_AT_OVERVIEW
        gotoScreen(TERRA_UI_OVERVIEW_ACT_MENU_ID);
    #endif
}

void TerraduinoBaseUI::setNeedsRedraw()
{
    if (_overview) { _overview->setNeedsFullRedraw(); }
    if (_homeMenu) { menuMgr.notifyStructureChanged(); }
}

SwitchInterruptMode TerraduinoBaseUI::getISRMode() const
{
    SwitchInterruptMode isrMode(SWITCHES_POLL_EVERYTHING);
    if (_allowISR && _input) {
        bool mainPinsInterruptable = _input->areMainPinsInterruptable();
        bool allPinsInterruptable = mainPinsInterruptable && _input->areAllPinsInterruptable();
        isrMode = (allPinsInterruptable ? SWITCHES_NO_POLLING : (mainPinsInterruptable ? SWITCHES_POLL_KEYS_ONLY : SWITCHES_POLL_EVERYTHING));
    }
    return isrMode;
}

void TerraduinoBaseUI::setBacklightEnable(bool enabled)
{
    if (_uiDispSetup.dispCfgType != UIDisplaySetup::LCD && _backlight) {
        if (enabled) {
            if (_backlight->isDigitalType()) {
                ((TerraDigitalPin *)_backlight)->activate();
            } else {
                ((TerraAnalogPin *)_backlight)->analogWrite(1.0f);
            }
        } else {
            if (_backlight->isDigitalType()) {
                ((TerraDigitalPin *)_backlight)->deactivate();
            } else {
                ((TerraAnalogPin *)_backlight)->analogWrite(0.0f); // todo: nice backlight-out anim
            }
            _blTimeout = 0;
        }
    } else if (_uiDispSetup.dispCfgType == UIDisplaySetup::LCD && _display) {
        if (enabled) {
            if (_uiDispSetup.getBacklightMode() != Terra_BacklightMode_PWM) {
                ((TerraDisplayLiquidCrystal *)_display)->getLCD().backlight();
            } else {
                ((TerraDisplayLiquidCrystal *)_display)->getLCD().setBacklight(255);
            }
        } else {
            if (_uiDispSetup.getBacklightMode() != Terra_BacklightMode_PWM) {
                ((TerraDisplayLiquidCrystal *)_display)->getLCD().noBacklight();
            } else {
                ((TerraDisplayLiquidCrystal *)_display)->getLCD().setBacklight(0); // todo: nice backlight-out anim
            }
            _blTimeout = 0;
        }
    }
}

void TerraduinoBaseUI::started(BaseMenuRenderer *currentRenderer)
{
    // overview screen started
    if (_display) {
        if (_overview) { _overview->setNeedsFullRedraw(); }

        _blTimeout = (_backlight || _uiDispSetup.dispCfgType == UIDisplaySetup::LCD ? unixNow() + TERRA_UI_BACKLIGHT_TIMEOUT : 0);
    }
}

void TerraduinoBaseUI::reset()
{
    // menu interaction timeout
    if (_display) {
        #if TERRA_UI_DEALLOC_AFTER_USE
            if (_homeMenu) { _homeMenu->unloadSubMenus(); }
        #endif

        if (!_overview) {
            _overview = _display->allocateOverview(_clockFont, _detailFont);
            TERRA_SOFT_ASSERT(_overview, SFP(TStr_Err_AllocationFailure));
        }

        _display->getBaseRenderer()->takeOverDisplay();
    }
}

void TerraduinoBaseUI::renderLoop(unsigned int currentValue, RenderPressMode userClick)
{
    // render overview screen until key interruption
    if (_display) {
        if (userClick == RPRESS_NONE) {
            if (_overview) { _overview->renderOverview(_display->isLandscape(), _display->getScreenSize()); }

            if (_blTimeout && unixNow() >= _blTimeout) { setBacklightEnable(false); }
        } else {
            _display->getBaseRenderer()->giveBackDisplay();

            setBacklightEnable(true);
            _blTimeout = 0;

            #if TERRA_UI_DEALLOC_AFTER_USE
                if (_overview) { delete _overview; _overview = nullptr; }
            #endif
        }
    }
}

#endif
