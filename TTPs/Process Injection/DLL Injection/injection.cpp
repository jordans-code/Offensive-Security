//Grabbed from a snippet of a larger project of mine. This was based on an open source example on github but was modified to fit my needs.

#pragma once
#include <iostream>
#include <Windows.h>
#include <winternl.h>
#include <tlhelp32.h>
#include <vector>
#include <string>
#include <DbgHelp.h>
#include <userenv.h>
#include <ktmw32.h>
//#include "Utility.cpp"
using namespace std;
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "Ntdll.lib")
#pragma comment(lib, "Userenv.lib")
#pragma comment(lib, "KtmW32.lib")

DWORD InjectDll(DWORD PID, const WCHAR *DllName) {
	//Performs DLL injection on given PID. 

	HANDLE hProcess = NULL;
	HANDLE hThread = NULL;
	DWORD Status = NULL;
	LPVOID BaseAddress = NULL;
	FARPROC LoadDllAddress = NULL;
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	//Gets handle of process matching PID
	if (!hProcess) {
		printf("Failed to Open handle to process PID %d  Error Code is0x%x\n", PID, GetLastError());
		return -1;
	}

	BaseAddress = VirtualAllocEx(hProcess, BaseAddress, wcslen(DllName)*2 +2, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	//Allocates block of memory for the wide character str of the path to the dll
	if (!BaseAddress) {
		printf("Failed to Allocate Memory in process PID %d  Error Code is0x%x\n", PID, GetLastError());
		return -1;
	}

	Status = WriteProcessMemory(hProcess, BaseAddress, DllName, wcslen(DllName)*2+2, NULL);
	//Writes dll path to address space of process into the block that we just alloccated 
	if (!Status) {
		printf("Failed to Write to Memory in process PID %d  Error Code is0x%x\n", PID, GetLastError());
		return -1;
	}

	LoadDllAddress = GetProcAddress(GetModuleHandleA("Kernel32.dll"), "LoadLibraryW");
	//Gets the address of the LoadLibraryW function inside the kernel32.dll library. 
	//Kernel32 is loaded in most processes and can be utilized to then inject the dll that we just wrote to memory
	if (!LoadDllAddress) {
		printf("Failed to Get CreateRemoteThread Address Error Code is0x%x\n", GetLastError());
		return -1;
	}

	hThread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadDllAddress, BaseAddress, NULL, NULL);
	//Calls the loadlibraryW inside victim process address space to load and execute the dll
	if (!hThread) {
		printf("Failed to Create Remote Thread in process PID %d  Error Code is0x%x\n", PID, GetLastError());
		return -1;
	}
	CloseHandle(hProcess);
	return 0;
}
