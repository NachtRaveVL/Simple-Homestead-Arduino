/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Display Drivers
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI

static inline const u8g2_cb_t *dispRotToU8g2Rot(Terra_DisplayRotation displayRotation)
{
    switch (displayRotation) {
        case Terra_DisplayRotation_R1: return U8G2_R1;
        case Terra_DisplayRotation_R2: return U8G2_R2;
        case Terra_DisplayRotation_R3: return U8G2_R3;
        case Terra_DisplayRotation_HorzMirror: return U8G2_MIRROR;
        case Terra_DisplayRotation_VertMirror: return U8G2_MIRROR_VERTICAL;
        case Terra_DisplayRotation_R0: default: return U8G2_R0;
    }
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateSSD1305SPI(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!(bool)TERRA_USE_SPI || displaySetup.cfgAs.spi.spi == TERRA_USE_SPI), SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X32_NONAME_F_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateSSD1305SPI1(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!(bool)TERRA_USE_SPI || displaySetup.cfgAs.spi.spi == TERRA_USE_SPI), SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X32_NONAME_F_2ND_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateSSD1305Wire(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!(bool)TERRA_USE_WIRE || displaySetup.cfgAs.i2c.wire == TERRA_USE_WIRE), SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X32_NONAME_F_HW_I2C(dispRotToU8g2Rot(displayRotation), resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateSSD1305Wire1(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!(bool)TERRA_USE_WIRE || ((bool)TERRA_USE_WIRE1 && displaySetup.cfgAs.i2c.wire == TERRA_USE_WIRE1)), SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X32_NONAME_F_2ND_HW_I2C(dispRotToU8g2Rot(displayRotation), resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateSSD1305x32AdaSPI(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!(bool)TERRA_USE_SPI || displaySetup.cfgAs.spi.spi == TERRA_USE_SPI), SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X32_ADAFRUIT_F_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateSSD1305x32AdaSPI1(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!(bool)TERRA_USE_SPI || displaySetup.cfgAs.spi.spi == TERRA_USE_SPI), SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X32_ADAFRUIT_F_2ND_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateSSD1305x32AdaWire(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!(bool)TERRA_USE_WIRE || displaySetup.cfgAs.i2c.wire == TERRA_USE_WIRE), SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X32_ADAFRUIT_F_HW_I2C(dispRotToU8g2Rot(displayRotation), resetPin));
}
inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateSSD1305x32AdaWire1(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!(bool)TERRA_USE_WIRE || ((bool)TERRA_USE_WIRE1 && displaySetup.cfgAs.i2c.wire == TERRA_USE_WIRE1)), SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X32_ADAFRUIT_F_2ND_HW_I2C(dispRotToU8g2Rot(displayRotation), resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateSSD1305x64AdaSPI(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!(bool)TERRA_USE_SPI || displaySetup.cfgAs.spi.spi == TERRA_USE_SPI), SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X64_ADAFRUIT_F_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateSSD1305x64AdaSPI1(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!(bool)TERRA_USE_SPI || displaySetup.cfgAs.spi.spi == TERRA_USE_SPI), SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X64_ADAFRUIT_F_2ND_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateSSD1305x64AdaWire(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!(bool)TERRA_USE_WIRE || displaySetup.cfgAs.i2c.wire == TERRA_USE_WIRE), SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X64_ADAFRUIT_F_HW_I2C(dispRotToU8g2Rot(displayRotation), resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateSSD1305x64AdaWire1(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!(bool)TERRA_USE_WIRE || ((bool)TERRA_USE_WIRE1 && displaySetup.cfgAs.i2c.wire == TERRA_USE_WIRE1)), SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X64_ADAFRUIT_F_2ND_HW_I2C(dispRotToU8g2Rot(displayRotation), resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateSSD1306SPI(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!(bool)TERRA_USE_SPI || displaySetup.cfgAs.spi.spi == TERRA_USE_SPI), SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateSSD1306SPI1(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!(bool)TERRA_USE_SPI || displaySetup.cfgAs.spi.spi == TERRA_USE_SPI), SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1306_128X64_NONAME_F_2ND_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateSSD1306Wire(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!(bool)TERRA_USE_WIRE || displaySetup.cfgAs.i2c.wire == TERRA_USE_WIRE), SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(dispRotToU8g2Rot(displayRotation), resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateSSD1306Wire1(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!(bool)TERRA_USE_WIRE || ((bool)TERRA_USE_WIRE1 && displaySetup.cfgAs.i2c.wire == TERRA_USE_WIRE1)), SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1306_128X64_NONAME_F_2ND_HW_I2C(dispRotToU8g2Rot(displayRotation), resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateSH1106SPI(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!(bool)TERRA_USE_SPI || displaySetup.cfgAs.spi.spi == TERRA_USE_SPI), SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateSH1106SPI1(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!(bool)TERRA_USE_SPI || displaySetup.cfgAs.spi.spi == TERRA_USE_SPI), SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SH1106_128X64_NONAME_F_2ND_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateSH1106Wire(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!(bool)TERRA_USE_WIRE || displaySetup.cfgAs.i2c.wire == TERRA_USE_WIRE), SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SH1106_128X64_NONAME_F_HW_I2C(dispRotToU8g2Rot(displayRotation), resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateSH1106Wire1(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!(bool)TERRA_USE_WIRE || ((bool)TERRA_USE_WIRE1 && displaySetup.cfgAs.i2c.wire == TERRA_USE_WIRE1)), SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SH1106_128X64_NONAME_F_2ND_HW_I2C(dispRotToU8g2Rot(displayRotation), resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateCustomOLEDI2C(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup, SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new TERRA_UI_CUSTOM_OLED_I2C(dispRotToU8g2Rot(displayRotation), resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateCustomOLEDSPI(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup, SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new TERRA_UI_CUSTOM_OLED_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateSSD1607SPI(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(!(bool)TERRA_USE_SPI || displaySetup.cfgAs.spi.spi == TERRA_USE_SPI, SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1607_200X200_F_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateSSD1607SPI1(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(!(bool)TERRA_USE_SPI || displaySetup.cfgAs.spi.spi == TERRA_USE_SPI, SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1607_200X200_F_2ND_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateIL3820SPI(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(!(bool)TERRA_USE_SPI || displaySetup.cfgAs.spi.spi == TERRA_USE_SPI, SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_IL3820_296X128_F_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateIL3820SPI1(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(!(bool)TERRA_USE_SPI || displaySetup.cfgAs.spi.spi == TERRA_USE_SPI, SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_IL3820_296X128_F_2ND_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateIL3820V2SPI(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(!(bool)TERRA_USE_SPI || displaySetup.cfgAs.spi.spi == TERRA_USE_SPI, SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_IL3820_V2_296X128_F_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline TerraDisplayU8g2OLED *TerraDisplayU8g2OLED::allocateIL3820V2SPI1(DeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    TERRA_SOFT_ASSERT(!(bool)TERRA_USE_SPI || displaySetup.cfgAs.spi.spi == TERRA_USE_SPI, SFP(TStr_Err_InvalidParameter));
    return new TerraDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_IL3820_V2_296X128_F_2ND_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}


template <class T>
TerraDisplayAdafruitGFX<T>::TerraDisplayAdafruitGFX(SPIDeviceSetup displaySetup, Terra_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
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

template <class T>
void TerraDisplayAdafruitGFX<T>::initBaseUIFromDefaults()
{
    getBaseUI()->init(TERRA_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), JOIN3(Terra_DisplayTheme, TERRA_UI_GFX_DISP_THEME_BASE, TERRA_UI_GFX_DISP_THEME_SMLMED)), Terra_TitleMode_Always, TERRA_UI_GFX_USE_ANALOG_SLIDER, TERRA_UI_GFX_USE_EDITING_ICONS);
}

template <class T>
void TerraDisplayAdafruitGFX<T>::begin()
{
    _gfx.begin();
    _screenSize[0] = _gfx.width();
    _screenSize[1] = _gfx.height();
    _gfx.setRotation((uint8_t)_rotation);
}

template <class T>
TerraOverview *TerraDisplayAdafruitGFX<T>::allocateOverview(const void *clockFont, const void *detailFont)
{
    return new TerraOverviewGFX<T>(this, clockFont, detailFont);
}

#endif
