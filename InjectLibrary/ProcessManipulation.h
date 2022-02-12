#pragma once

#include <windows.h>
#include <string>

namespace InjectLibrary
{
	void StartProcess(const DWORD processId);
	void StopProcess(const DWORD processId);
	const std::string GetProcessName(const DWORD processId);
};
