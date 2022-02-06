#pragma once

#include "pch.h"

namespace InjectLibrary
{
	void StartProcess(const DWORD processId);
	void StopProcess(const DWORD processId);
	const std::string GetProcessName(const DWORD processId);
};
