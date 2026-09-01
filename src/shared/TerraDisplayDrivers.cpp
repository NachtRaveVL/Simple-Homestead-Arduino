/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Display Drivers
*/

#include "TerraduinoUI.h"
#ifdef TERRA_USE_GUI
#include "BaseRenderers.h"
#include "graphics/BaseGraphicalRenderer.h"
#include "IoAbstractionWire.h"
#include "DfRobotInputAbstraction.h"

void TerraDisplayDriver::setupRendering(Terra_DisplayTheme displayTheme, Terra_TitleMode titleMode, const void *itemFont, const void *titleFont, bool analogSlider, bool editingIcons, bool tcUnicodeFonts)
{
    auto graphicsRenderer = getGraphicsRenderer();
    if (graphicsRenderer) {
        if (getController()->getControlInputMode() >= Terra_ControlInputMode_ResistiveTouch &&
            getController()->getControlInputMode() < Terra_ControlInputMode_RemoteControl) {
            graphicsRenderer->setHasTouchInterface(true);
        }
        graphicsRenderer->setTitleMode((BaseGraphicalRenderer::TitleMode)titleMode);
        graphicsRenderer->setUseSliderForAnalog(analogSlider);
        if (tcUnicodeFonts) { graphicsRenderer->enableTcUnicode(); }

        if (_displayTheme != displayTheme) {
            switch ((_displayTheme = displayTheme)) {
                case Terra_DisplayTheme_CoolBlue_ML:
                    installCoolBlueModernTheme(*graphicsRenderer, MenuFontDef(itemFont, TERRA_UI_MENU_ITEM_MAG_LEVEL), MenuFontDef(titleFont, TERRA_UI_MENU_TITLE_MAG_LEVEL), editingIcons);
                    break;
                case Terra_DisplayTheme_CoolBlue_SM:
                    installCoolBlueTraditionalTheme(*graphicsRenderer, MenuFontDef(itemFont, TERRA_UI_MENU_ITEM_MAG_LEVEL), MenuFontDef(titleFont, TERRA_UI_MENU_TITLE_MAG_LEVEL), editingIcons);
                    break;
                case Terra_DisplayTheme_DarkMode_ML:
                    installDarkModeModernTheme(*graphicsRenderer, MenuFontDef(itemFont, TERRA_UI_MENU_ITEM_MAG_LEVEL), MenuFontDef(titleFont, TERRA_UI_MENU_TITLE_MAG_LEVEL), editingIcons);
                    break;
                case Terra_DisplayTheme_DarkMode_SM:
                    installDarkModeTraditionalTheme(*graphicsRenderer, MenuFontDef(itemFont, TERRA_UI_MENU_ITEM_MAG_LEVEL), MenuFontDef(titleFont, TERRA_UI_MENU_TITLE_MAG_LEVEL), editingIcons);
                    break;
                case Terra_DisplayTheme_MonoOLED:
                    installMonoBorderedTheme(*graphicsRenderer, MenuFontDef(itemFont, TERRA_UI_MENU_ITEM_MAG_LEVEL), MenuFontDef(titleFont, TERRA_UI_MENU_TITLE_MAG_LEVEL), editingIcons);
                    break;
                case Terra_DisplayTheme_MonoOLED_Inv:
                    installMonoInverseTitleTheme(*graphicsRenderer, MenuFontDef(itemFont, TERRA_UI_MENU_ITEM_MAG_LEVEL), MenuFontDef(titleFont, TERRA_UI_MENU_TITLE_MAG_LEVEL), editingIcons);
                    break;
            }
        }
    }
}


TerraDisplayLiquidCrystal::TerraDisplayLiquidCrystal(Terra_DisplayOutputMode displayMode, I2CDeviceSetup displaySetup, Terra_BacklightMode ledMode)
    : TerraDisplayDriver(Terra_DisplayRotation_Undefined, displayMode < Terra_DisplayOutputMode_LCD20x4_EN ? 16 : 20, displayMode < Terra_DisplayOutputMode_LCD20x4_EN ? 2 : 4),
      _lcd(displayMode == Terra_DisplayOutputMode_LCD16x2_EN || displayMode == Terra_DisplayOutputMode_LCD20x4_EN ? 2 : 0, 1,
           displayMode == Terra_DisplayOutputMode_LCD16x2_EN || displayMode == Terra_DisplayOutputMode_LCD20x4_EN ? 0 : 2, 4, 5, 6, 7,
           ledMode == Terra_BacklightMode_Normal ? LiquidCrystal::BACKLIGHT_NORMAL : ledMode == Terra_BacklightMode_Inverted ? LiquidCrystal::BACKLIGHT_INVERTED : LiquidCrystal::BACKLIGHT_PWM,
           ioFrom8574(TERRA_UI_I2C_LCD_BASEADDR | displaySetup.address, 0xff, displaySetup.wire, false)),
      _renderer(_lcd, _screenSize[0], _screenSize[1], TerraDisplayDriver::getSystemName())
{
    _lcd.configureBacklightPin(3);
    _renderer.setTitleRequired(false);
}

TerraDisplayLiquidCrystal::TerraDisplayLiquidCrystal(bool, I2CDeviceSetup displaySetup, Terra_BacklightMode ledMode)
    : TerraDisplayDriver(Terra_DisplayRotation_Undefined, 16, 2),
      _lcd(8, 9, 4, 5, 6, 7,
           ledMode == Terra_BacklightMode_Normal ? LiquidCrystal::BACKLIGHT_NORMAL : ledMode == Terra_BacklightMode_Inverted ? LiquidCrystal::BACKLIGHT_INVERTED : LiquidCrystal::BACKLIGHT_PWM,
           ioFrom8574(TERRA_UI_I2C_LCD_BASEADDR | displaySetup.address, 0xff, displaySetup.wire, false)),
      _renderer(_lcd, _screenSize[0], _screenSize[1], TerraDisplayDriver::getSystemName())
{
    _lcd.configureBacklightPin(10);
    _renderer.setTitleRequired(false);
}

void TerraDisplayLiquidCrystal::initBaseUIFromDefaults()
{
    getBaseUI()->init(TERRA_UI_UPDATE_SPEED, Terra_DisplayTheme_Undefined, _screenSize[1] >= 4 ? Terra_TitleMode_Always : Terra_TitleMode_None);
}

void TerraDisplayLiquidCrystal::begin()
{
    _lcd.begin(_screenSize[0], _screenSize[1]);
}

void TerraDisplayLiquidCrystal::setupRendering(Terra_DisplayTheme displayTheme, Terra_TitleMode titleMode, const void *itemFont, const void *titleFont, bool analogSlider, bool editingIcons, bool tcUnicodeFonts)
{
    // TerraDisplayDriver::setupRendering(displayTheme, titleMode, itemFont, titleFont, analogSlider, editingIcons, tcUnicodeFonts); // simply returns
    _renderer.setTitleRequired(titleMode == Terra_TitleMode_Always);
}

TerraOverview *TerraDisplayLiquidCrystal::allocateOverview(const void *clockFont, const void *detailFont)
{
    return new TerraOverviewLCD(this);
}


TerraDisplayU8g2OLED::TerraDisplayU8g2OLED(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, U8G2 *gfx)
    : TerraDisplayDriver(displayRotation, gfx->getDisplayWidth(), gfx->getDisplayHeight()), // already rotated due to constructor, possibly incorrect until after begin
      _gfx(gfx), _drawable(nullptr), _renderer(nullptr)
{
    TERRA_SOFT_ASSERT(_gfx, SFP(TStr_Err_AllocationFailure));
    if (_gfx) {
        if (displaySetup.cfgType == DeviceSetup::I2CSetup) {
            _gfx->setI2CAddress(TERRA_UI_I2C_OLED_BASEADDR | displaySetup.cfgAs.i2c.address);
        }
        #ifdef TERRA_UI_ENABLE_STCHROMA_LDTC
            _drawable = new StChromaArtDrawable();
        #else
            if (displaySetup.cfgType == DeviceSetup::I2CSetup) {
                _drawable = new U8g2Drawable(_gfx, displaySetup.cfgAs.i2c.wire, getBaseUI() && getBaseUI()->isTcUnicodeFonts());
            } else {
                _drawable = new U8g2Drawable(_gfx, nullptr, getBaseUI() && getBaseUI()->isTcUnicodeFonts());
            }
        #endif
        TERRA_SOFT_ASSERT(_drawable, SFP(TStr_Err_AllocationFailure));

        if (_drawable) {
            _renderer = new GraphicsDeviceRenderer(TERRA_UI_RENDERER_BUFFERSIZE, TerraDisplayDriver::getSystemName(), _drawable);
            TERRA_SOFT_ASSERT(_renderer, SFP(TStr_Err_AllocationFailure));
        }
    }
}

TerraDisplayU8g2OLED::~TerraDisplayU8g2OLED()
{
    if (_renderer) { delete _renderer; }
    if (_drawable) { delete _drawable; }
    if (_gfx) { delete _gfx; }
}

void TerraDisplayU8g2OLED::initBaseUIFromDefaults()
{
    getBaseUI()->init(TERRA_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), Terra_DisplayTheme_MonoOLED), Terra_TitleMode_Always);
}

void TerraDisplayU8g2OLED::begin()
{
    if (_gfx) {
        _gfx->begin();
        _screenSize[0] = _gfx->getDisplayWidth();
        _screenSize[1] = _gfx->getDisplayHeight();
    }
}

TerraOverview *TerraDisplayU8g2OLED::allocateOverview(const void *clockFont, const void *detailFont)
{
    return new TerraOverviewOLED(this, clockFont, detailFont);
}


TerraDisplayAdafruitGFX<Adafruit_ST7735>::TerraDisplayAdafruitGFX(SPIDeviceSetup displaySetup, Terra_DisplayRotation displayRotation, Terra_ST77XXKind st77Kind, pintype_t dcPin, pintype_t resetPin)
    : TerraDisplayDriver(displayRotation, _gfx.width(), _gfx.height()), _kind(st77Kind),
      #ifndef ESP8266
          _gfx(displaySetup.spi, intForPin(dcPin), intForPin(displaySetup.cs), intForPin(resetPin)),
      #else
          _gfx(intForPin(displaySetup.cs), intForPin(dcPin), intForPin(resetPin)),
      #endif
      _drawable(&_gfx, getBaseUI() ? getBaseUI()->getVRAMBufferRows() : 0),
      _renderer(TERRA_UI_RENDERER_BUFFERSIZE, TerraDisplayDriver::getSystemName(), &_drawable)
{
    TERRA_SOFT_ASSERT(_kind != Terra_ST77XXKind_Undefined, SFP(TStr_Err_InvalidParameter));
    #ifdef ESP8266
        TERRA_SOFT_ASSERT(!(bool)TERRA_USE_SPI || displaySetup.spi == TERRA_USE_SPI, SFP(TStr_Err_InvalidParameter));
    #endif

    switch (_kind) {
        case Terra_ST7735Tag_Green144:
        case Terra_ST7735Tag_HalloWing:
            _screenSize[0] = 128; _screenSize[1] = 128;
            break;
        case Terra_ST7735Tag_Mini:
        case Terra_ST7735Tag_MiniPlugin:
            _screenSize[0] = 80; _screenSize[1] = 160;
            break;
        default:
            _screenSize[0] = 128; _screenSize[1] = 160;
            break;
    }
}

void TerraDisplayAdafruitGFX<Adafruit_ST7735>::initBaseUIFromDefaults()
{
    getBaseUI()->init(TERRA_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), JOIN3(Terra_DisplayTheme, TERRA_UI_GFX_DISP_THEME_BASE, TERRA_UI_GFX_DISP_THEME_SMLMED)), Terra_TitleMode_Always, TERRA_UI_GFX_USE_ANALOG_SLIDER, TERRA_UI_GFX_USE_EDITING_ICONS);
}

void TerraDisplayAdafruitGFX<Adafruit_ST7735>::begin()
{
    if (_kind == Terra_ST7735Tag_B) {
        _gfx.initB();
    } else {
        _gfx.initR((uint8_t)_kind);
    }
    _screenSize[0] = _gfx.width();
    _screenSize[1] = _gfx.height();
    _gfx.setRotation((uint8_t)_rotation);
}

TerraOverview *TerraDisplayAdafruitGFX<Adafruit_ST7735>::allocateOverview(const void *clockFont, const void *detailFont)
{
    return new TerraOverviewGFX<Adafruit_ST7735>(this, clockFont, detailFont);
}


TerraDisplayAdafruitGFX<Adafruit_ST7789>::TerraDisplayAdafruitGFX(SPIDeviceSetup displaySetup, Terra_DisplayRotation displayRotation, Terra_ST77XXKind st77Kind, pintype_t dcPin, pintype_t resetPin)
    : TerraDisplayDriver(displayRotation, _gfx.width(), _gfx.height()), _kind(st77Kind),
      #ifndef ESP8266
          _gfx(displaySetup.spi, intForPin(dcPin), intForPin(displaySetup.cs), intForPin(resetPin)),
      #else
          _gfx(intForPin(displaySetup.cs), intForPin(dcPin), intForPin(resetPin)),
      #endif
      _drawable(&_gfx, getBaseUI() ? getBaseUI()->getVRAMBufferRows() : 0),
      _renderer(TERRA_UI_RENDERER_BUFFERSIZE, TerraDisplayDriver::getSystemName(), &_drawable)
{
    TERRA_SOFT_ASSERT(_kind != Terra_ST77XXKind_Undefined, SFP(TStr_Err_InvalidParameter));
    #ifdef ESP8266
        TERRA_SOFT_ASSERT(!(bool)TERRA_USE_SPI || displaySetup.spi == TERRA_USE_SPI, SFP(TStr_Err_InvalidParameter));
    #endif

    switch (_kind) {
        case Terra_ST7789Res_128x128:
            _screenSize[0] = 128; _screenSize[1] = 128;
            break;
        case Terra_ST7789Res_135x240:
            _screenSize[0] = 135; _screenSize[1] = 240;
            break;
        case Terra_ST7789Res_170x320:
            _screenSize[0] = 170; _screenSize[1] = 320;
            break;
        case Terra_ST7789Res_172x320:
            _screenSize[0] = 172; _screenSize[1] = 320;
            break;
        case Terra_ST7789Res_240x240:
            _screenSize[0] = 240; _screenSize[1] = 240;
            break;
        case Terra_ST7789Res_240x280:
            _screenSize[0] = 240; _screenSize[1] = 280;
            break;
        case Terra_ST7789Res_240x320:
            _screenSize[0] = 240; _screenSize[1] = 320;
            break;
        default:
            _screenSize[0] = TFT_GFX_WIDTH; _screenSize[1] = TFT_GFX_HEIGHT;
            break;
    }
}

void TerraDisplayAdafruitGFX<Adafruit_ST7789>::initBaseUIFromDefaults()
{
    getBaseUI()->init(TERRA_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), JOIN3(Terra_DisplayTheme, TERRA_UI_GFX_DISP_THEME_BASE, TERRA_UI_GFX_DISP_THEME_SMLMED)), Terra_TitleMode_Always, TERRA_UI_GFX_USE_ANALOG_SLIDER, TERRA_UI_GFX_USE_EDITING_ICONS);
}

void TerraDisplayAdafruitGFX<Adafruit_ST7789>::begin()
{
    _gfx.init(_screenSize[0], _screenSize[1]);
    _screenSize[0] = _gfx.width();
    _screenSize[1] = _gfx.height();
    _gfx.setRotation((uint8_t)_rotation);
}

TerraOverview *TerraDisplayAdafruitGFX<Adafruit_ST7789>::allocateOverview(const void *clockFont, const void *detailFont)
{
    return new TerraOverviewGFX<Adafruit_ST7789>(this, clockFont, detailFont);
}


TerraDisplayAdafruitGFX<Adafruit_ILI9341>::TerraDisplayAdafruitGFX(SPIDeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
    : TerraDisplayDriver(displayRotation, _gfx.width(), _gfx.height()), // possibly incorrect until after begin
      #ifndef ESP8266
          _gfx(displaySetup.spi, intForPin(dcPin), intForPin(displaySetup.cs), intForPin(resetPin)),
      #else
          _gfx(intForPin(displaySetup.cs), intForPin(dcPin), intForPin(resetPin)),
      #endif
      _drawable(&_gfx, getBaseUI() ? getBaseUI()->getVRAMBufferRows() : 0),
      _renderer(TERRA_UI_RENDERER_BUFFERSIZE, TerraDisplayDriver::getSystemName(), &_drawable)
{
    #ifdef ESP8266
        TERRA_SOFT_ASSERT(!(bool)TERRA_USE_SPI || displaySetup.spi == TERRA_USE_SPI, SFP(TStr_Err_InvalidParameter));
    #endif
}

void TerraDisplayAdafruitGFX<Adafruit_ILI9341>::initBaseUIFromDefaults()
{
    getBaseUI()->init(TERRA_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), JOIN3(Terra_DisplayTheme, TERRA_UI_GFX_DISP_THEME_BASE, TERRA_UI_GFX_DISP_THEME_SMLMED)), Terra_TitleMode_Always, TERRA_UI_GFX_USE_ANALOG_SLIDER, TERRA_UI_GFX_USE_EDITING_ICONS);
}

void TerraDisplayAdafruitGFX<Adafruit_ILI9341>::begin()
{
    _gfx.begin(getController()->getDisplaySetup().cfgAs.spi.speed);
    _screenSize[0] = _gfx.width();
    _screenSize[1] = _gfx.height();
    _gfx.setRotation((uint8_t)_rotation);
}

TerraOverview *TerraDisplayAdafruitGFX<Adafruit_ILI9341>::allocateOverview(const void *clockFont, const void *detailFont)
{
    return new TerraOverviewGFX<Adafruit_ILI9341>(this, clockFont, detailFont);
}


TerraDisplayTFTeSPI::TerraDisplayTFTeSPI(SPIDeviceSetup displaySetup, Terra_DisplayRotation displayRotation, Terra_ST77XXKind st77Kind)
    : TerraDisplayDriver(displayRotation, TFT_GFX_WIDTH, TFT_GFX_HEIGHT),
      _kind(st77Kind),
      _gfx(TFT_GFX_WIDTH, TFT_GFX_HEIGHT),
      _drawable(&_gfx, getBaseUI() ? getBaseUI()->getVRAMBufferRows() : 0),
      _renderer(TERRA_UI_RENDERER_BUFFERSIZE, TerraDisplayDriver::getSystemName(), &_drawable)
{ ; }

void TerraDisplayTFTeSPI::initBaseUIFromDefaults()
{
    getBaseUI()->init(TERRA_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), JOIN3(Terra_DisplayTheme, TERRA_UI_GFX_DISP_THEME_BASE, TERRA_UI_GFX_DISP_THEME_MEDLRG)), Terra_TitleMode_Always, TERRA_UI_GFX_USE_ANALOG_SLIDER, TERRA_UI_GFX_USE_EDITING_ICONS);
}

void TerraDisplayTFTeSPI::begin()
{
    if (_kind == Terra_ST7735Tag_B || _kind >= Terra_ST7789Res_Start) {
        _gfx.begin();
    } else {
        _gfx.begin((uint8_t)_kind);
    }
    _gfx.setRotation((uint8_t)_rotation);
    _renderer.setDisplayDimensions(getScreenSize().first, getScreenSize().second);
}

TerraOverview *TerraDisplayTFTeSPI::allocateOverview(const void *clockFont, const void *detailFont)
{
    return new TerraOverviewTFT(this, clockFont, detailFont);
}

#endif
