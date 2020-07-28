/* This is a C89 platform independent immediate mode 2D graphics library.

Setup:
Define the 'NMD_GRAPHICS_IMPLEMENTATION' macro in one source file before the include statement to instantiate the implementation.
#define NMD_GRAPHICS_IMPLEMENTATION
#include "nmd_graphics.h"

Defining types manually:
Define the 'NMD_GRAPHICS_DEFINE_TYPES' macro to tell the library to define(typedef) the required types.
Be aware: This feature uses platform dependent macros.

Disabling default functions:
Define the 'NMD_GRAPHICS_DISABLE_DEFAULT_ALLOCATOR' macro to tell the library not to include default allocators.

Overview:
The 'nmd_context'(acessible by nmd_get_context()) global variable holds the state of the entire library, it
contains a nmd_drawlist variable which holds the vertex, index and commands buffers. Each command buffer 
translate to a call to a rendering's API draw function. Shapes can be rendered in the drawlist by calling
functions like nmd_add_line(), nmd_add_filled_rect(), ...

OpenGL Usage:
 Define 'NMD_GRAPHICS_OPENGL'.
 Call nmd_opengl_resize() for initialization.
 Call nmd_opengl_render() to render data in the drawlist.

D3D9 Usage:
 Define 'NMD_GRAPHICS_D3D9'.
 Call nmd_d3d9_set_device() and nmd_d3d9_resize() for initialization.
 Call nmd_d3d9_render() to render data in the drawlist.

D3D11 Usage:
 Define 'NMD_GRAPHICS_D3D11'.
 Call nmd_d3d11_set_device_context() and nmd_d3d11_resize() for initialization.
 Call nmd_d3d11_render() to render data in the drawlist.

Default fonts:
The 'Karla' true type font in included by default. Define the 'NMD_GRAPHICS_DISABLE_DEFAULT_FONT' macro to remove the font at compile time.

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

#ifdef NMD_GRAPHICS_OPENGL
bool nmd_opengl_resize(int width, int height);
bool nmd_opengl_render();
#endif /* NMD_GRAPHICS_OPENGL */

#ifdef NMD_GRAPHICS_D3D9
#include <d3d9.h>
void nmd_d3d9_set_device(LPDIRECT3DDEVICE9 pDevice);
void nmd_d3d9_resize(int width, int height);
void nmd_d3d9_render();
#endif /* NMD_GRAPHICS_D3D9 */

#ifdef NMD_GRAPHICS_D3D11
#include <d3d11.h>
void nmd_d3d11_set_device_context(ID3D11DeviceContext* pDeviceContext);
bool nmd_d3d11_resize(int width, int height);
void nmd_d3d11_render();
#endif /* NMD_GRAPHICS_D3D11 */

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

typedef uint16_t nmd_index;
typedef void* nmd_tex_id;

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
    nmd_vec2 p0;
    nmd_vec2 p1;
} nmd_rect;

typedef struct
{
    /* 'numVertices' has the type 'nmd_index' because the number of vertices is always less or equal the number of indices. */
    nmd_index numVertices; 

    nmd_index numIndices;
    nmd_tex_id userTextureId;

    nmd_rect rect;
    
} nmd_draw_command;

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

    nmd_index* indices;
    size_t numIndices; /* number of indices in the 'indices' buffer. */
    size_t indicesCapacity; /* size of the 'indices' buffer in bytes. */

    nmd_draw_command* drawCommands;
    size_t numDrawCommands; /* number of draw commands in the 'drawCommands' buffer. */
    size_t drawCommandsCapacity; /* size of the 'drawCommands' buffer in bytes. */

    /*
    void PushTextureDrawCommand(size_t numVertices, size_t numIndices, nmd_tex_id userTextureId);
    
    void AddText(const Vec2& pos, Color color, const char* text, size_t textLength);
    void AddText(const void* font, float fontSize, const Vec2& pos, Color color, const char* text, size_t textLength, float wrapWidth = 0.0f);

    void AddImage(nmd_tex_id userTextureId, const Vec2& p1, const Vec2& p2, const Vec2& uv1 = Vec2(0, 0), const Vec2& uv2 = Vec2(1, 1), Color color = Color::White);
    void AddImageQuad(nmd_tex_id userTextureId, const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, const Vec2& uv1 = Vec2(0, 0), const Vec2& uv2 = Vec2(1, 0), const Vec2& uv3 = Vec2(1, 1), const Vec2& uv4 = Vec2(0, 1), Color color = Color::White);
    void AddImageRounded(nmd_tex_id userTextureId, const Vec2& p1, const Vec2& p2, float rounding, uint32_t cornerFlags = CORNER_FLAGS_ALL, const Vec2& uv1 = Vec2(0, 0), const Vec2& uv2 = Vec2(1, 1), Color color = Color::White);

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

void nmd_add_dummy_text(float x, float y, const char* text, float height, nmd_color color, float spacing, float thickness);

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
/*void nmd_add_bezier_curve(nmd_vec2 p0, nmd_vec2 p1, nmd_vec2 p2, nmd_vec2 p3, nmd_color color, float thickness, size_t numSegments);*/

nmd_color nmd_rgb(uint8_t r, uint8_t g, uint8_t b);
nmd_color nmd_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

nmd_context* nmd_get_context();

/* Starts a new empty scene. Internally this function clears all vertices, indices and command buffers. */
void nmd_begin();

/* Ends a scene, so it can be rendered. Internally this functions creates draw commands. */
void nmd_end();

/*
Parameters:
 clipRect [opt/in] A pointer to a rect that specifies the drawable area. If this parameter is null, the entire screen will be set as drawable.
*/
void nmd_push_draw_command(const nmd_rect* clipRect);

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
