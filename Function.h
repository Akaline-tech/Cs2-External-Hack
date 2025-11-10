#pragma once
#include "Vector.h"

namespace Neko {
    bool WorldToScreen(const Vector3& world, Vector2& screen, float* m, int width, int height);
    bool WorldToScreenSo(Vector3 pWorldPos, Vector3& pScreenPos, float* pMatrixPtr, const FLOAT pWinWidth, const FLOAT pWinHeight);
    float Get3DDistance(const Vector3& src, const Vector3& dst);

}
namespace NDraw {
    void DrawRedDot(HDC hdc, int x, int y, int radius = 5, COLORREF color = RGB(255, 0, 0));
    void DrawTextAdvanced(HDC hdc, int x, int y, const char* text, COLORREF color = RGB(0, 255, 0));
    void DrawTextAdvancedString(HDC hdc, int x, int y, const std::wstring& text, COLORREF color = RGB(0, 255, 0));
    void DrawBox(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color, int thickness = 1);
    void DrawLine(HDC hdc, int x1, int y1, int x2, int y2, int thickness, COLORREF color);
}