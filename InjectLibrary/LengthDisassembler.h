#pragma once
#include "pch.h"

namespace InjectLibrary
{
	class LengthDisassemblerInterface
	{
	public:
		virtual const BYTE GetLength(const void* const address, const BYTE minLength, const bool x86_64_mode = false) const = 0;
	};

	class LengthDisassembler : public LengthDisassemblerInterface
	{
	public:
		LengthDisassembler() = default;
		virtual ~LengthDisassembler() = default;

		const BYTE GetLength(const void* const address, const BYTE minLength, const bool x86_64_mode = false) const override;
	};
};