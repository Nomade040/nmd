/* This is a C++ graphics library.

Setup:
Define the 'NMD_GRAPHICS_IMPLEMENTATION' macro in one source file before the include statement to instantiate the implementation.
#define NMD_GRAPHICS_IMPLEMENTATION
#include "nmd_graphics.hpp"

Low level overview:
The nmd::Context(acessible by nmd::GetContext()) global variable holds the state of the entire library, it
contains a nmd::DrawList variable(the only one used by the library) which holds the vertex, index and commands
buffers. Each command buffer translate to a call to a rendering's API draw function. The nmd::DrawList class has 
methods to draw basic geometry shapes(e.g. circles, rectangles and lines).

Supported rendering APIs: Direct3D 9(D3D9), Direct3D 11(D3D11).
To use a specific rendering api define the macro 'NMD_GRAPHICS_{RENDERING API}' before including "nmd_graphics.hpp".

Usage:

 - Windows:
    - Call nmd::Win32Init() on initialization.
    - Call nmd::WindowProc() in the window's procedure.

 - D3D9:
    - Call nmd::D3D9SetDevice() on initialization.
    - Call nmd::D3D9Render() after nmd::End().

 - D3D11:
    - Call nmd::D3D11SetDeviceContext() on initialization.
    - Call nmd::D3D11Render() after nmd::End()

TODO:
 - Add support for tetures in Direct3D 11.
 - Embed font rasterizer : https://github.com/nothings/stb/blob/master/stb_truetype.h.
 - Add AddText() method to DrawList.
 - Implement default widgets.
 - Add support for the remaining Rendering APIs : Direct3D 12, OpenGLand Vulkan.

Credits:
 - imgui - https://github.com/ocornut/imgui
 - stb_truetype - https://github.com/nothings/stb/blob/master/stb_truetype.h
*/

#ifndef NMD_GRAPHICS_H
#define NMD_GRAPHICS_H

// Common dependencies
#include <inttypes.h>
#include <stddef.h>
#include <vector>
#include <unordered_map>
#include <queue>
#include <math.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef NMD_GRAPHICS_D3D9
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")
#endif

#ifdef NMD_GRAPHICS_D3D11
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#endif

namespace nmd
{
//#ifdef _WIN32
//    void Win32Init(HWND hwnd);
//    LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//#endif

#ifdef NMD_GRAPHICS_D3D9
    void D3D9SetDevice(LPDIRECT3DDEVICE9 pDevice);
    void D3D9Render();
#endif

#ifdef NMD_GRAPHICS_D3D11
    void D3D11SetDeviceContext(ID3D11DeviceContext* pDeviceContext);
    void D3D11Render();
#endif
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
        Vec2(float xy) : x(xy), y(xy) {}
        Vec2(float x, float y) : x(x), y(y) {}

        Vec2 operator+(const Vec2& other) const;
        Vec2 operator-(const Vec2& other) const;
        Vec2 operator*(const Vec2& other) const;

        static Vec2 Clamp(const Vec2& x, const Vec2& low, const Vec2& high);
        static Vec2 Min(const Vec2& lhs, const Vec2& rhs);
        static Vec2 Max(const Vec2& lhs, const Vec2& rhs);
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

    enum CORNER_FLAGS
    {
        CORNER_FLAGS_NONE         = (1 << 0),
        CORNER_FLAGS_TOP_LEFT     = (1 << 1),
        CORNER_FLAGS_TOP_RIGHT    = (1 << 2),
        CORNER_FLAGS_BOTTOM_LEFT  = (1 << 3),
        CORNER_FLAGS_BOTTOM_RIGHT = (1 << 4),
        CORNER_FLAGS_ALL          = (1 << 5) - 1,
        CORNER_FLAGS_TOP          = CORNER_FLAGS_TOP_LEFT | CORNER_FLAGS_TOP_RIGHT,
        CORNER_FLAGS_BOTTOM       = CORNER_FLAGS_BOTTOM_LEFT | CORNER_FLAGS_BOTTOM_RIGHT,
        CORNER_FLAGS_LEFT         = CORNER_FLAGS_TOP_LEFT | CORNER_FLAGS_BOTTOM_LEFT,
        CORNER_FLAGS_RIGHT        = CORNER_FLAGS_TOP_RIGHT | CORNER_FLAGS_BOTTOM_RIGHT
    };

    typedef uint16_t IndexType;
    typedef void* TextureId;

    struct DrawCommand
    {
        IndexType numVertices; //numVertices uses IndexType because the number of vertices is always less or equal the number of indices.
        IndexType numIndices;
        TextureId userTextureId;

        DrawCommand(IndexType numVertices, IndexType numIndices, TextureId userTextureId)
            : numVertices(numVertices), numIndices(numIndices), userTextureId(userTextureId) {}
    };

    struct Vertex
    {
        Vec2 pos;
        Color color;
        Vec2 uv;

        Vertex() : pos(), color(), uv() {}
        Vertex(const Vec2& pos, const Color& color, const Vec2& uv) : pos(pos), color(color), uv(uv) {}
    };

    class DrawList
    {
    public:
        DrawList();
        DrawList(const DrawList&) = delete;

        Vec2 cachedCircleVertices12[12];
        uint8_t cachedCircleSegmentCounts64[64];
        float curveTessellationTolerance;
        void CalculateCircleSegments(float maxError);

        std::vector<Vec2> path;

        std::vector<Vertex> vertices;
        std::vector<IndexType> indices;

        std::vector<DrawCommand> drawCommands;
        void PushRemainingDrawCommands();
        void PushTextureDrawCommand(size_t numVertices, size_t numIndices, TextureId userTextureId);

        void AddLine(const Vec2& p1, const Vec2& p2, Color color, float thickness = 1.0f);

        void AddRect(const Vec2& p1, const Vec2& p2, Color color, float rounding = 0.0f, uint32_t cornerFlags = CORNER_FLAGS_ALL, float thickness = 1.0f);
        void AddRectFilled(const Vec2& p1, const Vec2& p2, Color color, float rounding = 0.0f, uint32_t cornerFlags = CORNER_FLAGS_ALL);
        void AddRectFilledMultiColor(const Vec2& p1, const Vec2& p2, Color colorUpperLeft, Color colorUpperRight, Color colorBottomRight, Color colorBottomLeft);

        void AddQuad(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, Color color, float thickness = 1.0f);
        void AddQuadFilled(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, Color color);

        void AddTriangle(const Vec2& p1, const Vec2& p2, const Vec2& p3, Color color, float thickness = 1.0f);
        void AddTriangleFilled(const Vec2& p1, const Vec2& p2, const Vec2& p3, Color color);

        //Set numSegments to zero(0) if you want the function to automatically determine the number of segmnts.
        void AddCircle(const Vec2& center, float radius, Color color, size_t numSegments = 12, float thickness = 1.0f);
        void AddCircleFilled(const Vec2& center, float radius, Color color, size_t numSegments = 12);

        void AddNgon(const Vec2& center, float radius, Color color, size_t numSegments, float thickness = 1.0f);
        void AddNgonFilled(const Vec2& center, float radius, Color color, size_t numSegments);

        void AddPolyline(const Vec2* points, size_t numPoints, Color color, bool closed = false, float thickness = 1.0f);
        void AddConvexPolyFilled(const Vec2* points, size_t numPoints, Color color);

        void AddBezierCurve(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, Color color, float thickness = 1.0f, size_t numSegments = 0);

        //void AddText(const Vec2& pos, Color color, const char* text, const char* textEnd = NULL);
        //void AddText(const Font* font, float fontSize, const Vec2& pos, Color color, const char* text, const char* textEnd = NULL, float wrapWidth = 0.0f);

        void AddImage(TextureId userTextureId, const Vec2& p1, const Vec2& p2, const Vec2& uv1 = Vec2(0, 0), const Vec2& uv2 = Vec2(1, 1), Color color = Color::White);
        void AddImageQuad(TextureId userTextureId, const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, const Vec2& uv1 = Vec2(0, 0), const Vec2& uv2 = Vec2(1, 0), const Vec2& uv3 = Vec2(1, 1), const Vec2& uv4 = Vec2(0, 1), Color color = Color::White);
        void AddImageRounded(TextureId userTextureId, const Vec2& p1, const Vec2& p2, float rounding, uint32_t cornerFlags = CORNER_FLAGS_ALL, const Vec2& uv1 = Vec2(0, 0), const Vec2& uv2 = Vec2(1, 1), Color color = Color::White);

        //Path API
        inline void PathLineTo(const Vec2& pos) { path.push_back(pos); }
        void PathRect(const Vec2& p1, const Vec2& p2, float rounding, uint32_t rounding_corners);

        //startAtCenter places the first vertex at the center, this can be used to create a pie chart when using PathFillConvex().
        //This functions uses twelve(12) chached vertices initialized during startup, it should be faster than PathArcTo.
        void PathArcToCached(const Vec2& center, float radius, size_t startAngleOf12, size_t endAngleOf12, bool startAtCenter = false);
        void PathArcTo(const Vec2& center, float radius, float startAngle, float endAngle, size_t numSegments = 10, bool startAtCenter = false);

        inline void PathStroke(Color color, bool closed, float thickness = 1.0f) { AddPolyline(path.data(), path.size(), color, closed, thickness); path.clear(); }
        inline void PathFillConvex(Color color) { AddConvexPolyFilled(path.data(), path.size(), color); path.clear(); }

        void PathBezierCurveTo(const Vec2& p2, const Vec2& p3, const Vec2& p4, size_t numSegments);

        void PrimRect(const Vec2& p1, const Vec2& p2, Color color);
        void PrimRectUV(const Vec2& p1, const Vec2& p2, const Vec2& uv1, const Vec2& uv2, Color color);
        void PrimQuadUV(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, const Vec2& uv1, const Vec2& uv2, const Vec2& uv3, const Vec2& uv4, Color color);
    };

    struct Context
    {
        DrawList drawList;
    };

    Context& GetContext();

    // Begin() clears all of the vertices, indices and command buffers. Call this function to start a new scene.
    void Begin();

    // End() calculates the remaining command buffers. Call this function before rendering.
    void End();

} // namespace nmd

#endif // NMD_GRAPHICS_H
