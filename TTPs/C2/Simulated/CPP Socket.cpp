//Basic web socket for sending data. This sends random data to the URL specified, starts a random process and creates a random file

#include <stdio.h>
#include <Windows.h>
#include <string>
#include <iostream>
#include <fstream>

using std::string;
using namespace std;

void mParseUrl(const char* mUrl, string& serverName, string& filepath, string& filename)
{
    string::size_type n;
    string url = mUrl;

    if (url.substr(0, 7) == "http://")
        url.erase(0, 7);

    if (url.substr(0, 8) == "https://")
        url.erase(0, 8);

    n = url.find('/');
    if (n != string::npos)
    {
        serverName = url.substr(0, n);
        filepath = url.substr(n);
        n = filepath.rfind('/');
        filename = filepath.substr(n + 1);
    }

    else
    {
        serverName = url;
        filepath = "/";
        filename = "";
    }
}

SOCKET connectToServer(char* szServerName, WORD portNum)
{
    struct hostent* hp;
    unsigned int addr;
    struct sockaddr_in server;
    SOCKET conn;

    conn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (conn == INVALID_SOCKET)
        return NULL;

    if (inet_addr(szServerName) == INADDR_NONE)
    {
        hp = gethostbyname(szServerName);
    }
    else
    {
        addr = inet_addr(szServerName);
        hp = gethostbyaddr((char*)&addr, sizeof(addr), AF_INET);
    }

    if (hp == NULL)
    {
        closesocket(conn);
        return NULL;
    }

    server.sin_addr.s_addr = *((unsigned long*)hp->h_addr);
    server.sin_family = AF_INET;
    server.sin_port = htons(portNum);
    if (connect(conn, (struct sockaddr*)&server, sizeof(server)))
    {
        closesocket(conn);
        return NULL;
    }
    return conn;
}

int getHeaderLength(char* content)
{
    const char* srchStr1 = "\r\n\r\n", * srchStr2 = "\n\r\n\r";
    char* findPos;
    int ofset = -1;

    findPos = strstr(content, srchStr1);
    if (findPos != NULL)
    {
        ofset = findPos - content;
        ofset += strlen(srchStr1);
    }

    else
    {
        findPos = strstr(content, srchStr2);
        if (findPos != NULL)
        {
            ofset = findPos - content;
            ofset += strlen(srchStr2);
        }
    }
    return ofset;
}

char* readUrl2(const char* szUrl, long& bytesReturnedOut, char** headerOut)
{
    const int bufSize = 512;
    char readBuffer[bufSize], sendBuffer[bufSize], tmpBuffer[bufSize];
    char* tmpResult = NULL, * result;
    SOCKET conn;
    string server, filepath, filename;
    long totalBytesRead, thisReadSize, headerLen;

    mParseUrl(szUrl, server, filepath, filename);

    ///////////// step 1, connect //////////////////////
    conn = connectToServer((char*)server.c_str(), 80);

    ///////////// step 2, send GET request /////////////
    sprintf(tmpBuffer, "GET %s HTTP/1.0", filepath.c_str());
    strcpy(sendBuffer, tmpBuffer);
    strcat(sendBuffer, "\r\n");
    sprintf(tmpBuffer, "Host: %s", server.c_str());
    strcat(sendBuffer, tmpBuffer);
    strcat(sendBuffer, "\r\n");
    strcat(sendBuffer, "\r\n");
    send(conn, sendBuffer, strlen(sendBuffer), 0);

    //    SetWindowText(edit3Hwnd, sendBuffer);
    printf("Buffer being sent:\n%s", sendBuffer);

    ///////////// step 3 - get received bytes ////////////////
    // Receive until the peer closes the connection
    totalBytesRead = 0;
    while (1)
    {
        memset(readBuffer, 0, bufSize);
        thisReadSize = recv(conn, readBuffer, bufSize, 0);

        if (thisReadSize <= 0)
            break;

        tmpResult = (char*)realloc(tmpResult, thisReadSize + totalBytesRead);

        memcpy(tmpResult + totalBytesRead, readBuffer, thisReadSize);
        totalBytesRead += thisReadSize;
    }

    headerLen = getHeaderLength(tmpResult);
    long contenLen = totalBytesRead - headerLen;
    result = new char[contenLen + 1];
    memcpy(result, tmpResult + headerLen, contenLen);
    result[contenLen] = 0x0;
    char* myTmp;

    myTmp = new char[headerLen + 1];
    strncpy(myTmp, tmpResult, headerLen);
    myTmp[headerLen] = NULL;
    delete(tmpResult);
    *headerOut = myTmp;

    bytesReturnedOut = contenLen;
    closesocket(conn);
    return(result);
}


int main()

{
    HINSTANCE hInst;
    WSADATA wsaData;
    const int bufLen = 1024;
    const char* szUrl = "http://www.google.com";
    long fileSize;
    char* memBuffer, * headerBuffer;
    FILE* fp;
    memBuffer = headerBuffer = NULL;

    wchar_t cmd[] = L"notepad.exe";//unicode string as parameters for strings are unicode for CreateProcessW
    //ping locale host forever
    //wchar_t cmd[] = L"C:\\Windows\\System32\\ping.exe";
    //wchar_t arg[] = L" -t 127.0.0.1";

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    BOOL OK = CreateProcessW(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi); //This is used to execute a process with arguments


    if (OK)// check if Process is created
    {
        printf("Applicaiton is running\n");
        printf("PID = %d\n", pi.dwProcessId);

        //Wait forever till Process object is signaled (terminated)
        DWORD status = WaitForSingleObject(pi.hProcess, INFINITE);

        if (status == WAIT_OBJECT_0)//The state of the specified object is signaled.

        {
            printf("PID = %d is closed!\n", pi.dwProcessId);
        }

        CloseHandle(pi.hProcess);//Handles must be explicitly closed if not parent process will hold on to it even if child process is terminated.
        CloseHandle(pi.hThread);
    }
    else
    {
        printf("Application NOT running! \t Error code %d", GetLastError());
    }
    if (WSAStartup(0x101, &wsaData) != 0)
        return -1;

    srand(time(NULL)); // Seeds the rand
    std::string filenames1[7] = {"zzzzzzzzzzzzz"}; // filenames made in C:\temp
    int len = sizeof(filenames1) / sizeof(filenames1[0]);
    int RandIndex = rand() % len; //generates a random number to grab filename from index
    std::string filePath = "C:\\temp\\";
    filePath += filenames1[RandIndex];

    memBuffer = readUrl2(szUrl, fileSize, &headerBuffer);
    printf("returned from readUrl\n");
    printf("data returned:\n%s", memBuffer);

    if (fileSize != 0)
    {
        std::ofstream outfile(filePath);
        outfile << memBuffer << "\\n";
        outfile.close();

        //printf("Got some data\n");
        delete(memBuffer);
        delete(headerBuffer);
    }

    WSACleanup();

    return 0;
}

