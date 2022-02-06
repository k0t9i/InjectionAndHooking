#include "pch.h"
#include "ProcessManipulation.h"

namespace InjectLibrary
{
	const std::string GetProcessName(const DWORD processId)
	{
		HANDLE handle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, processId);
		if (handle) {
			char path[MAX_PATH];
			char drive[_MAX_DRIVE];
			char dir[_MAX_DIR];
			char fname[_MAX_FNAME];
			char ext[_MAX_EXT];
			GetModuleFileNameExA(handle, 0, path, sizeof(path));
			_splitpath_s(path, drive, dir, fname, ext);
			CloseHandle(handle);
			return std::string(fname) + std::string(ext);
		}
		return "";
	}

	void StartProcess(const DWORD processId)
	{
		DWORD currThread = GetCurrentThreadId();
		HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		THREADENTRY32 thread;
		HANDLE threadHandle;
		if (snap != INVALID_HANDLE_VALUE) {
			thread.dwSize = sizeof(LPTHREADENTRY32);
			if (Thread32First(snap, &thread)) {
				do {
					if (thread.th32ThreadID != currThread && thread.th32OwnerProcessID != processId) {
						threadHandle = OpenThread(THREAD_SUSPEND_RESUME, FALSE, thread.th32ThreadID);
						if (threadHandle == 0) {
							break;
						}
						ResumeThread(threadHandle);
						CloseHandle(threadHandle);
					}
				} while (Thread32Next(snap, &thread));
			}
			CloseHandle(snap);
		}
	}

	void StopProcess(const DWORD processId)
	{
		DWORD currThread = GetCurrentThreadId();
		HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		THREADENTRY32 thread;
		HANDLE threadHandle;
		if (snap != INVALID_HANDLE_VALUE) {
			thread.dwSize = sizeof(LPTHREADENTRY32);
			if (Thread32First(snap, &thread)) {
				do {
					if (thread.th32ThreadID != currThread && thread.th32OwnerProcessID != processId) {
						threadHandle = OpenThread(THREAD_SUSPEND_RESUME, FALSE, thread.th32ThreadID);
						if (threadHandle == 0) {
							break;
						}
						SuspendThread(threadHandle);
						CloseHandle(threadHandle);
					}
				} while (Thread32Next(snap, &thread));
			}
			CloseHandle(snap);
		}
	}
}