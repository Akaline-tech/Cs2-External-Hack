#define NOMINMAX
#include <Windows.h>
#include <dwmapi.h>
#include <iostream>
#include "Vector.h"
#include <string>

namespace Neko {
    bool WorldToScreen(const Vector3& world, Vector2& screen, float* m, int width, int height) {
        float clipX = world.x * m[0] + world.y * m[4] + world.z * m[8] + m[12];
        float clipY = world.x * m[1] + world.y * m[5] + world.z * m[9] + m[13];
        float clipW = world.x * m[3] + world.y * m[7] + world.z * m[11] + m[15];

        if (clipW < 0.001f) return false;

        float ndcX = clipX / clipW;
        float ndcY = clipY / clipW;

        screen.x = (width / 2.0f) + (ndcX * width / 2.0f);
        screen.y = (height / 2.0f) - (ndcY * height / 2.0f);

        return true;
    }

    bool WorldToScreenSo(Vector3 pWorldPos, Vector3& pScreenPos, float* pMatrixPtr, const FLOAT pWinWidth, const FLOAT pWinHeight)
    {
        float matrix2[4][4];
        memcpy(matrix2, pMatrixPtr, 16 * sizeof(float));
        const float mX{ pWinWidth / 2 };
        const float mY{ pWinHeight / 2 };
        const float w{
            matrix2[3][0] * pWorldPos.x +
            matrix2[3][1] * pWorldPos.y +
            matrix2[3][2] * pWorldPos.z +
            matrix2[3][3] };
        if (w < 0.65f)return false;
        const float x{
            matrix2[0][0] * pWorldPos.x +
            matrix2[0][1] * pWorldPos.y +
            matrix2[0][2] * pWorldPos.z +
            matrix2[0][3]
        };
        const float y{
            matrix2[1][0] * pWorldPos.x +
            matrix2[1][1] * pWorldPos.y +
            matrix2[1][2] * pWorldPos.z +
            matrix2[1][3]
        };
        pScreenPos.x = (mX + mX * x / w);
        pScreenPos.y = (mY - mY * y / w);
        pScreenPos.z = 0;
        return true;
    }


    float Get3DDistance(const Vector3& src, const Vector3& dst){
        float dx = dst.x - src.x;
        float dy = dst.y - src.y;
        float dz = dst.z - src.z;
        return static_cast<float>(sqrtf(dx * dx + dy * dy + dz * dz));
    }

}
namespace NDraw {
    void DrawRedDot(HDC hdc, int x, int y, int radius = 5, COLORREF color = RGB(255, 0, 0)){
        HBRUSH brush = CreateSolidBrush(color);
        SelectObject(hdc, brush);
        Ellipse(hdc, x - radius, y - radius, x + radius, y + radius);
        DeleteObject(brush);
    }

    void DrawTextAdvanced(HDC hdc, int x, int y, const char* text, COLORREF color = RGB(0, 255, 0)){
        int len = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
        wchar_t* wtext = new wchar_t[len];
        MultiByteToWideChar(CP_UTF8, 0, text, -1, wtext, len);

        RECT rect = { x, y, x + 300, y + 50 };
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, color);
        DrawTextW(hdc, wtext, -1, &rect, DT_LEFT | DT_NOCLIP);

        delete[] wtext;
    }

    void DrawTextAdvancedString(HDC hdc, int x, int y, const std::wstring& text, COLORREF color = RGB(0, 255, 0)){
        RECT rect = { x, y, x + 200, y + 30 };
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, color);
        HFONT hFont = CreateFontW(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Î¢ÈíÑÅºÚ");
        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

        std::wstring wtext = std::wstring(text);

        DrawTextW(hdc, wtext.c_str(), -1, &rect, DT_NOCLIP | DT_LEFT);

        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);
    }
    
    void DrawBox(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color, int thickness)
    {
        if (!hdc) return;
        if (thickness <= 0) thickness = 1;

        if (x1 > x2) std::swap(x1, x2);
        if (y1 > y2) std::swap(y1, y2);

        if (x2 - x1 <= 0 || y2 - y1 <= 0) return;

        HBRUSH hBrush = CreateSolidBrush(color);
        RECT r;

        //Top edge
        r.left = x1;
        r.top = y1;
        r.right = x2;
        r.bottom = std::min(y1 + thickness, y2);
        FillRect(hdc, &r, hBrush);

        // Bottom edge
        r.left = x1;
        r.top = std::max(y2 - thickness, y1);
        r.right = x2;
        r.bottom = y2;
        FillRect(hdc, &r, hBrush);

        // Left edge
        r.left = x1;
        r.top = y1 + thickness;
        r.right = std::min(x1 + thickness, x2);
        r.bottom = std::max(y2 - thickness, y1);
        if (r.right > r.left && r.bottom > r.top)
            FillRect(hdc, &r, hBrush);

        // Right edge
        r.left = std::max(x2 - thickness, x1);
        r.top = y1 + thickness;
        r.right = x2;
        r.bottom = std::max(y2 - thickness, y1);
        if (r.right > r.left && r.bottom > r.top)
            FillRect(hdc, &r, hBrush);

        DeleteObject(hBrush);
    }
    void DrawLine(HDC hdc, int x1, int y1, int x2, int y2, int thickness, COLORREF color) {
        HPEN hPen = CreatePen(PS_SOLID, thickness, color);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        MoveToEx(hdc, x1, y1, NULL);
        LineTo(hdc, x2, y2);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }
}
