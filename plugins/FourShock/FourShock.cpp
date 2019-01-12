#include <windows.h>
#include <iostream>

#include "ViGEmClient.h"

#include "DualShock3.h"
#include "HidDevice.h"

using namespace std;

//
// Helper object to keep track of state during every frame
// 
struct EmulationTarget
{
	// Is the target device (virtual controller) currently alive
	// 
	bool isTargetConnected;
	//
	// Device object referring to the emulation target
	// 
	PVIGEM_TARGET target;
};

static EmulationTarget targets[4];
DualShock3Device** devices;

int Connect(PVIGEM_CLIENT client);
int EnumDualShock3s();
VOID my_ds4_notification(
	PVIGEM_CLIENT Client,
	PVIGEM_TARGET Target,
	UCHAR LargeMotor,
	UCHAR SmallMotor,
	DS4_LIGHTBAR_COLOR LightbarColor);

int main()
{
	if (!DualShock3Possible()) {
		cout << "Error" << endl;
		return 0;
	}
	cout << "Waiting for connection..." << endl;

	//
	// Alloc ViGEm client
	// 
	PVIGEM_CLIENT client = vigem_alloc();
	VIGEM_ERROR retval = vigem_connect(client);
	if (!VIGEM_SUCCESS(retval))
	{
		cout << "ViGEm Bus connection failed" << endl;
		return 0;
	}

	while (!(GetKeyState('A') & 0x8000)) {
		Connect(client);
		Sleep(3000);
	}

	vigem_disconnect(client);
	vigem_free(client);
	cout << "Program finished" << endl;
	system("pause");
	return 0;
}

VOID my_ds4_notification(
	PVIGEM_CLIENT Client,
	PVIGEM_TARGET Target,
	UCHAR LargeMotor,
	UCHAR SmallMotor,
	DS4_LIGHTBAR_COLOR LightbarColor)
{
	int i = 0;
	devices[i]->vibration[1] = SmallMotor;
	devices[i]->vibration[0] = LargeMotor;
	Sleep(1);
}

int Connect(PVIGEM_CLIENT client) {
	if (EnumDualShock3s() == 0) {
		return 0;
	}

	// Select 1st gamepad
	int i = 0;

	if (targets[i].isTargetConnected) {
		vigem_target_remove(client, targets[i].target);
		vigem_target_free(targets[i].target);
	}
	targets[i].target = vigem_target_ds4_alloc();
	auto pir = vigem_target_add(client, targets[i].target);
	if (!VIGEM_SUCCESS(pir))
	{
		cout << L"Target plugin failed" << endl;
		return 0;
	}
	targets[i].isTargetConnected = vigem_target_is_attached(targets[i].target);

	if (targets[i].isTargetConnected)
	{
		cout << "ViGEm device connected" << endl;
		DS4_REPORT rep;

		devices[i]->Activate();
		vigem_target_ds4_register_notification(client, targets[i].target, (PVIGEM_DS4_NOTIFICATION)my_ds4_notification);

		while (devices[i]->Update() && !(GetKeyState('A') & 0x8000)) {
			DS4_REPORT_INIT(&rep);
			USHORT wNewButtons = 0;
			unsigned char* state = devices[i]->getState;
			BYTE buttons1 = state[2] & 15;
			if (buttons1 & 1) wNewButtons |= DS4_BUTTON_SHARE;
			if (buttons1 & 2) wNewButtons |= DS4_BUTTON_THUMB_LEFT;
			if (buttons1 & 4) wNewButtons |= DS4_BUTTON_THUMB_RIGHT;
			if (buttons1 & 8) wNewButtons |= DS4_BUTTON_OPTIONS;
			BYTE buttons2 = state[3];
			if (buttons2 & 1) wNewButtons |= DS4_BUTTON_TRIGGER_LEFT;
			if (buttons2 & 2) wNewButtons |= DS4_BUTTON_TRIGGER_RIGHT;
			if (buttons2 & 4) wNewButtons |= DS4_BUTTON_SHOULDER_LEFT;
			if (buttons2 & 8) wNewButtons |= DS4_BUTTON_SHOULDER_RIGHT;
			if (buttons2 & 16) wNewButtons |= DS4_BUTTON_TRIANGLE;
			if (buttons2 & 32) wNewButtons |= DS4_BUTTON_CIRCLE;
			if (buttons2 & 64) wNewButtons |= DS4_BUTTON_CROSS;
			if (buttons2 & 128) wNewButtons |= DS4_BUTTON_SQUARE;

			rep.wButtons = wNewButtons;

			if (state[4] & 1) rep.bSpecial |= DS4_SPECIAL_BUTTON_PS;
			rep.bThumbLX = state[6];
			rep.bThumbLY = state[7];
			rep.bThumbRX = state[8];
			rep.bThumbRY = state[9];
			rep.bTriggerL = state[18];
			rep.bTriggerR = state[19];

			BYTE directions = state[2] & 240;
			if (directions == 0) DS4_SET_DPAD(&rep, DS4_BUTTON_DPAD_NONE);
			else if (directions & 16) DS4_SET_DPAD(&rep, DS4_BUTTON_DPAD_NORTH);
			else if (directions & 32) DS4_SET_DPAD(&rep, DS4_BUTTON_DPAD_EAST);
			else if (directions & 64) DS4_SET_DPAD(&rep, DS4_BUTTON_DPAD_SOUTH);
			else if (directions & 128) DS4_SET_DPAD(&rep, DS4_BUTTON_DPAD_WEST);
			else if (directions & 16 && directions & 32) DS4_SET_DPAD(&rep, DS4_BUTTON_DPAD_NORTHEAST);
			else if (directions & 16 && directions & 128) DS4_SET_DPAD(&rep, DS4_BUTTON_DPAD_NORTHWEST);
			else if (directions & 64 && directions & 32) DS4_SET_DPAD(&rep, DS4_BUTTON_DPAD_SOUTHEAST);
			else if (directions & 64 && directions & 128) DS4_SET_DPAD(&rep, DS4_BUTTON_DPAD_SOUTHWEST);

			short accelX = -(state[41] << 8 | state[42]);
			//if (accelX < -540 && accelX > -570) accelX = -550;
			accelX += 550;
			accelX *= 130;
			short accelY = state[43] << 8 | state[44];
			//if (accelY > 660 && accelY < 690) accelY = 670;
			accelY -= 670;
			accelY *= 130;
			short accelZ = state[45] << 8 | state[46];
			//if (accelZ > 360 && accelZ < 380) accelZ = 370;
			accelZ -= 370;
			accelZ *= 150;

			rep.bAccelX1 = accelX >> 8;
			rep.bAccelX2 = accelX & 0xFF;
			rep.bAccelY1 = accelY >> 8;
			rep.bAccelY2 = accelY & 0xFF;
			rep.bAccelZ1 = accelZ >> 8;
			rep.bAccelZ2 = accelZ & 0xFF;
			rep.bGyroX1 = accelY >> 8;
			rep.bGyroX2 = accelY & 0xFF;
			rep.bGyroY1 = accelX >> 8;
			rep.bGyroY2 = accelX & 0xFF;
			rep.bGyroZ1 = accelZ >> 8;
			rep.bGyroZ2 = accelZ & 0xFF;

			vigem_target_ds4_update(client, targets[i].target, rep);
			Sleep(1);
		}
	}
	devices[i]->Deactivate();
	cout << "ViGEm device disconnected" << endl;
	return 0;
}

int EnumDualShock3s()
{
	int numTargets = 0;
	HidDeviceInfo *foundDevs = 0;

	int numDevs = FindHids(&foundDevs, VID, PID);
	if (numDevs) {
		for (int i = 0; i < numDevs; i++) {
			if (foundDevs[i].caps.FeatureReportByteLength == 49 &&
				foundDevs[i].caps.InputReportByteLength == 49 &&
				foundDevs[i].caps.OutputReportByteLength == 49) {
				wchar_t temp[100];
				wsprintfW(temp, L"DualShock 3 #%i", numTargets + 1);
				devices = (DualShock3Device **)realloc(devices, sizeof(DualShock3Device *) * (numTargets + 1));
				devices[numTargets++] = new DualShock3Device(numTargets, temp, foundDevs[i].path);
			}
		}
		free(foundDevs);
	}
	return numTargets;
}
