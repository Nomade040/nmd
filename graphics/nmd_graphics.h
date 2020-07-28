/* This is a C89 platform independent immediate mode 2D graphics library.

Setup:
Define the 'NMD_GRAPHICS_IMPLEMENTATION' macro in one source file before the include statement to instantiate the implementation.
#define NMD_GRAPHICS_IMPLEMENTATION
#include "nmd_graphics.h"

Defining types manually:
Define the 'NMD_GRAPHICS_DEFINE_TYPES' macro to tell the library to define(typedef) the required types.
Be aware: This feature uses platform dependent macros.

Disable default functions:
Define the 'NMD_GRAPHICS_DISABLE_DEFAULT_ALLOCATOR' macro to tell the library not to include default allocators.

Low level overview:
The nmd::Context(acessible by nmd::GetContext()) global variable holds the state of the entire library, it
contains a nmd::DrawList variable which holds the vertex, index and commands buffers. Each command buffer 
translate to a call to a rendering's API draw function. The nmd::DrawList class has methods to draw basic 
geometry shapes(e.g. circles, rectangles and lines).

Supported rendering APIs: Direct3D 9(D3D9), Direct3D 11(D3D11).
To use a specific rendering api define the macro 'NMD_GRAPHICS_{RENDERING API}' before including "nmd_graphics.hpp".

Usage:
 - General:
    - Call API functions between nmd::Begin() and nmd::End()
 - D3D9:
    - Call nmd::D3D9SetDevice() and nmd::D3D9Resize() on initialization.
    - Call nmd::D3D9Render() after nmd::End().

 - D3D11:
    - Call nmd::D3D11SetDeviceContext() on initialization.
    - Call nmd::D3D11Render() after nmd::End()

Default fonts:
The 'Karla' true type font in included by default. Define the 'NMD_GRAPHICS_DISABLE_DEFAULT_FONT' macro to remove the
font at compile time. By doing so at least 15KB of code & data will be saved.

NOTE: A big part of this library's code has been derived from Imgui's and Nuklear's code. Huge credit to both projects.

TODO:
 - Add support for textures in Direct3D 11.
 - Add AddText() method to DrawList.
 - Add support for the remaining rendering APIs: Direct3D 12, OpenGL and Vulkan.

Credits:
 - imgui: https://github.com/ocornut/imgui
 - nuklear: https://github.com/Immediate-Mode-UI/Nuklear
 - stb_truetype: https://github.com/nothings/stb/blob/master/stb_truetype.h
*/

#ifndef NMD_GRAPHICS_H
#define NMD_GRAPHICS_H

#ifdef NMD_GRAPHICS_DEFINE_TYPES

#ifndef __cplusplus

#define bool  _Bool
#define false 0
#define true  1

#endif /* __cplusplus */

typedef signed char        int8_t;
typedef unsigned char      uint8_t;

typedef signed short       int16_t;
typedef unsigned short     uint16_t;

typedef signed int         int32_t;
typedef unsigned int       uint32_t;

typedef signed long long   int64_t;
typedef unsigned long long uint64_t;

#if defined(_WIN64) && defined(_MSC_VER)
	typedef unsigned __int64 size_t;
	typedef __int64          ptrdiff_t;
#elif (defined(_WIN32) || defined(WIN32)) && defined(_MSC_VER)
	typedef unsigned __int32 size_t
	typedef __int32          ptrdiff_t;
#elif defined(__GNUC__) || defined(__clang__)
	#if defined(__x86_64__) || defined(__ppc64__)
		typedef unsigned long size_t
		typedef long          ptrdiff_t
	#else
		typedef unsigned int size_t
		typedef int          ptrdiff_t
	#endif
#else
	typedef unsigned long size_t
	typedef long          ptrdiff_t
#endif

#else

/* Dependencies */
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#endif /* NMD_GRAPHICS_DEFINE_TYPES */

#ifndef NMD_GRAPHICS_DISABLE_DEFAULT_ALLOCATOR
#include <stdlib.h>

#ifndef NMD_MALLOC
#define NMD_MALLOC malloc
#endif /* NMD_MALLOC */

#ifndef NMD_REALLOC
#define NMD_REALLOC realloc
#endif /* NMD_REALLOC */

#endif /* NMD_GRAPHICS_DISABLE_DEFAULT_ALLOCATOR */

#ifndef NMD_GRAPHICS_DISABLE_DEFAULT_MATH_FUNCTIONS
#include <math.h>

#ifndef NMD_SQRT
#define NMD_SQRT sqrt
#endif /* NMD_SQRT */

#ifndef NMD_ACOS
#define NMD_ACOS acos
#endif /* NMD_ACOS */

#ifndef NMD_COS
#define NMD_COS cos
#endif /* NMD_COS */

#ifndef NMD_SIN
#define NMD_SIN sin
#endif /* NMD_SIN */

#endif /* NMD_GRAPHICS_DISABLE_DEFAULT_MATH_FUNCTIONS */

#ifndef NMD_INITIAL_PATH_BUFFER_SIZE
#define NMD_INITIAL_PATH_BUFFER_SIZE 256 /* 256B */
#endif /* NMD_INITIAL_PATH_BUFFER_SIZE */

#ifndef NMD_INITIAL_VERTICES_BUFFER_SIZE
#define NMD_INITIAL_VERTICES_BUFFER_SIZE 4096 /* 4KB */
#endif /* NMD_INITIAL_VERTICES_BUFFER_SIZE */

#ifndef NMD_INITIAL_INDICES_BUFFER_SIZE
#define NMD_INITIAL_INDICES_BUFFER_SIZE 2048 /* 2KB */
#endif /* NMD_INITIAL_INDICES_BUFFER_SIZE */

#ifndef NMD_INITIAL_DRAW_COMMANDS_BUFFER_SIZE
#define NMD_INITIAL_DRAW_COMMANDS_BUFFER_SIZE 128 /* 128B */
#endif /* NMD_INITIAL_DRAW_COMMANDS_BUFFER_SIZE */

#ifdef _WIN32
#include <Windows.h>
#endif /* _WIN32 */

#ifdef NMD_GRAPHICS_D3D9
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

void nmd_d3d9_set_device(LPDIRECT3DDEVICE9 pDevice);
void nmd_d3d9_resize(int width, int height);
void nmd_d3d9_render();
#endif /* NMD_GRAPHICS_D3D9 */

#ifdef NMD_GRAPHICS_D3D11
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

void nmd_d3d11_set_device_context(ID3D11DeviceContext* pDeviceContext);
void nmd_d3d11_render();
#endif /* NMD_GRAPHICS_D3D11 */

#ifdef NMD_GRAPHICS_OPENGL
bool nmd_opengl_resize(int width, int height);
bool nmd_opengl_render();
#endif /* NMD_GRAPHICS_OPENGL */

/*
#ifdef _WIN32
    void Win32Init(HWND hwnd);
    LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif
*/

enum NMD_CORNER
{
    NMD_CORNER_NONE         = (1 << 0),
    NMD_CORNER_TOP_LEFT     = (1 << 1),
    NMD_CORNER_TOP_RIGHT    = (1 << 2),
    NMD_CORNER_BOTTOM_LEFT  = (1 << 3),
    NMD_CORNER_BOTTOM_RIGHT = (1 << 4),

    /* While these are not actual corners they are nice to have. */
    NMD_CORNER_TOP          = NMD_CORNER_TOP_LEFT    | NMD_CORNER_TOP_RIGHT,
    NMD_CORNER_BOTTOM       = NMD_CORNER_BOTTOM_LEFT | NMD_CORNER_BOTTOM_RIGHT,
    NMD_CORNER_LEFT         = NMD_CORNER_TOP_LEFT    | NMD_CORNER_BOTTOM_LEFT,
    NMD_CORNER_RIGHT        = NMD_CORNER_TOP_RIGHT   | NMD_CORNER_BOTTOM_RIGHT,

    NMD_CORNER_ALL          = (1 << 5) - 1
};

typedef uint16_t IndexType;
typedef void* TextureId;

typedef struct
{
    /* 'numVertices' has the type 'IndexType' because the number of vertices is always less or equal the number of indices. */
    IndexType numVertices; 

    IndexType numIndices;
    TextureId userTextureId;
} nmd_draw_command;

typedef struct
{
    float x, y;
} nmd_vec2;

typedef struct
{
    float x, y, z;
} nmd_vec3;

typedef struct
{
    uint8_t r, g, b, a;
} nmd_color;

typedef struct
{
    nmd_vec2 pos;
    nmd_vec2 uv;
    nmd_color color;
} nmd_vertex;

typedef struct
{
    nmd_vec2 cachedCircleVertices12[12];
    uint8_t cachedCircleSegmentCounts64[64];
    float curveTessellationTolerance;

    nmd_vec2* path;
    size_t numPoints; /* number of points('nmd_vec2') in the 'path' buffer. */
    size_t pathCapacity; /* size of the 'path' buffer in bytes. */

    nmd_vertex* vertices;
    size_t numVertices; /* number of vertices in the 'vertices' buffer. */
    size_t verticesCapacity; /* size of the 'vertices' buffer in bytes. */

    IndexType* indices;
    size_t numIndices; /* number of indices in the 'indices' buffer. */
    size_t indicesCapacity; /* size of the 'indices' buffer in bytes. */

    nmd_draw_command* drawCommands;
    size_t numDrawCommands; /* number of draw commands in the 'drawCommands' buffer. */
    size_t drawCommandsCapacity; /* size of the 'drawCommands' buffer in bytes. */

    /*
    void PushTextureDrawCommand(size_t numVertices, size_t numIndices, TextureId userTextureId);
    
    void AddText(const Vec2& pos, Color color, const char* text, size_t textLength);
    void AddText(const void* font, float fontSize, const Vec2& pos, Color color, const char* text, size_t textLength, float wrapWidth = 0.0f);

    void AddImage(TextureId userTextureId, const Vec2& p1, const Vec2& p2, const Vec2& uv1 = Vec2(0, 0), const Vec2& uv2 = Vec2(1, 1), Color color = Color::White);
    void AddImageQuad(TextureId userTextureId, const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, const Vec2& uv1 = Vec2(0, 0), const Vec2& uv2 = Vec2(1, 0), const Vec2& uv3 = Vec2(1, 1), const Vec2& uv4 = Vec2(0, 1), Color color = Color::White);
    void AddImageRounded(TextureId userTextureId, const Vec2& p1, const Vec2& p2, float rounding, uint32_t cornerFlags = CORNER_FLAGS_ALL, const Vec2& uv1 = Vec2(0, 0), const Vec2& uv2 = Vec2(1, 1), Color color = Color::White);

    void PathBezierCurveTo(const Vec2& p2, const Vec2& p3, const Vec2& p4, size_t numSegments);

    void PrimRectUV(const Vec2& p1, const Vec2& p2, const Vec2& uv1, const Vec2& uv2, Color color);
    void PrimQuadUV(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, const Vec2& uv1, const Vec2& uv2, const Vec2& uv3, const Vec2& uv4, Color color);
    */
} nmd_drawlist;

typedef struct
{
    nmd_drawlist drawList;
} nmd_context;

void nmd_path_to(float x0, float y0);
void nmd_path_rect(float x0, float y0, float x1, float y1, float rounding, uint32_t roundingCorners);

/*
'startAtCenter' places the first vertex at the center, this can be used to create a pie chart when using nmd_path_fill_convex().
This functions uses twelve(12) chached vertices initialized during startup, it should be faster than PathArcTo.
*/
void nmd_path_arc_to_cached(float x0, float y0, float radius, size_t startAngleOf12, size_t endAngleOf12, bool startAtCenter);
void nmd_path_arc_to(float x0, float y0, float radius, float startAngle, float endAngle, size_t numSegments, bool startAtCenter);

void nmd_path_fill_convex(nmd_color color);
void nmd_path_stroke(nmd_color color, bool closed, float thickness);

void nmd_add_line(float x0, float y0, float x1, float y1, nmd_color color, float thickness);

void nmd_add_rect(float x0, float y0, float x1, float y1, nmd_color color, float rounding, uint32_t cornerFlags, float thickness);
void nmd_add_rect_filled(float x0, float y0, float x1, float y1, nmd_color color, float rounding, uint32_t cornerFlags);
void nmd_add_rect_filled_multi_color(float x0, float y0, float x1, float y1, nmd_color colorUpperLeft, nmd_color colorUpperRight, nmd_color colorBottomRight, nmd_color colorBottomLeft);

void nmd_add_quad(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, nmd_color color, float thickness);
void nmd_add_quad_filled(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, nmd_color color);

void nmd_add_triangle(float x0, float y0, float x1, float y1, float x2, float y2, nmd_color color, float thickness);
void nmd_add_triangle_filled(float x0, float y0, float x1, float y1, float x2, float y2, nmd_color color);

/*
Set numSegments to zero if you want the function to automatically determine the number of segmnts.
numSegments = 12
*/
void nmd_add_circle(float x0, float y0, float radius, nmd_color color, size_t numSegments, float thickness);
void nmd_add_circle_filled(float x0, float y0, float radius, nmd_color color, size_t numSegments);

void nmd_add_ngon(float x0, float y0, float radius, nmd_color color, size_t numSegments, float thickness);
void nmd_add_ngon_filled(float x0, float y0, float radius, nmd_color color, size_t numSegments);

void nmd_add_polyline(const nmd_vec2* points, size_t numPoints, nmd_color color, bool closed, float thickness);
void nmd_add_convex_polygon_filled(const nmd_vec2* points, size_t numPoints, nmd_color color);
//void nmd_add_bezier_curve(nmd_vec2 p0, nmd_vec2 p1, nmd_vec2 p2, nmd_vec2 p3, nmd_color color, float thickness, size_t numSegments);

nmd_color nmd_rgb(uint8_t r, uint8_t g, uint8_t b);
nmd_color nmd_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

nmd_context* nmd_get_context();

/* Starts a new empty scene. Internally this function clears all vertices, indices and command buffers. */
void nmd_begin();

/* Ends a scene, so it can be rendered. Internally this functions creates the remaining draw commands. */
void nmd_end();

#define NMD_COLOR_BLACK         nmd_rgb(0,   0,   0  )
#define NMD_COLOR_WHITE         nmd_rgb(255, 255, 255)
#define NMD_COLOR_RED           nmd_rgb(255, 0,   0  )
#define NMD_COLOR_GREEN         nmd_rgb(0,   255, 0  )
#define NMD_COLOR_BLUE          nmd_rgb(0,   0,   255)
#define NMD_COLOR_ORANGE        nmd_rgb(255, 165, 0  )
#define NMD_COLOR_AMBER         nmd_rgb(255, 191, 0  )
#define NMD_COLOR_ANDROID_GREEN nmd_rgb(164, 198, 57 )
#define NMD_COLOR_AZURE         nmd_rgb(0,   127, 255)
#define NMD_COLOR_BRONZE        nmd_rgb(205, 127, 50 )
#define NMD_COLOR_CORN          nmd_rgb(251, 236, 93 )
#define NMD_COLOR_EMERALD       nmd_rgb(80,  200, 120)
#define NMD_COLOR_LAPIS_LAZULI  nmd_rgb(38,  97,  156)
#define NMD_COLOR_LAVA          nmd_rgb(207, 16,  32 )

#endif /* NMD_GRAPHICS_H */
