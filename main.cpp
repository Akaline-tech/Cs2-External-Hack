#define NOMINMAX
#include <Windows.h>
#include <dwmapi.h>
#include <TlHelp32.h>
#include <iostream>
#include "NekoMemory.h"
#include "Vector.h"
#include "memory.h"
#include <cmath>
#include <string>
#include <sstream>
#include "offset.h"
#include "Function.h"
#include "Val.h"
#include "output/offsets.hpp"
#include "output/client_dll.hpp"
#include <thread>
#include <psapi.h>

int width;
int height;

#pragma comment(lib, "dwmapi.lib")

using namespace std;

const wchar_t* OVERLAY_CLASS = L"OverlayWindow";

#define M_PI 3.14159265358
#define FOR(i,s,e) for(int i = s;i < e;i++)
#define 小于 <
#define 大于 >
#define 小于等于 <=
#define 大于等于 >=

HWND hGameWnd = nullptr;
HWND hOverlayWnd = nullptr;
RECT gameRect = { 0 };

LRESULT CALLBACK OverlayProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_ERASEBKGND:
        return 1;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// ==== 创建透明 Overlay ====
HWND CreateOverlay(HINSTANCE hInstance)
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, OverlayProc,
        0, 0, hInstance, NULL, NULL, NULL, NULL, OVERLAY_CLASS, NULL };
    RegisterClassEx(&wc);

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
        OVERLAY_CLASS,
        L"613E2F",
        WS_POPUP,
        0, 0, 800, 600,
        NULL, NULL, hInstance, NULL);

    // 设置透明背景
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    // 鼠标穿透
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT | WS_EX_LAYERED);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    return hwnd;
}
template <typename T>
T clamp(const T& v, const T& lo, const T& hi) {
    return (v < lo) ? lo : (v > hi) ? hi : v;
}
Vector3 CalcAimAngles(const Vector3& source, const Vector3& target) {
    Vector3 delta = { target.x - source.x, target.y - source.y, target.z - source.z };

    // Calculate yaw (horizontal angle)
    float yaw = atan2(delta.y, delta.x) * (180.0f / M_PI) + 90.0f;


    // Calculate pitch (vertical angle)
    float distance = sqrt(delta.x * delta.x + delta.y * delta.y);
    float pitch = atan2(delta.z, distance) * (180.0f / M_PI);

    return Vector3(pitch, yaw, 0.0f); // Roll is typically 0 for aiming
}


uintptr_t GetBaseEntityFromHandle(uint32_t uHandle, uintptr_t client) {

    auto entListBase = *reinterpret_cast<std::uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwEntityList);
    if (entListBase == 0) {
        return 0;
    }

    const int nIndex = uHandle & 0x7FFF;

    auto entitylistbase = *reinterpret_cast<std::uintptr_t*>(entListBase + 8 * (nIndex >> 9) + 16);
    if (entitylistbase == 0) {
        return 0;
    }

    return *reinterpret_cast<std::uintptr_t*>(entitylistbase + 0x78 * (nIndex & 0x1FF));

}

char* wstringToChar(const wstring& wstr)
{
    if (wstr.empty()) return nullptr;

    // 计算转换后所需的字节数（包括 '\0'）
    int sizeNeeded = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (sizeNeeded == 0) return nullptr;

    // 分配内存（记得释放！）
    char* result = new char[sizeNeeded];
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, result, sizeNeeded, NULL, NULL);

    return result;
}

std::optional<Vector3> GetEyePos(HANDLE hProcess, uintptr_t addr) noexcept
{
    Vector3 origin{}, viewOffset{};

    // Origin
    if (!ReadProcessMemory(hProcess,
        reinterpret_cast<LPCVOID>(addr + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin),
        &origin, sizeof(origin), nullptr))
    {
        return std::nullopt;
    }

    // ViewOffset
    if (!ReadProcessMemory(hProcess,
        reinterpret_cast<LPCVOID>(addr + cs2_dumper::schemas::client_dll::C_BaseModelEntity::m_vecViewOffset),
        &viewOffset, sizeof(viewOffset), nullptr))
    {
        return std::nullopt;
    }

    Vector3 localEye = {
        origin.x + viewOffset.x,
        origin.y + viewOffset.y,
        origin.z + viewOffset.z
    };

    if (!std::isfinite(localEye.x) || !std::isfinite(localEye.y) || !std::isfinite(localEye.z))
        return std::nullopt;

    if (localEye.Length() < 0.1f)
        return std::nullopt;

    return localEye;
}

static void FillCircle(HDC hdc, int cx, int cy, int r, COLORREF fillColor, COLORREF borderColor) {
    HBRUSH hBrush = CreateSolidBrush(fillColor);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
    HPEN hPen = CreatePen(PS_SOLID, 1, borderColor);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);

    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hBrush);
    DeleteObject(hPen);
}

static void DrawRadar(HDC hdc, int centerX, int centerY, int radius,const Vector3& localPos, float localYawDeg,const std::vector<Vector3>& enemyPositions,float scale){

    FillCircle(hdc, centerX, centerY, radius, RGB(20, 20, 20), RGB(120, 120, 120));

    float yawRad = (localYawDeg -90) * (3.14159265358979323846f / 180.0f);
    int arrowLen = radius - 64;
    int ax = centerX + (int)(cosf(yawRad+4.75) * arrowLen);
    int ay = centerY + (int)(sinf(yawRad+ 4.75) * arrowLen);
    NDraw::DrawLine(hdc, centerX, centerY, ax, ay, 2, RGB(0, 255, 0));

    for (const Vector3& epos : enemyPositions) {
        float dx = epos.x - localPos.x;
        float dy = epos.y - localPos.y;

        float cs = cosf(-yawRad);
        float sn = sinf(-yawRad);
        float rx = dx * cs - dy * sn;
        float ry = dx * sn + dy * cs;

        float px = rx * scale;
        float py = ry * scale;

        int drawX = centerX + (int)px;
        int drawY = centerY - (int)py;

        float distPx = sqrtf(px * px + py * py);
        if (distPx > radius - 4) {
            float nx = px / distPx * (radius - 6);
            float ny = py / distPx * (radius - 6);
            drawX = centerX + (int)nx;
            drawY = centerY - (int)ny;
        }

        HBRUSH b = CreateSolidBrush(RGB(255, 0, 0));
        HBRUSH oldb = (HBRUSH)SelectObject(hdc, b);
        Ellipse(hdc, drawX - 3, drawY - 3, drawX + 3, drawY + 3);
        SelectObject(hdc, oldb);
        DeleteObject(b);
    }
}



int main() {

    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    cout << "[Overlay] Starting..." << endl;


    HWND hGameWnd = FindWindow(NULL, L"Counter-Strike 2");
    if (!hGameWnd) {
        cout << "Not Finded Window！" << endl;
        system("pause");
        return 0;
    }

    auto proc = Neko::GetProcessHandle("Counter-Strike 2");
    uintptr_t client = Neko::GetModuleBaseAddress64(_T("client.dll"), proc.pid);
    cout << "[debug]GetModuleBaseAddress: " << client << "\n";
    READ(proc.handle,client + cs2_dumper::offsets::client_dll::dwEntityList, EntityList);
    // 创建 Overlay
    HINSTANCE hInstance = GetModuleHandle(NULL);
    HWND hOverlayWnd = CreateOverlay(hInstance);

    MSG msg = { 0 };
    RECT gameRect = {};

    // --- 双缓冲相关资源（在外部保存，按需重建）---
    HBITMAP hBackBitmap = nullptr;
    HBITMAP hOldBitmap = nullptr;
    HDC hMemDC = nullptr;
    int back_width = 0, back_height = 0;

    while (true) {

        float viewMatrix[16]{};
        READ(proc.handle, client + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn, LocalPlayerPawn);
        if (!LocalPlayerPawn)continue;
        READ(proc.handle, client + cs2_dumper::offsets::client_dll::dwViewMatrix, viewMatrix, sizeof(64));

        Vector3 LocalplayerPos{};
        READ(proc.handle, LocalPlayerPawn + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin, LocalplayerPos);

        Vector3 LocalAngles{};
        READ(proc.handle, client + cs2_dumper::offsets::client_dll::dwViewAngles, LocalAngles);

        int LocalTeam;
        READ(proc.handle, LocalPlayerPawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum, LocalTeam);

        std::vector<Vector3> enemies;

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (IsWindow(hGameWnd)) {
            GetClientRect(hGameWnd, &gameRect);
            POINT ul = { gameRect.left, gameRect.top };
            POINT lr = { gameRect.right, gameRect.bottom };
            ClientToScreen(hGameWnd, &ul);
            ClientToScreen(hGameWnd, &lr);
            gameRect.left = ul.x; gameRect.top = ul.y;
            gameRect.right = lr.x; gameRect.bottom = lr.y;
            width = gameRect.right - gameRect.left;
            height = gameRect.bottom - gameRect.top;
            MoveWindow(hOverlayWnd, gameRect.left, gameRect.top, width, height/3.5, TRUE);
        }

        // 获取目标 DC（用于最终 blt）
        HDC hdc = GetDC(hOverlayWnd);
        RECT rc;
        GetClientRect(hOverlayWnd, &rc);

        if (back_width != (rc.right - rc.left) || back_height != (rc.bottom - rc.top) || hMemDC == nullptr) {
            // 释放老资源
            if (hMemDC) {
                SelectObject(hMemDC, hOldBitmap); // 恢复原位图
                DeleteObject(hBackBitmap);
                DeleteDC(hMemDC);
                hMemDC = nullptr;
                hBackBitmap = nullptr;
                hOldBitmap = nullptr;
            }

            back_width = rc.right - rc.left;
            back_height = rc.bottom - rc.top;
            if (back_width <= 0) back_width = 1;
            if (back_height <= 0) back_height = 1;

            hMemDC = CreateCompatibleDC(hdc);
            hBackBitmap = CreateCompatibleBitmap(hdc, back_width, back_height);
            hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBackBitmap);
        }

        //在内存 DC 上先清空（注意：使用与你的透明色相同的背景色）
        HBRUSH clearBrush = CreateSolidBrush(RGB(0, 0, 0));
        RECT rcMem = { 0, 0, back_width, back_height };
        FillRect(hMemDC, &rcMem, clearBrush);
        DeleteObject(clearBrush);

        //printf("X %d Y%d ", screenPos.x, screenPos.y);

        FOR(i, 0, 64) {

            uintptr_t listEntityController; READ(proc.handle, EntityList + ((8 * (i & 0x7FFF) >> 9) + 16), listEntityController);
            uintptr_t entityController; READ(proc.handle, listEntityController + 0x70 * (i & 0x1FF), entityController);

            uintptr_t entityControllerPawn; READ(proc.handle, entityController + cs2_dumper::schemas::client_dll::CCSPlayerController::m_hPlayerPawn, entityControllerPawn);
            uintptr_t listEntity; READ(proc.handle, EntityList + (0x8 * ((entityControllerPawn & 0x7FFF) >> 9) + 16), listEntity);
            READ(proc.handle, listEntity + 0x70 * (entityControllerPawn & 0x1FF), entity);
            if (entity == LocalPlayerPawn)continue;

            int Health; READ(proc.handle, entity + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth, Health);
            int HealthMax; READ(proc.handle, entity + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iMaxHealth, HealthMax);
            if (Health 小于等于 0)continue;

            int m_iTeamNum; READ(proc.handle, entity + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum, m_iTeamNum);
            if (m_iTeamNum == LocalTeam)continue;

            uintptr_t m_sSanitizedPlayerNameBase; READ(proc.handle, entityController + cs2_dumper::schemas::client_dll::CCSPlayerController::m_sSanitizedPlayerName, m_sSanitizedPlayerNameBase);

            char m_sSanitizedPlayerName[128]=""; READ(proc.handle, m_sSanitizedPlayerNameBase, m_sSanitizedPlayerName);

            uintptr_t Collision; READ(proc.handle, entity + cs2_dumper::schemas::client_dll::C_BaseEntity::m_pCollision, Collision);
            if (!Collision) continue;

            Vector3 mins; READ(proc.handle, Collision + cs2_dumper::schemas::client_dll::CCollisionProperty::m_vecSurroundingMins, mins);
            Vector3 maxs; READ(proc.handle, Collision + cs2_dumper::schemas::client_dll::CCollisionProperty::m_vecSurroundingMaxs, maxs);
            Vector3 m_vOldOrigin; READ(proc.handle, entity + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin, m_vOldOrigin);
            Vector3 screenPos;
            Vector3 headScreen, footScreen;
            auto player_eyepos_opt = GetEyePos(proc.handle, entity);
            if (!player_eyepos_opt.has_value()) continue;
            Vector3 player_eyepos = player_eyepos_opt.value();
            Vector3 footPos{};

            if (!Neko::WorldToScreenSo(player_eyepos, headScreen, viewMatrix, width, height));

            if (!Neko::WorldToScreenSo(m_vOldOrigin, footScreen, viewMatrix, width, height));

            if (Pointer) {
                NDraw::DrawRedDot(hMemDC, footScreen.x, 5);

                std::wstring HealthText = L"HP: " + std::to_wstring(Health);
                NDraw::DrawTextAdvancedString(hMemDC, footScreen.x + 1, 7, HealthText, RGB(40, 40, 40));
                NDraw::DrawTextAdvancedString(hMemDC, footScreen.x, 6, HealthText, RGB(0, 220, 30));

                NDraw::DrawTextAdvanced(hMemDC, footScreen.x, 21, m_sSanitizedPlayerName, RGB(40, 40, 40));
                NDraw::DrawTextAdvanced(hMemDC, footScreen.x, 20, m_sSanitizedPlayerName, RGB(100, 100, 200));
            }

            //cout << m_vOldOrigin.x <<" | "<< m_vOldOrigin.y <<" | "<< m_vOldOrigin.z << endl;
            enemies.push_back(m_vOldOrigin);
        }

        int radarRadius = 90;
        int radarX = back_width - radarRadius - 20; // 右上角偏移
        int radarY = radarRadius + 20;
        float radarRange = 1200.0f; // 世界单位半径（可调）
        float scale = radarRadius / radarRange; // pixels per world unit
        if(Rader)
            DrawRadar(hMemDC, radarX, radarY, radarRadius, LocalplayerPos, LocalAngles.y, enemies, scale);

        NDraw::DrawTextAdvanced(hMemDC, 5, 5, "Akline External Cheat", RGB(0, 255, 0));

        bool insertPressedNow = GetAsyncKeyState(VK_INSERT) & 0x8000;
        if (insertPressedNow && !insertPressedLast) { // 边缘触发
            ShowMenu = !ShowMenu;
        }
        insertPressedLast = insertPressedNow;

        if (ShowMenu) {
            bool f5PressedNow = GetAsyncKeyState(VK_F5) & 0x8000;
            if (f5PressedNow && !f5PressedLast) {
                Pointer = !Pointer;
            }
            f5PressedLast = f5PressedNow;

            // F6 切换 Rader
            bool f6PressedNow = GetAsyncKeyState(VK_F6) & 0x8000;
            if (f6PressedNow && !f6PressedLast) {
                Rader = !Rader;
            }
            f6PressedLast = f6PressedNow;

            if (Pointer)
                NDraw::DrawTextAdvanced(hMemDC, 300, 15, "Pointer Hack [ON]  F5", RGB(0, 240, 0));
            else
                NDraw::DrawTextAdvanced(hMemDC, 300, 15, "Pointer Hack [OFF] F5", RGB(240, 0, 0));

            if (Rader)
                NDraw::DrawTextAdvanced(hMemDC, 300, 35, "Rader Hack   [ON]  F6", RGB(0, 240, 0));
            else
                NDraw::DrawTextAdvanced(hMemDC, 300, 35, "Rader Hack   [OFF] F6", RGB(240, 0, 0));
        }


        BitBlt(hdc, 0, 0, back_width, back_height, hMemDC, 0, 0, SRCCOPY);
        ReleaseDC(hOverlayWnd, hdc);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (hMemDC) {
        SelectObject(hMemDC, hOldBitmap);
        DeleteObject(hBackBitmap);
        DeleteDC(hMemDC);
        hMemDC = nullptr;
        hBackBitmap = nullptr;
        hOldBitmap = nullptr;
    }

    return 0;
}
