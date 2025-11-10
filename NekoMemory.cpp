#include "NekoMemory.h"
#include <iostream>

using namespace std; 

namespace Neko {

    DWORD GetModuleBaseAddress32(const TCHAR* lpszModuleName, DWORD pID) {
        DWORD dwModuleBaseAddress = 0;
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pID);
        MODULEENTRY32 ModuleEntry32 = { 0 };
        ModuleEntry32.dwSize = sizeof(MODULEENTRY32);

        if (Module32First(hSnapshot, &ModuleEntry32)) {
            do {
                if (_tcscmp(ModuleEntry32.szModule, lpszModuleName) == 0) {
                    dwModuleBaseAddress = (DWORD)ModuleEntry32.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnapshot, &ModuleEntry32));
        }
        CloseHandle(hSnapshot);
        return dwModuleBaseAddress;
    }

    uintptr_t GetModuleBaseAddress64(const TCHAR* lpszModuleName, DWORD pID)
    {
        uintptr_t moduleBaseAddress = 0;
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pID);
        MODULEENTRY32 ModuleEntry32 = { 0 };
        ModuleEntry32.dwSize = sizeof(MODULEENTRY32);

        if (Module32First(hSnapshot, &ModuleEntry32)) {
            do {
                if (_tcscmp(ModuleEntry32.szModule, lpszModuleName) == 0) {
                    moduleBaseAddress = reinterpret_cast<uintptr_t>(ModuleEntry32.modBaseAddr);
                    break;
                }
            } while (Module32Next(hSnapshot, &ModuleEntry32));
        }
        CloseHandle(hSnapshot);
        return moduleBaseAddress;
    }

    DWORD GetPointerAddress(HWND hwnd, DWORD gameBaseAddr, DWORD address, vector<DWORD> offsets)
    {
        DWORD pID = 0;
        GetWindowThreadProcessId(hwnd, &pID); // 注意传入 &pID
        HANDLE phandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID); // 三个参数
        if (!phandle || phandle == INVALID_HANDLE_VALUE) {
            // 打开进程失败，返回 0 或其它错误值
            return 0;
        }

        DWORD offset_null = 0;
        if (!ReadProcessMemory(phandle, (LPCVOID)(gameBaseAddr + address), &offset_null, sizeof(offset_null), nullptr)) {
            CloseHandle(phandle);
            return 0;
        }

        DWORD pointeraddress = offset_null;
        for (size_t i = 0; i + 1 < offsets.size(); ++i) {
            if (!ReadProcessMemory(phandle, (LPCVOID)(pointeraddress + offsets[i]), &pointeraddress, sizeof(pointeraddress), nullptr)) {
                CloseHandle(phandle);
                return 0;
            }
        }

        DWORD finalAddr = pointeraddress + (offsets.empty() ? 0 : offsets.back());
        CloseHandle(phandle);
        return finalAddr;
    }

    DWORD GetBaseAddress(HANDLE phandle, DWORD moduleBase, DWORD EntityBaseAdder)
    {
        DWORD PtrAddr = moduleBase + EntityBaseAdder;
        DWORD Base = 0;

        if (!ReadProcessMemory(phandle, (LPCVOID)PtrAddr, &Base, sizeof(Base), nullptr)) {
            cout << "Error (code: " << GetLastError() << ")" << endl;
            return 0;
        }

        return Base;
    }

    ProcessInfo GetProcessHandle(const char* windowName)
    {
        ProcessInfo info{ NULL, 0 };

        HWND hwnd = FindWindowA(NULL, windowName);
        if (!hwnd) {
            cout << "Window not found: " << windowName << endl;
            return info;
        }

        GetWindowThreadProcessId(hwnd, &info.pid); // 传入 &info.pid
        if (info.pid == 0) {
            cout << "pID Get Fail" << endl;
            return info;
        }

        info.handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, info.pid); // 三个参数
        if (!info.handle || info.handle == INVALID_HANDLE_VALUE) {
            cout << "Open Process Fail (" << GetLastError() << ")" << endl;
            info.handle = NULL;
            return info;
        }

        //cout << "Successfully opened process! PID: " << info.pid << endl;
        return info;
    }

} // namespace Neko
