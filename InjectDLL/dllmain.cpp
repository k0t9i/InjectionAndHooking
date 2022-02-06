// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "../InjectLibrary/Injector.h"
#include "../InjectLibrary/ProcessManipulation.h"

InjectLibrary::Injector injector("UniqMutexName", WH_CALLWNDPROC);

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    DWORD processId = GetCurrentProcessId();
    const std::string processName = InjectLibrary::GetProcessName(processId);
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        injector.SetHook(hModule);
        if (processName == "notepad.exe") {
            MessageBoxA(NULL, "Attached", "Injector", MB_OK);
        }
        break;
    case DLL_PROCESS_DETACH:
        injector.SetHook();
        if (processName == "notepad.exe") {
            MessageBoxA(NULL, "Detached", "Injector", MB_OK);
        }
        break;
    }
    return TRUE;
}

