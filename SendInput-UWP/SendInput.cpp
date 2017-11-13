﻿#include "SendInput.h"
#include <Windows.h>



#ifdef MS_UWP
#include "../MRAppService/MRAppServiceListener.h"
#include <ppltasks.h>
#include <string>

using namespace Concurrency;
using namespace MRAppService;
using namespace Windows::ApplicationModel::AppService;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::System;

static MRAppServiceListener^ s_appServiceListener = nullptr;

Concurrency::task<AppServiceConnectionStatus> ConnectToAppService(const std::wstring& id);
Concurrency::task<bool> LaunchWin32App();
#endif

extern "C" {

#ifdef MS_UWP
	DLL_API short GetKeyState(unsigned short virtualKeyCode)
	{
		return 0;
	}

	DLL_API short GetAsyncKeyState(unsigned short virtualKeyCode)
	{
		return 0;
	}

    DLL_API bool SendMouseInputUWP(MOUSEINPUT* input)
    {
        if (s_appServiceListener && s_appServiceListener->IsConnected())
        {
            ValueSet^ message = ref new ValueSet;
            message->Insert(L"MOUSEINPUT", true);
            message->Insert(L"dx", static_cast<int>(input->dx));
            message->Insert(L"dy", static_cast<int>(input->dy));
            message->Insert(L"mouseData", static_cast<unsigned int>(input->mouseData));
            message->Insert(L"dwFlags", static_cast<unsigned int>(input->dwFlags));
            message->Insert(L"time", static_cast<unsigned int>(input->time));
            s_appServiceListener->SendAppServiceMessage(L"Win32-App", message);
            return true;
        }

        return false;
    }

    DLL_API bool SendKeyboardInputUWP(KEYBDINPUT* input)
    {
        if (s_appServiceListener && s_appServiceListener->IsConnected())
        {
            ValueSet^ message = ref new ValueSet;
            message->Insert(L"KEYBDINPUT", true);
            message->Insert(L"wVk", static_cast<int>(input->wVk));
            message->Insert(L"wScan", static_cast<int>(input->wScan));
            message->Insert(L"dwFlags", static_cast<unsigned int>(input->dwFlags));
            message->Insert(L"time", static_cast<unsigned int>(input->time));
            s_appServiceListener->SendAppServiceMessage(L"Win32-App", message);
            return true;
        }
        return false;
    }

	DLL_API bool SendHardwareInputUWP(HARDWAREINPUT* input)
	{
		if (s_appServiceListener && s_appServiceListener->IsConnected())
		{
			ValueSet^ message = ref new ValueSet;
			message->Insert(L"HARDWAREINPUT", true);
			message->Insert(L"uMsg", static_cast<unsigned int>(input->uMsg));
			message->Insert(L"wParamL", static_cast<int>(input->wParamL));
			message->Insert(L"wParamH", static_cast<int>(input->wParamH));
            s_appServiceListener->SendAppServiceMessage(L"Win32-App", message);
            return true;
		}
		return false;
	}


	bool DoSendInput(int numInputs, INPUT* inputs, int cbSize)
	{
		bool result = true;

		for (int i = 0; i < numInputs; ++i)
		{
			auto input = inputs[i];
			switch (input.type)
			{
			case INPUT_MOUSE:
				SendMouseInputUWP(&input.mi);
				break;

			case INPUT_KEYBOARD:
				SendKeyboardInputUWP(&input.ki);
				break;

			case INPUT_HARDWARE:
				SendHardwareInputUWP(&input.hi);
				break;

			default:
				result = false;
				break;
			}
		}

		return result;
	}

	DLL_API bool SendInput(int numInputs, INPUT* inputs, int cbSize)
	{
		bool result = true;

		if (s_appServiceListener == nullptr)
		{
			auto task = ConnectToAppService(L"UWP-App");
			try
			{
				task.get(); // blocks until Connection task completes

				if (s_appServiceListener != nullptr && s_appServiceListener->IsConnected())
				{
					result = DoSendInput(numInputs, inputs, cbSize);
				}
			}
			catch (Platform::Exception^ ex)
			{
				OutputDebugString(ex->Message->Data());
			}

		}
		else
		{
			result = DoSendInput(numInputs, inputs, cbSize);
		}

		return result;
}
#else
    DLL_API bool SendInputWin32(INPUT* input, int numInputs)
    {
        auto count = SendInput(numInputs, input, sizeof(input));
        return count = numInputs;
    }
#endif
}

#ifdef MS_UWP
Concurrency::task<AppServiceConnectionStatus> ConnectToAppService(const std::wstring& id)
{
    Platform::String^ listenId = ref new Platform::String(id.c_str());
    s_appServiceListener = ref new MRAppServiceListener(listenId);
    return s_appServiceListener->ConnectToAppService(MRAPPSERVICE_ID, MRAppService::MRAppServiceListener::GetPackageFamilyName());
}

Concurrency::task<bool> LaunchWin32App()
{
    // Launch the Win32 App that will support SendInput for UWP apps
    auto uri = ref new Uri("sendinput-win32:"); // The protocol handled by the launched app
    auto options = ref new LauncherOptions();
    return create_task(Launcher::LaunchUriAsync(uri, options));
}

#endif


