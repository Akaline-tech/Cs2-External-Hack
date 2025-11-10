#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <tchar.h>

namespace Neko {

    // 用于同时返回 handle 和 pid
    struct ProcessInfo {
        HANDLE handle;
        DWORD pid;
    };

    // 函数声明
    ProcessInfo GetProcessHandle(const char* windowName);
    DWORD GetModuleBaseAddress32(const TCHAR* lpszModuleName, DWORD pID);
    uintptr_t GetModuleBaseAddress64(const TCHAR* lpszModuleName, DWORD pID);
    DWORD GetPointerAddress(HWND hwnd, DWORD gameBaseAddr, DWORD address, std::vector<DWORD> offsets);
    DWORD GetBaseAddress(HANDLE phandle, DWORD moduleBase, DWORD EntityBaseAdder);
#define READ(handle, address, buffer) \
    ReadProcessMemory(handle, (LPCVOID)(address), &(buffer), sizeof(buffer), nullptr)

#define WRITE(handle, address, value) \
    WriteProcessMemory(handle, (LPVOID)(address), &(value), sizeof(value), nullptr)
} 
// 字母键映射 (A-Z)
#define NK_A 0x41
#define NK_B 0x42
#define NK_C 0x43
#define NK_D 0x44
#define NK_E 0x45
#define NK_F 0x46
#define NK_G 0x47
#define NK_H 0x48
#define NK_I 0x49
#define NK_J 0x4A
#define NK_K 0x4B
#define NK_L 0x4C
#define NK_M 0x4D
#define NK_N 0x4E
#define NK_O 0x4F
#define NK_P 0x50
#define NK_Q 0x51
#define NK_R 0x52
#define NK_S 0x53
#define NK_T 0x54
#define NK_U 0x55
#define NK_V 0x56
#define NK_W 0x57
#define NK_X 0x58
#define NK_Y 0x59
#define NK_Z 0x5A
