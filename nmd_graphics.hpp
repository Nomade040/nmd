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


#ifdef NMD_GRAPHICS_IMPLEMENTATION

#define NMD_PI  3.141592653f
#define NMD_2PI 6.283185306f

#define NMD_CLAMP(x, low, high) ((x < low) ? low : (x > high) ? high : x)
#define NMD_MIN(a, b) (a < b ? a : b)
#define NMD_MAX(a, b) (a > b ? a : b)

#define CIRCLE_AUTO_SEGMENT_MIN 12
#define CIRCLE_AUTO_SEGMENT_MAX 512
#define CIRCLE_AUTO_SEGMENT_CALC(radius, maxError) NMD_CLAMP(NMD_2PI / acosf((radius - maxError) / radius), CIRCLE_AUTO_SEGMENT_MIN, CIRCLE_AUTO_SEGMENT_MAX)

namespace nmd
{
	Vec2 Vec2::operator+(const Vec2& other) const { return Vec2(this->x + other.x, this->y + other.x); }
	Vec2 Vec2::operator-(const Vec2& other) const { return Vec2(this->x - other.x, this->y - other.x); }
	Vec2 Vec2::operator*(const Vec2& other) const { return Vec2(this->x * other.x, this->y * other.x); }

	Vec2 Vec2::Clamp(const Vec2& x, const Vec2& low, const Vec2& high) { return Vec2(NMD_CLAMP(x.x, low.x, high.x), NMD_CLAMP(x.y, low.y, high.y)); }
	Vec2 Vec2::Min(const Vec2& lhs, const Vec2& rhs) { return Vec2(NMD_MIN(lhs.x, rhs.x), NMD_MIN(lhs.y, rhs.y)); }
	Vec2 Vec2::Max(const Vec2& lhs, const Vec2& rhs) { return Vec2(NMD_MAX(lhs.x, rhs.x), NMD_MAX(lhs.y, rhs.y)); }

	bool IsPointInRect(const Vec4& rect, const Vec2& p) { return p.x >= rect.pos.x && p.x <= rect.pos.x + rect.size.x && p.y >= rect.pos.y && p.y <= rect.pos.y + rect.size.y; }
} // namespace nmd


namespace nmd
{
    void PathBezierToCasteljau(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, size_t level)
    {
        const float dx = x4 - x1;
        const float dy = y4 - y1;
        float d2 = ((x2 - x4) * dy - (y2 - y4) * dx);
        float d3 = ((x3 - x4) * dy - (y3 - y4) * dx);
        d2 = (d2 >= 0) ? d2 : -d2;
        d3 = (d3 >= 0) ? d3 : -d3;
        if ((d2 + d3) * (d2 + d3) < GetContext().drawList.curveTessellationTolerance * (dx * dx + dy * dy))
            GetContext().drawList.path.emplace_back(x4, y4);
        else if (level < 10)
        {
            const float x12 = (x1 + x2) * 0.5f, y12 = (y1 + y2) * 0.5f;
            const float x23 = (x2 + x3) * 0.5f, y23 = (y2 + y3) * 0.5f;
            const float x34 = (x3 + x4) * 0.5f, y34 = (y3 + y4) * 0.5f;
            const float x123 = (x12 + x23) * 0.5f, y123 = (y12 + y23) * 0.5f;
            const float x234 = (x23 + x34) * 0.5f, y234 = (y23 + y34) * 0.5f;
            const float x1234 = (x123 + x234) * 0.5f, y1234 = (y123 + y234) * 0.5f;
            PathBezierToCasteljau(x1, y1, x12, y12, x123, y123, x1234, y1234, level + 1);
            PathBezierToCasteljau(x1234, y1234, x234, y234, x34, y34, x4, y4, level + 1);
        }
    }

    Vec2 BezierCalc(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, float t)
    {
        const float u = 1.0f - t;
        const float w1 = u * u * u;
        const float w2 = 3 * u * u * t;
        const float w3 = 3 * u * t * t;
        const float w4 = t * t * t;
        return Vec2(w1 * p1.x + w2 * p2.x + w3 * p3.x + w4 * p4.x, w1 * p1.y + w2 * p2.y + w3 * p3.y + w4 * p4.y);
    }

    // Distribute UV over (a, b) rectangle
    void ShadeVertsLinearUV(size_t startVertexIndex, const Vec2& p1, const Vec2& p2, const Vec2& uv1, const Vec2& uv2, bool clamp)
    {
        const Vec2 size = p2 - p1;
        const Vec2 uv_size = uv2 - uv1;
        const Vec2 scale = Vec2(size.x != 0.0f ? (uv_size.x / size.x) : 0.0f, size.y != 0.0f ? (uv_size.y / size.y) : 0.0f);

        Vertex* const startVertex = GetContext().drawList.vertices.data() + startVertexIndex;
        const Vertex* const endVertex = &GetContext().drawList.vertices.back();
        if (clamp)
        {
            const Vec2 min = Vec2::Min(uv1, uv2), max = Vec2::Max(uv1, uv2);
            for (Vertex* vertex = startVertex; vertex < endVertex; ++vertex)
                vertex->uv = Vec2::Clamp(uv1 + ((Vec2(vertex->pos.x, vertex->pos.y) - p1) * scale), min, max);
        }
        else
        {
            for (Vertex* vertex = startVertex; vertex < endVertex; ++vertex)
                vertex->uv = uv1 + ((Vec2(vertex->pos.x, vertex->pos.y) - p1) * scale);
        }
    }

    DrawList::DrawList()
        : curveTessellationTolerance(1.25f)
    {
        for (size_t i = 0; i < 12; i++)
        {
            //const float angle = (static_cast<float>(i) / 12.0f) * GUI_2PI;
            const float angle = (static_cast<float>(i) / 6.0f) * NMD_PI; // Simplified version of the line above.
            cachedCircleVertices12[i] = Vec2(cosf(angle), sinf(angle));
        }

        CalculateCircleSegments(1.6f);
    }

    void DrawList::CalculateCircleSegments(float maxError)
    {
        for (size_t i = 0; i < 64; i++)
        {
            const size_t segment_count = CIRCLE_AUTO_SEGMENT_CALC(static_cast<float>(i) + 1.0f, maxError);
            cachedCircleSegmentCounts64[i] = NMD_MIN(segment_count, 255);
        }
    }

    void DrawList::PushRemainingDrawCommands()
    {
        size_t numAccountedVertices = 0, numAccountedIndices = 0;
        for (auto& drawCommand : drawCommands)
            numAccountedVertices += drawCommand.numVertices, numAccountedIndices += drawCommand.numIndices;

        size_t numUnaccountedIndices = indices.size() - numAccountedIndices;

        while (numUnaccountedIndices > 0)
        {
            if (numUnaccountedIndices <= (2 << (8 * sizeof(IndexType) - 1)))
            {
                drawCommands.emplace_back(static_cast<IndexType>(vertices.size() - numAccountedVertices), static_cast<IndexType>(numUnaccountedIndices), static_cast<TextureId>(NULL));
                numUnaccountedIndices = 0;
                return;
            }
            else
            {
                size_t numIndices = (2 << (8 * sizeof(IndexType) - 1)) - 1;
                IndexType lastIndex = indices[numIndices - 1];

                bool isLastIndexReferenced = false;
                do
                {
                    for (size_t i = numIndices; i < numUnaccountedIndices; i++)
                    {
                        if (indices[i] == lastIndex)
                        {
                            isLastIndexReferenced = true;
                            numIndices -= 3;
                            lastIndex = indices[numIndices - 1];
                            break;
                        }
                    }
                } while (isLastIndexReferenced);

                drawCommands.emplace_back(static_cast<IndexType>(lastIndex + 1), static_cast<IndexType>(numIndices), static_cast<TextureId>(NULL));
                numUnaccountedIndices -= numIndices;
            }
        }
    }

    void DrawList::PushTextureDrawCommand(size_t numVertices, size_t numIndices, TextureId userTextureId)
    {
        if (!drawCommands.empty() && drawCommands.back().userTextureId == userTextureId)
            drawCommands.back().numVertices += static_cast<IndexType>(numVertices), drawCommands.back().numIndices += static_cast<IndexType>(numIndices);
        else
            drawCommands.emplace_back(static_cast<IndexType>(numVertices), static_cast<IndexType>(numIndices), userTextureId);
    }

    void DrawList::AddRect(const Vec2& p1, const Vec2& p2, Color color, float rounding, uint32_t cornerFlags, float thickness)
    {
        if (!color.a || thickness == 0.0f)
            return;
        
        PathRect(p1 + Vec2(0.5f, 0.5f), p2 - Vec2(0.5f, 0.5f), rounding, cornerFlags);

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

    void DrawList::AddRectFilledMultiColor(const Vec2& p1, const Vec2& p2, Color colorUpperLeft, Color colorUpperRight, Color colorBottomRight, Color colorBottomLeft)
    {
        const IndexType nextIndex = static_cast<IndexType>(vertices.size());

        vertices.emplace_back(p1, colorUpperLeft, Vec2());
        vertices.emplace_back(Vec2(p2.x, p1.y), colorUpperRight, Vec2());
        vertices.emplace_back(p2, colorBottomRight, Vec2());
        vertices.emplace_back(Vec2(p1.x, p2.y), colorBottomLeft, Vec2());

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 1);
        indices.push_back(nextIndex + 2);

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 2);
        indices.push_back(nextIndex + 3);
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

        const IndexType nextIndex = static_cast<IndexType>(vertices.size());
        vertices.emplace_back(p1, color, Vec2());
        vertices.emplace_back(p2, color, Vec2());
        vertices.emplace_back(p3, color, Vec2());
        for (size_t i = 0; i < 3; i++)
            indices.push_back(nextIndex + static_cast<IndexType>(i));
    }

    void DrawList::AddCircle(const Vec2& center, float radius, Color color, size_t numSegments, float thickness)
    {
        if (!color.a || radius <= 0.0f)
            return;

        if (numSegments == 0)
            numSegments = (static_cast<size_t>(radius) - 1 < 64) ? cachedCircleSegmentCounts64[static_cast<size_t>(radius) - 1] : CIRCLE_AUTO_SEGMENT_CALC(radius, 1.6f);
        else
            numSegments = NMD_CLAMP(numSegments, 3, CIRCLE_AUTO_SEGMENT_MAX);

        if (numSegments == 12)
            PathArcToCached(center, radius - 0.5f, 0, 12);
        else
            PathArcTo(center, radius - 0.5f, 0.0f, NMD_2PI * (static_cast<float>(numSegments - 1) / static_cast<float>(numSegments)), numSegments - 1);

        PathStroke(color, true, thickness);
    }

    void DrawList::AddCircleFilled(const Vec2& center, float radius, Color color, size_t numSegments)
    {
        if (!color.a || radius <= 0.0f)
            return;

        if (numSegments <= 0)
            numSegments = (static_cast<size_t>(radius) - 1 < 64) ? cachedCircleSegmentCounts64[static_cast<size_t>(radius) - 1] : CIRCLE_AUTO_SEGMENT_CALC(radius, 1.6f);
        else
            numSegments = NMD_CLAMP(numSegments, 3, CIRCLE_AUTO_SEGMENT_MAX);

        if (numSegments == 12)
            PathArcToCached(center, radius, 0, 12);
        else
            PathArcTo(center, radius, 0.0f, NMD_2PI * ((static_cast<float>(numSegments) - 1.0f) / static_cast<float>(numSegments)), numSegments - 1);

        PathFillConvex(color);
    }

    void DrawList::AddNgon(const Vec2& center, float radius, Color color, size_t numSegments, float thickness)
    {
        if (!color.a || numSegments < 3)
            return;

        //Remove one(1) from numSegment because it's a closed shape.
        PathArcTo(center, radius - 0.5f, 0.0f, NMD_2PI * ((static_cast<float>(numSegments) - 1.0f) / static_cast<float>(numSegments)), numSegments - 1);
        PathStroke(color, true, thickness);
    }

    void DrawList::AddNgonFilled(const Vec2& center, float radius, Color color, size_t numSegments)
    {
        if (!color.a || numSegments < 3)
            return;

        //Remove one(1) from numSegment because it's a closed shape.
        PathArcTo(center, radius, 0.0f, NMD_2PI * ((static_cast<float>(numSegments) - 1.0f) / static_cast<float>(numSegments)), numSegments - 1);
        PathFillConvex(color);
    }

    void DrawList::PathRect(const Vec2& p1, const Vec2& p2, float rounding, uint32_t cornerFlags)
    {
        if (rounding <= 0.0f || cornerFlags == 0)
        {
            PathLineTo(p1);
            PathLineTo(Vec2(p2.x, p1.y));
            PathLineTo(p2);
            PathLineTo(Vec2(p1.x, p2.y));
        }
        else
        {
            const float roundingTopLeft = (cornerFlags & CORNER_FLAGS_TOP_LEFT) ? rounding : 0.0f;
            const float roundingTopRight = (cornerFlags & CORNER_FLAGS_TOP_RIGHT) ? rounding : 0.0f;
            const float roundingBottomRight = (cornerFlags & CORNER_FLAGS_BOTTOM_RIGHT) ? rounding : 0.0f;
            const float roundingBottomLeft = (cornerFlags & CORNER_FLAGS_BOTTOM_LEFT) ? rounding : 0.0f;
            PathArcToCached(Vec2(p1.x + roundingTopLeft, p1.y + roundingTopLeft), roundingTopLeft, 6, 9);
            PathArcToCached(Vec2(p2.x - roundingTopRight, p1.y + roundingTopRight), roundingTopRight, 9, 12);
            PathArcToCached(Vec2(p2.x - roundingBottomRight, p2.y - roundingBottomRight), roundingBottomRight, 0, 3);
            PathArcToCached(Vec2(p1.x + roundingBottomLeft, p2.y - roundingBottomLeft), roundingBottomLeft, 3, 6);
        }
    }

    void DrawList::PathArcTo(const Vec2& center, float radius, float startAngle, float endAngle, size_t numSegments, bool startAtCenter)
    {
        if (startAtCenter)
            path.push_back(center);

        for (size_t i = 0; i <= numSegments; i++)
        {
            const float angle = startAngle + (static_cast<float>(i) / static_cast<float>(numSegments)) * (endAngle - startAngle);
            path.emplace_back(center.x + cosf(angle) * radius, center.y + sinf(angle) * radius);
        }
    }

    void DrawList::PathArcToCached(const Vec2& center, float radius, size_t startAngleOf12, size_t endAngleOf12, bool startAtCenter)
    {
        if (startAtCenter)
            path.push_back(center);

        for (size_t angle = startAngleOf12; angle <= endAngleOf12; angle++)
        {
            const Vec2& point = cachedCircleVertices12[angle % 12];
            path.emplace_back(center.x + point.x * radius, center.y + point.y * radius);
        }
    }

    void DrawList::PathBezierCurveTo(const Vec2& p2, const Vec2& p3, const Vec2& p4, size_t numSegments)
    {
        const Vec2& p1 = path.back();
        if (numSegments == 0)
            PathBezierToCasteljau(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, 0);
        else
        {
            const float tStep = 1.0f / static_cast<float>(numSegments);
            for (size_t iStep = 1; iStep <= numSegments; iStep++)
                path.push_back(BezierCalc(p1, p2, p3, p4, tStep * iStep));
        }
    }

    void DrawList::PrimRect(const Vec2& p1, const Vec2& p2, Color color)
    {
        const IndexType nextIndex = static_cast<IndexType>(vertices.size());

        vertices.emplace_back(p1, color, Vec2());
        vertices.emplace_back(Vec2(p2.x, p1.y), color, Vec2());
        vertices.emplace_back(p2, color, Vec2());
        vertices.emplace_back(Vec2(p1.x, p2.y), color, Vec2());

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 1);
        indices.push_back(nextIndex + 2);

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 2);
        indices.push_back(nextIndex + 3);
    }

    void DrawList::PrimRectUV(const Vec2& p1, const Vec2& p2, const Vec2& uv1, const Vec2& uv2, Color color)
    {
        const IndexType nextIndex = static_cast<IndexType>(vertices.size());

        vertices.emplace_back(p1, color, uv1);
        vertices.emplace_back(Vec2(p2.x, p1.y), color, Vec2(uv2.x, uv1.y));
        vertices.emplace_back(p2, color, uv2);
        vertices.emplace_back(Vec2(p1.x, p2.y), color, Vec2(uv1.x, uv2.y));

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 1);
        indices.push_back(nextIndex + 2);

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 2);
        indices.push_back(nextIndex + 3);
    }

    void DrawList::PrimQuadUV(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, const Vec2& uv1, const Vec2& uv2, const Vec2& uv3, const Vec2& uv4, Color color)
    {
        const IndexType nextIndex = static_cast<IndexType>(vertices.size());

        vertices.emplace_back(p1, color, uv1);
        vertices.emplace_back(p2, color, uv2);
        vertices.emplace_back(p3, color, uv3);
        vertices.emplace_back(p4, color, uv4);

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 1);
        indices.push_back(nextIndex + 2);

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 2);
        indices.push_back(nextIndex + 3);
    }

    void DrawList::AddLine(const Vec2& p1, const Vec2& p2, Color color, float thickness)
    {
        if (!color.a)
            return;

        PathLineTo(p1 + Vec2(0.5f, 0.5f));
        PathLineTo(p2 + Vec2(0.5f, 0.5f));
        PathStroke(color, false, thickness);
    }

    void DrawList::AddPolyline(const Vec2* points, size_t numPoints, Color color, bool closed, float thickness)
    {
        if (numPoints < 2)
            return;

        IndexType nextIndex = static_cast<IndexType>(vertices.size());
        const float halfThickness = (thickness * 0.5f);
        for (size_t i = 0; i < (closed ? numPoints : numPoints - 1); i++, nextIndex += 4)
        {
            const Vec2& p1_tmp = points[i], & p2_tmp = points[(i + 1) == numPoints ? 0 : i + 1];
            const float dx = p2_tmp.x - p1_tmp.x;
            const float dy = p2_tmp.y - p1_tmp.y;

            //If we didn't swap the points in the cases the triangles would be drawn in the counter clockwise direction, which can cause problems in some rendering APIs.
            const bool swapPoints = (dx < 0.0f || dy < 0.0f) || (dx > 0.0f && dy > 0.0f);
            const Vec2& p1 = swapPoints ? p2_tmp : p1_tmp, & p2 = swapPoints ? p1_tmp : p2_tmp;

            if (dy == 0) // Horizontal line
            {
                int factor = dx > 0.0f ? 1 : -1;
                vertices.emplace_back(Vec2(p1.x - halfThickness * factor, p1.y - halfThickness), color, Vec2());
                vertices.emplace_back(Vec2(p2.x + halfThickness * factor, p2.y - halfThickness), color, Vec2());
                vertices.emplace_back(Vec2(p2.x + halfThickness * factor, p2.y + halfThickness), color, Vec2());
                vertices.emplace_back(Vec2(p1.x - halfThickness * factor, p1.y + halfThickness), color, Vec2());
            }
            else if (dx == 0) // Vertical line
            {
                int factor = dy > 0.0f ? 1 : -1;
                vertices.emplace_back(Vec2(p1.x + halfThickness, p1.y - halfThickness * factor), color, Vec2());
                vertices.emplace_back(Vec2(p2.x + halfThickness, p2.y + halfThickness * factor), color, Vec2());
                vertices.emplace_back(Vec2(p2.x - halfThickness, p2.y + halfThickness * factor), color, Vec2());
                vertices.emplace_back(Vec2(p1.x - halfThickness, p1.y - halfThickness * factor), color, Vec2());
            }
            else // Inclined line
            {
                const float lineWidth = sqrtf(dx * dx + dy * dy);

                const float cosine = dx / lineWidth;
                const float sine = dy / lineWidth;

                const float xFactor = cosine * halfThickness;
                const float yFactor = sine * halfThickness;

                vertices.emplace_back(Vec2(p1.x - yFactor, p1.y + xFactor), color, Vec2());
                vertices.emplace_back(Vec2(p2.x - yFactor, p2.y + xFactor), color, Vec2());
                vertices.emplace_back(Vec2(p2.x + yFactor, p2.y - xFactor), color, Vec2());
                vertices.emplace_back(Vec2(p1.x + yFactor, p1.y - xFactor), color, Vec2());
            }

            indices.push_back(nextIndex + 0);
            indices.push_back(nextIndex + 1);
            indices.push_back(nextIndex + 2);

            indices.push_back(nextIndex + 0);
            indices.push_back(nextIndex + 2);
            indices.push_back(nextIndex + 3);
        }
    }

    void DrawList::AddConvexPolyFilled(const Vec2* points, size_t numPoints, Color color)
    {
        if (numPoints < 3)
            return;

        const IndexType nextIndex = static_cast<IndexType>(vertices.size());
        for (size_t i = 0; i < numPoints; i++)
            vertices.emplace_back(points[i], color, Vec2());

        for (size_t i = 2; i < numPoints; i++)
            indices.push_back(nextIndex), indices.push_back(nextIndex + static_cast<IndexType>(i - 1)), indices.push_back(nextIndex + static_cast<IndexType>(i));
    }

    void DrawList::AddBezierCurve(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, Color color, float thickness, size_t numSegments)
    {
        if (!color.a)
            return;

        PathLineTo(p1);
        PathBezierCurveTo(p2, p3, p4, numSegments);
        PathStroke(color, false, thickness);
    }

    //void DrawList::AddText(const Vec2& pos, Color color, const char* text, const char* textEnd)
    //{
    //    AddText(NULL, 0.0f, pos, color, text, textEnd);
    //}
    //
    //void DrawList::AddText(const Font* font, float fontSize, const Vec2& pos, Color color, const char* text, const char* textEnd, float wrapWidth)
    //{
    //    //if (!color.a || text == textEnd)
    //    //    return;
    //    //
    //    //if (!textEnd)
    //    //    textEnd = text + strlen(text);
    //    //
    //    //while (text < textEnd)
    //    //{
    //    //    const Glyph* glyph = font->FindGlyph(*text);
    //    //
    //    //    const size_t nextIndex = vertices.size();
    //    //    vertices.emplace_back(Vec2(glyph->x0, glyph->y0), color, Vec2(glyph->u0, glyph->v0));
    //    //    vertices.emplace_back(Vec2(glyph->x1, glyph->y0), color, Vec2(glyph->u1, glyph->v0));
    //    //    vertices.emplace_back(Vec2(glyph->x1, glyph->y1), color, Vec2(glyph->u1, glyph->v1));
    //    //    vertices.emplace_back(Vec2(glyph->x0, glyph->y1), color, Vec2(glyph->u0, glyph->v1));
    //    //
    //    //    indices.push_back(nextIndex + 0);
    //    //    indices.push_back(nextIndex + 1);
    //    //    indices.push_back(nextIndex + 2);
    //    //
    //    //    indices.push_back(nextIndex + 0);
    //    //    indices.push_back(nextIndex + 2);
    //    //    indices.push_back(nextIndex + 3);
    //    //
    //    //    text++;
    //    //}
    //}

    void DrawList::AddImage(TextureId userTextureId, const Vec2& p1, const Vec2& p2, const Vec2& uv1, const Vec2& uv2, Color color)
    {
        if (!color.a)
            return;

        PushRemainingDrawCommands();

        PrimRectUV(p1, p2, uv1, uv2, color);

        PushTextureDrawCommand(4, 6, userTextureId);
    }

    void DrawList::AddImageQuad(TextureId userTextureId, const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, const Vec2& uv1, const Vec2& uv2, const Vec2& uv3, const Vec2& uv4, Color color)
    {
        if (!color.a)
            return;

        PushRemainingDrawCommands();

        PrimQuadUV(p1, p2, p3, p4, uv1, uv2, uv3, uv4, color);

        PushTextureDrawCommand(4, 6, userTextureId);
    }

    void DrawList::AddImageRounded(TextureId userTextureId, const Vec2& p1, const Vec2& p2, float rounding, uint32_t cornerFlags, const Vec2& uv1, const Vec2& uv2, Color color)
    {
        if (!color.a)
            return;

        if (rounding <= 0.0f || !cornerFlags)
            AddImage(userTextureId, p1, p2, uv1, uv2, color);
        else
        {
            PushRemainingDrawCommands();

            PathRect(p1, p2, rounding, cornerFlags);

            const size_t v0 = vertices.size(), i0 = indices.size();
            PathFillConvex(color);
            ShadeVertsLinearUV(v0, p1, p2, uv1, uv2, true);

            PushTextureDrawCommand(vertices.size() - v0, indices.size() - i0, userTextureId);
        }
    }

} // namespace nmd

namespace nmd
{
    static Context g_context;

    Context& GetContext() { return g_context; }

    void Begin()
    {
        g_context.drawList.vertices.clear();
        g_context.drawList.indices.clear();
        g_context.drawList.drawCommands.clear();
    }

    void End()
    {
        g_context.drawList.PushRemainingDrawCommands();
    }

} // namespace nmd


namespace nmd
{
	const Color Color::Black        = Color(0,   0,   0,   255);
	const Color Color::White        = Color(255, 255, 255, 255);
	const Color Color::Red          = Color(255, 0,   0,   255);
	const Color Color::Green        = Color(0,   255, 0,   255);
	const Color Color::Blue         = Color(0,   0,   255, 255);
	const Color Color::Orange       = Color(255, 165, 0,   255);
	const Color Color::Amber        = Color(255, 191, 0,   255);
	const Color Color::AndroidGreen = Color(164, 198, 57,  255);
	const Color Color::Azure        = Color(0,   127, 255, 255);
	const Color Color::Bronze       = Color(205, 127, 50,  255);
	const Color Color::Corn         = Color(251, 236, 93,  255);
	const Color Color::Emerald      = Color(80,  200, 120, 255);
	const Color Color::LapisLazuli  = Color(38,  97,  156, 255);
	const Color Color::Lava         = Color(207, 16,  32,  255);
} // namespace nmd

namespace nmd
{

#ifdef NMD_GRAPHICS_D3D9
    static LPDIRECT3DDEVICE9 g_pD3D9Device = nullptr;
    static LPDIRECT3DVERTEXBUFFER9 g_pD3D9VertexBuffer = nullptr;
    static LPDIRECT3DINDEXBUFFER9 g_pD3D9IndexBuffer = nullptr;
    //static LPDIRECT3DTEXTURE9 g_pFontTexture = nullptr;
    static size_t g_D3D9VertexBufferSize, g_D3D9IndexBufferSize;

    struct CustomVertex
    {
        Vec3 pos;
        D3DCOLOR color;
        Vec2 uv;

        CustomVertex() : pos(), color(), uv() {}
        CustomVertex(const Vec3& pos, D3DCOLOR color, const Vec2& uv) : pos(pos), color(color), uv(uv) {}

        static DWORD FVF;
    };

    DWORD CustomVertex::FVF = (D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_DIFFUSE);

    void D3D9SetDevice(LPDIRECT3DDEVICE9 pD3D9Device) { g_pD3D9Device = pD3D9Device; }

    void D3D9Render()
    {
        const Context& context = GetContext();

        // Don't render if screen is minimized.
        //if (context.io.displaySize.x <= 0.0f || context.io.displaySize.y <= 0.0f)
        //    return;

        // Create/recreate vertex/index buffer if it doesn't exist or more space is needed.
        if (!g_pD3D9VertexBuffer || g_D3D9VertexBufferSize < context.drawList.vertices.size())
        {
            if (g_pD3D9VertexBuffer)
                g_pD3D9VertexBuffer->Release(), g_pD3D9VertexBuffer = NULL;

            g_D3D9VertexBufferSize = context.drawList.vertices.size() + 5000;
            if (g_pD3D9Device->CreateVertexBuffer(static_cast<UINT>(g_D3D9VertexBufferSize * sizeof(CustomVertex)), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, CustomVertex::FVF, D3DPOOL_DEFAULT, &g_pD3D9VertexBuffer, NULL) != D3D_OK)
                return;
        }

        if (!g_pD3D9IndexBuffer || g_D3D9IndexBufferSize < context.drawList.indices.size())
        {
            if (g_pD3D9IndexBuffer)
                g_pD3D9IndexBuffer->Release(), g_pD3D9IndexBuffer = NULL;

            g_D3D9IndexBufferSize = context.drawList.indices.size() + 10000;
            if (g_pD3D9Device->CreateIndexBuffer(static_cast<UINT>(g_D3D9IndexBufferSize * sizeof(IndexType)), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(IndexType) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &g_pD3D9IndexBuffer, NULL) < 0)
                return;
        }
        
        void* pData = NULL;

        // Copy vertices to the gpu
        if (g_pD3D9VertexBuffer->Lock(0, static_cast<UINT>(context.drawList.vertices.size() * sizeof(CustomVertex)), &pData, D3DLOCK_DISCARD) != D3D_OK)
            return;
        size_t i = 0;
        for (auto& vertex : context.drawList.vertices)
            reinterpret_cast<CustomVertex*>(pData)[i++] = CustomVertex(Vec3(vertex.pos.x, vertex.pos.y, 0.0f), D3DCOLOR_ARGB(vertex.color.a, vertex.color.r, vertex.color.g, vertex.color.b), vertex.uv);
        g_pD3D9VertexBuffer->Unlock();

        // Copy indices to the gpu.
        if (g_pD3D9IndexBuffer->Lock(0, static_cast<UINT>(context.drawList.indices.size() * sizeof(IndexType)), &pData, D3DLOCK_DISCARD) != D3D_OK)
            return;
        memcpy(pData, context.drawList.indices.data(), context.drawList.indices.size() * sizeof(IndexType));
        g_pD3D9IndexBuffer->Unlock();
        
        // Backup current render state.
        IDirect3DStateBlock9* stateBlock;
        D3DMATRIX lastWorldTransform, lastViewTransform, lastProjectionTransform;

        if (g_pD3D9Device->CreateStateBlock(D3DSBT_ALL, &stateBlock) != D3D_OK)
            return;

        g_pD3D9Device->GetTransform(D3DTS_WORLD, &lastWorldTransform);
        g_pD3D9Device->GetTransform(D3DTS_VIEW, &lastViewTransform);
        g_pD3D9Device->GetTransform(D3DTS_PROJECTION, &lastProjectionTransform);
        
        // Set render state.
        g_pD3D9Device->SetStreamSource(0, g_pD3D9VertexBuffer, 0, sizeof(CustomVertex));
        g_pD3D9Device->SetIndices(g_pD3D9IndexBuffer);
        g_pD3D9Device->SetFVF(CustomVertex::FVF);
        g_pD3D9Device->SetPixelShader(NULL);
        g_pD3D9Device->SetVertexShader(NULL);
        g_pD3D9Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        g_pD3D9Device->SetRenderState(D3DRS_LIGHTING, false);
        g_pD3D9Device->SetRenderState(D3DRS_ZENABLE, false);
        g_pD3D9Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
        g_pD3D9Device->SetRenderState(D3DRS_ALPHATESTENABLE, false);
        g_pD3D9Device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        g_pD3D9Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        g_pD3D9Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        g_pD3D9Device->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
        g_pD3D9Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
        g_pD3D9Device->SetRenderState(D3DRS_FOGENABLE, false);
        g_pD3D9Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        g_pD3D9Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        g_pD3D9Device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        g_pD3D9Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        g_pD3D9Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        g_pD3D9Device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        g_pD3D9Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        g_pD3D9Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

        float L = 0.5f;
        float R = 800.5f; //context.io.displaySize.x + 0.5f;
        float T = 0.5f;
        float B = 600.5f; // context.io.displaySize.y + 0.5f;
        D3DMATRIX mat_identity = { 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f };
        D3DMATRIX mat_projection = {
            2.0f / (R - L),    0.0f,              0.0f,  0.0f,
            0.0f,              2.0f / (T - B),    0.0f,  0.0f,
            0.0f,              0.0f,              0.5f,  0.0f,
            (L + R) / (L - R), (T + B) / (B - T), 0.5f,  1.0f
        };
        g_pD3D9Device->SetTransform(D3DTS_WORLD, &mat_identity);
        g_pD3D9Device->SetTransform(D3DTS_VIEW, &mat_identity);
        g_pD3D9Device->SetTransform(D3DTS_PROJECTION, &mat_projection);
        
        
        // Issue draw calls to the GPU.
        size_t vertexBufferOffset = 0, indexBufferOffset = 0;
        for (auto& drawCommand : context.drawList.drawCommands)
        {
            //g_pD3D9Device->SetTexture(0, reinterpret_cast<LPDIRECT3DTEXTURE9>(drawCommand.userTextureId));
            //const RECT rect = {0,0,230+asasi,300};
            //g_pD3D9Device->SetScissorRect(&rect);
            g_pD3D9Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, static_cast<UINT>(drawCommand.numVertices), static_cast<UINT>(indexBufferOffset), static_cast<UINT>(drawCommand.numIndices / 3));
            vertexBufferOffset += drawCommand.numVertices, indexBufferOffset += drawCommand.numIndices;
        }

        // Restore render state.
        g_pD3D9Device->SetTransform(D3DTS_WORLD, &lastWorldTransform);
        g_pD3D9Device->SetTransform(D3DTS_VIEW, &lastViewTransform);
        g_pD3D9Device->SetTransform(D3DTS_PROJECTION, &lastProjectionTransform);

        stateBlock->Apply();
        stateBlock->Release();
    }
#endif // NMD_GRAPHICS_D3D9

} // namespace nmd


namespace nmd
{

#ifdef NMD_GRAPHICS_D3D11
    static ID3D11Device* g_pD3D11Device = NULL;
    static ID3D11DeviceContext* g_pD3D11DeviceContext = NULL;
    static ID3D11Buffer* g_pD3D11VertexBuffer = NULL;
    static ID3D11Buffer* g_pD3D11IndexBuffer = NULL;
    static size_t g_D3D11VertexBufferSize = 0, g_D3D11IndexBufferSize = 0;
    static ID3DBlob* g_pD3D11ShaderBlob = NULL;
    static ID3D11VertexShader* g_pD3D11VertexShader = NULL;
    static ID3D11PixelShader* g_pD3D11PixelShader = NULL;
    static ID3D11SamplerState* g_pD3D11SamplerState = NULL;
    static ID3D11InputLayout* g_pD3D11InputLayout = NULL;
    static ID3D11Buffer* g_pD3D11VertexConstantBuffer = NULL;
    static ID3D11BlendState* g_pD3D11BlendState = NULL;
    static ID3D11RasterizerState* g_pD3D11RasterizerState = NULL;
    static ID3D11DepthStencilState* g_pD3D11DepthStencilState = NULL;
    static ID3D11ShaderResourceView* g_pD3D11FontTextureView = NULL;

    struct D3D11_RENDER_STATE
    {
        UINT ScissorRectsCount, ViewportsCount;
        D3D11_RECT ScissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
        D3D11_VIEWPORT Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
        ID3D11RasterizerState* RS;
        ID3D11BlendState* BlendState;
        FLOAT BlendFactor[4];
        UINT SampleMask;
        UINT StencilRef;
        ID3D11DepthStencilState* DepthStencilState;
        ID3D11ShaderResourceView* PSShaderResource;
        ID3D11SamplerState* PSSampler;
        ID3D11PixelShader* PS;
        ID3D11VertexShader* VS;
        ID3D11GeometryShader* GS;
        UINT PSInstancesCount, VSInstancesCount, GSInstancesCount;
        ID3D11ClassInstance* PSInstances[256], * VSInstances[256], * GSInstances[256];
        D3D11_PRIMITIVE_TOPOLOGY PrimitiveTopology;
        ID3D11Buffer* IndexBuffer, * VertexBuffer, * VSConstantBuffer;
        UINT IndexBufferOffset, VertexBufferStride, VertexBufferOffset;
        DXGI_FORMAT IndexBufferFormat;
        ID3D11InputLayout* InputLayout;
    };

    struct VERTEX_CONSTANT_BUFFER { float mvp[4][4]; };

    void D3D11SetDeviceContext(ID3D11DeviceContext* pD3D11DeviceContext)
    {
        g_pD3D11DeviceContext = pD3D11DeviceContext;
        g_pD3D11DeviceContext->GetDevice(&g_pD3D11Device);
    }

    void D3D11Render()
    {
        Context& context = GetContext();

        //Don't render if screen is minimized.
        //if (context.io.displaySize.x <= 0.0f || context.io.displaySize.y <= 0.0f)
        //    return;

        //If not initialized, create pixel shader, vertex shader, input layout, etc..
        if (!g_pD3D11VertexShader)
        {
            const char* const pixelShaderCode = "\
            struct PS_INPUT { float4 pos : SV_POSITION; float4 color : COLOR0; };\
            float4 main(PS_INPUT ps_input) : SV_TARGET\
            {\
                return ps_input.color;\
            }";

            if (D3DCompile(pixelShaderCode, strlen(pixelShaderCode), NULL, NULL, NULL, "main", "ps_4_0", 0, 0, &g_pD3D11ShaderBlob, NULL) != S_OK)
                return;

            if (g_pD3D11Device->CreatePixelShader(g_pD3D11ShaderBlob->GetBufferPointer(), g_pD3D11ShaderBlob->GetBufferSize(), NULL, &g_pD3D11PixelShader) != S_OK)
                return;

            const char* const vertexShaderCode = "\
            cbuffer vertexBuffer : register(b0) { float4x4 projectionMatrix; };\
            struct VS_INPUT { float2 pos : POSITION; float4 color : COLOR0; };\
            struct PS_INPUT { float4 pos : SV_POSITION; float4 color : COLOR0; };\
            PS_INPUT main(VS_INPUT vs_input)\
            {\
                PS_INPUT ps_input;\
                ps_input.pos = mul(projectionMatrix, float4(vs_input.pos.xy, 0.0f, 1.0f));\
                ps_input.color = vs_input.color;\
                return ps_input;\
            }";

            if (D3DCompile(vertexShaderCode, strlen(vertexShaderCode), NULL, NULL, NULL, "main", "vs_4_0", 0, 0, &g_pD3D11ShaderBlob, NULL) != S_OK)
                return;

            if (g_pD3D11Device->CreateVertexShader(g_pD3D11ShaderBlob->GetBufferPointer(), g_pD3D11ShaderBlob->GetBufferSize(), NULL, &g_pD3D11VertexShader) != S_OK)
                return;

            D3D11_INPUT_ELEMENT_DESC inputs[] = {
                { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };
            if (g_pD3D11Device->CreateInputLayout(inputs, 2, g_pD3D11ShaderBlob->GetBufferPointer(), g_pD3D11ShaderBlob->GetBufferSize(), &g_pD3D11InputLayout) != S_OK)
                return;

            g_pD3D11ShaderBlob->Release();

            D3D11_BUFFER_DESC desc;
            desc.ByteWidth = sizeof(VERTEX_CONSTANT_BUFFER);
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;
            g_pD3D11Device->CreateBuffer(&desc, NULL, &g_pD3D11VertexConstantBuffer);
        }


        //Create/recreate vertex/index buffer if it doesn't exist or more space is needed.
        {
            if (!g_pD3D11VertexBuffer || g_D3D11VertexBufferSize < context.drawList.vertices.size())
            {
                if (g_pD3D11VertexBuffer)
                    g_pD3D11VertexBuffer->Release(), g_pD3D11VertexBuffer = NULL;

                g_D3D11VertexBufferSize = context.drawList.vertices.size() + 5000;

                D3D11_BUFFER_DESC desc;
                desc.Usage = D3D11_USAGE_DYNAMIC;
                desc.ByteWidth = static_cast<UINT>(g_D3D11VertexBufferSize * sizeof(Vertex));
                desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                desc.MiscFlags = 0;

                if (g_pD3D11Device->CreateBuffer(&desc, NULL, &g_pD3D11VertexBuffer) != S_OK)
                    return;
            }

            if (!g_pD3D11IndexBuffer || g_D3D11IndexBufferSize < context.drawList.indices.size())
            {
                if (g_pD3D11IndexBuffer)
                    g_pD3D11IndexBuffer->Release(), g_pD3D11IndexBuffer = NULL;

                g_D3D11IndexBufferSize = context.drawList.indices.size() + 10000;

                D3D11_BUFFER_DESC desc;
                desc.Usage = D3D11_USAGE_DYNAMIC;
                desc.ByteWidth = static_cast<UINT>(g_D3D11IndexBufferSize * sizeof(IndexType));
                desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
                desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                desc.MiscFlags = 0;

                if (g_pD3D11Device->CreateBuffer(&desc, NULL, &g_pD3D11IndexBuffer) != S_OK)
                    return;
            }
        }

        //Copy data to GPU.
        {
            D3D11_MAPPED_SUBRESOURCE mappedResource = { NULL, 0, 0 };

            //Copy vertices.
            if (g_pD3D11DeviceContext->Map(g_pD3D11VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource) != S_OK)
                return;
            size_t i = 0;
            for (auto& vertex : context.drawList.vertices)
                memcpy(reinterpret_cast<uint8_t*>(mappedResource.pData) + i * 12, &vertex, 12), i++;
            g_pD3D11DeviceContext->Unmap(g_pD3D11VertexBuffer, 0);

            //Copy indices.
            if (g_pD3D11DeviceContext->Map(g_pD3D11IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource) != S_OK)
                return;
            memcpy(mappedResource.pData, context.drawList.indices.data(), sizeof(IndexType) * context.drawList.indices.size());
            g_pD3D11DeviceContext->Unmap(g_pD3D11IndexBuffer, 0);

            //Copy matrix allowing us to specify pixel coordinates(e.g. (250, 250)) instead of normalized coordinates([-1, +1]).
            if (g_pD3D11DeviceContext->Map(g_pD3D11VertexConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource) != S_OK)
                return;
            const float L = 0.0f;
            const float R = 800; // 0.0f + context.io.displaySize.x;
            const float T = 0.0f;
            const float B = 600; // 0.0f + context.io.displaySize.y;
            const float mvp[4][4] =
            {
                { 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
                { 0.0f,         2.0f / (T - B),     0.0f,       0.0f },
                { 0.0f,         0.0f,           0.5f,       0.0f },
                { (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
            };
            memcpy(mappedResource.pData, mvp, sizeof(mvp));
            g_pD3D11DeviceContext->Unmap(g_pD3D11VertexConstantBuffer, 0);
        }

        //Backup current render state.
        D3D11_RENDER_STATE renderState;
        {
            renderState.ScissorRectsCount = renderState.ViewportsCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
            g_pD3D11DeviceContext->RSGetScissorRects(&renderState.ScissorRectsCount, renderState.ScissorRects);
            g_pD3D11DeviceContext->RSGetViewports(&renderState.ViewportsCount, renderState.Viewports);
            g_pD3D11DeviceContext->RSGetState(&renderState.RS);
            g_pD3D11DeviceContext->OMGetBlendState(&renderState.BlendState, renderState.BlendFactor, &renderState.SampleMask);
            g_pD3D11DeviceContext->OMGetDepthStencilState(&renderState.DepthStencilState, &renderState.StencilRef);
            g_pD3D11DeviceContext->PSGetShaderResources(0, 1, &renderState.PSShaderResource);
            g_pD3D11DeviceContext->PSGetSamplers(0, 1, &renderState.PSSampler);
            renderState.PSInstancesCount = renderState.VSInstancesCount = renderState.GSInstancesCount = 256;
            g_pD3D11DeviceContext->PSGetShader(&renderState.PS, renderState.PSInstances, &renderState.PSInstancesCount);
            g_pD3D11DeviceContext->VSGetShader(&renderState.VS, renderState.VSInstances, &renderState.VSInstancesCount);
            g_pD3D11DeviceContext->VSGetConstantBuffers(0, 1, &renderState.VSConstantBuffer);
            g_pD3D11DeviceContext->GSGetShader(&renderState.GS, renderState.GSInstances, &renderState.GSInstancesCount);
            g_pD3D11DeviceContext->IAGetPrimitiveTopology(&renderState.PrimitiveTopology);
            g_pD3D11DeviceContext->IAGetIndexBuffer(&renderState.IndexBuffer, &renderState.IndexBufferFormat, &renderState.IndexBufferOffset);
            g_pD3D11DeviceContext->IAGetVertexBuffers(0, 1, &renderState.VertexBuffer, &renderState.VertexBufferStride, &renderState.VertexBufferOffset);
            g_pD3D11DeviceContext->IAGetInputLayout(&renderState.InputLayout);
        }

        //Set our render state
        {
            D3D11_VIEWPORT vp;
            vp.Width = 800;// context.io.displaySize.x;
            vp.Height = 600;// context.io.displaySize.y;
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;
            vp.TopLeftX = vp.TopLeftY = 0;
            g_pD3D11DeviceContext->RSSetViewports(1, &vp);

            const UINT stride = 12, offset = 0;
            g_pD3D11DeviceContext->IASetInputLayout(g_pD3D11InputLayout);
            g_pD3D11DeviceContext->IASetVertexBuffers(0, 1, &g_pD3D11VertexBuffer, &stride, &offset);
            g_pD3D11DeviceContext->IASetIndexBuffer(g_pD3D11IndexBuffer, sizeof(IndexType) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
            g_pD3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            g_pD3D11DeviceContext->VSSetShader(g_pD3D11VertexShader, NULL, 0);
            g_pD3D11DeviceContext->PSSetShader(g_pD3D11PixelShader, NULL, 0);
            g_pD3D11DeviceContext->VSSetConstantBuffers(0, 1, &g_pD3D11VertexConstantBuffer);
        }

        //Issue draw calls to the GPU.
        {
            size_t vertexBufferOffset = 0, indexBufferOffset = 0;
            for (auto& drawCommand : context.drawList.drawCommands)
            {
                //ID3D11ShaderResourceView* pD3D11Texture = reinterpret_cast<ID3D11ShaderResourceView*>(drawCommand.userTextureId);
                //g_pD3D11DeviceContext->PSSetShaderResources(0, 1, &pD3D11Texture);
                g_pD3D11DeviceContext->DrawIndexed(static_cast<UINT>(drawCommand.numIndices), static_cast<UINT>(indexBufferOffset), 0/*static_cast<INT>(vertexBufferOffset)*/);
                vertexBufferOffset += drawCommand.numVertices, indexBufferOffset += drawCommand.numIndices;
            }
        }

        //Restore render state.
        {
            g_pD3D11DeviceContext->RSSetScissorRects(renderState.ScissorRectsCount, renderState.ScissorRects);
            g_pD3D11DeviceContext->RSSetViewports(renderState.ViewportsCount, renderState.Viewports);
            g_pD3D11DeviceContext->RSSetState(renderState.RS);
            g_pD3D11DeviceContext->OMSetBlendState(renderState.BlendState, renderState.BlendFactor, renderState.SampleMask);
            g_pD3D11DeviceContext->OMSetDepthStencilState(renderState.DepthStencilState, renderState.StencilRef);
            g_pD3D11DeviceContext->PSSetShaderResources(0, 1, &renderState.PSShaderResource);
            g_pD3D11DeviceContext->PSSetSamplers(0, 1, &renderState.PSSampler);
            g_pD3D11DeviceContext->PSSetShader(renderState.PS, renderState.PSInstances, renderState.PSInstancesCount);
            g_pD3D11DeviceContext->VSSetShader(renderState.VS, renderState.VSInstances, renderState.VSInstancesCount);
            g_pD3D11DeviceContext->VSSetConstantBuffers(0, 1, &renderState.VSConstantBuffer);
            g_pD3D11DeviceContext->IASetPrimitiveTopology(renderState.PrimitiveTopology);
            g_pD3D11DeviceContext->IASetIndexBuffer(renderState.IndexBuffer, renderState.IndexBufferFormat, renderState.IndexBufferOffset);
            g_pD3D11DeviceContext->IASetVertexBuffers(0, 1, &renderState.VertexBuffer, &renderState.VertexBufferStride, &renderState.VertexBufferOffset);
            g_pD3D11DeviceContext->IASetInputLayout(renderState.InputLayout);
        }
    }
#endif // NMD_GRAPHICS_D3D11

} // namespace nmd


//#ifdef NMD_GUI_OPENGL
//
//    void OpenGLRender()
//    {
//        //Setup render state
//        glEnable(GL_BLEND);
//        glBlendEquation(GL_FUNC_ADD);
//        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//        glDisable(GL_CULL_FACE);
//        glDisable(GL_DEPTH_TEST);
//        glEnable(GL_SCISSOR_TEST);
//
//        glViewport(0, 0, static_cast<GLsizei>(g_context.io.displaySize.x), static_cast<GLsizei>(g_context.io.displaySize.y));
//        float L = 0.0f;
//        float R = 0.0f + g_context.io.displaySize.x;
//        float T = 0.0f;
//        float B = 0.0f + g_context.io.displaySize.y;
//        const float ortho_projection[4][4] =
//        {
//            { 2.0f / (R - L),   0.0f,         0.0f,   0.0f },
//            { 0.0f,         2.0f / (T - B),   0.0f,   0.0f },
//            { 0.0f,         0.0f,        -1.0f,   0.0f },
//            { (R + L) / (L - R),  (T + B) / (B - T),  0.0f,   1.0f },
//        };
//        glUseProgram(g_ShaderHandle);
//        glUniform1i(g_AttribLocationTex, 0);
//        glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
//
//        size_t vertexBufferOffset = 0, indexBufferOffset = 0;
//        for (auto& drawCommand : g_context.drawList.drawCommands)
//        {
//            glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(drawCommand.numVertices) * sizeof(Vertex), reinterpret_cast<const GLvoid*>(g_context.drawList.vertices.data() + vertexBufferOffset), GL_STREAM_DRAW);
//            glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(drawCommand.numIndices) * sizeof(IndexType), reinterpret_cast<const GLvoid*>(g_context.drawList.indices.data() + indexBufferOffset), GL_STREAM_DRAW);
//
//            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(drawCommand.numIndices / 3), sizeof(IndexType) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, reinterpret_cast<void*>(static_cast<intptr_t>(indexBufferOffset * sizeof(IndexType))));
//        }
//    }
//
//#endif // NMD_GRAPHICS_OPENGL

#endif // NMD_GRAPHICS_IMPLEMENTATION
