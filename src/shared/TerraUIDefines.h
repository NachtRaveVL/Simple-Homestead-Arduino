/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino UI Defines
*/

#ifndef TerraUIDefines_H
#define TerraUIDefines_H

#define XPT2046_RAW_MAX                 4096                // XPT2046 touch screen raw maximum value

#if F_SPD >= 48000000                                       // Resolve an appropriate UI update speed (1-10)
#define TERRA_UI_UPDATE_SPEED           10
#elif F_SPD >= 32000000
#define TERRA_UI_UPDATE_SPEED           5
#else
#define TERRA_UI_UPDATE_SPEED           2
#endif

// The following sizes apply to all architectures
#define TERRA_UI_RENDERER_BUFFERSIZE    32                  // Buffer size for display renderers
#define TERRA_UI_STARFIELD_MAXSIZE      16                  // Starfield map maxsize
#define TERRA_UI_SPRITE_MAXYSIZE        16                  // Sprite max Y (pixel height) - aka # rows for VRAM buffer, when enabled
// The following sizes only apply to architectures that do not have STL support (AVR/SAM)
#define TERRA_UI_REMOTECONTROLS_MAXSIZE 2                   // Maximum array size for remote controls list (max # of remote controls)

// CustomOLED U8g2 device string
#ifndef TERRA_UI_CUSTOM_OLED_I2C
#define TERRA_UI_CUSTOM_OLED_I2C        U8G2_SSD1309_128X64_NONAME0_F_HW_I2C    // Custom OLED for i2c setup (must be _HW_I2C variant /w 2 init params: rotation, resetPin - Wire# not assertion checked since baked into define)
#endif
#ifndef TERRA_UI_CUSTOM_OLED_SPI
#define TERRA_UI_CUSTOM_OLED_SPI        U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI // Custom OLED for SPI setup (must be _4W_HW_SPI variant /w 4 init params: rotation, csPin, dcPin, resetPin - SPI# not assertion checked since baked into define)
#endif

#define TERRA_UI_I2C_LCD_BASEADDR       0x20                // Base address of I2C LiquidCrystalIO LCDs (bitwise or'ed with passed address - technically base address of i2c expander in use)
#define TERRA_UI_I2C_OLED_BASEADDR      0x78                // Base address of I2C U8g2 OLEDs (bitwise or'ed with passed address, some devices may need 0x7e)
#define TERRA_UI_BACKLIGHT_TIMEOUT      5 * SECS_PER_MIN    // Backlight timeout, in seconds
#define TERRA_UI_START_AT_OVERVIEW      false               // UI starts at overview screen (true), else menu screen (false)
#define TERRA_UI_DEALLOC_AFTER_USE      !HAS_LARGE_SRAM     // If menu data should be unloaded after use (true = lower memory usage, less responsive transitions), or stay memory-resident (false = higher memory usage, more responsive transitions)

#define TERRA_UI_KEYREPEAT_SPEED        20                  // Default key press repeat speed, in ticks (lower = faster)
#define TERRA_UI_REMOTESERVER_PORT      3333                // Default remote control server's listening port
#define TERRA_UI_2X2MATRIX_KEYS         "#BA*"              // 2x2 matrix keyboard keys (R/S1,D/S2,U/S3,L/S4), forced PROGMEM chars
#define TERRA_UI_3X4MATRIX_KEYS         "123456789*0#"      // 3x4 matrix keyboard keys (123,456,789,*0#), forced PROGMEM chars
#define TERRA_UI_4X4MATRIX_KEYS         "123A456B789C*0#D"  // 4x4 matrix keyboard keys (123A,456B,789C,*0#D), forced PROGMEM chars
#define TERRA_UI_MATRIX_ACTIONS         "#*AB"              // Assigned enter/select char, delete/exit char, back char, and next char on keyboard, forced PROGMEM
#define TERRA_UI_TFTTOUCH_USES_RAW      false               // Raw touch usage for TFTTouch

#define TERRA_UI_GFX_USE_ANALOG_SLIDER  true                // Default analog slider usage for graphical displays displaying variable value ranges
#define TERRA_UI_GFX_USE_EDITING_ICONS  false               // Default editing icon usage for graphical displays
#define TERRA_UI_MENU_TITLE_MAG_LEVEL   2                   // Menu title font magnification level (int only)
#define TERRA_UI_MENU_ITEM_MAG_LEVEL    2                   // Menu item font magnification level (int only)
#define TERRA_UI_IOT_MONITOR_TEXT       "IoT Monitor"       // Menu IoT monitor item text, forced PROGMEM chars
#define TERRA_UI_AUTHENTICATOR_TEXT     "Authenticator"     // Menu authenticator item text, forced PROGMEM chars
#define TERRA_UI_SIMHUB_STATUS_MENU_ID  60                  // Simhub remote status boolean menu item ID
#define TERRA_UI_OVERVIEW_ACT_MENU_ID   7                   // Overview action menu item ID

// Default graphical display theme base (CoolBlue, DarkMode) - keep on separate lines
#define TERRA_UI_GFX_DISP_THEME_BASE    CoolBlue
#define TERRA_UI_GFX_DISP_THEME_SMLMED  SM
#define TERRA_UI_GFX_DISP_THEME_MEDLRG  ML


// Remote Control
// Type of remote control.
enum Terra_RemoteControl : signed char {
    Terra_RemoteControl_Disabled,                           // Disabled remote control
    Terra_RemoteControl_Serial,                             // Remote control by Serial/Bluetooth AT, requires UART setup
    Terra_RemoteControl_Simhub,                             // Remote control by Simhub serial connector, requires UART setup
    Terra_RemoteControl_WiFi,                               // Remote control by WiFi device, requires enabled WiFi
    Terra_RemoteControl_Ethernet,                           // Remote control by Ethernet device, requires enabled Ethernet

    Terra_RemoteControl_Count,                              // Placeholder
    Terra_RemoteControl_Undefined = -1                      // Placeholder
};

// Display Rotation
// Amount of display rotation, or in some cases mirror'ing.
enum Terra_DisplayRotation : signed char {
    Terra_DisplayRotation_R0,                               // 0° clockwise display rotation (0° counter-clockwise device mounting)
    Terra_DisplayRotation_R1,                               // 90° clockwise display rotation (90° counter-clockwise device mounting)
    Terra_DisplayRotation_R2,                               // 180° clockwise display rotation (180° counter-clockwise device mounting)
    Terra_DisplayRotation_R3,                               // 270° clockwise display rotation  (270° counter-clockwise device mounting)
    Terra_DisplayRotation_HorzMirror,                       // Horizontally mirrored (iff supported, touchscreen tuning orientation pass-through w/o rotation)
    Terra_DisplayRotation_VertMirror,                       // Vertically mirrored (iff supported, touchscreen tuning orientation pass-through w/o rotation)

    Terra_DisplayRotation_Count,                            // Placeholder
    Terra_DisplayRotation_Undefined = -1                    // Placeholder
};

// Touchscreen Orientation
// Touchscreens can be attached differently than displays, so these allow finer touchscreen setup.
enum Terra_TouchscreenOrientation : signed char {
    Terra_TouchscreenOrientation_Same,                      // Apply same orientation as display rotation (converts display rotation to swapXY/invX/invY values)
    Terra_TouchscreenOrientation_Plus1,                     // Apply same orientation as display rotation + R1, %4->[R0,R3]
    Terra_TouchscreenOrientation_Plus2,                     // Apply same orientation as display rotation + R2, %4->[R0,R3]
    Terra_TouchscreenOrientation_Plus3,                     // Apply same orientation as display rotation + R3, %4->[R0,R3]
    Terra_TouchscreenOrientation_None,                      // No applied orientation (no invX, invY, or swapXY)
    Terra_TouchscreenOrientation_InvertX,                   // Only invert X axis (no invY or swapXY)
    Terra_TouchscreenOrientation_InvertY,                   // Only invert Y axis (no invX or swapXY)
    Terra_TouchscreenOrientation_InvertXY,                  // Invert X & Y axis (no swapXY)
    Terra_TouchscreenOrientation_SwapXY,                    // Only swap X/Y coordinates (aka transpose, no invX or invY)
    Terra_TouchscreenOrientation_InvertX_SwapXY,            // Invert X axis, then swap X/Y coordinates (no invY)
    Terra_TouchscreenOrientation_InvertY_SwapXY,            // Invert Y axis, then swap X/Y coordinates (no invX)
    Terra_TouchscreenOrientation_InvertXY_SwapXY,           // Invert X & Y axis, then swap X/Y coordinates

    Terra_TouchscreenOrientation_Count,                     // Placeholder
    Terra_TouchscreenOrientation_Undefined = -1             // Placeholder
};

// Display Theme
// General color theme and aesthetics.
enum Terra_DisplayTheme : signed char {
    Terra_DisplayTheme_CoolBlue_ML,                         // Cool blue theme for medium to large color displays (larger fonts/more padding)
    Terra_DisplayTheme_CoolBlue_SM,                         // Cool blue theme for small to medium color displays (smaller fonts/less padding)
    Terra_DisplayTheme_DarkMode_ML,                         // Dark mode theme for medium to large color displays (larger fonts/more padding)
    Terra_DisplayTheme_DarkMode_SM,                         // Dark mode theme for small to medium color displays (smaller fonts/less padding)
    Terra_DisplayTheme_MonoOLED,                            // Monochrome/OLED theme for small to medium monochrome displays, /w standard border
    Terra_DisplayTheme_MonoOLED_Inv,                        // Monochrome/OLED theme for small to medium monochrome displays, /w inverted colors

    Terra_DisplayTheme_Count,                               // Placeholder
    Terra_DisplayTheme_Undefined = -1                       // Placeholder
};

// Title Mode
// Way in which the title is shown in menu.
enum Terra_TitleMode : signed char {
    Terra_TitleMode_None,                                   // Title not displayed
    Terra_TitleMode_FirstRow,                               // Title only appears when first row selected (if supported)
    Terra_TitleMode_Always,                                 // Title is always displayed

    Terra_TitleMode_Count,                                  // Placeholder
    Terra_TitleMode_Undefined = -1                          // Placeholder
};

// ST77XX Device Kind
// Special device kind identifier for common ST7735 B/S/R color tags and common ST7789 screen resolutions.
enum Terra_ST77XXKind : signed char {
    Terra_ST7735Tag_B                   = (int8_t)0xff,
    Terra_ST7735Tag_Green               = (int8_t)0x00,
    Terra_ST7735Tag_Green18             = (int8_t)0x00,
    Terra_ST7735Tag_Red                 = (int8_t)0x01,
    Terra_ST7735Tag_Red18               = (int8_t)0x01,
    Terra_ST7735Tag_Black               = (int8_t)0x02,
    Terra_ST7735Tag_Black18             = (int8_t)0x02,
    Terra_ST7735Tag_Green144            = (int8_t)0x01,
    Terra_ST7735Tag_Mini                = (int8_t)0x04,
    Terra_ST7735Tag_MiniPlugin          = (int8_t)0x06,
    Terra_ST7735Tag_HalloWing           = (int8_t)0x05,

    Terra_ST7789Res_128x128             = (int8_t)0x10,
    Terra_ST7789Res_135x240,
    Terra_ST7789Res_170x320,
    Terra_ST7789Res_172x320,
    Terra_ST7789Res_240x240,
    Terra_ST7789Res_240x280,
    Terra_ST7789Res_240x320,
    Terra_ST7789Res_CustomTFT,

    Terra_ST77XXKind_Undefined          = (int8_t)0xff,
    Terra_ST7735Tag_Undefined           = (int8_t)0xff,
    Terra_ST7789Res_Undefined           = (int8_t)0xff,
    Terra_ST7789Res_Start               = Terra_ST7789Res_128x128
};

// Backlight Operation Mode
// How the backlight gets handled. Derived from LCD usage.
enum Terra_BacklightMode : signed char {
    Terra_BacklightMode_Normal,
    Terra_BacklightMode_Inverted,
    Terra_BacklightMode_PWM,

    Terra_BacklightMode_Count,
    Terra_BacklightMode_Undefined = -1
};

// Rotary Encoder Speed
// Essentially how far the rotary encoder must physically travel before the UI responds.
enum Terra_EncoderSpeed : signed char {
    Terra_EncoderSpeed_FullCycle,
    Terra_EncoderSpeed_HalfCycle,
    Terra_EncoderSpeed_QuarterCycle,

    Terra_EncoderSpeed_Count,
    Terra_EncoderSpeed_Undefined = -1
};

// ESP32 Touch Key High Reference Voltage
enum Terra_ESP32Touch_HighRef : signed char {
    Terra_ESP32Touch_HighRef_Keep,
    Terra_ESP32Touch_HighRef_V_2V4,
    Terra_ESP32Touch_HighRef_V_2V5,
    Terra_ESP32Touch_HighRef_V_2V6,
    Terra_ESP32Touch_HighRef_V_2V7,
    Terra_ESP32Touch_HighRef_Max,

    Terra_ESP32Touch_HighRef_Count,
    Terra_ESP32Touch_HighRef_Undefined = -1
};

// ESP32 Touch Key Low Reference Voltage
enum Terra_ESP32Touch_LowRef : signed char {
    Terra_ESP32Touch_LowRef_Keep,
    Terra_ESP32Touch_LowRef_V_0V5,
    Terra_ESP32Touch_LowRef_V_0V6,
    Terra_ESP32Touch_LowRef_V_0V7,
    Terra_ESP32Touch_LowRef_V_0V8,
    Terra_ESP32Touch_LowRef_Max,

    Terra_ESP32Touch_LowRef_Count,
    Terra_ESP32Touch_LowRef_Undefined = -1
};

// ESP32 Touch Key High Ref Volt Attenuation
enum Terra_ESP32Touch_HighRefAtten : signed char {
    Terra_ESP32Touch_HighRefAtten_Keep,
    Terra_ESP32Touch_HighRefAtten_V_1V5,
    Terra_ESP32Touch_HighRefAtten_V_1V,
    Terra_ESP32Touch_HighRefAtten_V_0V5,
    Terra_ESP32Touch_HighRefAtten_V_0V,
    Terra_ESP32Touch_HighRefAtten_Max,

    Terra_ESP32Touch_HighRefAtten_Count,
    Terra_ESP32Touch_HighRefAtten_Undefined = -1
};


class TerraduinoBaseUI;
class TerraDisplayDriver;
class TerraInputDriver;
class TerraRemoteControl;
class TerraMenu;
class TerraHomeMenu;
class TerraOverview;
struct TerraUIData;


// tcMenu Callbacks
#define CALLBACK_FUNCTION
#define NO_ADDRESS                      0xffff              // No EEPROM address
extern void CALLBACK_FUNCTION gotoScreen(int id);
extern void CALLBACK_FUNCTION debugAction(int id);

#endif // /ifndef TerraUIDefines_H
