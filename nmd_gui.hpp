// This is a C++ retained mode graphical user interface library.
//
// define the 'NMD_GUI_IMPLEMENTATION' macro in one and only one source file.
// Example:
// #include <...>
// #include <...>
// #define NMD_GUI_IMPLEMENTATION
// #include "nmd_gui.hpp"
//
//Credits:
// - imgui - https://github.com/ocornut/imgui
// - stb_truetype - https://github.com/nothings/stb/blob/master/stb_truetype.h
// - stb_image - https://github.com/nothings/stb/blob/master/stb_image.h

#ifndef NMD_GUI_H
#define NMD_GUI_H

//Common dependencies
#include <cinttypes>
#include <vector>
#include <unordered_map>
#include <memory>
#include <queue>

//You may use 'uint32_t' instead of 'const char*' if you preffer. It will likely improve performance.
typedef const char* IdentifierType;

//You may use uint32_t if uint16_t is not enough.
typedef uint16_t IndexType;

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef GUI_D3D9
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")
#endif

#if GUI_D3D11
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#endif

namespace Gui
{
#ifdef _WIN32
    void Win32Init(HWND hwnd);
    LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

#ifdef GUI_D3D9
    void D3D9Init(LPDIRECT3DDEVICE9 pDevice);
    void D3D9Render();
#endif

#if GUI_D3D11
    void D3D11Render();
#endif

    struct EventCallback;
    class Widget;
    class Layer;

    enum class WIDGET_TYPE
    {
        UNKNOWN,
        LAYER,
        BUTTON,
        CHECKBOX,
        LABEL,
        INPUT
    };

    struct Color
    {
        static const Color Black;
        static const Color White;
        static const Color Red;
        static const Color Green;
        static const Color Blue;
        static const Color Orange;
        static const Color Amber;
        static const Color AndroidGreen;
        static const Color Azure;
        static const Color Bronze;
        static const Color Corn;
        static const Color Emerald;
        static const Color LapisLazuli;
        static const Color Lava;

        union
        {
            uint32_t color;
            struct { uint8_t r, g, b, a; };
        };

        Color(uint32_t color)
            : color(color) {}

        Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
            : r(r), g(g), b(b), a(a) {}

        Color() : Color(0) {}
    };

    struct Vec2
    {
        float x, y;

        Vec2() : x(0.0f), y(0.0f) {}
        Vec2(float x, float y) : x(x), y(y) {}
    };

    struct Vec3
    {
        float x, y, z;

        Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
        Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

        Vec3 operator+(const Vec3& other) { return Vec3(this->x + other.x, this->y + other.y, this->z + other.z); }
        void operator+=(const Vec3& other) { this->x += other.x; this->y += other.y; this->z += other.z; }
    };

    struct Vec4
    {
        union
        {
            struct { float x, y, z, w; };
            struct { float left, top, right, bottom; };
            struct { Vec2 pos, size; };
        };

        Vec4() : pos(), size() {}
        Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
        Vec4(const Vec2& pos, const Vec2& size) : pos(pos), size(size) {}

        Vec4 operator+(const Vec4& other) { return Vec4(this->x + other.x, this->y + other.y, this->z + other.z, this->w + other.w); }
        void operator+=(const Vec4& other) { this->x += other.x, this->y + other.y, this->z + other.z, this->w + other.w; }
    };

    bool IsPointInRect(const Vec4& rect, const Vec2& p);

    struct Style
    {
        uint32_t cornerFlags;
        float rounding;
    };

    struct Vertex
    {
        Vec2 pos;
        Color color;
        Vec2 uv;

        Vertex() : pos(), color(), uv() {}
        Vertex(const Vec2& pos, const Color& color, const Vec2& uv) : pos(pos), color(color), uv(uv) {}
    };

    enum CORNER_FLAGS
    {
        CORNER_FLAGS_NONE         = (1 << 0),
        CORNER_FLAGS_TOP_LEFT     = (1 << 1),
        CORNER_FLAGS_TOP_RIGHT    = (1 << 2),
        CORNER_FLAGS_BOTTOM_LEFT  = (1 << 3),
        CORNER_FLAGS_BOTTOM_RIGHT = (1 << 4),
        CORNER_FLAGS_ALL          = (1 << 5) - 1,
        CORNER_FLAGS_TOP          = CORNER_FLAGS_TOP_LEFT    | CORNER_FLAGS_TOP_RIGHT,
        CORNER_FLAGS_BOTTOM       = CORNER_FLAGS_BOTTOM_LEFT | CORNER_FLAGS_BOTTOM_RIGHT,
        CORNER_FLAGS_LEFT         = CORNER_FLAGS_TOP_LEFT    | CORNER_FLAGS_BOTTOM_LEFT,
        CORNER_FLAGS_RIGHT        = CORNER_FLAGS_TOP_RIGHT   | CORNER_FLAGS_BOTTOM_RIGHT
    };

    class DrawList
    {
        std::vector<Vec2> _Path;

    public:
        std::vector<Vertex> vertices;
        std::vector<IndexType> indices;

        void AddLine(const Vec2& p1, const Vec2& p2, Color color, float thickness = 1.0f);
        void AddRect(const Vec2& p1, const Vec2& p2, Color color, float rounding = 0.0f, uint32_t cornerFlags = CORNER_FLAGS_ALL, float thickness = 1.0f);
        void AddRectFilled(const Vec2& p1, const Vec2& p2, Color color, float rounding = 0.0f, uint32_t cornerFlags = CORNER_FLAGS_ALL);
        void AddQuad(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, Color color, float thickness = 1.0f);
        void AddQuadFilled(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, Color color);
        void AddTriangle(const Vec2& p1, const Vec2& p2, const Vec2& p3, Color color, float thickness = 1.0f);
        void AddTriangleFilled(const Vec2& p1, const Vec2& p2, const Vec2& p3, Color color);
        void AddCircle(const Vec2& p, float radius, Color color, size_t numSegments = 12, float thickness = 1.0f);
        void AddCircleFilled(const Vec2& p, float radius, Color color, size_t numSegments = 12);
        void AddNgon(const Vec2& center, float radius, Color color, int numSegments, float thickness = 1.0f);
        void AddNgonFilled(const Vec2& center, float radius, Color color, int numSegments);
        void AddPolyline(const Vec2* points, size_t numPoints, Color color, bool closed, float thickness);
        void AddConvexPolyFilled(const Vec2* points, size_t numPoints, Color color);

        inline void PathLineTo(const Vec2& pos) { _Path.push_back(pos); }
        void PathRect(const Vec2& p1, const Vec2& p2, float rounding, uint32_t rounding_corners);
        inline void PathStroke(Color color, bool closed, float thickness = 1.0f) { AddPolyline(_Path.data(), _Path.size(), color, closed, thickness); _Path.clear(); }
        void PathFillConvex(Color color) { AddConvexPolyFilled(_Path.data(), _Path.size(), color); _Path.clear(); }
        void PathArcTo(const Vec2& center, float radius, float a_min, float a_max, int numSegments = 10);
        void PathArcToFast(const Vec2& center, float radius, int a_min_of_12, int a_max_of_12);

        void PrimRect(const Vec2& p1, const Vec2& p2, Color color);
    };

    struct IO
    {
        struct MouseUpEvent
        {
            int key;
            Vec2 pos;

            MouseUpEvent(int key, const Vec2& pos) : key(key), pos(pos) {}
        };

        Vec2 displaySize;

        Vec2 lastMousePos, mousePos;
        bool mouseDown[5];
        float mouseWheel;
        float mouseWheelH;
        bool keyCtrl;
        bool keyShift;
        bool keyAlt;
        bool keySuper;
        bool keysDown[512];

        Vec2 mouseClickedPos[5]; //Mouse's position when the button was last clicked

        std::vector<uint16_t> charatersQueue;
        std::queue<MouseUpEvent> mouseUpQueue;
        MouseUpEvent* firstMouseUpEvent;

        bool IsAnyMouseDown();
        void AddInputCharacter(uint16_t c);
    };

    enum CONFIG_FLAGS
    {
        CONFIG_FLAGS_ANTI_ALISED_LINES = 1
    };

    struct Config
    {
        uint32_t flags;

        Config()
            : flags(CONFIG_FLAGS_ANTI_ALISED_LINES) {}
    };

    struct Context
    {
        DrawList drawList;

        std::unordered_map<IdentifierType, Layer> layers;
        std::vector<Layer*> layersStack;
        std::unordered_map<IdentifierType, Style> styles;

        Config config;
        IO io;
    };

    enum EVENT_TYPE
    {
        EVENT_UNKNOWN      = (1 <<  0),
        EVENT_MOUSE_CLICK  = (1 <<  1),
        EVENT_MOUSE_HOVER  = (1 <<  2),
        EVENT_MOUSE_ENTER  = (1 <<  3),
        EVENT_MOUSE_LEAVE  = (1 <<  4),
        EVENT_MOUSE_DOWN   = (1 <<  5),
        EVENT_MOUSE_UP     = (1 <<  6),
        EVENT_MOUSE_SCROLL = (1 <<  7),
        EVENT_KEY_DOWN     = (1 <<  8),
        EVENT_KEY_UP       = (1 <<  9),
        EVENT_KEY_CHAR     = (1 << 10)
    };

    struct Event
    {
        uint32_t type;
        Widget* widget;
        Vec2 position;
        uint16_t key;

        Event(EVENT_TYPE type, Widget* widget, const Vec2& position, uint16_t key)
            : type(type),
            widget(widget),
            position(position),
            key(key) {}
        Event() : Event(EVENT_UNKNOWN, nullptr, Vec2(), 0) {}
    };

    class Widget
    {
    private:
        bool mouseOver, wasMouseOver;

    protected:
        std::vector<EventCallback> m_eventCallbacks;

    public:
        //WIDGET_TYPE widgetType;
        bool active;
        bool visible;
        Vec4 rect;
        Color backgroundColor;
        Style* style;

        const Widget* const parent;

        Widget(const Widget* const parent)
            : parent(parent),
            active(false),
            visible(true),
            rect(),
            backgroundColor(),
            mouseOver(false), wasMouseOver(false) {}

        void RegisterEventCallback(EVENT_TYPE eventType, void(*callback)(const Event&));
        void UnregisterEventCallback(void(*callback)(const Event&));

        virtual void Render() = 0;

        virtual void Notify();
    };

    struct EventCallback
    {
        EVENT_TYPE type;
        void(*callback)(const Event&);

        EventCallback() : type(EVENT_UNKNOWN), callback(nullptr) {};
        EventCallback(EVENT_TYPE type, void(*callback)(const Event&)) : type(type), callback(callback) {};
    };

    struct Button : Widget
    {
        const char* text;

        Button(const Widget* const parent) : Widget(parent), text(nullptr) {}
        Button(const Widget* const parent, const char* text) : Widget(parent), text(text) {}

        void Render();
    };

    struct CheckBox : Widget
    {
        bool* checked;

        CheckBox(const Widget* const parent) : Widget(parent), checked(nullptr) {}
        CheckBox(const Widget* const parent, bool* checked) : Widget(parent), checked(checked) {}

        void Render();
    };

    class Layer : public Widget
    {
        std::unordered_map<IdentifierType, Widget*> m_widgets;

        void (*m_renderCallback)(DrawList& drawList);
    public:
        Layer() : Layer(nullptr) {}
        Layer(const Widget* const parent) : Widget(parent), m_renderCallback(nullptr) {}

        void MoveToTop(bool fixLayer = false);
        void MoveToBottom(bool fixLayer = false);

        inline Widget* GetWidget(IdentifierType widgetId) { return m_widgets[widgetId]; };

        Button* AddButton(IdentifierType buttonId, const char* text, Style* style);
        CheckBox* AddCheckBox(IdentifierType checkBoxId, bool* checked);

        inline void SetRenderCallback(void (*renderCallback)(DrawList&)) { m_renderCallback = renderCallback; }

        void Render();

        void Notify() override;
    };

    Layer* GetTopLayer();
    Layer* GetBottomLayer();

    Layer* GetLayer(IdentifierType layerId);

    Style* GetStyle(IdentifierType styleId);

    Context* GetContext();

    void Render();

    bool KeyUp();

#ifdef NMD_GUI_IMPLEMENTATION
    static Context g_context;

    static Vec2 g_circleVtx12[12] = { {1.0f, 0.0f}, {0.866025388f, 0.5f}, {0.499999970f, 0.866025448f}, {-4.37113883e-08f, 1.0f}, {-0.500000060f, 0.866025388f}, {-0.866025507f, 0.499999821f}, {-1.0f, -8.74227766e-08f}, {-0.866025269f, -0.500000179f}, {-0.499999911f, -0.866025448f}, {1.19248806e-08f, -1.0f}, {0.500000358f, -0.866025209f}, {0.866025567f, -0.499999762f} };
    static uint8_t g_circleSegmentCounts[64] = { 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F, 0x10, 0x10, 0x11, 0x11, 0x11, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13, 0x14, 0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x16, 0x16, 0x16, 0x16, 0x17, 0x17, 0x17, 0x18, 0x18, 0x18, 0x18, 0x19, 0x19, 0x19, 0x19, 0x19, 0x1A, 0x1A, 0x1A, 0x1A, 0x1B, 0x1B, 0x1B, 0x1B, 0x1C };

#ifdef _WIN32
    static HWND g_hwnd;

    void Win32Init(HWND hwnd)
    {
        g_hwnd = hwnd;
    }

    void Win32NewFrame()
    {
        RECT rect;
        GetClientRect(g_hwnd, &rect);
        g_context.io.displaySize = Vec2(static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top));

        g_context.io.keyCtrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
        g_context.io.keyShift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
        g_context.io.keyAlt = (::GetKeyState(VK_MENU) & 0x8000) != 0;
        g_context.io.keySuper = false;
    }

    LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_MOUSEMOVE:
            g_context.io.lastMousePos = g_context.io.mousePos;
            g_context.io.mousePos = Vec2(static_cast<float>(lParam & 0xffff), static_cast<float>(lParam >> 16));
            return 0;

        case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
        case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
        {
            int button = 0;
            if (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK) { button = 0; }
            else if (uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONDBLCLK) { button = 1; }
            else if (uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONDBLCLK) { button = 2; }
            else if (uMsg == WM_XBUTTONDOWN || uMsg == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
            if (!g_context.io.IsAnyMouseDown() && ::GetCapture() == NULL)
                ::SetCapture(g_hwnd);
            g_context.io.mouseDown[button] = true;

            g_context.io.mouseClickedPos[button] = Vec2(static_cast<float>(lParam & 0xffff), static_cast<float>(lParam >> 16));
            return 0;
        }
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_XBUTTONUP:
        {
            int button = 0;
            if (uMsg == WM_LBUTTONUP) { button = 0; }
            else if (uMsg == WM_RBUTTONUP) { button = 1; }
            else if (uMsg == WM_MBUTTONUP) { button = 2; }
            else if (uMsg == WM_XBUTTONUP) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
            g_context.io.mouseDown[button] = false;
            g_context.io.mouseUpQueue.emplace(button, g_context.io.mousePos);
            if (!g_context.io.IsAnyMouseDown() && ::GetCapture() == g_hwnd)
                ::ReleaseCapture();
            return 0;
        }
        case WM_MOUSEWHEEL:
            g_context.io.mouseWheel += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
            return 0;
        case WM_MOUSEHWHEEL:
            g_context.io.mouseWheelH += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
            return 0;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            if (wParam < 256)
                g_context.io.keysDown[wParam] = 1;
            return 0;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            if (wParam < 256)
                g_context.io.keysDown[wParam] = 0;
            return 0;
        case WM_CHAR:
            g_context.io.AddInputCharacter(static_cast<uint16_t>(wParam));
            return 0;
        }

        return 0;
    }

#endif

#ifdef GUI_D3D9
    static LPDIRECT3DDEVICE9 g_pDevice = nullptr;
    static LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer = nullptr;
    static LPDIRECT3DINDEXBUFFER9 g_pIndexBuffer = nullptr;
    static LPDIRECT3DTEXTURE9 g_pFontTexture = nullptr;
    static size_t g_vertexBufferSize, g_indexBufferSize;

    struct CustomVertex
    {
        Vec3 pos;
        D3DCOLOR color;
        Vec2 uv;

        CustomVertex() : pos(), color(), uv() {}
        CustomVertex(const Vec3& pos, D3DCOLOR color, const Vec2& uv) : pos(pos), color(color), uv(uv) {}

        static DWORD FVF;
    };

    DWORD CustomVertex::FVF = (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

    void D3D9Init(LPDIRECT3DDEVICE9 pDevice) { g_pDevice = pDevice; }

    void D3D9SetupRenderState()
    {
        g_pDevice->SetPixelShader(NULL);
        g_pDevice->SetVertexShader(NULL);
        g_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        g_pDevice->SetRenderState(D3DRS_LIGHTING, false);
        g_pDevice->SetRenderState(D3DRS_ZENABLE, false);
        g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
        g_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
        g_pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        g_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        g_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        g_pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
        g_pDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
        g_pDevice->SetRenderState(D3DRS_FOGENABLE, false);
        g_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        g_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        g_pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        g_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        g_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        g_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        g_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        g_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

        float L = 0.5f;
        float R = g_context.io.displaySize.x + 0.5f;
        float T = 0.5f;
        float B = g_context.io.displaySize.y + 0.5f;
        D3DMATRIX mat_identity = { { { 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f } } };
        D3DMATRIX mat_projection =
        { { {
            2.0f / (R - L),   0.0f,         0.0f,  0.0f,
            0.0f,         2.0f / (T - B),   0.0f,  0.0f,
            0.0f,         0.0f,         0.5f,  0.0f,
            (L + R) / (L - R),  (T + B) / (B - T),  0.5f,  1.0f
        } } };
        g_pDevice->SetTransform(D3DTS_WORLD, &mat_identity);
        g_pDevice->SetTransform(D3DTS_VIEW, &mat_identity);
        g_pDevice->SetTransform(D3DTS_PROJECTION, &mat_projection);
    }

    void D3D9Render()
    {
        if (g_context.io.displaySize.x <= 0.0f || g_context.io.displaySize.y <= 0.0f)
            return;

        if (!g_pVertexBuffer || g_vertexBufferSize < g_context.drawList.vertices.size())
        {
            if (g_pVertexBuffer) { g_pVertexBuffer->Release(); g_pVertexBuffer = nullptr; }
            g_vertexBufferSize = g_context.drawList.vertices.size() + 5000;
            if (g_pDevice->CreateVertexBuffer(static_cast<UINT>(g_vertexBufferSize * sizeof(CustomVertex)), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, CustomVertex::FVF, D3DPOOL_DEFAULT, &g_pVertexBuffer, NULL) != D3D_OK)
                return;
        }

        if (!g_pIndexBuffer || g_indexBufferSize < g_context.drawList.indices.size())
        {
            if (g_pIndexBuffer) { g_pIndexBuffer->Release(); g_pIndexBuffer = NULL; }
            g_indexBufferSize = g_context.drawList.indices.size() + 10000;
            if (g_pDevice->CreateIndexBuffer(static_cast<UINT>(g_indexBufferSize * sizeof(IndexType)), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(IndexType) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &g_pIndexBuffer, NULL) < 0)
                return;
        }

        IDirect3DStateBlock9* stateBlock;
        if (g_pDevice->CreateStateBlock(D3DSBT_ALL, &stateBlock) != D3D_OK)
            return;

        D3DMATRIX lastWorldTransform, lastViewTransform, lastProjectionTransform;
        g_pDevice->GetTransform(D3DTS_WORLD, &lastWorldTransform);
        g_pDevice->GetTransform(D3DTS_VIEW, &lastViewTransform);
        g_pDevice->GetTransform(D3DTS_PROJECTION, &lastProjectionTransform);

        CustomVertex* pVertex;  IndexType* pIndices;
        if (g_pVertexBuffer->Lock(0, static_cast<UINT>(g_context.drawList.vertices.size() * sizeof(CustomVertex)), reinterpret_cast<void**>(&pVertex), D3DLOCK_DISCARD) != D3D_OK ||
            g_pIndexBuffer->Lock(0, static_cast<UINT>(g_context.drawList.indices.size() * sizeof(IndexType)), reinterpret_cast<void**>(&pIndices), D3DLOCK_DISCARD) != D3D_OK)
            return;

        size_t i = 0;
        for (auto& vertex : g_context.drawList.vertices)
            pVertex[i++] = CustomVertex(Vec3(vertex.pos.x, vertex.pos.y, 0.0f), D3DCOLOR_ARGB(vertex.color.a, vertex.color.r, vertex.color.g, vertex.color.b), vertex.uv);

        memcpy(pIndices, g_context.drawList.indices.data(), g_context.drawList.indices.size() * sizeof(IndexType));

        g_pVertexBuffer->Unlock(), g_pIndexBuffer->Unlock();
        g_pDevice->SetStreamSource(0, g_pVertexBuffer, 0, sizeof(CustomVertex));
        g_pDevice->SetIndices(g_pIndexBuffer);
        g_pDevice->SetFVF(CustomVertex::FVF);

        D3D9SetupRenderState();

        g_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, static_cast<UINT>(g_context.drawList.vertices.size()), 0, static_cast<UINT>(g_context.drawList.indices.size() / 3));

        g_pDevice->SetTransform(D3DTS_WORLD, &lastWorldTransform);
        g_pDevice->SetTransform(D3DTS_VIEW, &lastViewTransform);
        g_pDevice->SetTransform(D3DTS_PROJECTION, &lastProjectionTransform);

        stateBlock->Apply();
        stateBlock->Release();
    }


#elif GUI_D3D11
    static ID3D11Device* g_pDevice = nullptr;
    static ID3D11DeviceContext* g_pDeviceContext = nullptr;
    static IDXGIFactory* g_pFactory = nullptr;
    static ID3D11Buffer* g_pVertexBuffer = nullptr;
    static ID3D11Buffer* g_pIndexBuffer = nullptr;
    static ID3D10Blob* g_pVertexShaderBlob = NULL;
    static ID3D11VertexShader* g_pVertexShader = NULL;
    static ID3D11InputLayout* g_pInputLayout = NULL;
    static ID3D11Buffer* g_pVertexConstantBuffer = NULL;
    static ID3D10Blob* g_pPixelShaderBlob = NULL;
    static ID3D11PixelShader* g_pPixelShader = NULL;
    static ID3D11SamplerState* g_pFontSampler = NULL;
    static ID3D11ShaderResourceView* g_pFontTextureView = NULL;
    static ID3D11RasterizerState* g_pRasterizerState = NULL;
    static ID3D11BlendState* g_pBlendState = NULL;
    static ID3D11DepthStencilState* g_pDepthStencilState = nullptr;
    static size_t g_vertexBufferSize, g_indexBufferSize;

    void D3D11SetupRenderState()
    {
        // Setup viewport
        D3D11_VIEWPORT vp;
        memset(&vp, 0, sizeof(D3D11_VIEWPORT));
        vp.Width = g_context.io.displaySize.x;
        vp.Height = g_context.io.displaySize.y;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = vp.TopLeftY = 0;
        g_pDeviceContext->RSSetViewports(1, &vp);

        // Setup shader and vertex buffers
        unsigned int stride = sizeof(Vertex);
        unsigned int offset = 0;
        g_pDeviceContext->IASetInputLayout(g_pInputLayout);
        g_pDeviceContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
        g_pDeviceContext->IASetIndexBuffer(g_pIndexBuffer, sizeof(IndexType) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
        g_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        g_pDeviceContext->VSSetShader(g_pVertexShader, NULL, 0);
        g_pDeviceContext->VSSetConstantBuffers(0, 1, &g_pVertexConstantBuffer);
        g_pDeviceContext->PSSetShader(g_pPixelShader, NULL, 0);
        g_pDeviceContext->PSSetSamplers(0, 1, &g_pFontSampler);
        g_pDeviceContext->GSSetShader(NULL, NULL, 0);
        g_pDeviceContext->HSSetShader(NULL, NULL, 0); // In theory we should backup and restore this as well.. very infrequently used..
        g_pDeviceContext->DSSetShader(NULL, NULL, 0); // In theory we should backup and restore this as well.. very infrequently used..
        g_pDeviceContext->CSSetShader(NULL, NULL, 0); // In theory we should backup and restore this as well.. very infrequently used..

        // Setup blend state
        const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
        g_pDeviceContext->OMSetBlendState(g_pBlendState, blend_factor, 0xffffffff);
        g_pDeviceContext->OMSetDepthStencilState(g_pDepthStencilState, 0);
        g_pDeviceContext->RSSetState(g_pRasterizerState);
    }

    void D3D11Render()
    {
        if (g_context.io.displaySize.x <= 0.0f || g_context.io.displaySize.y <= 0.0f)
            return;

        if (!g_pVertexBuffer || g_vertexBufferSize < g_context.drawList.vertices.size())
        {
            if (g_pVertexBuffer) { g_pVertexBuffer->Release(); g_pVertexBuffer = nullptr; }
            g_vertexBufferSize = g_context.drawList.vertices.size() + 5000;
            D3D11_BUFFER_DESC desc;
            memset(&desc, 0, sizeof(D3D11_BUFFER_DESC));
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.ByteWidth = g_vertexBufferSize * sizeof(Vertex);
            desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;
            if (g_pDevice->CreateBuffer(&desc, NULL, &g_pVertexBuffer) != S_OK)
                return;
        }

        if (!g_pIndexBuffer || g_indexBufferSize < g_context.drawList.indices.size())
        {
            if (g_pIndexBuffer) { g_pIndexBuffer->Release(); g_pIndexBuffer = nullptr; }
            g_indexBufferSize = g_context.drawList.indices.size() + 10000;
            D3D11_BUFFER_DESC desc;
            memset(&desc, 0, sizeof(D3D11_BUFFER_DESC));
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.ByteWidth = g_indexBufferSize * sizeof(IndexType);
            desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            if (g_pDevice->CreateBuffer(&desc, NULL, &g_pIndexBuffer) != S_OK)
                return;
        }

        D3D11SetupRenderState();
    }
#endif

    static inline Vec2 operator+(const Vec2& first, const Vec2& second) { return Vec2(first.x + second.x, first.y + second.y); }
    static inline Vec2 operator-(const Vec2& first, const Vec2& second) { return Vec2(first.x - second.x, first.y - second.y); }

    static inline Vec2 operator*(const Vec2& first, const Vec2& second) { return Vec2(first.x * second.x, first.y * second.y); }
    static inline Vec2 operator*(const Vec2& first, const float second) { return Vec2(first.x + second, first.y - second); }

    bool IsPointInRect(const Vec4& rect, const Vec2& p)
    {
        return p.x >= rect.pos.x && p.x <= rect.pos.x + rect.size.x && p.y >= rect.pos.y && p.y <= rect.pos.y + rect.size.y;
    }

    void DrawList::AddRect(const Vec2& p1, const Vec2& p2, Color color, float rounding, uint32_t cornerFlags, float thickness)
    {
        if (!color.a || thickness == 0.0f)
            return;

        if (g_context.config.flags & CONFIG_FLAGS_ANTI_ALISED_LINES)
            PathRect(p1 + Vec2(0.50f, 0.50f), p2 - Vec2(0.50f, 0.50f), rounding, cornerFlags);
        else
            PathRect(p1, p2, rounding, cornerFlags);

        PathStroke(color, true, thickness);
    }

    void DrawList::AddRectFilled(const Vec2& p1, const Vec2& p2, Color color, float rounding, uint32_t cornerFlags)
    {
        if (!color.a)
            return;

        if (rounding > 0.0f)
        {
            PathRect(p1, p2, rounding, cornerFlags);
            PathFillConvex(color);
        }
        else
            PrimRect(p1, p2, color);
    }

    void DrawList::AddQuad(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, Color color, float thickness)
    {
        if (!color.a)
            return;

        PathLineTo(p1);
        PathLineTo(p2);
        PathLineTo(p3);
        PathLineTo(p4);
        PathStroke(color, true, thickness);
    }

    void DrawList::AddQuadFilled(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, Color color)
    {
        if (!color.a)
            return;

        PathLineTo(p1);
        PathLineTo(p2);
        PathLineTo(p3);
        PathLineTo(p4);
        PathFillConvex(color);
    }

    void DrawList::AddTriangle(const Vec2& p1, const Vec2& p2, const Vec2& p3, Color color, float thickness)
    {
        if (!color.a)
            return;

        PathLineTo(p1);
        PathLineTo(p2);
        PathLineTo(p3);
        PathStroke(color, true, thickness);
    }

    void DrawList::AddTriangleFilled(const Vec2& p1, const Vec2& p2, const Vec2& p3, Color color)
    {
        if (!color.a)
            return;

        PathLineTo(p1);
        PathLineTo(p2);
        PathLineTo(p3);
        PathFillConvex(color);
    }

    template<typename T> static inline T Clamp(T v, T mn, T mx) { return (v < mn) ? mn : (v > mx) ? mx : v; }
    static inline Vec2 Clamp(const Vec2& v, const Vec2& mn, Vec2 mx) { return Vec2((v.x < mn.x) ? mn.x : (v.x > mx.x) ? mx.x : v.x, (v.y < mn.y) ? mn.y : (v.y > mx.y) ? mx.y : v.y); }

#define IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MIN 12
#define IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX 512
#define IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(_RAD,_MAXERROR) Clamp((int)((3.141592653f * 2.0f) / acosf((_RAD - _MAXERROR) / _RAD)), IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MIN, IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX)

    void DrawList::AddCircle(const Vec2& p, float radius, Color color, size_t numSegments, float thickness)
    {
        if (!color.a || radius <= 0.0f)
            return;

        // Obtain segment count
        if (numSegments <= 0)
        {
            // Automatic segment count
            const int radius_idx = (int)radius - 1;
            if (radius_idx < 64)
                numSegments = g_circleSegmentCounts[radius_idx];
            else
                numSegments = IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(radius, 1.6f);
        }
        else
            numSegments = Clamp(numSegments, static_cast<size_t>(3), static_cast<size_t>(IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX));

        const float a_max = (3.141592653 * 2.0f) * ((float)numSegments - 1.0f) / (float)numSegments;
        if (numSegments == 12)
            PathArcToFast(p, radius - 0.5f, 0, 12);
        else
            PathArcTo(p, radius - 0.5f, 0.0f, a_max, numSegments - 1);
        PathStroke(color, true, thickness);
    }

    void DrawList::AddCircleFilled(const Vec2& p, float radius, Color color, size_t numSegments)
    {
        if (!color.a || radius <= 0.0f)
            return;

        // Obtain segment count
        if (numSegments <= 0)
        {
            // Automatic segment count
            const size_t radius_idx = static_cast<size_t>(radius) - 1;
            if (radius_idx < 64)
                numSegments = g_circleSegmentCounts[radius_idx]; // Use cached value
            else
                numSegments = IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(radius, 1.6f);
        }
        else
        {
            // Explicit segment count (still clamp to avoid drawing insanely tessellated shapes)
            numSegments = Clamp(numSegments, static_cast<size_t>(3), static_cast<size_t>(IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX));
        }

        // Because we are filling a closed shape we remove 1 from the count of segments/points
        const float a_max = (3.141592653f * 2.0f) * ((float)numSegments - 1.0f) / (float)numSegments;
        if (numSegments == 12)
            PathArcToFast(p, radius, 0, 12);
        else
            PathArcTo(p, radius, 0.0f, a_max, numSegments - 1);
        PathFillConvex(color);
    }

    void DrawList::AddNgon(const Vec2& center, float radius, Color color, int numSegments, float thickness)
    {
        if (!color.a || numSegments <= 2)
            return;

        // Because we are filling a closed shape we remove 1 from the count of segments/points
        const float a_max = (3.141592653 * 2.0f) * ((float)numSegments - 1.0f) / (float)numSegments;
        PathArcTo(center, radius - 0.5f, 0.0f, a_max, numSegments - 1);
        PathStroke(color, true, thickness);
    }

    void DrawList::AddNgonFilled(const Vec2& center, float radius, Color color, int numSegments)
    {
        if (!color.a || numSegments <= 2)
            return;

        // Because we are filling a closed shape we remove 1 from the count of segments/points
        const float a_max = (3.141592653 * 2.0f) * ((float)numSegments - 1.0f) / (float)numSegments;
        PathArcTo(center, radius, 0.0f, a_max, numSegments - 1);
        PathFillConvex(color);
    }

    void DrawList::PathRect(const Vec2& p1, const Vec2& p2, float rounding, uint32_t cornerFlags)
    {
        rounding = fminf(rounding, fabsf(p2.x - p1.x) * ((cornerFlags & CORNER_FLAGS_TOP) || (cornerFlags & CORNER_FLAGS_BOTTOM) ? 0.5f : 1.0f) - 1.0f);
        rounding = fminf(rounding, fabsf(p2.y - p1.y) * ((cornerFlags & CORNER_FLAGS_LEFT) || (cornerFlags & CORNER_FLAGS_RIGHT) ? 0.5f : 1.0f) - 1.0f);

        if (rounding <= 0.0f || cornerFlags == 0)
        {
            PathLineTo(p1);
            PathLineTo(Vec2(p2.x, p1.y));
            PathLineTo(p2);
            PathLineTo(Vec2(p1.x, p2.y));
        }
        else
        {
            const float rounding_tl = (cornerFlags & CORNER_FLAGS_TOP_LEFT) ? rounding : 0.0f;
            const float rounding_tr = (cornerFlags & CORNER_FLAGS_TOP_RIGHT) ? rounding : 0.0f;
            const float rounding_br = (cornerFlags & CORNER_FLAGS_BOTTOM_RIGHT) ? rounding : 0.0f;
            const float rounding_bl = (cornerFlags & CORNER_FLAGS_BOTTOM_LEFT) ? rounding : 0.0f;
            PathArcToFast(Vec2(p1.x + rounding_tl, p1.y + rounding_tl), rounding_tl, 6, 9);
            PathArcToFast(Vec2(p2.x - rounding_tr, p1.y + rounding_tr), rounding_tr, 9, 12);
            PathArcToFast(Vec2(p2.x - rounding_br, p2.y - rounding_br), rounding_br, 0, 3);
            PathArcToFast(Vec2(p1.x + rounding_bl, p2.y - rounding_bl), rounding_bl, 3, 6);
        }
    }

    void DrawList::PathArcTo(const Vec2& center, float radius, float a_min, float a_max, int numSegments)
    {
        if (radius == 0.0f)
        {
            _Path.push_back(center);
            return;
        }

        _Path.reserve(_Path.size() + (numSegments + 1));
        for (int i = 0; i <= numSegments; i++)
        {
            const float a = a_min + ((float)i / (float)numSegments) * (a_max - a_min);
            _Path.push_back(Vec2(center.x + cosf(a) * radius, center.y + sinf(a) * radius));
        }
    }

    void DrawList::PathArcToFast(const Vec2& center, float radius, int a_min_of_12, int a_max_of_12)
    {
        if (radius == 0.0f || a_min_of_12 > a_max_of_12)
        {
            _Path.push_back(center);
            return;
        }
        _Path.reserve(_Path.size() + (a_max_of_12 - a_min_of_12 + 1));
        for (int a = a_min_of_12; a <= a_max_of_12; a++)
        {
            const Vec2& c = g_circleVtx12[a % 12];
            _Path.push_back(Vec2(center.x + c.x * radius, center.y + c.y * radius));
        }
    }

    void DrawList::PrimRect(const Vec2& p1, const Vec2& p2, Color color)
    {
        size_t nextIndex = vertices.size();

        Vec2 uv;
        vertices.emplace_back(p1, color, uv);
        vertices.emplace_back(Vec2(p2.x, p1.y), color, uv);
        vertices.emplace_back(p2, color, uv);
        vertices.emplace_back(Vec2(p1.x, p2.y), color, uv);

        indices.push_back(static_cast<IndexType>(nextIndex + 3));
        indices.push_back(static_cast<IndexType>(nextIndex + 0));
        indices.push_back(static_cast<IndexType>(nextIndex + 1));
        indices.push_back(static_cast<IndexType>(nextIndex + 3));
        indices.push_back(static_cast<IndexType>(nextIndex + 1));
        indices.push_back(static_cast<IndexType>(nextIndex + 2));
    }

    void DrawList::AddLine(const Vec2& p1, const Vec2& p2, Color color, float thickness)
    {
        if (!color.a)
            return;

        PathLineTo(p1);
        PathLineTo(p2);
        PathStroke(color, false, thickness);
    }

#define IM_NORMALIZE2F_OVER_ZERO(VX,VY) { float d2 = VX*VX + VY*VY; if (d2 > 0.0f) { float inv_len = 1.0f / sqrtf(d2); VX *= inv_len; VY *= inv_len; } }
#define IM_FIXNORMAL2F(VX,VY) { float d2 = VX*VX + VY*VY; if (d2 < 0.5f) d2 = 0.5f; float inv_lensq = 1.0f / d2; VX *= inv_lensq; VY *= inv_lensq; }

    void DrawList::AddPolyline(const Vec2* points, size_t numPoints, Color color, bool closed, float thickness)
    {
        if (numPoints < 2)
            return;

        size_t nextIndex = vertices.size();
        const float halfThickness = thickness / 2;

        size_t count = numPoints;
        if (!closed)
            count = numPoints - 1;

        const bool thick_line = thickness > 1.0f;
        if (g_context.config.flags & CONFIG_FLAGS_ANTI_ALISED_LINES)
        {
            // Anti-aliased stroke
            const float AA_SIZE = 1.0f;
            const Color color_trans = Color(color.r, color.g, color.b, 0);

            const size_t idx_count = thick_line ? count * 18 : count * 12;
            const size_t vtx_count = thick_line ? numPoints * 4 : numPoints * 3;

            // Temporary buffer
            Vec2* temp_normals = reinterpret_cast<Vec2*>(alloca(numPoints * (thick_line ? 5 : 3) * sizeof(Vec2)));
            Vec2* temp_points = temp_normals + numPoints;

            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1 + 1) == numPoints ? 0 : i1 + 1;
                float dx = points[i2].x - points[i1].x;
                float dy = points[i2].y - points[i1].y;
                IM_NORMALIZE2F_OVER_ZERO(dx, dy);
                temp_normals[i1].x = dy;
                temp_normals[i1].y = -dx;
            }
            if (!closed)
                temp_normals[numPoints - 1] = temp_normals[numPoints - 2];

            if (!thick_line)
            {
                if (!closed)
                {
                    temp_points[0] = points[0] + temp_normals[0] * AA_SIZE;
                    temp_points[1] = points[0] - temp_normals[0] * AA_SIZE;
                    temp_points[(numPoints - 1) * 2 + 0] = points[numPoints - 1] + temp_normals[numPoints - 1] * AA_SIZE;
                    temp_points[(numPoints - 1) * 2 + 1] = points[numPoints - 1] - temp_normals[numPoints - 1] * AA_SIZE;
                }

                size_t idx1 = vertices.size();
                for (int i1 = 0; i1 < count; i1++)
                {
                    const int i2 = (i1 + 1) == numPoints ? 0 : i1 + 1;
                    unsigned int idx2 = (i1 + 1) == numPoints ? vertices.size() : idx1 + 3;

                    // Average normals
                    float dm_x = (temp_normals[i1].x + temp_normals[i2].x) * 0.5f;
                    float dm_y = (temp_normals[i1].y + temp_normals[i2].y) * 0.5f;
                    IM_FIXNORMAL2F(dm_x, dm_y)
                        dm_x *= AA_SIZE;
                    dm_y *= AA_SIZE;

                    // Add temporary vertexes
                    Vec2* out_vtx = &temp_points[i2 * 2];
                    out_vtx[0].x = points[i2].x + dm_x;
                    out_vtx[0].y = points[i2].y + dm_y;
                    out_vtx[1].x = points[i2].x - dm_x;
                    out_vtx[1].y = points[i2].y - dm_y;

                    // Add indexes
                    indices.push_back(idx2 + 0); indices.push_back(idx1 + 0); indices.push_back(idx1 + 2);
                    indices.push_back(idx1 + 2); indices.push_back(idx2 + 2); indices.push_back(idx2 + 0);
                    indices.push_back(idx2 + 1); indices.push_back(idx1 + 1); indices.push_back(idx1 + 0);
                    indices.push_back(idx1 + 0); indices.push_back(idx2 + 0); indices.push_back(idx2 + 1);
                    idx1 = idx2;
                }

                // Add vertexes
                for (int i = 0; i < numPoints; i++)
                {
                    vertices.emplace_back(points[i], color, Vec2());
                    vertices.emplace_back(temp_points[i * 2 + 0], color_trans, Vec2());
                    vertices.emplace_back(temp_points[i * 2 + 1], color_trans, Vec2());
                }
            }
            else
            {
                const float half_inner_thickness = (thickness - AA_SIZE) * 0.5f;
                if (!closed)
                {
                    temp_points[0] = points[0] + temp_normals[0] * (half_inner_thickness + AA_SIZE);
                    temp_points[1] = points[0] + temp_normals[0] * (half_inner_thickness);
                    temp_points[2] = points[0] - temp_normals[0] * (half_inner_thickness);
                    temp_points[3] = points[0] - temp_normals[0] * (half_inner_thickness + AA_SIZE);
                    temp_points[(numPoints - 1) * 4 + 0] = points[numPoints - 1] + temp_normals[numPoints - 1] * (half_inner_thickness + AA_SIZE);
                    temp_points[(numPoints - 1) * 4 + 1] = points[numPoints - 1] + temp_normals[numPoints - 1] * (half_inner_thickness);
                    temp_points[(numPoints - 1) * 4 + 2] = points[numPoints - 1] - temp_normals[numPoints - 1] * (half_inner_thickness);
                    temp_points[(numPoints - 1) * 4 + 3] = points[numPoints - 1] - temp_normals[numPoints - 1] * (half_inner_thickness + AA_SIZE);
                }

                // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
                uint32_t idx1 = vertices.size();
                for (int i1 = 0; i1 < count; i1++)
                {
                    const int i2 = (i1 + 1) == numPoints ? 0 : i1 + 1;
                    unsigned int idx2 = (i1 + 1) == numPoints ? idx1 : idx1 + 4;

                    // Average normals
                    float dm_x = (temp_normals[i1].x + temp_normals[i2].x) * 0.5f;
                    float dm_y = (temp_normals[i1].y + temp_normals[i2].y) * 0.5f;
                    IM_FIXNORMAL2F(dm_x, dm_y);
                    float dm_out_x = dm_x * (half_inner_thickness + AA_SIZE);
                    float dm_out_y = dm_y * (half_inner_thickness + AA_SIZE);
                    float dm_in_x = dm_x * half_inner_thickness;
                    float dm_in_y = dm_y * half_inner_thickness;

                    // Add temporary vertexes
                    Vec2* out_vtx = &temp_points[i2 * 4];
                    out_vtx[0].x = points[i2].x + dm_out_x;
                    out_vtx[0].y = points[i2].y + dm_out_y;
                    out_vtx[1].x = points[i2].x + dm_in_x;
                    out_vtx[1].y = points[i2].y + dm_in_y;
                    out_vtx[2].x = points[i2].x - dm_in_x;
                    out_vtx[2].y = points[i2].y - dm_in_y;
                    out_vtx[3].x = points[i2].x - dm_out_x;
                    out_vtx[3].y = points[i2].y - dm_out_y;

                    // Add indexes
                    indices.push_back(idx2 + 1); indices.push_back(idx1 + 1); indices.push_back(idx1 + 2);
                    indices.push_back(idx1 + 2); indices.push_back(idx2 + 2); indices.push_back(idx2 + 1);
                    indices.push_back(idx2 + 1); indices.push_back(idx1 + 1); indices.push_back(idx1 + 0);
                    indices.push_back(idx1 + 0); indices.push_back(idx2 + 0); indices.push_back(idx2 + 1);
                    indices.push_back(idx2 + 2); indices.push_back(idx1 + 2); indices.push_back(idx1 + 3);
                    indices.push_back(idx1 + 3); indices.push_back(idx2 + 3); indices.push_back(idx2 + 2);

                    idx1 = idx2;
                }

                // Add vertexes
                for (int i = 0; i < numPoints; i++)
                {
                    vertices.emplace_back(temp_points[i * 4 + 0], color_trans, Vec2());
                    vertices.emplace_back(temp_points[i * 4 + 1], color, Vec2());
                    vertices.emplace_back(temp_points[i * 4 + 2], color, Vec2());
                    vertices.emplace_back(temp_points[i * 4 + 3], color_trans, Vec2());
                }
            }
        }
        else
        {
            for (size_t i = 0; i < count; i++, nextIndex += 4)
            {
                const Vec2& p1 = points[i], & p2 = points[(i + 1) == numPoints ? 0 : i + 1];

                const float dx = p2.x - p1.x, dy = p2.y - p1.y;
                const float angle = dx == 0 ? (dx > 0 ? -1 : 1) : atanf(dy / dx);
                const float yFactor = (dx == 0 ? 0 : 1) * halfThickness;

                vertices.emplace_back(Vec2(p1.x + angle * halfThickness, p1.y - yFactor), color, Vec2());
                vertices.emplace_back(Vec2(p1.x - angle * halfThickness, p1.y + yFactor), color, Vec2());
                vertices.emplace_back(Vec2(p2.x + angle * halfThickness, p2.y - yFactor), color, Vec2());
                vertices.emplace_back(Vec2(p2.x - angle * halfThickness, p2.y + yFactor), color, Vec2());

                indices.push_back(nextIndex + 0);
                indices.push_back(nextIndex + 1);
                indices.push_back(nextIndex + 2);
                indices.push_back(nextIndex + 1);
                indices.push_back(nextIndex + 2);
                indices.push_back(nextIndex + 3);
            }
        }
    }

    void DrawList::AddConvexPolyFilled(const Vec2* points, size_t numPoints, Color color)
    {
        if (numPoints < 3)
            return;

        const size_t nextIndex = vertices.size();

        if (g_context.config.flags & CONFIG_FLAGS_ANTI_ALISED_LINES)
        {
            // Anti-aliased Fill
            const float AA_SIZE = 1.0f;
            const Color color_trans = Color(color.r, color.g, color.b, 0);
            const int idx_count = (numPoints - 2) * 3 + numPoints * 6;
            const int vtx_count = (numPoints * 2);

            // Add indexes for fill
            unsigned int vtx_inner_idx = nextIndex;
            unsigned int vtx_outer_idx = nextIndex + 1;
            for (int i = 2; i < numPoints; i++)
            {
                indices.push_back(vtx_inner_idx); indices.push_back(vtx_inner_idx + ((i - 1) << 1)); indices.push_back(vtx_inner_idx + (i << 1));
            }

            // Compute normals
            Vec2* temp_normals = reinterpret_cast<Vec2*>(alloca(numPoints * sizeof(Vec2))); //-V630
            for (int i0 = numPoints - 1, i1 = 0; i1 < numPoints; i0 = i1++)
            {
                const Vec2& p0 = points[i0];
                const Vec2& p1 = points[i1];
                float dx = p1.x - p0.x;
                float dy = p1.y - p0.y;
                IM_NORMALIZE2F_OVER_ZERO(dx, dy);
                temp_normals[i0].x = dy;
                temp_normals[i0].y = -dx;
            }

            for (int i0 = numPoints - 1, i1 = 0; i1 < numPoints; i0 = i1++)
            {
                // Average normals
                const Vec2& n0 = temp_normals[i0];
                const Vec2& n1 = temp_normals[i1];
                float dm_x = (n0.x + n1.x) * 0.5f;
                float dm_y = (n0.y + n1.y) * 0.5f;
                IM_FIXNORMAL2F(dm_x, dm_y);
                dm_x *= AA_SIZE * 0.5f;
                dm_y *= AA_SIZE * 0.5f;

                // Add vertices
                vertices.emplace_back(Vec2(points[i1].x - dm_x, points[i1].y - dm_y), color, Vec2());
                vertices.emplace_back(Vec2(points[i1].x + dm_x, points[i1].y + dm_y), color_trans, Vec2());

                // Add indexes for fringes
                indices.push_back(vtx_inner_idx + (i1 << 1)); indices.push_back(vtx_inner_idx + (i0 << 1)); indices.push_back(vtx_outer_idx + (i0 << 1));
                indices.push_back(vtx_outer_idx + (i0 << 1)); indices.push_back(vtx_outer_idx + (i1 << 1)); indices.push_back(vtx_inner_idx + (i1 << 1));
            }
            //_VtxCurrentIdx += (ImDrawIdx)vtx_count;
        }
        else
        {
            const size_t idx_count = (numPoints - 2) * 3;
            const size_t vtx_count = numPoints;
            for (int i = 0; i < vtx_count; i++)
                vertices.emplace_back(points[i], color, Vec2());

            for (int i = 2; i < numPoints; i++)
            {
                indices.push_back(nextIndex); indices.push_back(nextIndex + i - 1); indices.push_back(nextIndex + i);
            }
        }
    }

    Layer* GetLayer(IdentifierType layerId)
    {
        const bool isNewLayer = g_context.layers.find(layerId) == g_context.layers.end();
        Layer* newLayer = &g_context.layers[layerId];

        if (isNewLayer)
            g_context.layersStack.push_back(newLayer);

        return newLayer;
    }

    Layer* GetTopLayer() { return g_context.layersStack.back(); }
    Layer* GetBottomLayer() { return g_context.layersStack.front(); }
    Style* GetStyle(IdentifierType styleName) { return &g_context.styles[styleName]; }
    Context* GetContext() { return &g_context; }

    void Render()
    {
#ifdef _WIN32
        Win32NewFrame();
#endif
        g_context.drawList.vertices.clear(), g_context.drawList.indices.clear();

        bool mouseUpEvent = false;
        if (g_context.io.mouseUpQueue.size())
        {
            g_context.io.firstMouseUpEvent = &g_context.io.mouseUpQueue.front();
            mouseUpEvent = true;
        }
        else
            g_context.io.firstMouseUpEvent = nullptr;
        for (Layer* layer : g_context.layersStack)
            layer->Notify();
        if (mouseUpEvent)
            g_context.io.mouseUpQueue.pop();

        for (auto layer : g_context.layersStack)
        {
            if (layer->visible)
                layer->Render();
        }
    }

    void Widget::RegisterEventCallback(EVENT_TYPE eventType, void(*callback)(const Event&))
    {
        m_eventCallbacks.emplace_back(eventType, callback);
    }

    void Widget::UnregisterEventCallback(void(*callback)(const Event&))
    {
        for (size_t i = 0; i < m_eventCallbacks.size(); i++)
        {
            if (m_eventCallbacks[i].callback == callback)
            {
                m_eventCallbacks.erase(m_eventCallbacks.begin() + i);
                return;
            }
        }
    }

    void Widget::Notify()
    {
        wasMouseOver = mouseOver;
        mouseOver = IsPointInRect(rect, g_context.io.mousePos);
        for (auto& eventCallback : m_eventCallbacks)
        {
            switch (eventCallback.type)
            {
            case EVENT_MOUSE_HOVER:
                if (mouseOver)
                    eventCallback.callback(Event(EVENT_MOUSE_HOVER, this, g_context.io.mousePos, 0));
                break;
            case EVENT_MOUSE_ENTER:
                if (mouseOver && !wasMouseOver)
                    eventCallback.callback(Event(EVENT_MOUSE_ENTER, this, g_context.io.mousePos, 0));
                break;
            case EVENT_MOUSE_LEAVE:
                if (!mouseOver && wasMouseOver)
                    eventCallback.callback(Event(EVENT_MOUSE_ENTER, this, g_context.io.mousePos, 0));
                break;
            case EVENT_MOUSE_DOWN:
                for (int i = 0; i < 5; i++)
                {
                    if (g_context.io.mouseDown[i] && mouseOver)
                        eventCallback.callback(Event(EVENT_MOUSE_DOWN, this, g_context.io.mousePos, i));
                }
                break;
            default:
                if (g_context.io.firstMouseUpEvent && IsPointInRect(rect, g_context.io.firstMouseUpEvent->pos))
                {
                    if(eventCallback.type == EVENT_MOUSE_UP)
                        eventCallback.callback(Event(EVENT_MOUSE_UP, this, g_context.io.firstMouseUpEvent->pos, g_context.io.firstMouseUpEvent->key));
                    if (eventCallback.type == EVENT_MOUSE_CLICK)
                    {
                        if (IsPointInRect(rect, g_context.io.mouseClickedPos[g_context.io.firstMouseUpEvent->key]))
                            eventCallback.callback(Event(EVENT_MOUSE_CLICK, this, g_context.io.mousePos, 0));
                    }
                }
            }
        }
    }

    void Layer::MoveToTop(bool fixLayer)
    {
        size_t i = 0;
        for (; g_context.layersStack[i] != this; i++);

        for (size_t j = i; j < g_context.layersStack.size() - 1; j++)
            g_context.layersStack[j] = g_context.layersStack[j + 1];

        g_context.layersStack[g_context.layersStack.size() - 1] = this;
    }

    void Layer::MoveToBottom(bool fixLayer)
    {
        size_t i = 0;
        for (; g_context.layersStack[i] != this; i++);

        for (size_t j = i; j > 0; j--)
            g_context.layersStack[j] = g_context.layersStack[j - 1];

        g_context.layersStack[0] = this;
    }

    Button* Layer::AddButton(IdentifierType checkBoxId, const char* text, Style* style)
    {
        Button* button = new Button(this, text);
        button->RegisterEventCallback(EVENT_MOUSE_ENTER, [](const Event& e) {e.widget->backgroundColor = Gui::Color::Red; });
        button->RegisterEventCallback(EVENT_MOUSE_LEAVE, [](const Event& e) {e.widget->backgroundColor = Gui::Color::Blue; });
        return reinterpret_cast<Button*>(m_widgets[checkBoxId] = button);
    }

    CheckBox* Layer::AddCheckBox(IdentifierType checkBoxId, bool* checked)
    {
        return reinterpret_cast<CheckBox*>(m_widgets[checkBoxId] = new CheckBox(this, checked));
    }

    void Layer::Render()
    {
        g_context.drawList.AddRectFilled(rect.pos, rect.pos + rect.size, backgroundColor);

        for (auto& widget : m_widgets)
        {
            if (widget.second->visible)
                widget.second->Render();
        }

        if (m_renderCallback)
            m_renderCallback(g_context.drawList);
    };

    void Layer::Notify()
    {
        Widget::Notify();

        //bool isMouseOverRect = IsPointInRect(rect, g_context.io.mousePos);
        //for (auto& eventCallback : m_eventCallbacks)
        //{
        //    switch (eventCallback.type)
        //    {
        //    case EVENT_MOUSE_HOVER:
        //        if(isMouseOverRect)
        //            eventCallback.callback(Event(EVENT_MOUSE_HOVER, this, g_context.io.mousePos, 0));
        //        break;
        //    }
        //}

        return;
    }

    void Button::Render()
    {
        g_context.drawList.AddRectFilled(parent->rect.pos + rect.pos, parent->rect.pos + rect.pos + rect.size, backgroundColor, 6.0f);
    }

    void CheckBox::Render()
    {
    }

    bool IO::IsAnyMouseDown()
    {
        for (int i = 0; i < sizeof(mouseDown); i++)
        {
            if (mouseDown[i])
                return true;
        }

        return false;
    }

    void IO::AddInputCharacter(uint16_t c)
    {
        charatersQueue.push_back(c);
    }

    const Color Color::Black = Color(0, 0, 0, 255);
    const Color Color::White = Color(255, 255, 255, 255);
    const Color Color::Red = Color(255, 0, 0, 255);
    const Color Color::Green = Color(0, 255, 0, 255);
    const Color Color::Blue = Color(0, 0, 255, 255);
    const Color Color::Orange = Color(255, 165, 0, 255);
    const Color Color::Amber = Color(255, 191, 0, 255);
    const Color Color::AndroidGreen = Color(164, 198, 57, 255);
    const Color Color::Azure = Color(0, 127, 255, 255);
    const Color Color::Bronze = Color(205, 127, 50, 255);
    const Color Color::Corn = Color(251, 236, 93, 255);
    const Color Color::Emerald = Color(80, 200, 120, 255);
    const Color Color::LapisLazuli = Color(38, 97, 156, 255);
    const Color Color::Lava = Color(207, 16, 32, 255);

#endif // NMD_GUI_IMPLEMENTATION

};

#endif // NMD_GUI_H