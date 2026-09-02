/*  Terraduino: Full/RW UI.
    Copyright (C) 2026 NachtRaveVL
*/

#include "TerraduinoUI.h"
#ifdef TERRA_USE_GUI

TerraduinoFullUI::TerraduinoFullUI(String deviceUUID, UIControlSetup uiControlSetup, UIDisplaySetup uiDisplaySetup, bool isActiveLowIO, bool allowInterruptableIO, bool enableTcUnicodeFonts, bool enableBufferedVRAM)
    : TerraduinoBaseUI(deviceUUID, uiControlSetup, uiDisplaySetup, isActiveLowIO, allowInterruptableIO, enableTcUnicodeFonts, enableBufferedVRAM)
{
    auto controller = getController();
    TERRA_HARD_ASSERT(controller, SFP(TStr_Err_InitializationFailure));

    if (controller) {
        // Input driver setup
        auto ctrlInMode = controller->getControlInputMode();
        auto ctrlInPins = controller->getControlInputPins();
        switch (ctrlInMode) {
            case Terra_ControlInputMode_RotaryEncoderOk:
            case Terra_ControlInputMode_RotaryEncoderOkLR: {
                TERRA_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Encoder, SFP(TStr_Err_InvalidParameter));
                _input = new TerraInputRotary(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.encoder.encoderSpeed);
            } break;

            case Terra_ControlInputMode_UpDownButtonsOk:
            case Terra_ControlInputMode_UpDownButtonsOkLR: {
                TERRA_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Buttons, SFP(TStr_Err_InvalidParameter));
                if (!_uiCtrlSetup.ctrlCfgAs.buttons.isDFRobotShield) {
                    _input = new TerraInputUpDownButtons(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.buttons.repeatSpeed);
                } else {
                    _input = new TerraInputUpDownButtons(true, _uiCtrlSetup.ctrlCfgAs.buttons.repeatSpeed);
                }
            } break;

            case Terra_ControlInputMode_UpDownESP32TouchOk:
            case Terra_ControlInputMode_UpDownESP32TouchOkLR: {
                TERRA_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::ESP32Touch, SFP(TStr_Err_InvalidParameter));
                _input = new TerraInputESP32TouchKeys(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.espTouch.repeatSpeed, _uiCtrlSetup.ctrlCfgAs.espTouch.switchThreshold,
                                                      _uiCtrlSetup.ctrlCfgAs.espTouch.highVoltage, _uiCtrlSetup.ctrlCfgAs.espTouch.lowVoltage, _uiCtrlSetup.ctrlCfgAs.espTouch.attenuation);
            } break;

            case Terra_ControlInputMode_AnalogJoystickOk: {
                TERRA_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Joystick, SFP(TStr_Err_InvalidParameter));
                if (_uiData) {
                    _input = new TerraInputJoystick(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.joystick.repeatDelay, _uiCtrlSetup.ctrlCfgAs.joystick.decreaseDivisor,
                                                    _uiData->joystickCalib[0], _uiData->joystickCalib[1], _uiData->joystickCalib[2]);
                } else {
                    _input = new TerraInputJoystick(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.joystick.repeatDelay, _uiCtrlSetup.ctrlCfgAs.joystick.decreaseDivisor);
                }
            } break;

            case Terra_ControlInputMode_Matrix2x2UpDownButtonsOkL: {
                TERRA_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Matrix, SFP(TStr_Err_InvalidParameter));
                _input = new TerraInputMatrix2x2(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.matrix.repeatDelay, _uiCtrlSetup.ctrlCfgAs.matrix.repeatInterval);
            } break;

            case Terra_ControlInputMode_Matrix3x4Keyboard_OptRotEncOk:
            case Terra_ControlInputMode_Matrix3x4Keyboard_OptRotEncOkLR: {
                TERRA_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Matrix, SFP(TStr_Err_InvalidParameter));
                _input = new TerraInputMatrix3x4(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.matrix.repeatDelay, _uiCtrlSetup.ctrlCfgAs.matrix.repeatInterval,
                                                 _uiCtrlSetup.ctrlCfgAs.matrix.encoderSpeed);
            } break;

            case Terra_ControlInputMode_Matrix4x4Keyboard_OptRotEncOk:
            case Terra_ControlInputMode_Matrix4x4Keyboard_OptRotEncOkLR: {
                TERRA_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Matrix, SFP(TStr_Err_InvalidParameter));
                _input = new TerraInputMatrix4x4(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.matrix.repeatDelay, _uiCtrlSetup.ctrlCfgAs.matrix.repeatInterval,
                                                 _uiCtrlSetup.ctrlCfgAs.matrix.encoderSpeed);
            } break;

            default: break;
        }
        TERRA_SOFT_ASSERT(!(ctrlInMode >= Terra_ControlInputMode_RotaryEncoderOk && ctrlInMode != Terra_ControlInputMode_ResistiveTouch && ctrlInMode <= Terra_ControlInputMode_TouchScreen) || _input, SFP(TStr_Err_AllocationFailure));

        // Display driver setup
        auto dispOutMode = controller->getDisplayOutputMode();
        auto displaySetup = controller->getDisplaySetup();

        // LiquidCrystalIO supports only i2c, /w LCD setup
        TERRA_SOFT_ASSERT(!(dispOutMode >= Terra_DisplayOutputMode_LCD16x2_EN && dispOutMode <= Terra_DisplayOutputMode_LCD20x4_RS) || displaySetup.cfgType == DeviceSetup::I2CSetup, SFP(TStr_Err_InvalidParameter));
        TERRA_SOFT_ASSERT(!(dispOutMode >= Terra_DisplayOutputMode_LCD16x2_EN && dispOutMode <= Terra_DisplayOutputMode_LCD20x4_RS) || _uiDispSetup.dispCfgType == UIDisplaySetup::LCD, SFP(TStr_Err_InvalidParameter));
        // U8g2 supports either i2c or SPI, /w Pixel setup
        TERRA_SOFT_ASSERT(!(dispOutMode >= Terra_DisplayOutputMode_SSD1305 && dispOutMode <= Terra_DisplayOutputMode_CustomOLED) || (displaySetup.cfgType == DeviceSetup::I2CSetup || displaySetup.cfgType == DeviceSetup::SPISetup), SFP(TStr_Err_InvalidParameter));
        TERRA_SOFT_ASSERT(!(dispOutMode >= Terra_DisplayOutputMode_SSD1607 && dispOutMode <= Terra_DisplayOutputMode_IL3820_V2) || displaySetup.cfgType == DeviceSetup::SPISetup, SFP(TStr_Err_InvalidParameter));
        TERRA_SOFT_ASSERT(!(dispOutMode >= Terra_DisplayOutputMode_SSD1305 && dispOutMode <= Terra_DisplayOutputMode_IL3820_V2) || _uiDispSetup.dispCfgType == UIDisplaySetup::Pixel, SFP(TStr_Err_InvalidParameter));
        // AdafruitGFX supports only SPI, /w Pixel setup
        TERRA_SOFT_ASSERT(!(dispOutMode >= Terra_DisplayOutputMode_ST7735 && dispOutMode <= Terra_DisplayOutputMode_ILI9341) || displaySetup.cfgType == DeviceSetup::SPISetup, SFP(TStr_Err_InvalidParameter));
        TERRA_SOFT_ASSERT(!(dispOutMode >= Terra_DisplayOutputMode_ST7735 && dispOutMode <= Terra_DisplayOutputMode_ILI9341) || _uiDispSetup.dispCfgType == UIDisplaySetup::Pixel, SFP(TStr_Err_InvalidParameter));
        // TFT_eSPI supports only SPI, /w TFT setup
        TERRA_SOFT_ASSERT(dispOutMode != Terra_DisplayOutputMode_TFT || displaySetup.cfgType == DeviceSetup::SPISetup, SFP(TStr_Err_InvalidParameter));
        TERRA_SOFT_ASSERT(dispOutMode != Terra_DisplayOutputMode_TFT || _uiDispSetup.dispCfgType == UIDisplaySetup::TFT, SFP(TStr_Err_InvalidParameter));

        switch (dispOutMode) {
            // LiquidCrystalIO
            case Terra_DisplayOutputMode_LCD16x2_EN:
            case Terra_DisplayOutputMode_LCD16x2_RS:
            case Terra_DisplayOutputMode_LCD20x4_EN:
            case Terra_DisplayOutputMode_LCD20x4_RS: {
                if (!_uiDispSetup.dispCfgAs.lcd.isDFRobotShield) {
                    _display = new TerraDisplayLiquidCrystal(dispOutMode, displaySetup.cfgAs.i2c, _uiDispSetup.dispCfgAs.lcd.ledMode);
                } else {
                    _display = new TerraDisplayLiquidCrystal(true, displaySetup.cfgAs.i2c, _uiDispSetup.dispCfgAs.lcd.ledMode);
                }
            } break;

            // U8g2OLED
            case Terra_DisplayOutputMode_SSD1305: {
                if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == TERRA_USE_WIRE) {
                    _display = TerraDisplayU8g2OLED::allocateSSD1305Wire(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == TERRA_USE_WIRE1) {
                    _display = TerraDisplayU8g2OLED::allocateSSD1305Wire1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == TERRA_USE_SPI) {
                    _display = TerraDisplayU8g2OLED::allocateSSD1305SPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == TERRA_USE_SPI1) {
                    _display = TerraDisplayU8g2OLED::allocateSSD1305SPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    TERRA_SOFT_ASSERT(false, SFP(TStr_Err_InvalidParameter));
                }
            } break;
            case Terra_DisplayOutputMode_SSD1305_x32Ada: {
                if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == TERRA_USE_WIRE) {
                    _display = TerraDisplayU8g2OLED::allocateSSD1305x32AdaWire(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == TERRA_USE_WIRE1) {
                    _display = TerraDisplayU8g2OLED::allocateSSD1305x32AdaWire1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == TERRA_USE_SPI) {
                    _display = TerraDisplayU8g2OLED::allocateSSD1305x32AdaSPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == TERRA_USE_SPI1) {
                    _display = TerraDisplayU8g2OLED::allocateSSD1305x32AdaSPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    TERRA_SOFT_ASSERT(false, SFP(TStr_Err_InvalidParameter));
                }
            } break;
            case Terra_DisplayOutputMode_SSD1305_x64Ada: {
                if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == TERRA_USE_WIRE) {
                    _display = TerraDisplayU8g2OLED::allocateSSD1305x64AdaWire(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == TERRA_USE_WIRE1) {
                    _display = TerraDisplayU8g2OLED::allocateSSD1305x64AdaWire1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == TERRA_USE_SPI) {
                    _display = TerraDisplayU8g2OLED::allocateSSD1305x64AdaSPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == TERRA_USE_SPI1) {
                    _display = TerraDisplayU8g2OLED::allocateSSD1305x64AdaSPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    TERRA_SOFT_ASSERT(false, SFP(TStr_Err_InvalidParameter));
                }
            } break;
            case Terra_DisplayOutputMode_SSD1306: {
                if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == TERRA_USE_WIRE) {
                    _display = TerraDisplayU8g2OLED::allocateSSD1306Wire(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == TERRA_USE_WIRE1) {
                    _display = TerraDisplayU8g2OLED::allocateSSD1306Wire1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == TERRA_USE_SPI) {
                    _display = TerraDisplayU8g2OLED::allocateSSD1306SPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == TERRA_USE_SPI1) {
                    _display = TerraDisplayU8g2OLED::allocateSSD1306SPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    TERRA_SOFT_ASSERT(false, SFP(TStr_Err_InvalidParameter));
                }
            } break;
            case Terra_DisplayOutputMode_SH1106: {
                if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == TERRA_USE_WIRE) {
                    _display = TerraDisplayU8g2OLED::allocateSH1106Wire(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == TERRA_USE_WIRE1) {
                    _display = TerraDisplayU8g2OLED::allocateSH1106Wire1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == TERRA_USE_SPI) {
                    _display = TerraDisplayU8g2OLED::allocateSH1106SPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == TERRA_USE_SPI1) {
                    _display = TerraDisplayU8g2OLED::allocateSH1106SPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    TERRA_SOFT_ASSERT(false, SFP(TStr_Err_InvalidParameter));
                }
            } break;
            case Terra_DisplayOutputMode_CustomOLED: {
                if (displaySetup.cfgType == DeviceSetup::I2CSetup) {
                    _display = TerraDisplayU8g2OLED::allocateCustomOLEDI2C(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup) {
                    _display = TerraDisplayU8g2OLED::allocateCustomOLEDSPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    TERRA_SOFT_ASSERT(false, SFP(TStr_Err_InvalidParameter));
                }
            } break;
            case Terra_DisplayOutputMode_SSD1607: {
                if (displaySetup.cfgAs.spi.spi == TERRA_USE_SPI) {
                    _display = TerraDisplayU8g2OLED::allocateSSD1607SPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgAs.spi.spi == TERRA_USE_SPI1) {
                    _display = TerraDisplayU8g2OLED::allocateSSD1607SPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    TERRA_SOFT_ASSERT(false, SFP(TStr_Err_InvalidParameter));
                }
            } break;
            case Terra_DisplayOutputMode_IL3820: {
                if (displaySetup.cfgAs.spi.spi == TERRA_USE_SPI) {
                    _display = TerraDisplayU8g2OLED::allocateIL3820SPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgAs.spi.spi == TERRA_USE_SPI1) {
                    _display = TerraDisplayU8g2OLED::allocateIL3820SPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    TERRA_SOFT_ASSERT(false, SFP(TStr_Err_InvalidParameter));
                }
            } break;
            case Terra_DisplayOutputMode_IL3820_V2: {
                if (displaySetup.cfgAs.spi.spi == TERRA_USE_SPI) {
                    _display = TerraDisplayU8g2OLED::allocateIL3820V2SPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgAs.spi.spi == TERRA_USE_SPI1) {
                    _display = TerraDisplayU8g2OLED::allocateIL3820V2SPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    TERRA_SOFT_ASSERT(false, SFP(TStr_Err_InvalidParameter));
                }
            } break;

            // AdafruitGFX
            case Terra_DisplayOutputMode_ST7735: {
                _display = new TerraDisplayAdafruitGFX<Adafruit_ST7735>(displaySetup.cfgAs.spi, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.st77Kind, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
            } break;
            case Terra_DisplayOutputMode_ST7789: {
                _display = new TerraDisplayAdafruitGFX<Adafruit_ST7789>(displaySetup.cfgAs.spi, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.st77Kind, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
            } break;
            case Terra_DisplayOutputMode_ILI9341: {
                _display = new TerraDisplayAdafruitGFX<Adafruit_ILI9341>(displaySetup.cfgAs.spi, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
            } break;

            // TFT_eSPI
            case Terra_DisplayOutputMode_TFT: {
                TERRA_SOFT_ASSERT(!(bool)TERRA_USE_SPI || displaySetup.cfgAs.spi.spi == TERRA_USE_SPI, SFP(TStr_Err_InvalidParameter));
                #ifdef TFT_CS
                    TERRA_SOFT_ASSERT(displaySetup.cfgAs.spi.cs == TFT_CS, SFP(TStr_Err_NotConfiguredProperly));
                #else
                    TERRA_HARD_ASSERT(false, SFP(TStr_Err_NotConfiguredProperly));
                #endif
                _display = new TerraDisplayTFTeSPI(displaySetup.cfgAs.spi, _uiDispSetup.dispCfgAs.tft.rotation, _uiDispSetup.dispCfgAs.tft.st77Kind);
            } break;

            default: break;
        }
        TERRA_SOFT_ASSERT(!(dispOutMode >= Terra_DisplayOutputMode_LCD16x2_EN && dispOutMode <= Terra_DisplayOutputMode_TFT) || _display, SFP(TStr_Err_AllocationFailure));

        // Late input driver setup
        switch (ctrlInMode) {
            case Terra_ControlInputMode_ResistiveTouch: {
                TERRA_SOFT_ASSERT(_display, SFP(TStr_Err_NotYetInitialized));
                TERRA_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Touchscreen, SFP(TStr_Err_InvalidParameter));
                _input = new TerraInputResistiveTouch(ctrlInPins, _display, _uiDispSetup.getDisplayRotation(), _uiCtrlSetup.ctrlCfgAs.touchscreen.orient);
            } break;

            case Terra_ControlInputMode_TouchScreen: {
                TERRA_SOFT_ASSERT(_display, SFP(TStr_Err_NotYetInitialized));
                TERRA_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Touchscreen, SFP(TStr_Err_InvalidParameter));
                #ifdef TERRA_UI_ENABLE_XPT2046TS
                    TERRA_SOFT_ASSERT(ctrlInPins.first && ctrlInPins.second && isValidPin(ctrlInPins.second[0]), SFP(TStr_Err_InvalidPinOrType));
                #endif
                _input = new TerraInputTouchscreen(ctrlInPins, _display, _uiDispSetup.getDisplayRotation(), _uiCtrlSetup.ctrlCfgAs.touchscreen.orient);
            } break;

            // TFT_eSPI
            case Terra_ControlInputMode_TFTTouch: {
                TERRA_SOFT_ASSERT(_display, SFP(TStr_Err_NotYetInitialized));
                TERRA_SOFT_ASSERT(dispOutMode == Terra_DisplayOutputMode_TFT, SFP(TStr_Err_InvalidParameter));
                TERRA_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Touchscreen, SFP(TStr_Err_InvalidParameter));
                TERRA_SOFT_ASSERT(ctrlInPins.first && ctrlInPins.second && isValidPin(ctrlInPins.second[0]), SFP(TStr_Err_InvalidPinOrType));
                #ifdef TOUCH_CS
                    TERRA_SOFT_ASSERT(ctrlInPins.first && ctrlInPins.second && ctrlInPins.second[0] == TOUCH_CS, SFP(TStr_Err_NotConfiguredProperly));
                #else
                    TERRA_HARD_ASSERT(false, SFP(TStr_Err_NotConfiguredProperly));
                #endif
                _input = new TerraInputTFTTouch(ctrlInPins, (TerraDisplayTFTeSPI *)_display, _uiDispSetup.getDisplayRotation(), _uiCtrlSetup.ctrlCfgAs.touchscreen.orient, TERRA_UI_TFTTOUCH_USES_RAW);
            } break;

            default: break;
        }
        TERRA_SOFT_ASSERT(!(ctrlInMode == Terra_ControlInputMode_ResistiveTouch || ctrlInMode == Terra_ControlInputMode_TFTTouch) || _input, SFP(TStr_Err_AllocationFailure));
    }
}

TerraduinoFullUI::~TerraduinoFullUI()
{ ; }

void TerraduinoFullUI::addRemote(Terra_RemoteControl rcType, UARTDeviceSetup rcSetup, uint16_t rcServerPort)
{
    TerraRemoteControl *remoteControl = nullptr;

    switch (rcType) {
        case Terra_RemoteControl_Serial: {
            remoteControl = new TerraRemoteSerialControl(rcSetup);
            TERRA_SOFT_ASSERT(remoteControl, SFP(TStr_Err_AllocationFailure));
        } break;

        case Terra_RemoteControl_Simhub: {
            remoteControl = new TerraRemoteSimhubControl(rcSetup, TERRA_UI_SIMHUB_STATUS_MENU_ID);
            TERRA_SOFT_ASSERT(remoteControl, SFP(TStr_Err_AllocationFailure));
        } break;

        case Terra_RemoteControl_WiFi: {
            #ifdef TERRA_USE_WIFI
                remoteControl = new TerraRemoteWiFiControl(rcServerPort);
                TERRA_SOFT_ASSERT(remoteControl, SFP(TStr_Err_AllocationFailure));
            #endif
        } break;

        case Terra_RemoteControl_Ethernet: {
            #ifdef TERRA_USE_ETHERNET
                remoteControl = new TerraRemoteEthernetControl(rcServerPort);
                TERRA_SOFT_ASSERT(remoteControl, SFP(TStr_Err_AllocationFailure));
            #endif
        } break;

        default: break;
    }

    if (remoteControl && remoteControl->getConnection()) {
        if (!_remoteServer) { _remoteServer = new TcMenuRemoteServer(getApplicationInfo()); }
        if (_remoteServer) { _remoteServer->addConnection(remoteControl->getConnection()); }
        _remotes.push_back(remoteControl);
    } else {
        if (remoteControl) { delete remoteControl; }
    }
}

bool TerraduinoFullUI::isFullUI()
{
    return true;
}

#endif
