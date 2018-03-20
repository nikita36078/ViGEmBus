/*  LilyPad - Pad plugin for PS2 Emulator
 *  Copyright (C) 2002-2014  PCSX2 Dev Team/ChickenLiver
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU Lesser General Public License as published by the Free
 *  Software Found- ation, either version 3 of the License, or (at your option)
 *  any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with PCSX2.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "DualShock3.h"

 // Unresponsive period required before calling DS3Check().
#define DEVICE_CHECK_DELAY 2000
// Unresponsive period required before calling DS3Enum().  Note that enum is always called on first check.
#define DEVICE_ENUM_DELAY 10000

// Delay between when DS3Check() and DS3Enum() actually do stuff.
#define DOUBLE_CHECK_DELAY 1000
#define DOUBLE_ENUM_DELAY 20000

// Send at least one message every 3 seconds - basically just makes sure the right light(s) are on.
// Not really necessary.
#define UPDATE_INTERVAL 300

unsigned int lastDS3Check = 0;
unsigned int lastDS3Enum = 0;

typedef void(__cdecl *_usb_init)(void);
typedef int(__cdecl *_usb_close)(usb_dev_handle *dev);
typedef int(__cdecl *_usb_get_string_simple)(usb_dev_handle *dev, int index, char *buf, size_t buflen);
typedef usb_dev_handle *(__cdecl *_usb_open)(struct usb_device *dev);
typedef int(__cdecl *_usb_find_busses)(void);
typedef int(__cdecl *_usb_find_devices)(void);
typedef struct usb_bus *(__cdecl *_usb_get_busses)(void);
typedef usb_dev_handle *(__cdecl *_usb_open)(struct usb_device *dev);
typedef int(__cdecl *_usb_control_msg)(usb_dev_handle *dev, int requesttype, int request, int value, int index, char *bytes, int size, int timeout);

_usb_init pusb_init;
_usb_close pusb_close;
_usb_get_string_simple pusb_get_string_simple;
_usb_open pusb_open;
_usb_find_busses pusb_find_busses;
_usb_find_devices pusb_find_devices;
_usb_get_busses pusb_get_busses;
_usb_control_msg pusb_control_msg;

HMODULE hModLibusb = 0;

void UninitLibUsb()
{
	if (hModLibusb) {
		FreeLibrary(hModLibusb);
		hModLibusb = 0;
	}
}

void TryInitDS3(usb_device *dev)
{
	while (dev) {
		if (dev->descriptor.idVendor == VID && dev->descriptor.idProduct == PID) {
			usb_dev_handle *handle = pusb_open(dev);
			if (handle) {
				char junk[20];
				// This looks like HidD_GetFeature with a feature report id of 0xF2 to me and a length of 17.
				// That doesn't work, however, and 17 is shorter than the report length.
				pusb_control_msg(handle, 0xa1, 1, 0x03f2, dev->config->interface->altsetting->bInterfaceNumber, junk, 17, 1000);
				pusb_close(handle);
			}
		}
		if (dev->num_children) {
			for (int i = 0; i < dev->num_children; i++) {
				TryInitDS3(dev->children[i]);
			}
		}
		dev = dev->next;
	}
}

void DS3Enum(unsigned int time)
{
	if (time - lastDS3Enum < DOUBLE_ENUM_DELAY) {
		return;
	}
	lastDS3Enum = time;
	pusb_find_busses();
	pusb_find_devices();
}

void DS3Check(unsigned int time)
{
	if (time - lastDS3Check < DOUBLE_CHECK_DELAY) {
		return;
	}
	if (!lastDS3Check) {
		DS3Enum(time);
	}
	lastDS3Check = time;

	usb_bus *bus = pusb_get_busses();
	while (bus) {
		TryInitDS3(bus->devices);
		bus = bus->next;
	}
}

int InitLibUsb()
{
	if (hModLibusb) {
		return 1;
	}
	hModLibusb = LoadLibraryA("C:\\windows\\system32\\libusb0.dll");
	if (hModLibusb) {
		if ((pusb_init = (_usb_init)GetProcAddress(hModLibusb, "usb_init")) &&
			(pusb_close = (_usb_close)GetProcAddress(hModLibusb, "usb_close")) &&
			(pusb_get_string_simple = (_usb_get_string_simple)GetProcAddress(hModLibusb, "usb_get_string_simple")) &&
			(pusb_open = (_usb_open)GetProcAddress(hModLibusb, "usb_open")) &&
			(pusb_find_busses = (_usb_find_busses)GetProcAddress(hModLibusb, "usb_find_busses")) &&
			(pusb_find_devices = (_usb_find_devices)GetProcAddress(hModLibusb, "usb_find_devices")) &&
			(pusb_get_busses = (_usb_get_busses)GetProcAddress(hModLibusb, "usb_get_busses")) &&
			(pusb_control_msg = (_usb_control_msg)GetProcAddress(hModLibusb, "usb_control_msg"))) {
			pusb_init();
			return 1;
		}
		UninitLibUsb();
	}
	return 0;
}

int DualShock3Possible()
{
	return InitLibUsb();
}

#include <pshpack1.h>
#include <poppack.h>

int DualShock3Device::StartRead()
{
	int res = ReadFile(hFile, &getState, sizeof(getState), 0, &readop);
	return (res || GetLastError() == ERROR_IO_PENDING);
}

void DualShock3Device::QueueWrite()
{
	// max of 2 queued writes allowed, one for either motor.
	if (writeQueued < 2) {
		writeQueued++;
		StartWrite();
	}
}

int DualShock3Device::StartWrite()
{
	if (!writing && writeQueued) {
		lastWrite = GetTickCount();
		writing++;
		writeQueued--;
		sendState.motors[0].duration = 0x50;
		sendState.motors[1].duration = 0x50;

		sendState.motors[1].force = (unsigned char)(vibration[0]);
		sendState.motors[0].force = (unsigned char)(vibration[1] ? 1 : 0);
		// Can't seem to have them both non-zero at once.
		if (sendState.motors[writeCount & 1].force) {
			sendState.motors[(writeCount & 1) ^ 1].force = 0;
			//sendState.motors[(writeCount & 1) ^ 1].duration = 0;
		}

		writeCount++;
		int res = WriteFile(hFile, &sendState, sizeof(sendState), 0, &writeop);
		return (res || GetLastError() == ERROR_IO_PENDING);
	}
	return 1;
}

DualShock3Device::DualShock3Device(int index, wchar_t *name, wchar_t *path)
{
	writeCount = 0;
	writing = 0;
	writeQueued = 0;
	memset(&readop, 0, sizeof(readop));
	memset(&writeop, 0, sizeof(writeop));
	memset(&sendState, 0, sizeof(sendState));
	sendState.id = 1;
	int temp = (index & 4);
	sendState.lightFlags = (1 << (temp + 1));
	sendState.lights[3 - temp].duration = 0xFF;
	sendState.lights[3 - temp].dunno[0] = 1;
	sendState.lights[3 - temp].on = 1;
	vibration[0] = vibration[1] = 0;
	this->index = index;
	hFile = INVALID_HANDLE_VALUE;
	instanceID = _wcsdup(path);
}

int DualShock3Device::Activate()
{
	if (active)
		Deactivate();
	// Give grace period before get mad.
	lastWrite = dataLastReceived = GetTickCount();
	readop.hEvent = CreateEvent(0, 0, 0, 0);
	writeop.hEvent = CreateEvent(0, 0, 0, 0);
	hFile = CreateFileW(instanceID, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
	if (!readop.hEvent || !writeop.hEvent || hFile == INVALID_HANDLE_VALUE ||
		!StartRead()) {
		Deactivate();
		return 0;
	}
	active = 1;
	return 1;
}

int DualShock3Device::Update()
{
	if (!active)
		return 0;
	HANDLE h[2] = {
		readop.hEvent,
		writeop.hEvent };
	unsigned int time = GetTickCount();
	if (time - lastWrite > UPDATE_INTERVAL) {
		QueueWrite();
	}
	while (1) {
		DWORD res = WaitForMultipleObjects(2, h, 0, 0);
		if (res == WAIT_OBJECT_0) {
			dataLastReceived = time;
			if (!StartRead()) {
				Deactivate();
				return 0;
			}
			continue;
		}
		else if (res == WAIT_OBJECT_0 + 1) {
			writing = 0;
			if (!writeQueued && (vibration[0] | vibration[1])) {
				QueueWrite();
			}
			if (!StartWrite()) {
				Deactivate();
				return 0;
			}
		}
		else {
			if (time - dataLastReceived >= DEVICE_CHECK_DELAY) {
				if (time - dataLastReceived >= DEVICE_ENUM_DELAY) {
					DS3Enum(time);
				}
				DS3Check(time);
				QueueWrite();
			}
		}
		break;
	}
	return 1;
}

void DualShock3Device::Deactivate()
{
	if (hFile != INVALID_HANDLE_VALUE) {
		CancelIo(hFile);
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	if (readop.hEvent) {
		CloseHandle(readop.hEvent);
	}
	if (writeop.hEvent) {
		CloseHandle(writeop.hEvent);
	}
	writing = 0;
	writeQueued = 0;
	vibration[0] = vibration[1] = 0;

	active = 0;
}

DualShock3Device::~DualShock3Device()
{
}
