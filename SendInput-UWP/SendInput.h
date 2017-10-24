#pragma once

#include <Windows.h>

#if defined(WINAPI_FAMILY) && WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP
#define MS_UWP 

typedef struct tagHARDWAREINPUT {
	DWORD   uMsg;
	WORD    wParamL;
	WORD    wParamH;
} HARDWAREINPUT, *PHARDWAREINPUT, FAR* LPHARDWAREINPUT;

#define INPUT_MOUSE     0
#define INPUT_KEYBOARD  1
#define INPUT_HARDWARE  2

typedef struct tagINPUT {
	DWORD   type;

	union
	{
		MOUSEINPUT      mi;
		KEYBDINPUT      ki;
		HARDWAREINPUT   hi;
	} DUMMYUNIONNAME;
} INPUT, *PINPUT, FAR* LPINPUT;

#endif

#define DLL_API __declspec(dllexport) 

extern "C" {
#ifdef MS_UWP
	DLL_API bool Initialize();
    DLL_API bool SendInput(int numInputs, INPUT* inputs, int cbSize);
	DLL_API short GetKeyState(unsigned short virtualKeyCode);
	DLL_API short GetAsyncKeyState(unsigned short virtualKeyCode);
#endif

}