#include "pch.h"
#include "LengthDisassembler.h"
#include "Nomade040_ldisasm.h"

namespace InjectLibrary
{
	const BYTE LengthDisassembler::GetLength(const void* const address, const BYTE minLength, const bool x86_64_mode) const 
	{
		size_t length = 0;

		while (length < minLength) {
			void* instructionAddress = (void*)((BYTE*)address + length);
			length += ldisasm(instructionAddress, x86_64_mode);
		}

		return static_cast<BYTE>(length);
	}
}
