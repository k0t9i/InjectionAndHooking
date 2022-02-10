// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "../InjectLibrary/Injector.h"
#include "../InjectLibrary/DllTrampolineInstaller.h"
#include "../InjectLibrary/ProcessManipulation.h"
#include "../InjectLibrary/LengthDisassembler.h"

InjectLibrary::LengthDisassembler lengthDisassembler;
InjectLibrary::Injector injector("UniqMutexName", WH_CALLWNDPROC);
InjectLibrary::DllTrampolineInstaller installer(&lengthDisassembler, InjectLibrary::SIZE_OF_JUMP);
HWND hMainWnd = nullptr;
HWND hEdit = nullptr;
HMENU hSubmenu = nullptr;
const UINT_PTR START_MENU_ID = 999;
HMODULE hDll = nullptr;

typedef HWND(__stdcall* CreateWindowExType)(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
CreateWindowExType createWindowExTrampoline = nullptr;
typedef LRESULT(__stdcall* DispatchMessageType)(const MSG* lpMsg);
DispatchMessageType dispatchMessageTrampoline = nullptr;

DWORD __stdcall ExitAppThread(LPVOID lpParam)
{
    FreeLibraryAndExitThread(hDll, 0);
    return 0;
}

void AddMenuItems(HWND hWnd)
{
    hSubmenu = CreateMenu();
    HMENU hMenu = GetMenu(hWnd);

    AppendMenuA(hMenu, MF_POPUP, UINT_PTR(hSubmenu), "Дополнительно");
    AppendMenuA(hSubmenu, MF_STRING, START_MENU_ID, "ВЕРХНИЙ РЕГИСТР");
    AppendMenuA(hSubmenu, MF_STRING, START_MENU_ID + 1, "нижний регистр");
    AppendMenuA(hSubmenu, MF_STRING, START_MENU_ID + 2, "иНВЕРТИРОВАТЬ РЕГИСТР");
    AppendMenuA(hSubmenu, MF_SEPARATOR, 0, NULL);
    AppendMenuA(hSubmenu, MF_STRING, START_MENU_ID + 3, "Выгрузить плагин");
}

HWND __stdcall CreateWindowExHookPayload(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
    HWND hWnd = (*createWindowExTrampoline)(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    const std::wstring className = std::wstring(lpClassName);
    if (!hMainWnd && className == L"Notepad") {
        hMainWnd = hWnd;
        AddMenuItems(hMainWnd);
    }
    if (!hEdit && className == L"Edit") {
        hEdit = hWnd;
    }
    return hWnd;
}

LRESULT __stdcall DispatchMessageHookPayload(const MSG* lpMsg)
{
    if (hMainWnd && hMainWnd == lpMsg->hwnd) {
        switch (lpMsg->message) {
        case WM_COMMAND:
            WORD menuId = LOWORD(lpMsg->wParam);
            if (menuId >= START_MENU_ID && menuId < START_MENU_ID + 3) {
                DWORD selStart = 0;
                DWORD selEnd = 0;
                SendMessageA(hEdit, EM_GETSEL, WPARAM(&selStart), LPARAM(&selEnd));
                if (selStart < selEnd) {
                    const DWORD len = selEnd + 1; // zero-terminated
                    WCHAR* buf = new WCHAR[len];
                    SendMessageW(hEdit, WM_GETTEXT, (WPARAM)len, (LPARAM)buf);
                    std::wstring text = std::wstring(buf).substr(selStart, len);
                    delete[] buf;

                    switch (menuId) {
                    case START_MENU_ID:
                        std::transform(text.begin(), text.end(), text.begin(), ::towupper);
                        break;
                    case START_MENU_ID + 1:
                        std::transform(text.begin(), text.end(), text.begin(), ::towlower);
                        break;
                    case START_MENU_ID + 2:
                        std::transform(text.begin(), text.end(), text.begin(), [](wchar_t c) { return ::iswlower(c) ? ::towupper(c) : ::towlower(c) ; });
                        break;
                    }

                    SendMessageW(hEdit, EM_REPLACESEL, WPARAM(1), LPARAM(text.c_str()));
                }
            }
            else if (menuId == START_MENU_ID + 3) {
                CreateThread(NULL, 0, ExitAppThread, NULL, 0, 0);
            }
            break;
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
            hDll = hModule;

            //Увеличим количество ссылок на библиотеку, чтобы она не выгрузилась когда закроется приложение по установке хука
            char libName[MAX_PATH];
            GetModuleFileNameA(hModule, libName, MAX_PATH);
            LoadLibraryA(libName);
            injector.SetHook();

            setlocale(LC_ALL, "");

            InjectLibrary::StopProcess(processId);
            try {
                (FARPROC&)createWindowExTrampoline = installer.InstallTrampoline("user32.dll", "CreateWindowExW", CreateWindowExHookPayload);
                (FARPROC&)dispatchMessageTrampoline = installer.InstallTrampoline("user32.dll", "DispatchMessageW", DispatchMessageHookPayload);
            }
            catch (const std::exception& e) {
                OutputDebugStringA(e.what());
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
                DeleteMenu(hMenu, UINT_PTR(hSubmenu), MF_BYCOMMAND);
                DrawMenuBar(hMainWnd);
            }
            InjectLibrary::StartProcess(processId);
        }
        injector.SetHook();
        break;
    }
    return TRUE;
}

