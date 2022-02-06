#include "pch.h"
#include "Trampoline.h"

namespace InjectLibrary
{
    Trampoline::Trampoline(void* hookedFunctionAddress, void* hookPayloadFunctionAddress, const BYTE oldCodeSize) :
        _hookedFunctionAddress(hookedFunctionAddress), _hookPayloadFunctionAddress(hookPayloadFunctionAddress)
    {
        auto size = oldCodeSize;
        if (size <= 0) {
            size = SIZE_OF_JUMP;
        }
        _trampolineLayout = new TrampolineLayout(size);
        // ���, ������� ����� ������������ � ������� ������ ����� ���������� �� ����������
        VirtualProtect(_trampolineLayout->code, _trampolineLayout->GetFullSize(), PAGE_EXECUTE_READWRITE, &_protect);
    }

    Trampoline::~Trampoline()
    {
        VirtualProtect(_trampolineLayout->code, _trampolineLayout->GetFullSize(), _protect, &_protect);
        delete _trampolineLayout;
    }

    const FARPROC Trampoline::Install()
    {
        FillLayout();
        InstallHook();
        return GetAddress();
    }

    void Trampoline::Uninstall()
    {
        DWORD oldProtect;
        VirtualProtect(_hookedFunctionAddress, SIZE_OF_JUMP, PAGE_EXECUTE_READWRITE, &oldProtect);
        // ��� �������� ���� ������ �� ����� �������� ���������� � ��������������� �������
        CopyMemory(_hookedFunctionAddress, _trampolineLayout->code, SIZE_OF_JUMP);
        VirtualProtect(_hookedFunctionAddress, SIZE_OF_JUMP, oldProtect, &oldProtect);
    }

    const FARPROC Trampoline::GetAddress() const
    {
        return (FARPROC)(void*)_trampolineLayout->code;
    }

    void Trampoline::FillLayout()
    {
        const auto oldCodeSize = _trampolineLayout->GetOldCodeSize();
        // ��������� ������ oldCodeSize ���� ���� �� ��������������� ������� � ��� ��������
        CopyMemory(_trampolineLayout->code, _hookedFunctionAddress, oldCodeSize);
        // ���������� 32������ �������� ������ � ������� � ��� �������� ����� ���� �������, ������������� ����
        _trampolineLayout->jumpInstruction->rel32 = (DWORD)_hookedFunctionAddress - ((DWORD)_trampolineLayout->code + oldCodeSize);
    }

    void Trampoline::InstallHook() const
    {
        DWORD oldProtect;
        // ��� �� �������� ��� ��������������� �������, ������� ������ ������ ����� ���������� �� ������
        VirtualProtect(_hookedFunctionAddress, SIZE_OF_JUMP, PAGE_EXECUTE_READWRITE, &oldProtect);
        RelativeJumpLayout* instr = (RelativeJumpLayout*)((BYTE*)_hookedFunctionAddress);
        // ���������� 32������ �������� ������ � ������� ��� ������ � ������� ���������� ������ � ������ ��������������� �������
        // ����� ����� �������� � ���� �������, ��� ����������� �������� ������ ����� ��������� � �������������� ������� �� ���������� ���������
        instr->opcode = 0xe9;
        instr->rel32 = (DWORD)_hookPayloadFunctionAddress - ((DWORD)_hookedFunctionAddress + SIZE_OF_JUMP);
        VirtualProtect(_hookedFunctionAddress, SIZE_OF_JUMP, oldProtect, &oldProtect);
    }
}
