/* This is a C89 platform independent 2D immediate mode graphics library.

This library just generates triangles(represented by vertices and indices) from functions like nmd_add_rect_filled().
There's no GUI or IO.

Setup:
Define the 'NMD_GRAPHICS_IMPLEMENTATION' macro in one source file before the include statement to instantiate the implementation.
#define NMD_GRAPHICS_IMPLEMENTATION
#include "nmd_graphics.h"

The general workflow for using the library is the following:
 1 - Call nmd_new_frame() to clear all vertex, index and command buffers.
 2 - Call whatever primitive function you need(e.g. nmd_add_line(), nmd_add_rect_filled()).
 3 - Call nmd_end_frame() to create the necessary draw command(s) that describe the data(vertices and indices) for redering.
 4 - Now you can do anything with the data. You probably want to call nmd_opengl_render(), nmd_d3d9_render() or nmd_d3d11_render() here.

OpenGL Usage:
 You MUST define the 'NMD_GRAPHICS_OPENGL' macro once project-wise(compiler dependent) or for every include statement that you'll use any of the following functions.
 You MUST call nmd_opengl_resize() once for initialization and every time the window resizes.
 You MUST call nmd_opengl_render() to issue draw calls that render the data in the drawlist.
 You may call nmd_opengl_create_texture() if a helper function for texture creation is desired.
 You may define the 'NMD_GRAPHICS_OPENGL_DONT_BACKUP_RENDER_STATE' macro if you don't mind the library overriding the render state.
 You may define the 'NMD_GRAPHICS_OPENGL_OPTIMIZE_RENDER_STATE' macro so the render state only changes when necessary. Note that this option may only be used if this library is the only component that uses OpenGL.

D3D9 Usage:
 You MUST define the 'NMD_GRAPHICS_D3D9' macro once project-wise(compiler dependent) or for every include statement that you'll use any of the following functions.
 You MUST call nmd_d3d9_set_device() once for initialization and every time the device changes.
 You MUST call nmd_d3d9_resize() once for initialization and every time the window resizes.
 You MUST call nmd_d3d9_render() to issue draw calls that render the data in the drawlist.
 You may call nmd_d3d9_create_texture() if a helper function for texture creation is desired.
 You may define the 'NMD_GRAPHICS_D3D9_DONT_BACKUP_RENDER_STATE' macro if you don't mind the library overriding the render state.
 You may define the 'NMD_GRAPHICS_D3D9_OPTIMIZE_RENDER_STATE' macro so the render state only changes when necessary. Note that this option may only be used if this library is the only component that uses Direct3D 9.

D3D11 Usage:
 You MUST define the 'NMD_GRAPHICS_D3D11' macro once project-wise(compiler dependent) or for every include statement that you'll use any of the following functions.
 You MUST call nmd_d3d11_set_device_context() once for initialization and every time the device context changes.
 You MUST call nmd_d3d11_resize() once for initialization and every time the window resizes.
 You MUST call nmd_d3d11_render() to issue draw calls that render the data in the drawlist.
 You may call nmd_d3d11_create_texture() if a helper function for texture creation is desired.
 You may define the 'NMD_GRAPHICS_D3D11_DONT_BACKUP_RENDER_STATE' macro if you don't mind the library overriding the render state.
 You may define the 'NMD_GRAPHICS_D3D11_OPTIMIZE_RENDER_STATE' macro so the render state only changes when necessary. Note that this option may only be used if this library is the only component that uses Direct3D 11.

Internals:
The 'nmd_context'(acessible by nmd_get_context()) global variable holds the state of the entire library, it
contains a 'nmd_drawlist' variable which holds the vertex, index and command buffers. Each command buffer
translate to a call to a rendering's API draw function. Shapes can be rendered in the drawlist by calling
functions like nmd_add_line() and nmd_add_filled_rect().

Defining types manually:
Define the 'NMD_GRAPHICS_DEFINE_TYPES' macro to tell the library to define(typedef) the required types.
Be aware: This feature uses platform dependent macros.

Disabling default functions:
Define the 'NMD_GRAPHICS_DISABLE_DEFAULT_ALLOCATOR' macro to tell the library not to include default allocators.

Default fonts:
The 'Karla' true type font in included by default. Define the 'NMD_GRAPHICS_DISABLE_DEFAULT_FONT' macro to remove the font at compile time.

NOTE: A big part of this library's code has been derived from Imgui's and Nuklear's code. Huge credit to both projects.

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
#include <stdio.h>
#include <malloc.h>

#ifndef NMD_ALLOC
#define NMD_ALLOC malloc
#endif /* NMD_ALLOC */

#ifndef NMD_FREE
#define NMD_FREE free
#endif /* NMD_FREE */

#ifndef NMD_ALLOCA
#define NMD_ALLOCA alloca
#endif /* NMD_ALLOCA */

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

/* The number of points the buffer intially supports */
#ifndef NMD_PATH_BUFFER_INITIAL_SIZE
#define NMD_PATH_BUFFER_INITIAL_SIZE 32
#endif /* NMD_PATH_BUFFER_INITIAL_SIZE */

/* The number of vertices the buffer intially supports */
#ifndef NMD_VERTEX_BUFFER_INITIAL_SIZE
#define NMD_VERTEX_BUFFER_INITIAL_SIZE 2500
#endif /* NMD_VERTEX_BUFFER_INITIAL_SIZE */

/* The number of indices the buffer intially supports */
#ifndef NMD_INDEX_BUFFER_INITIAL_SIZE
#define NMD_INDEX_BUFFER_INITIAL_SIZE 5000
#endif /* NMD_INDEX_BUFFER_INITIAL_SIZE */

/* The number of draw commands the buffer intially supports */
#ifndef NMD_DRAW_COMMANDS_BUFFER_INITIAL_SIZE
#define NMD_DRAW_COMMANDS_BUFFER_INITIAL_SIZE 8
#endif /* NMD_DRAW_COMMANDS_BUFFER_INITIAL_SIZE */

#ifdef _WIN32
#include <Windows.h>
#endif /* _WIN32 */

typedef uint16_t nmd_index;
typedef void* nmd_tex_id;

#ifdef NMD_GRAPHICS_OPENGL
bool nmd_opengl_resize(int width, int height);
void nmd_opengl_render();
nmd_tex_id nmd_opengl_create_texture(void* pixels, int width, int height);
#endif /* NMD_GRAPHICS_OPENGL */

#ifdef NMD_GRAPHICS_D3D9
#include <d3d9.h>
void nmd_d3d9_set_device(LPDIRECT3DDEVICE9 pDevice);
void nmd_d3d9_resize(int width, int height);
void nmd_d3d9_render();
nmd_tex_id nmd_d3d9_create_texture(void* pixels, int width, int height);
#endif /* NMD_GRAPHICS_D3D9 */

#ifdef NMD_GRAPHICS_D3D11
#include <d3d11.h>
void nmd_d3d11_set_device_context(ID3D11DeviceContext* pDeviceContext);
bool nmd_d3d11_resize(int width, int height);
void nmd_d3d11_render();
nmd_tex_id nmd_d3d11_create_texture(void* pixels, int width, int height);
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

/* Describes a texture atlas */
typedef struct
{
    void* fontData;
    int width;
    int height;
    uint8_t* pixels8;
    nmd_color* pixels32;
    void* bakedChars; /* internal */
    nmd_tex_id font;
} nmd_atlas;

typedef struct
{
    bool lineAntiAliasing; /* If true, all lines will have AA applied to them. */
    bool fillAntiAliasing; /* If true, all filled polygons will have AA applied to them. */

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

    nmd_tex_id font;

    /*
    void PathBezierCurveTo(const Vec2& p2, const Vec2& p3, const Vec2& p4, size_t numSegments);*/
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

void nmd_add_dummy_text(float x, float y, const char* text, float height, nmd_color color, float spacing);

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

/*void nmd_add_text(float x, float y, const char* text, nmd_color color);*/
void nmd_add_text(const nmd_atlas* font, float x, float y, const char* text, const char* textEnd, nmd_color color);

void nmd_add_image(nmd_tex_id userTextureId, float x0, float y0, float x1, float y1, nmd_color color);
void nmd_add_image_uv(nmd_tex_id userTextureId, float x0, float y0, float x1, float y1, float uv_x0, float uv_y0, float uv_x1, float uv_y1, nmd_color color);

void nmd_add_image_quad(nmd_tex_id userTextureId, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, nmd_color color);
void nmd_add_image_quad_uv(nmd_tex_id userTextureId, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float uv_x0, float uv_y0, float uv_x1, float uv_y1, float uv_x2, float uv_y2, float uv_x3, float uv_y3, nmd_color color);

void nmd_add_image_rounded(nmd_tex_id userTextureId, float x0, float y0, float x1, float y1, float rounding, uint32_t cornerFlags, nmd_color color);
void nmd_add_image_rounded_uv(nmd_tex_id userTextureId, float x0, float y0, float x1, float y1, float rounding, uint32_t cornerFlags, float uv_x0, float uv_y0, float uv_x1, float uv_y1, nmd_color color);

void nmd_prim_rect_uv(float x0, float y0, float x1, float y1, float uv_x0, float uv_y0, float uv_x1, float uv_y1, nmd_color color);
void nmd_prim_quad_uv(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float uv_x0, float uv_y0, float uv_x1, float uv_y1, float uv_x2, float uv_y2, float uv_x3, float uv_y3, nmd_color color);

nmd_color nmd_rgb(uint8_t r, uint8_t g, uint8_t b);
nmd_color nmd_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

nmd_context* nmd_get_context();

/* Starts a new empty scene/frame. Internally this function clears all vertices, indices and command buffers. */
void nmd_new_frame();

/* "Ends" a frame. Wrapper around nmd_push_draw_command(). */
void nmd_end_frame();

/*
Creates one or more draw commands for the unaccounted vertices and indices.
Parameters:
 clipRect [opt/in] A pointer to a rect that specifies the clip area. This parameter can be null.
*/
void nmd_push_draw_command(const nmd_rect* clipRect);

void nmd_push_texture_draw_command(nmd_tex_id userTextureId, const nmd_rect* clipRect);

bool nmd_bake_font(const char* fontPath, nmd_atlas* atlas, float size);

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
