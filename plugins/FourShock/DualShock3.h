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

#include "Global.h"
#include "usb.h"

#define VID 0x054c
#define PID 0x0268

int DualShock3Possible();

struct MotorState
{
	unsigned char duration;
	unsigned char force;
};

struct LightState
{
	// 0xFF makes it stay on.
	unsigned char duration;
	// Have to make one or the other non-zero to turn on light.
	unsigned char dunno[2];
	// 0 is fully lit.
	unsigned char dimness;
	// Have to make non-zero to turn on light.
	unsigned char on;
};

// Data sent to DS3 to set state.
struct DS3Command
{
	unsigned char id;
	unsigned char unsure;
	// Small is first, then big. Small is (0-1), big is (0-255)
	MotorState motors[2];
	unsigned char noClue[4];
	// 2 is pad 1 light, 4 is pad 2, 8 is pad 3, 16 is pad 4.  No clue about the others.
	unsigned char lightFlags;
	// Lights are in reverse order.  pad 1 is last.
	LightState lights[4];
	unsigned char dunno[18];
};

class DualShock3Device
{
	int index;
	HANDLE hFile;
	OVERLAPPED readop;
	OVERLAPPED writeop;
	int writeCount;
	int lastWrite;

	unsigned int dataLastReceived;

	int writeQueued;
	int writing;

	int active = 0;
	wchar_t *instanceID = 0;

	int StartRead();
	void QueueWrite();
	int StartWrite();
	~DualShock3Device();

public:
	// Cached last vibration values by pad and motor.
	// Need this, as only one value is changed at a time.
	int vibration[2];
	DS3Command sendState;
	unsigned char getState[49];

	DualShock3Device(int index, wchar_t * name, wchar_t * path);
	int Activate();
	int Update();
	void Deactivate();

};
