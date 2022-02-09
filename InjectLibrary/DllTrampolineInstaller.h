#pragma once
#include "pch.h"
#include "Trampoline.h"
#include "LengthDisassembler.h"

namespace InjectLibrary
{
	class DllTrampolineInstaller
	{
	public:
		DllTrampolineInstaller(const LengthDisassemblerInterface* lengthDisassembler);
		virtual ~DllTrampolineInstaller();
		const FARPROC InstallTrampoline(const std::string dllName, const std::string functionName, void* hookPayloadFunctionAddress);
		void UninstallTrampoline(const std::string dllName, const std::string functionName);
		const FARPROC GetTrampolineAddress(const std::string dllName, const std::string functionName) const;
	private:
		const std::string GetKey(const std::string dllName, const std::string functionName) const;
		void* GetHookedFunctionAddress(const std::string dllName, const std::string functionName) const;
		const bool IsTrampolineExist(const std::string key) const;

	private:
		std::map<const std::string, Trampoline*> _trampolines;
		const LengthDisassemblerInterface* _lengthDisassembler = nullptr;
	};
};