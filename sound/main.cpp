#pragma comment(lib,"winmm.lib")

#include <windows.h>
#include <mmsystem.h>
#include <string>

NOTIFYICONDATA Icon = { 0 };
HWND window;
LRESULT CALLBACK SoftwareMainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
HHOOK keyboardHook;
HHOOK hMouseHook;
char array[3]{ 0x31, 0x32, 0x33 };
short indexOfArray = 0;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdsh)
{
	MSG SoftwareMainMessage = { 0 };

	WNDCLASSEX main = { 0 };
	main.cbSize = sizeof(WNDCLASSEX);
	main.hInstance = hInst;
	main.lpszClassName = TEXT("Main");
	main.lpfnWndProc = SoftwareMainProcedure;
	RegisterClassEx(&main);

	window = CreateWindowEx(0, TEXT("Main"), NULL, 0, 0, 0, 0, 0, NULL, NULL, hInst, NULL);

	Icon.cbSize = sizeof(NOTIFYICONDATA);
	Icon.hWnd = window;
	Icon.uVersion = NOTIFYICON_VERSION;
	Icon.uCallbackMessage = WM_USER;
	Icon.hIcon = (HICON)LoadImage(NULL, TEXT("golds.ico"), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	Icon.uFlags = NIF_MESSAGE | NIF_ICON;
	Shell_NotifyIcon(NIM_ADD, &Icon);

	hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);
	keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);

	while (GetMessage(&SoftwareMainMessage, NULL, NULL, NULL))
	{
		TranslateMessage(&SoftwareMainMessage);
		DispatchMessage(&SoftwareMainMessage);
	}
	return 0;
}


LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	MOUSEHOOKSTRUCT* pMouseStruct = (MOUSEHOOKSTRUCT*)lParam;

	if (pMouseStruct != NULL)
	{
		if (wParam == WM_MOUSEWHEEL)
		{
			bool down = false;
			MSLLHOOKSTRUCT* info = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
			down = static_cast<std::make_signed_t<WORD>>(HIWORD(info->mouseData)) < 0;
			INPUT inputs[1];
			UINT ret;

			inputs[0].type = INPUT_KEYBOARD;
			inputs[0].ki.wScan = 0;
			inputs[0].ki.dwFlags = 0;
			inputs[0].ki.time = 0;
			inputs[0].ki.dwExtraInfo = 0;

			if (down) 
			{
				--indexOfArray;

				if (indexOfArray == -1)
					indexOfArray = 2;

				inputs[0].ki.wVk = array[indexOfArray];
				ret = SendInput(1, inputs, sizeof(INPUT));
			}
			else
			{
				++indexOfArray;

				if (indexOfArray == 3)
					indexOfArray = 0;

				inputs[0].ki.wVk = array[indexOfArray];
				ret = SendInput(1, inputs, sizeof(INPUT));
			}
		}
	}
	return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) 
{
	if (nCode >= 0 && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN))
	{
		KBDLLHOOKSTRUCT* pKeyStruct = (KBDLLHOOKSTRUCT*)lParam;

		if (pKeyStruct->vkCode == 0x31)
			indexOfArray = 0;
		if (pKeyStruct->vkCode == 0x32)
			indexOfArray = 1;
		if(pKeyStruct->vkCode == 0x33)
			indexOfArray = 2;
	}

	return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

LRESULT CALLBACK SoftwareMainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_CREATE:
		break;

	case WM_USER:
		if (lp == WM_RBUTTONDOWN)
			if (MessageBox(NULL, TEXT("Delete process?"), TEXT("Tray"), MB_YESNO) == IDYES)
				DestroyWindow(window);
		break;

	case WM_DESTROY:
		Shell_NotifyIcon(NIM_DELETE, &Icon);
		PostQuitMessage(0);
		break;

	default: return DefWindowProc(hWnd, msg, wp, lp);
	}
}
