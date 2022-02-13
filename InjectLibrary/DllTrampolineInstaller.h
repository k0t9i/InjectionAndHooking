#pragma once

#include <windows.h>
#include <string>
#include <map>
#include "Trampoline.h"
#include "LengthDisassembler.h"

namespace InjectLibrary
{
	class DllTrampolineInstaller
	{
	public:
		DllTrampolineInstaller(const LengthDisassemblerInterface* lengthDisassembler, const BYTE minSpliceLength);
		virtual ~DllTrampolineInstaller();
		const FARPROC InstallTrampoline(const std::string& dllName, const std::string& functionName, void* hookPayloadFunctionAddress);
		void UninstallTrampoline(const std::string& dllName, const std::string& functionName);
		const FARPROC GetTrampolineAddress(const std::string& dllName, const std::string& functionName) const;

		DllTrampolineInstaller(const DllTrampolineInstaller&) = delete;
		DllTrampolineInstaller& operator=(const DllTrampolineInstaller&) = delete;
		DllTrampolineInstaller(const DllTrampolineInstaller&&) = delete;
		DllTrampolineInstaller& operator=(const DllTrampolineInstaller&&) = delete;
	private:
		const std::string GetKey(const std::string& dllName, const std::string& functionName) const;
		void* GetHookedFunctionAddress(const std::string& dllName, const std::string& functionName) const;
		const bool IsTrampolineExist(const std::string& key) const;

	private:
		std::map<const std::string, Trampoline*> _trampolines;
		const LengthDisassemblerInterface* _lengthDisassembler = nullptr;
		const BYTE _minSpliceLength = 0;
	};
};