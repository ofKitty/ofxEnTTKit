#pragma once
//
//  TransformHandle2D.h  —  ofxEnTTKit
//
//  Lightweight 2D bounding-box transform handle drawn with ImDrawList.
//
//  • Drag anywhere inside the rect  → translate
//  • Drag 8 corner / edge handles   → scale  (+ Shift = lock aspect ratio)
//
//  All coordinates are in the caller's "canvas" space.  The caller supplies
//  two lambdas to convert between canvas and screen pixels so the widget
//  works at any zoom / pan level.
//
//  Usage (immediate-mode, call every frame):
//
//    static ecs::TransformHandle2D handle;
//    ecs::Rect2D r { entity_x, entity_y, entity_w, entity_h };
//    handle.draw(dl, r, toScreen, toCanvas);
//    // r is updated in-place; write back to your data if changed.
//
#include "imgui.h"
#include <functional>
#include <cmath>
#include <algorithm>

namespace ecs {

/// Axis-aligned bounding box in canvas coordinates.
struct Rect2D {
    float x = 0, y = 0, w = 0, h = 0;
    bool operator==(const Rect2D& o) const {
        return x == o.x && y == o.y && w == o.w && h == o.h;
    }
    bool operator!=(const Rect2D& o) const { return !(*this == o); }
};

/// 2D bounding-box transform handle (header-only).
class TransformHandle2D {
public:
    void draw(ImDrawList* dl,
              Rect2D&     rect,
              std::function<ImVec2(float cx, float cy)> toScreen,
              std::function<ImVec2(float sx, float sy)> toCanvas,
              ImU32  borderColor  = IM_COL32(64, 200, 255, 220),
              float  handleRadius = 6.f);

    bool isDragging() const { return m_active >= 0 || m_body; }

private:
    int   m_active {-1};
    bool  m_body   {false};
    float m_dragStartX {0}, m_dragStartY {0};
    float m_r0 {0}, m_t0 {0}, m_w0 {0}, m_h0 {0};
};

inline void TransformHandle2D::draw(
    ImDrawList* dl, Rect2D& rect,
    std::function<ImVec2(float, float)> toScreen,
    std::function<ImVec2(float, float)> toCanvas,
    ImU32 borderColor, float hr)
{
    if (!dl) return;
    const float r = rect.x, t = rect.y, rw = rect.w, rh = rect.h;

    // 8 handle positions: corners TL TR BR BL, then edges T R B L
    float hx[8] = { r,      r+rw,   r+rw, r,       r+rw*.5f, r+rw,  r+rw*.5f, r      };
    float hy[8] = { t,      t,      t+rh, t+rh,    t,         t+rh*.5f, t+rh,  t+rh*.5f };

    ImVec2 sh[8];
    for (int i = 0; i < 8; ++i) sh[i] = toScreen(hx[i], hy[i]);

    // Outline
    dl->AddQuad(sh[0], sh[1], sh[2], sh[3], borderColor, 1.5f);
    // Handles
    const ImU32 fillCol = IM_COL32(255, 255, 255, 230);
    for (int i = 0; i < 8; ++i) {
        dl->AddCircleFilled(sh[i], hr, fillCol);
        dl->AddCircle(sh[i], hr, borderColor, 0, 1.5f);
    }

    ImVec2 mp  = ImGui::GetMousePos();
    bool lmbDn = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    bool lmbCl = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    bool lmbRl = ImGui::IsMouseReleased(ImGuiMouseButton_Left);

    if (lmbRl) { m_active = -1; m_body = false; }

    if (lmbDn && (m_active >= 0 || m_body)) {
        ImVec2 cv = toCanvas(mp.x, mp.y);
        float dx = cv.x - m_dragStartX, dy = cv.y - m_dragStartY;
        bool lock = ImGui::GetIO().KeyShift;

        if (m_body) {
            rect.x = m_r0 + dx;
            rect.y = m_t0 + dy;
        } else {
            int h = m_active;
            float nx = m_r0, ny = m_t0, nw = m_w0, nh = m_h0;
            float asp = (m_h0 > 0) ? m_w0 / m_h0 : 1.f;
            if (h == 0) { nx += dx; ny += dy; nw -= dx; nh -= dy; }
            if (h == 1) {            ny += dy; nw += dx; nh -= dy; }
            if (h == 2) {                      nw += dx; nh += dy; }
            if (h == 3) { nx += dx;            nw -= dx; nh += dy; }
            if (h == 4) {            ny += dy;            nh -= dy; }
            if (h == 5) {                       nw += dx;           }
            if (h == 6) {                                 nh += dy; }
            if (h == 7) { nx += dx;             nw -= dx;           }
            if (lock && h < 4) {
                if (std::abs(nw - m_w0) >= std::abs(nh - m_h0)) nh = nw / asp;
                else                                              nw = nh * asp;
            }
            rect.x = nx; rect.y = ny;
            rect.w = std::max(1.f, nw);
            rect.h = std::max(1.f, nh);
        }
        return;
    }

    if (lmbCl) {
        for (int i = 0; i < 8; ++i) {
            float ddx = mp.x - sh[i].x, ddy = mp.y - sh[i].y;
            if (ddx*ddx + ddy*ddy <= (hr+2)*(hr+2)) {
                m_active = i;
                ImVec2 cv = toCanvas(mp.x, mp.y);
                m_dragStartX = cv.x; m_dragStartY = cv.y;
                m_r0 = rect.x; m_t0 = rect.y; m_w0 = rect.w; m_h0 = rect.h;
                return;
            }
        }
        ImVec2 sTL = toScreen(rect.x, rect.y);
        ImVec2 sBR = toScreen(rect.x + rect.w, rect.y + rect.h);
        if (mp.x >= sTL.x && mp.x <= sBR.x && mp.y >= sTL.y && mp.y <= sBR.y) {
            m_body = true;
            ImVec2 cv = toCanvas(mp.x, mp.y);
            m_dragStartX = cv.x; m_dragStartY = cv.y;
            m_r0 = rect.x; m_t0 = rect.y; m_w0 = rect.w; m_h0 = rect.h;
        }
    }

    // Cursor hint
    for (int i = 0; i < 8; ++i) {
        float ddx = mp.x - sh[i].x, ddy = mp.y - sh[i].y;
        if (ddx*ddx + ddy*ddy <= (hr+2)*(hr+2)) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
            break;
        }
    }
}

} // namespace ecs
