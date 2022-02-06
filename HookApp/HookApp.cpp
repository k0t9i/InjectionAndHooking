// HookApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>

int main()
{
    HMODULE hDll = LoadLibraryA("NotepadPluginDll.dll");
    if (!hDll) {
        return 1;
    }
    while (!std::cin.get()) {

    }
    FreeLibrary(hDll);
}
