/*
MIT License

Copyright (c) 2017 Benjamin "Nefarius" H�glinger

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#pragma once

//
// Represents the desired target type for the emulated device.
//  
typedef enum _VIGEM_TARGET_TYPE
{
    // 
    // Microsoft Xbox 360 Controller (wired)
    // 
    Xbox360Wired,
    // 
    // Microsoft Xbox One Controller (wired)
    // 
    XboxOneWired,
    //
    // Sony DualShock 4 (wired)
    // 
    DualShock4Wired

} VIGEM_TARGET_TYPE, *PVIGEM_TARGET_TYPE;

//
// Possible XUSB report buttons.
// 
typedef enum _XUSB_BUTTON
{
    XUSB_GAMEPAD_DPAD_UP            = 0x0001,
    XUSB_GAMEPAD_DPAD_DOWN          = 0x0002,
    XUSB_GAMEPAD_DPAD_LEFT          = 0x0004,
    XUSB_GAMEPAD_DPAD_RIGHT         = 0x0008,
    XUSB_GAMEPAD_START              = 0x0010,
    XUSB_GAMEPAD_BACK               = 0x0020,
    XUSB_GAMEPAD_LEFT_THUMB         = 0x0040,
    XUSB_GAMEPAD_RIGHT_THUMB        = 0x0080,
    XUSB_GAMEPAD_LEFT_SHOULDER      = 0x0100,
    XUSB_GAMEPAD_RIGHT_SHOULDER     = 0x0200,
    XUSB_GAMEPAD_GUIDE              = 0x0400,
    XUSB_GAMEPAD_A                  = 0x1000,
    XUSB_GAMEPAD_B                  = 0x2000,
    XUSB_GAMEPAD_X                  = 0x4000,
    XUSB_GAMEPAD_Y                  = 0x8000

} XUSB_BUTTON, *PXUSB_BUTTON;

//
// Represents an XINPUT_GAMEPAD-compatible report structure.
// 
typedef struct _XUSB_REPORT
{
    USHORT wButtons;
    BYTE bLeftTrigger;
    BYTE bRightTrigger;
    SHORT sThumbLX;
    SHORT sThumbLY;
    SHORT sThumbRX;
    SHORT sThumbRY;

} XUSB_REPORT, *PXUSB_REPORT;

//
// Initializes a _XUSB_REPORT structure.
// 
VOID FORCEINLINE XUSB_REPORT_INIT(
    _Out_ PXUSB_REPORT Report
)
{
    RtlZeroMemory(Report, sizeof(XUSB_REPORT));
}

//
// The color value (RGB) of a DualShock 4 Lightbar
// 
typedef struct _DS4_LIGHTBAR_COLOR
{
    //
    // Red part of the Lightbar (0-255).
    //
    UCHAR Red;

    //
    // Green part of the Lightbar (0-255).
    //
    UCHAR Green;

    //
    // Blue part of the Lightbar (0-255).
    //
    UCHAR Blue;

} DS4_LIGHTBAR_COLOR, *PDS4_LIGHTBAR_COLOR;

//
// DualShock 4 digital buttons
// 
typedef enum _DS4_BUTTONS
{
    DS4_BUTTON_THUMB_RIGHT      = 1 << 15,
    DS4_BUTTON_THUMB_LEFT       = 1 << 14,
    DS4_BUTTON_OPTIONS          = 1 << 13,
    DS4_BUTTON_SHARE            = 1 << 12,
    DS4_BUTTON_TRIGGER_RIGHT    = 1 << 11,
    DS4_BUTTON_TRIGGER_LEFT     = 1 << 10,
    DS4_BUTTON_SHOULDER_RIGHT   = 1 << 9,
    DS4_BUTTON_SHOULDER_LEFT    = 1 << 8,
    DS4_BUTTON_TRIANGLE         = 1 << 7,
    DS4_BUTTON_CIRCLE           = 1 << 6,
    DS4_BUTTON_CROSS            = 1 << 5,
    DS4_BUTTON_SQUARE           = 1 << 4

} DS4_BUTTONS, *PDS4_BUTTONS;

//
// DualShock 4 special buttons
// 
typedef enum _DS4_SPECIAL_BUTTONS
{
    DS4_SPECIAL_BUTTON_PS           = 1 << 0,
    DS4_SPECIAL_BUTTON_TOUCHPAD     = 1 << 1

} DS4_SPECIAL_BUTTONS, *PDS4_SPECIAL_BUTTONS;

//
// DualShock 4 directional pad (HAT) values
// 
typedef enum _DS4_DPAD_DIRECTIONS
{
    DS4_BUTTON_DPAD_NONE        = 0x8,
    DS4_BUTTON_DPAD_NORTHWEST   = 0x7,
    DS4_BUTTON_DPAD_WEST        = 0x6,
    DS4_BUTTON_DPAD_SOUTHWEST   = 0x5,
    DS4_BUTTON_DPAD_SOUTH       = 0x4,
    DS4_BUTTON_DPAD_SOUTHEAST   = 0x3,
    DS4_BUTTON_DPAD_EAST        = 0x2,
    DS4_BUTTON_DPAD_NORTHEAST   = 0x1,
    DS4_BUTTON_DPAD_NORTH       = 0x0

} DS4_DPAD_DIRECTIONS, *PDS4_DPAD_DIRECTIONS;

//
// DualShock 4 HID Input report
// 
typedef struct _DS4_REPORT
{
    BYTE bThumbLX;
    BYTE bThumbLY;
    BYTE bThumbRX;
    BYTE bThumbRY;
    USHORT wButtons;
    BYTE bSpecial;
    BYTE bTriggerL;
    BYTE bTriggerR;
    BYTE unknown;
    BYTE unknown2;
    BYTE unknown3;
    BYTE unknown4;
    BYTE bGyroX1;
    BYTE bGyroX2;
    BYTE bGyroY1;
    BYTE bGyroY2;
    BYTE bGyroZ1;
    BYTE bGyroZ2;
    BYTE bAccelX1;
    BYTE bAccelX2;
    BYTE bAccelY1;
    BYTE bAccelY2;
    BYTE bAccelZ1;
    BYTE bAccelZ2;

} DS4_REPORT, *PDS4_REPORT;

//
// Sets the current state of the D-PAD on a DualShock 4 report.
// 
VOID FORCEINLINE DS4_SET_DPAD(
    _Out_ PDS4_REPORT Report,
    _In_ DS4_DPAD_DIRECTIONS Dpad
)
{
    Report->wButtons &= ~0xF;
    Report->wButtons |= (USHORT)Dpad;
}

VOID FORCEINLINE DS4_REPORT_INIT(
    _Out_ PDS4_REPORT Report
)
{
    RtlZeroMemory(Report, sizeof(DS4_REPORT));

    Report->bThumbLX = 0x80;
    Report->bThumbLY = 0x80;
    Report->bThumbRX = 0x80;
    Report->bThumbRY = 0x80;

    DS4_SET_DPAD(Report, DS4_BUTTON_DPAD_NONE);
}

