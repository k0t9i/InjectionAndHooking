#pragma once
#include "pch.h"
#include "Trampoline.h"

namespace InjectLibrary
{
	class DllTrampolineInstaller
	{
	public:
		DllTrampolineInstaller() = default;
		virtual ~DllTrampolineInstaller();
		const FARPROC InstallTrampoline(const std::string dllName, const std::string functionName, void* hookPayloadFunctionAddress, const BYTE oldCodeSize = 0);
		void UninstallTrampoline(const std::string dllName, const std::string functionName);
		const FARPROC GetTrampolineAddress(const std::string dllName, const std::string functionName) const;
	private:
		const std::string GetKey(const std::string dllName, const std::string functionName) const;
		void* GetHookedFunctionAddress(const std::string dllName, const std::string functionName) const;
		const bool IsTrampolineExist(const std::string key) const;

	private:
		std::map<const std::string, Trampoline*> _trampolines;
	};
};