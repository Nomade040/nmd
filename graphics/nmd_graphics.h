/* This is a C89 platform independent 2D immediate mode graphics library.

This library just generates triangles(represented by vertices and indices) from functions like nmd_add_rect_filled().
There's a GUI component. There're helper functions to handle input on the following platforms:
 - Windows: nmd_win32_wnd_proc

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

Fixed width integer types:
By default the library includes <stdint.h> and <stddef.h> to include int types.
If these header-files are not available in your environment you may define the 'NMD_DEFINE_INT_TYPES' macro so the library will define them.
By defining the 'NMD_IGNORE_INT_TYPES' macro, the library will neither include nor define int types.

Define the 'NMD_GRAPHICS_DISABLE_DEFAULT_ALLOCATOR' macro to tell the library not to include default allocators.
Define the 'NMD_GRAPHICS_DISABLE_FILE_IO' macro to tell the library not to support file operations for fonts.
Define the 'NMD_GRAPHICS_AVOID_ALLOCA' macro to tell the library to use malloc/free instead of alloca
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

#ifndef _NMD_DEFINE_INT_TYPES
 #ifdef NMD_DEFINE_INT_TYPES
  #define _NMD_DEFINE_INT_TYPES
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
  
 #else /* NMD_DEFINE_INT_TYPES */
  #ifndef NMD_IGNORE_INT_TYPES
    #include <stdbool.h>
    #include <stdint.h>
    #include <stddef.h>
  #endif /* NMD_IGNORE_INT_TYPES */
 #endif /* NMD_DEFINE_INT_TYPES */
#endif /* _NMD_DEFINE_INT_TYPES */

#ifndef NMD_MALLOC
#include <stdlib.h>
#define NMD_MALLOC malloc
#define NMD_FREE free
#endif /* NMD_MALLOC */

#ifndef NMD_MEMSET
#include <string.h>
#define NMD_MEMSET memset
#endif /* NMD_MEMSET */

#ifndef NMD_MEMCPY
#include <string.h>
#define NMD_MEMCPY memcpy
#endif /* NMD_MEMCPY */

#ifndef NMD_STRLEN
#include <string.h>
#define NMD_STRLEN strlen
#endif /* NMD_STRLEN */

#ifndef NMD_GRAPHICS_AVOID_ALLOCA
    #ifndef NMD_ALLOCA
    #include <malloc.h>
    #define NMD_ALLOCA alloca
    #endif /* NMD_ALLOCA */
#endif /* NMD_GRAPHICS_AVOID_ALLOCA */

#ifndef NMD_SPRINTF
#include <stdio.h>
#define NMD_SPRINTF sprintf
#endif /* NMD_SPRINTF */

#ifndef NMD_VSPRINTF
#include <stdio.h>
#define NMD_VSPRINTF vsprintf
#endif /* NMD_VSPRINTF */

#ifndef NMD_SQRT
#include <math.h>
#define NMD_SQRT sqrt
#endif /* NMD_SQRT */

#ifndef NMD_ACOS
#include <math.h>
#define NMD_ACOS acos
#endif /* NMD_ACOS */

#ifndef NMD_COS
#include <math.h>
#define NMD_COS cos
#endif /* NMD_COS */

#ifndef NMD_SIN
#include <math.h>
#define NMD_SIN sin
#endif /* NMD_SIN */

#define STBTT_malloc(x,u) ((void)(u),NMD_MALLOC(x))
#define STBTT_free(x,u) ((void)(u),NMD_FREE(x))
#define STBTT_strlen(x) NMD_STRLEN(x)
#define STBTT_memcpy NMD_MEMCPY
#define STBTT_memset NMD_MEMSET

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
#define NMD_DRAW_COMMANDS_BUFFER_INITIAL_SIZE 32
#endif /* NMD_DRAW_COMMANDS_BUFFER_INITIAL_SIZE */

/* The number of windows the buffer intially supports */
#ifndef NMD_WINDOWS_BUFFER_INITIAL_SIZE
#define NMD_WINDOWS_BUFFER_INITIAL_SIZE 4
#endif /* NMD_WINDOWS_BUFFER_INITIAL_SIZE */

#ifdef _WIN32
#include <Windows.h>
LRESULT nmd_win32_wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
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
void nmd_d3d11_delete_objects();
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
    /* 'num_vertices' has the type 'nmd_index' because the number of vertices is always less or equal the number of indices. */
    nmd_index num_vertices; 

    nmd_index num_indices;
    nmd_tex_id user_texture_id;

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
    int width;
    int height;
    uint8_t* pixels8;
    nmd_color* pixels32;
    void* baked_chars; /* internal */
    nmd_tex_id font_id;
} nmd_atlas;

typedef struct
{
    bool line_anti_aliasing; /* If true, all lines will have AA applied to them. */
    bool fill_anti_aliasing; /* If true, all filled polygons will have AA applied to them. */

    nmd_vec2 cached_circle_vertices12[12];
    uint8_t cached_circle_segment_counts64[64];
    float curve_tessellation_tolerance;

    nmd_vec2* path;
    size_t num_points; /* number of points('nmd_vec2') in the 'path' buffer. */
    size_t path_capacity; /* size of the 'path' buffer in bytes. */

    nmd_vertex* vertices;
    size_t num_vertices; /* number of vertices in the 'vertices' buffer. */
    size_t vertices_capacity; /* size of the 'vertices' buffer in bytes. */

    nmd_index* indices;
    size_t num_indices; /* number of indices in the 'indices' buffer. */
    size_t indices_capacity; /* size of the 'indices' buffer in bytes. */

    nmd_draw_command* draw_commands;
    size_t num_draw_commands; /* number of draw commands in the 'draw_commands' buffer. */
    size_t draw_commands_capacity; /* size of the 'draw_commands' buffer in bytes. */

    nmd_atlas default_atlas;
    nmd_tex_id blank_tex_id;

    /*
    void PathBezierCurveTo(const Vec2& p2, const Vec2& p3, const Vec2& p4, size_t num_segments);*/
} nmd_drawlist;

typedef struct
{
    uint32_t id; /* A 32-bit number that identifies the window */
    nmd_rect rect; /* Specifies the area that the windows lies */
    bool visible; /* True if the window is visible */
    bool moving; /* True if the window is being moved by the mouse */
    bool collapsed; /* True if the window is collapsed */
    bool allow_close; /* The has a close button and can be closed(visble=0) */
    bool allow_move_title_bar; /* The window can be moved when the mouse drags the title bar */
    bool allow_move_body; /* The window can be moved when the mouse drags the window's body */
    bool allow_collapse; /* The window has a collpse button and can be collapsed */
    bool allow_resize; /* The window can be resized */
    int y_offset;
} nmd_window;

typedef struct
{
    /* The position of the mouse relative to the window */
    nmd_vec2 mouse_pos;

    /* The state of keyboard keys */
    bool keys_down[256];

    /* The state of mouse's buttons
    mouse_down[0]: left button
    mouse_down[1]: right button
    mouse_down[2]: middle button
    mouse_down[3]/mouse_down[4]: special buttons
    */
    bool mouse_down[5];

    bool mouse_released[5]; /* Mouse was released. nmd_end_frame() clears this array, so it's only used one frame */
    nmd_vec2 mouse_clicked_pos[5]; /* Position when mouse button was clicked */

    nmd_vec2 window_move_delta; /* The delta pos between the top left corner of the window being moved and the mouse's position when it was pressed*/
} nmd_io;

typedef struct
{
    nmd_window* window; /* The current window being accessed */
    nmd_window* windows; /* An array of windows */
    size_t num_windows; /* The number of windows in the 'windows' array */
    size_t windows_capacity; /* The capacity of the 'windows' array */
    nmd_vec2 window_pos; /* The window's initial position */
    char fmt_buffer[1024]; /* temporary buffer */
} nmd_gui;

typedef struct
{
    nmd_drawlist draw_list; /* Vertices, indices, draw commands */
    nmd_io io; /* IO data */
    nmd_gui gui; /* Windows, gui related data */

#ifdef _WIN32
    HWND hWnd;
#endif /* _WIN32*/
} nmd_context;

#ifdef _WIN32
void nmd_win32_set_hwnd(HWND hWnd);
#endif /* _WIN32*/

void nmd_path_to(float x0, float y0);
void nmd_path_rect(float x0, float y0, float x1, float y1, float rounding, uint32_t rounding_corners);

/*
'start_at_center' places the first vertex at the center, this can be used to create a pie chart when using nmd_path_fill_convex().
This functions uses twelve(12) chached vertices initialized during startup, it should be faster than PathArcTo.
*/
void nmd_path_arc_to_cached(float x0, float y0, float radius, size_t start_angle_of12, size_t end_angle_of12, bool start_at_center);
void nmd_path_arc_to(float x0, float y0, float radius, float start_angle, float end_angle, size_t num_segments, bool start_at_center);

void nmd_path_fill_convex(nmd_color color);
void nmd_path_stroke(nmd_color color, bool closed, float thickness);

void nmd_add_line(float x0, float y0, float x1, float y1, nmd_color color, float thickness);

void nmd_add_rect(float x0, float y0, float x1, float y1, nmd_color color, float rounding, uint32_t corner_flags, float thickness);
void nmd_add_rect_filled(float x0, float y0, float x1, float y1, nmd_color color, float rounding, uint32_t corner_flags);
void nmd_add_rect_filled_multi_color(float x0, float y0, float x1, float y1, nmd_color color_upper_left, nmd_color color_upper_right, nmd_color color_bottom_right, nmd_color color_bottom_left);

void nmd_add_quad(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, nmd_color color, float thickness);
void nmd_add_quad_filled(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, nmd_color color);

void nmd_add_triangle(float x0, float y0, float x1, float y1, float x2, float y2, nmd_color color, float thickness);
void nmd_add_triangle_filled(float x0, float y0, float x1, float y1, float x2, float y2, nmd_color color);

void nmd_add_dummy_text(float x, float y, const char* text, float height, nmd_color color, float spacing);

/*
Set num_segments to zero if you want the function to automatically determine the number of segmnts.
num_segments = 12
*/
void nmd_add_circle(float x0, float y0, float radius, nmd_color color, size_t num_segments, float thickness);
void nmd_add_circle_filled(float x0, float y0, float radius, nmd_color color, size_t num_segments);

void nmd_add_ngon(float x0, float y0, float radius, nmd_color color, size_t num_segments, float thickness);
void nmd_add_ngon_filled(float x0, float y0, float radius, nmd_color color, size_t num_segments);

void nmd_add_polyline(const nmd_vec2* points, size_t num_points, nmd_color color, bool closed, float thickness);
void nmd_add_convex_polygon_filled(const nmd_vec2* points, size_t num_points, nmd_color color);
/*void nmd_add_bezier_curve(nmd_vec2 p0, nmd_vec2 p1, nmd_vec2 p2, nmd_vec2 p3, nmd_color color, float thickness, size_t num_segments);*/

/*void nmd_add_text(float x, float y, const char* text, nmd_color color);*/
void nmd_get_text_size(const nmd_atlas* font, const char* text, const char* text_end, nmd_vec2* size_out);
void nmd_add_text(const nmd_atlas* font, float x, float y, const char* text, const char* text_end, nmd_color color);

void nmd_add_image(nmd_tex_id user_texture_id, float x0, float y0, float x1, float y1, nmd_color color);
void nmd_add_image_uv(nmd_tex_id user_texture_id, float x0, float y0, float x1, float y1, float uv_x0, float uv_y0, float uv_x1, float uv_y1, nmd_color color);

void nmd_add_image_quad(nmd_tex_id user_texture_id, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, nmd_color color);
void nmd_add_image_quad_uv(nmd_tex_id user_texture_id, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float uv_x0, float uv_y0, float uv_x1, float uv_y1, float uv_x2, float uv_y2, float uv_x3, float uv_y3, nmd_color color);

void nmd_add_image_rounded(nmd_tex_id user_texture_id, float x0, float y0, float x1, float y1, float rounding, uint32_t corner_flags, nmd_color color);
void nmd_add_image_rounded_uv(nmd_tex_id user_texture_id, float x0, float y0, float x1, float y1, float rounding, uint32_t corner_flags, float uv_x0, float uv_y0, float uv_x1, float uv_y1, nmd_color color);

void nmd_prim_rect_uv(float x0, float y0, float x1, float y1, float uv_x0, float uv_y0, float uv_x1, float uv_y1, nmd_color color);
void nmd_prim_quad_uv(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float uv_x0, float uv_y0, float uv_x1, float uv_y1, float uv_x2, float uv_y2, float uv_x3, float uv_y3, nmd_color color);

nmd_color nmd_rgb(uint8_t r, uint8_t g, uint8_t b);
nmd_color nmd_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

nmd_context* nmd_get_context();

/* Specifies the begin of the window. Widgets can be added after calling this function. Returns true if the window is not minimized */
bool nmd_begin(const char* window_name);

/* Specifies the end of the window. Widgets won't be added after calling this function */
void nmd_end();

/* Appends text to the window. Be careful, the formatted string must not exceed 1024 bytes + null terminator */
void nmd_text(const char* fmt, ...);

/* Adds a button widget. Returns true if the button is clicked */
bool nmd_button(const char* label);

/* Adds a checkbox widget. Returns true if the checkbox state changes. */
bool nmd_checkbox(const char* label, bool* checked);

/* Adds a float slider widget. Returns true if the value changes. */
bool nmd_slider_float(const char* label, float* value, float min_value, float max_value);

/* Starts a new empty scene/frame. Internally this function clears all vertices, indices and command buffers. */
void nmd_new_frame();

/* "Ends" a frame. Wrapper around nmd_push_draw_command(). */
void nmd_end_frame();

/*
Creates one or more draw commands for the unaccounted vertices and indices.
Parameters:
 clip_rect [opt/in] A pointer to a rect that specifies the clip area. This parameter can be null.
*/
void nmd_push_draw_command(const nmd_rect* clip_rect);

void nmd_push_texture_draw_command(nmd_tex_id user_texture_id, const nmd_rect* clip_rect);

bool nmd_bake_font_from_memory(const void* font_data, nmd_atlas* atlas, float size);

bool nmd_bake_font(const char* font_path, nmd_atlas* atlas, float size);

#define NMD_COLOR_BLACK                 nmd_rgb(  0,   0,   0)
#define NMD_COLOR_WHITE                 nmd_rgb(255, 255, 255)
#define NMD_COLOR_RED                   nmd_rgb(255,   0,   0)
#define NMD_COLOR_GREEN                 nmd_rgb(  0, 255,   0)
#define NMD_COLOR_BLUE                  nmd_rgb(  0,   0, 255)
#define NMD_COLOR_ORANGE                nmd_rgb(255, 165,   0)
#define NMD_COLOR_AMBER                 nmd_rgb(255, 191,   0)
#define NMD_COLOR_ANDROID_GREEN         nmd_rgb(164, 198,  57)
#define NMD_COLOR_AZURE                 nmd_rgb(  0, 127, 255)
#define NMD_COLOR_BRONZE                nmd_rgb(205, 127,  50)
#define NMD_COLOR_CORN                  nmd_rgb(251, 236,  93)
#define NMD_COLOR_EMERALD               nmd_rgb( 80, 200, 120)
#define NMD_COLOR_LAPIS_LAZULI          nmd_rgb( 38,  97, 156)
#define NMD_COLOR_LAVA                  nmd_rgb(207,  16,  32)

#define NMD_COLOR_GUI_MAIN              nmd_rgb( 58, 109,  41)
#define NMD_COLOR_GUI_WIDGET_BACKGROUND nmd_rgb( 38,  66,  29)
#define NMD_COLOR_GUI_WIDGET_HOVER      nmd_rgb( 51,  91,  37)
#define NMD_COLOR_GUI_BUTTON_BACKGROUND NMD_COLOR_GUI_WIDGET_HOVER
#define NMD_COLOR_GUI_BUTTON_HOVER      nmd_rgb( 60, 122,  37)
#define NMD_COLOR_GUI_HOVER             nmd_rgb( 91, 178,  62)
#define NMD_COLOR_GUI_PRESSED           nmd_rgb( 69, 160,  38)
#define NMD_COLOR_GUI_ACTIVE            nmd_rgb( 53, 160,  17)
#define NMD_COLOR_GUI_BACKGROUND        nmd_rgb( 20,  20,  20)

#endif /* NMD_GRAPHICS_H */
