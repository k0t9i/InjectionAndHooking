// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "../InjectLibrary/Injector.h"
#include "../InjectLibrary/DllTrampolineInstaller.h"
#include "../InjectLibrary/ProcessManipulation.h"

InjectLibrary::Injector injector("UniqMutexName", WH_CALLWNDPROC);
InjectLibrary::DllTrampolineInstaller installer;
HWND hMainWnd = nullptr;
const UINT_PTR MENU_ID = 999;

typedef HWND(__stdcall* CreateWindowExType)(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
CreateWindowExType createWindowExTrampoline = nullptr;
typedef LRESULT(__stdcall* DispatchMessageType)(const MSG* lpMsg);
DispatchMessageType dispatchMessageTrampoline = nullptr;

HWND __stdcall CreateWindowExHookPayload(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
    HWND hwnd = (*createWindowExTrampoline)(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    const std::wstring className = std::wstring(lpClassName);
    if (!hMainWnd && className == L"Notepad") {
        hMainWnd = hwnd;
        HMENU hMenu = GetMenu(hMainWnd);

        AppendMenuA(hMenu, MF_STRING, MENU_ID, "О перехвате API");
    }
    return hwnd;
}

LRESULT __stdcall DispatchMessageHookPayload(const MSG* lpMsg)
{
    if (hMainWnd && hMainWnd == lpMsg->hwnd) {
        if (lpMsg->message == WM_COMMAND) {
            if (LOWORD(lpMsg->wParam) == MENU_ID) {
                MessageBoxExA(hMainWnd, "Это диалоговое окно было вызвано из пункта меню, добавленного на лету", "Диалог из внедренного кода", MB_OK, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
            }
        }
    }
    return(*dispatchMessageTrampoline)(lpMsg);
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    const DWORD processId = GetCurrentProcessId();
    const std::string processName = InjectLibrary::GetProcessName(processId);
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        injector.SetHook(hModule);
        if (processName == "notepad.exe") {
            InjectLibrary::StopProcess(processId);
            try {
                (FARPROC&)createWindowExTrampoline = installer.InstallTrampoline("user32.dll", "CreateWindowExW", CreateWindowExHookPayload);
                (FARPROC&)dispatchMessageTrampoline = installer.InstallTrampoline("user32.dll", "DispatchMessageW", DispatchMessageHookPayload);
            }
            catch (const std::exception& e) {
                OutputDebugStringA(e.what());
                injector.SetHook();
            }
            InjectLibrary::StartProcess(processId);
        }
        break;
    case DLL_PROCESS_DETACH:
        if (processName == "notepad.exe") {
            InjectLibrary::StopProcess(processId);
            try {
                installer.UninstallTrampoline("user32.dll", "DispatchMessageW");
                installer.UninstallTrampoline("user32.dll", "CreateWindowExW");
            }
            catch (const std::exception& e) {
                OutputDebugStringA(e.what());
            }
            if (hMainWnd) {
                HMENU hMenu = GetMenu(hMainWnd);
                DeleteMenu(hMenu, MENU_ID, MF_BYCOMMAND);
                DrawMenuBar(hMainWnd);
            }
            InjectLibrary::StartProcess(processId);
        }
        injector.SetHook();
        break;
    }
    return TRUE;
}

