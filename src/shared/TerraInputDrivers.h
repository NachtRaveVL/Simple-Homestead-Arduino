/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Input Drivers
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraInputDrivers_H
#define TerraInputDrivers_H

class TerraInputDriver;
class TerraInputRotary;
class TerraInputUpDownButtons;
class TerraInputESP32TouchKeys;
class TerraInputJoystick;
class TerraInputMatrix2x2;
class TerraInputMatrix3x4;
class TerraInputMatrix4x4;
class TerraInputResistiveTouch;
class TerraInputTouchscreen;
class TerraInputTFTTouch;

#include "TerraduinoUI.h"
#include "KeyboardManager.h"
#include "tcMenuKeyboard.h"
#include "graphics/MenuTouchScreenEncoder.h"
#include "JoystickSwitchInput.h"

// Input Driver Base
// Base input driver class that manages control input mode selection.
class TerraInputDriver {
public:
    inline TerraInputDriver(Pair<uint8_t, const pintype_t *> controlPins) : _pins(controlPins) { ; }
    virtual ~TerraInputDriver() = default;

    // Called upon base UI begin, after display driver begin. Derived method is expected
    // to always call any menuMgr.init()-like initializer. Neither parameter is guaranteed
    // to be non-null.
    virtual void begin(TerraDisplayDriver *displayDriver, MenuItem *initialItem) = 0;

    // Determines if all pins specified as control input pins are interruptable, thus able
    // to use tcMenu's SWITCHES_NO_POLLING fully-enabled-ISR mode. This allows for fastest
    // control response timings. Derived classes with analog inputs can provide their own
    // implementation which excludes such pins.
    virtual bool areAllPinsInterruptable() const;

    // Determines if main (by default first two) control input pins are interruptable, thus
    // able to use tcMenu's SWITCHES_POLL_KEYS_ONLY partially-enabled-ISR mode. This allows
    // for at least fast control response timings on main input.
    virtual bool areMainPinsInterruptable() const;

    // Returns the IOAbstraction object associated with this input controller. This is used
    // in switches/keyboard initializations, with default implementation returning nullptr.
    virtual IoAbstractionRef getIoAbstraction() const;

    // Control input pins array accessor
    inline const Pair<uint8_t, const pintype_t *> &getPins() const { return _pins; }

protected:
    const Pair<uint8_t, const pintype_t *> _pins;           // Control input pins array, from controller initialization
};


// Rotary Encoder Input Driver
// Rotary encoder that uses a twisting motion, along with momentary push-down.
// Control input mode pin array:
// - RotaryEncoderOk: {eA,eB,Ok},
// - RotaryEncoderOkLR: {eA,eB,Ok,Bk,Nx}
// Note: CLK,DT,SW designated pins same as eA,eB,Ok pins.
class TerraInputRotary : public TerraInputDriver {
public:
    TerraInputRotary(Pair<uint8_t, const pintype_t *> controlPins, Terra_EncoderSpeed encoderSpeed);
    virtual ~TerraInputRotary() = default;

    virtual void begin(TerraDisplayDriver *displayDriver, MenuItem *initialItem) override;

    // Encoder detent speed accessor
    inline Terra_EncoderSpeed getEncoderSpeed() const { return _encoderSpeed; }

protected:
    const Terra_EncoderSpeed _encoderSpeed;                 // Encoder detent speed setting
};


// Up/Down Buttons Input Driver
// Standard momentary buttons input.
// Control input mode pin array:
// - UpDownButtonsOk: {Up,Dw,Ok}
// - UpDownButtonsOkLR: {Up,Dw,Ok,Bk,Nx}
// Note: Back/next pins optional (tpin_none/-1).
class TerraInputUpDownButtons : public TerraInputDriver {
public:
    TerraInputUpDownButtons(Pair<uint8_t, const pintype_t *> controlPins, uint16_t keyRepeatSpeed);
    // Special constructor for DFRobotShield /w buttons (isDFRobotShield_unused tossed, only used for constructor resolution)
    TerraInputUpDownButtons(bool isDFRobotShield_unused, uint16_t keyRepeatSpeed);
    virtual ~TerraInputUpDownButtons();

    virtual void begin(TerraDisplayDriver *displayDriver, MenuItem *initialItem) override;

    virtual IoAbstractionRef getIoAbstraction() const override { return _dfRobotIORef; }

    // Key repeat speed accessor
    inline uint16_t getKeySpeed() const { return _keySpeed; }

protected:
    const uint16_t _keySpeed;                               // Key repeat speed, in ticks (lower = faster)
    IoAbstractionRef _dfRobotIORef;                         // DFRobot ioRef (if used), else nullptr
};


// ESP32 ESPTouch Keys Input Driver
// For ESP32 only, uses integrated touch keys library.
// Control input mode pin array:
// - UpDownESP32TouchOk: {Up,Dw,Ok}
// - UpDownESP32TouchOkLR: {Up,Dw,Ok,Bk,Nx}
// Note: All touch keys are analog inputs.
class TerraInputESP32TouchKeys : public TerraInputDriver {
public:
    TerraInputESP32TouchKeys(Pair<uint8_t, const pintype_t *> controlPins, uint16_t keyRepeatSpeed, uint16_t switchThreshold, Terra_ESP32Touch_HighRef highVoltage, Terra_ESP32Touch_LowRef lowVoltage, Terra_ESP32Touch_HighRefAtten attenuation);
    virtual ~TerraInputESP32TouchKeys() = default;

    virtual void begin(TerraDisplayDriver *displayDriver, MenuItem *initialItem) override;

    virtual bool areAllPinsInterruptable() const override { return false; }

#ifdef ESP32
    virtual IoAbstractionRef getIoAbstraction() override { return &_esp32Touch; }
#endif

    // Key repeat speed accessor
    inline uint16_t getKeySpeed() const { return _keySpeed; }

protected:
    const uint16_t _keySpeed;                           // Key repeat speed, in ticks (lower = faster)
#ifdef ESP32
    ESP32TouchKeysAbstraction _esp32Touch;              // ESP32Touch IO abstraction
#endif
};


// Analog Joystick Input Driver
// Analog joystick control with momentary press button.
// Control input mode pin array:
// - AnalogJoystickOk: {aX,aY,Ok}
// Note: aX,aY pins are analog inputs.
// Note: aX pin optional (tpin_none/-1), but if defined used for Back/Next control.
class TerraInputJoystick : public TerraInputDriver {
public:
    TerraInputJoystick(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, float decreaseDivisor, float jsCenterX = 0.5f, float jsCenterY = 0.5f, float jsZeroTol = 0.05f);
    virtual ~TerraInputJoystick();

    virtual void begin(TerraDisplayDriver *displayDriver, MenuItem *initialItem) override;

    virtual bool areAllPinsInterruptable() const override;

    virtual IoAbstractionRef getIoAbstraction() const override { return (IoAbstractionRef)_joystickMultiIo; }

    // Repeat delay millis accessor
    inline millis_t getRepeatDelay() const { return _repeatDelay; }
    // Joystick decrease divider accessor
    inline float getDecreaseDivisor() const { return _decreaseDivisor; }
    // Joystick calibration array accessor (centerX, centerY, zeroTolerance)
    inline const float *getJoystickCalib() const { return _joystickCalib; }

protected:
    const millis_t _repeatDelay;                            // Button repeat delay millis
    const float _decreaseDivisor;                           // Joystick decrease divisor
    const float _joystickCalib[3];                          // Joystick calibration (midX,midY,0tol)
    MultiIoAbstraction *_joystickMultiIo;                   // Joystick multi-IO abstraction
    AnalogJoystickToButtons *_joystickIoXAxis;              // Joystick X axis IO abstraction
};


// 2x2 Button Matrix Input Driver
// For matrix-style standard input with 2 rows and 2 columns of cross-tied momentary buttons.
// Key actions mapped via TERRA_UI_2X2MATRIX_KEYS & TERRA_UI_MATRIX_ACTIONS.
// Control input mode pin array:
// - Matrix2x2UpDownButtonsOkL: {r0,r1,c0,c1}
// Note: Left/right designated pins same as row/column pins.
class TerraInputMatrix2x2 : public TerraInputDriver {
public:
    TerraInputMatrix2x2(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, millis_t repeatInterval);
    virtual ~TerraInputMatrix2x2() = default;

    virtual void begin(TerraDisplayDriver *displayDriver, MenuItem *initialItem) override;

    // Determines if row pins are interruptable. Used in keyboard initialization for faster
    // ISR-enabled control input.
    bool areRowPinsInterruptable() const;

    // Matrix keyboard accessor
    inline MatrixKeyboardManager &getKeyboard() { return _keyboard; }

protected:
    MatrixKeyboardManager _keyboard;                        // Matrix keyboard
    KeyboardLayout _keyboardLayout;                         // Matrix keyboard layout
    MenuEditingKeyListener _tcMenuKeyListener;              // Matrix key listener
};


// 3x4 Button Matrix Input Driver
// For matrix-style numeric input with 4 rows and 3 columns of cross-tied momentary buttons.
// Optionally attached additional rotary encoder, else tpin_none/-1 for eA.
// Key actions mapped via TERRA_UI_3X4MATRIX_KEYS & TERRA_UI_MATRIX_ACTIONS.
// Control input mode pin array:
// - Matrix3x4Keyboard_OptRotEncOk: {r0,r1,r2,r3,c0,c1,c2,eA,eB,Ok}
// - Matrix3x4Keyboard_OptRotEncOkLR: {r0,r1,r2,r3,c0,c1,c2,eA,eB,Ok,Bk,Nx}
// Note: Left/right designated pins same as row/column pins.
class TerraInputMatrix3x4 : public TerraInputDriver {
public:
    TerraInputMatrix3x4(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, millis_t repeatInterval, Terra_EncoderSpeed encoderSpeed);
    virtual ~TerraInputMatrix3x4();

    virtual void begin(TerraDisplayDriver *displayDriver, MenuItem *initialItem) override;

    // Determines if row pins are interruptable. Used in keyboard initialization for faster
    // ISR-enabled control input.
    bool areRowPinsInterruptable() const;
    virtual bool areMainPinsInterruptable() const override;

    // Matrix keyboard accessor
    inline MatrixKeyboardManager &getKeyboard() { return _keyboard; }
    // Optional rotary encoder accessor
    inline TerraInputRotary *getRotaryEncoder() { return _rotaryEncoder; }

protected:
    MatrixKeyboardManager _keyboard;                        // Matrix keyboard
    KeyboardLayout _keyboardLayout;                         // Matrix keyboard layout
    MenuEditingKeyListener _tcMenuKeyListener;              // Matrix key listener
    TerraInputRotary *_rotaryEncoder;                       // Optional rotary encoder, else nullptr
};


// 4x4 Button Matrix Input Driver
// For matrix-style alpha-numeric input with 4 rows and 4 columns of cross-tied momentary buttons.
// Optionally attached additional rotary encoder, else tpin_none/-1 for eA.
// Key actions mapped via TERRA_UI_4X4MATRIX_KEYS & TERRA_UI_MATRIX_ACTIONS.
// Control input mode pin array:
// - Matrix4x4Keyboard_OptRotEncOk: {r0,r1,r2,r3,c0,c1,c2,c3,eA,eB,Ok}
// - Matrix4x4Keyboard_OptRotEncOkLR: {r0,r1,r2,r3,c0,c1,c2,c3,eA,eB,Ok,Bk,Nx}
// Note: Left/right designated pins same as row/column pins.
class TerraInputMatrix4x4 : public TerraInputDriver {
public:
    TerraInputMatrix4x4(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, millis_t repeatInterval, Terra_EncoderSpeed encoderSpeed = Terra_EncoderSpeed_HalfCycle);
    virtual ~TerraInputMatrix4x4();

    virtual void begin(TerraDisplayDriver *displayDriver, MenuItem *initialItem) override;

    // Determines if row pins are interruptable. Used in keyboard initialization for faster
    // ISR-enabled control input.
    bool areRowPinsInterruptable() const;
    virtual bool areMainPinsInterruptable() const override;

    // Matrix keyboard accessor
    inline MatrixKeyboardManager &getKeyboard() { return _keyboard; }
    // Optional rotary encoder accessor
    inline TerraInputRotary *getRotaryEncoder() { return _rotaryEncoder; }

protected:
    MatrixKeyboardManager _keyboard;                        // Matrix keyboard
    KeyboardLayout _keyboardLayout;                         // Matrix keyboard layout
    MenuEditingKeyListener _tcMenuKeyListener;              // Matrix key listener
    TerraInputRotary *_rotaryEncoder;                       // Optional rotary encoder, else nullptr
};


// Resistive Touch Screen Input Driver
// A style of touch screen that uses resistive measurements for touch detection.
// Control input mode pin array:
// - ResistiveTouch: {X+,X-,Y+,Y-}
// Note: X-/Y- pins are analog inputs, X+/Y+ pins are digital inputs.
class TerraInputResistiveTouch : public TerraInputDriver {
public:
    TerraInputResistiveTouch(Pair<uint8_t, const pintype_t *> controlPins, TerraDisplayDriver *displayDriver, Terra_DisplayRotation displayRotation, Terra_TouchscreenOrientation touchOrient);
    virtual ~TerraInputResistiveTouch() = default;

    virtual void begin(TerraDisplayDriver *displayDriver, MenuItem *initialItem) override;

    virtual bool areMainPinsInterruptable() const override { return false; }

    // Touchscreen accessor
    inline MenuTouchScreenManager &getTouchScreen() { return _touchScreen; }

protected:
    iotouch::ResistiveTouchInterrogator _touchInterrogator; // Resistive touch interrogator
    iotouch::TouchOrientationSettings _touchOrientation;    // Touchscreen orientation
    MenuTouchScreenManager _touchScreen;                    // Touchscreen manager
};


// Touch Screen Input Driver
// Standard touch screen driver using FT6206 (i2c based) or XPT2046 (SPI based).
// StChromaArt BSP touchscreen 
// Control input mode pin array:
// - TouchScreen (FT6206): {}
// - TouchScreen (XPT2046): {tCS,tIRQ}
// Note: Adafruit FT6206 library hardcoded to always use Wire - can be changed only by manual file edit.
// Note: XPT2046 touchscreen IRQ pin optional (tpin_none/-1), but recommended for fastest control response timings.
class TerraInputTouchscreen : public TerraInputDriver {
public:
    TerraInputTouchscreen(Pair<uint8_t, const pintype_t *> controlPins, TerraDisplayDriver *displayDriver, Terra_DisplayRotation displayRotation, Terra_TouchscreenOrientation touchOrient);
    virtual ~TerraInputTouchscreen() = default;

    virtual void begin(TerraDisplayDriver *displayDriver, MenuItem *initialItem) override;

    virtual bool areMainPinsInterruptable() const override;

#ifdef TERRA_UI_ENABLE_XPT2046TS
    // Touchscreen accessor
    inline XPT2046_Touchscreen &getTouchScreen() { return _touchScreen; }
#else
    // Touchscreen accessor
    inline Adafruit_FT6206 &getTouchScreen() { return _touchScreen; }
#endif
protected:
#ifdef TERRA_UI_ENABLE_XPT2046TS
    XPT2046_Touchscreen _touchScreen;                       // XPT2046 touchscreen
#else
    Adafruit_FT6206 _touchScreen;                           // FT6206 touchscreen
#endif
#ifdef TERRA_UI_ENABLE_BSP_TOUCH
    StBspTouchInterrogator _touchInterrogator;              // StChromaArt touch interrogator
#else
    iotouch::AdaLibTouchInterrogator _touchInterrogator;    // Adafruit touch interrogator
#endif
    iotouch::TouchOrientationSettings _touchOrientation;    // Touchscreen orientation
};


// TFT Touch Screen Input Driver
// Standard XPT2046 touch screen, but using TFT_eSPI library. Must be paired with TFTeSPI display driver.
// Control input mode pin array:
// - TFTTouch (XPT2046): {tCS,tIRQ}
// Note: XPT2046 touchscreen IRQ pin optional (tpin_none/-1), but recommended for fastest control response timings.
class TerraInputTFTTouch : public TerraInputDriver {
public:
    TerraInputTFTTouch(Pair<uint8_t, const pintype_t *> controlPins, TerraDisplayTFTeSPI *displayDriver, Terra_DisplayRotation displayRotation, Terra_TouchscreenOrientation touchOrient, bool useRawTouch = false);
    virtual ~TerraInputTFTTouch() = default;

    virtual void begin(TerraDisplayDriver *displayDriver, MenuItem *initialItem) override;

    virtual bool areMainPinsInterruptable() const override;

    // Touchscreen accessor
    inline MenuTouchScreenManager &getTouchScreen() { return _touchScreen; }

protected:
    iotouch::TftSpiTouchInterrogator _touchInterrogator;    // TFTSPITouch touch interrogator
    iotouch::TouchOrientationSettings _touchOrientation;    // Touchscreen orientation
    MenuTouchScreenManager _touchScreen;                    // Touchscreen manager
};

#endif // /ifndef TerraInputDrivers_H
#endif
