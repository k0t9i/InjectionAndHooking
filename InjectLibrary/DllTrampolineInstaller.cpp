#include "pch.h"
#include "DllTrampolineInstaller.h"

namespace InjectLibrary
{
	DllTrampolineInstaller::DllTrampolineInstaller(const LengthDisassemblerInterface* lengthDisassembler)
		: _lengthDisassembler(lengthDisassembler)
	{
	}

	DllTrampolineInstaller::~DllTrampolineInstaller()
	{
		for (const auto& val : _trampolines) {
			val.second->Uninstall();
			delete val.second;
		}
	}

	const FARPROC DllTrampolineInstaller::InstallTrampoline(const std::string dllName, const std::string functionName, void* hookPayloadFunctionAddress)
	{
		void* addr = GetHookedFunctionAddress(dllName, functionName);

		BYTE oldCodeSize = _lengthDisassembler->GetLength(addr, SIZE_OF_JUMP);

		const std::string key = GetKey(dllName, functionName);
		if (IsTrampolineExist(key)) {
			throw std::runtime_error((key + " trampoline already installed").c_str());
		}

		_trampolines[key] = new Trampoline(addr, hookPayloadFunctionAddress, oldCodeSize);
		return _trampolines[key]->Install();
	}

	void DllTrampolineInstaller::UninstallTrampoline(const std::string dllName, const std::string functionName)
	{
		void* addr = GetHookedFunctionAddress(dllName, functionName);
		const std::string key = GetKey(dllName, functionName);
		if (!IsTrampolineExist(key)) {
			throw std::runtime_error((key + " trampoline not installed").c_str());
		}

		_trampolines.at(key)->Uninstall();
		delete _trampolines.at(key);
		_trampolines.erase(key);
	}

	const FARPROC DllTrampolineInstaller::GetTrampolineAddress(const std::string dllName, const std::string functionName) const
	{
		void* addr = GetHookedFunctionAddress(dllName, functionName);
		const std::string key = GetKey(dllName, functionName);
		if (!IsTrampolineExist(key)) {
			throw std::runtime_error((key + " trampoline not installed").c_str());
		}

		return _trampolines.at(key)->GetAddress();
	}

	const std::string DllTrampolineInstaller::GetKey(const std::string dllName, const std::string functionName) const
	{
		return dllName + "::" + functionName;
	}

	void* DllTrampolineInstaller::GetHookedFunctionAddress(const std::string dllName, const std::string functionName) const
	{
		HMODULE hDll = GetModuleHandleA(dllName.c_str());

		if (!hDll) {
			throw std::runtime_error((dllName + " dll not loaded").c_str());
		}

		const std::string key = GetKey(dllName, functionName);

		void* result = GetProcAddress(hDll, functionName.c_str());

		if (!result) {
			throw std::runtime_error((key + " function not found").c_str());
		}

		return result;
	}

	const bool DllTrampolineInstaller::IsTrampolineExist(const std::string key) const
	{
		return _trampolines.find(key) != _trampolines.end();
	}

}
