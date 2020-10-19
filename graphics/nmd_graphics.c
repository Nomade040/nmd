#include "nmd_common.h"

nmd_context _nmd_context;
bool _nmd_initialized = false;

nmd_color nmd_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return nmd_rgba(r, g, b, 0xff);
}

nmd_color nmd_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    nmd_color color;
    color.r = NMD_CLAMP(r, 0, 255);
    color.g = NMD_CLAMP(g, 0, 255);
    color.b = NMD_CLAMP(b, 0, 255);
    color.a = NMD_CLAMP(a, 0, 255);

    return color;
}

nmd_context* nmd_get_context()
{
    return &_nmd_context;
}

/*
Creates one or more draw commands for the unaccounted vertices and indices.
Parameters:
 clip_rect [opt/in] A pointer to a rect that specifies the clip area. This parameter can be null.
*/
void nmd_push_draw_command(const nmd_rect* clip_rect)
{
    /* Calculate the number of vertices and indices present in draw commands */
    size_t num_accounted_vertices = 0, num_accounted_indices = 0;
    size_t i = 0;
    for (; i < _nmd_context.draw_list.num_draw_commands; i++)
    {
        num_accounted_vertices += _nmd_context.draw_list.draw_commands[i].num_vertices;
        num_accounted_indices += _nmd_context.draw_list.draw_commands[i].num_indices;
    }

    /* Calculate the number of vertices and indices NOT present in draw commands */
    size_t num_unaccounted_indices = _nmd_context.draw_list.num_indices - num_accounted_indices;

    /* Create draw commands until all vertices and indices are present in draw commands */
    while (num_unaccounted_indices > 0)
    {
        /* If the number of unaccounted indices is less than the maximum number of indices that can be hold by 'nmd_index'(usually 2^16) */
        if (num_unaccounted_indices < (1 << (8 * sizeof(nmd_index))))
        {
            /* Add draw command */
            _nmd_context.draw_list.draw_commands[_nmd_context.draw_list.num_draw_commands].num_vertices = _nmd_context.draw_list.num_vertices - num_accounted_vertices;
            _nmd_context.draw_list.draw_commands[_nmd_context.draw_list.num_draw_commands].num_indices = _nmd_context.draw_list.num_indices - num_accounted_indices;
            _nmd_context.draw_list.draw_commands[_nmd_context.draw_list.num_draw_commands].user_texture_id = _nmd_context.draw_list.blank_tex_id;
            if (clip_rect)
                _nmd_context.draw_list.draw_commands[_nmd_context.draw_list.num_draw_commands].rect = *clip_rect;
            else
                _nmd_context.draw_list.draw_commands[_nmd_context.draw_list.num_draw_commands].rect.p1.x = -1.0f;

            _nmd_context.draw_list.num_draw_commands++;
            return;
        }
        else
        {
            size_t num_indices = (1 << (8 * sizeof(nmd_index)));
            nmd_index last_index = _nmd_context.draw_list.indices[num_indices - 1];

            bool is_last_index_referenced = false;
            do
            {
                for (size_t i = num_indices; i < num_unaccounted_indices; i++)
                {
                    if (_nmd_context.draw_list.indices[i] == last_index)
                    {
                        is_last_index_referenced = true;
                        num_indices -= 3;
                        last_index = _nmd_context.draw_list.indices[num_indices - 1];
                        break;
                    }
                }
            } while (is_last_index_referenced);

            _nmd_context.draw_list.draw_commands[_nmd_context.draw_list.num_draw_commands].num_vertices = last_index + 1;
            _nmd_context.draw_list.draw_commands[_nmd_context.draw_list.num_draw_commands].num_indices = num_indices;
            _nmd_context.draw_list.draw_commands[_nmd_context.draw_list.num_draw_commands].user_texture_id = _nmd_context.draw_list.blank_tex_id;

            _nmd_context.draw_list.num_draw_commands++;

            num_unaccounted_indices -= num_indices;
        }
    }
}

void nmd_push_texture_draw_command(nmd_tex_id user_texture_id, const nmd_rect* clip_rect)
{
    size_t num_accounted_vertices = 0, num_accounted_indices = 0;
    size_t i = 0;
    for (; i < _nmd_context.draw_list.num_draw_commands; i++)
    {
        num_accounted_vertices += _nmd_context.draw_list.draw_commands[i].num_vertices;
        num_accounted_indices += _nmd_context.draw_list.draw_commands[i].num_indices;
    }

    const size_t num_unaccounted_indices = _nmd_context.draw_list.num_indices - num_accounted_indices;

    _nmd_context.draw_list.draw_commands[_nmd_context.draw_list.num_draw_commands].num_vertices = _nmd_context.draw_list.num_vertices - num_accounted_vertices;
    _nmd_context.draw_list.draw_commands[_nmd_context.draw_list.num_draw_commands].num_indices = num_unaccounted_indices;
    _nmd_context.draw_list.draw_commands[_nmd_context.draw_list.num_draw_commands].user_texture_id = user_texture_id;
    if (clip_rect)
        _nmd_context.draw_list.draw_commands[_nmd_context.draw_list.num_draw_commands].rect = *clip_rect;
    else
        _nmd_context.draw_list.draw_commands[_nmd_context.draw_list.num_draw_commands].rect.p1.x = -1.0f;
    _nmd_context.draw_list.num_draw_commands++;
}

void _nmd_calculate_circle_segments(float max_error)
{
    for (size_t i = 0; i < 64; i++)
    {
        const uint8_t segment_count = NMD_CIRCLE_AUTO_SEGMENT_CALC(i + 1.0f, max_error);
        _nmd_context.draw_list.cached_circle_segment_counts64[i] = NMD_MIN(segment_count, 255);
    }
}

#ifdef _WIN32
void nmd_win32_set_hwnd(HWND hWnd)
{
    _nmd_context.hWnd = hWnd;
}
#endif /* _WIN32*/

/* Starts a new empty scene/frame. Internally this function clears all vertices, indices and command buffers. */
void nmd_new_frame()
{
    if (!_nmd_initialized)
    {
        _nmd_initialized = true;

        _nmd_context.draw_list.line_anti_aliasing = true;
        _nmd_context.draw_list.fill_anti_aliasing = true;

        for (size_t i = 0; i < 12; i++)
        {
            const float angle = (i / 12.0f) * NMD_2PI;
            _nmd_context.draw_list.cached_circle_vertices12[i].x = NMD_COS(angle);
            _nmd_context.draw_list.cached_circle_vertices12[i].y = NMD_SIN(angle);
        }
        
        _nmd_calculate_circle_segments(1.6f);

        /* Allocate buffers */
        _nmd_context.draw_list.path = (nmd_vec2*)NMD_MALLOC(NMD_PATH_BUFFER_INITIAL_SIZE * sizeof(nmd_vec2));
        _nmd_context.draw_list.path_capacity = NMD_PATH_BUFFER_INITIAL_SIZE * sizeof(nmd_vec2);

        _nmd_context.draw_list.vertices = (nmd_vertex*)NMD_MALLOC(NMD_VERTEX_BUFFER_INITIAL_SIZE * sizeof(nmd_vertex));
        _nmd_context.draw_list.vertices_capacity = NMD_VERTEX_BUFFER_INITIAL_SIZE * sizeof(nmd_vertex);

        _nmd_context.draw_list.indices = (nmd_index*)NMD_MALLOC(NMD_INDEX_BUFFER_INITIAL_SIZE * sizeof(nmd_index));
        _nmd_context.draw_list.indices_capacity = NMD_INDEX_BUFFER_INITIAL_SIZE * sizeof(nmd_index);

        _nmd_context.draw_list.draw_commands = (nmd_draw_command*)NMD_MALLOC(NMD_DRAW_COMMANDS_BUFFER_INITIAL_SIZE * sizeof(nmd_draw_command));
        _nmd_context.draw_list.draw_commands_capacity = NMD_DRAW_COMMANDS_BUFFER_INITIAL_SIZE * sizeof(nmd_draw_command);

        _nmd_context.gui.num_windows = 0;
        _nmd_context.gui.windows = (nmd_window*)NMD_MALLOC(NMD_WINDOWS_BUFFER_INITIAL_SIZE * sizeof(nmd_window));
        _nmd_context.gui.windows_capacity = NMD_WINDOWS_BUFFER_INITIAL_SIZE;
        _nmd_context.gui.window = 0;
        _nmd_context.gui.window_pos.x = 60;
        _nmd_context.gui.window_pos.y = 60;
    }

    _nmd_context.draw_list.num_vertices = 0;
    _nmd_context.draw_list.num_indices = 0;
    _nmd_context.draw_list.num_draw_commands = 0;

#ifdef _WIN32
    POINT point;
    if (_nmd_context.hWnd && GetCursorPos(&point) && ScreenToClient(_nmd_context.hWnd, &point))
    {
        _nmd_context.io.mouse_pos.x = point.x;
        _nmd_context.io.mouse_pos.y = point.y;
    }
#endif /* _WIN32 */
}

/* "Ends" a frame. Wrapper around nmd_push_draw_command(). */
void nmd_end_frame()
{
    /* Clears the mouse released state because it should only be used once */
    for (size_t i = 0; i < 5; i++)
        _nmd_context.io.mouse_released[i] = false;

    nmd_push_draw_command(0);
}

bool nmd_bake_font_from_memory(const void* font_data, nmd_atlas* atlas, float size)
{
    atlas->width = 512;
    atlas->height = 512;

    atlas->pixels8 = (uint8_t*)NMD_MALLOC(atlas->width * atlas->height);
    atlas->pixels32 = (nmd_color*)NMD_MALLOC(atlas->width * atlas->height * 4);
    atlas->baked_chars = NMD_MALLOC(sizeof(stbtt_bakedchar) * 96);

    stbtt_BakeFontBitmap((const unsigned char*)font_data, 0, size, atlas->pixels8, atlas->width, atlas->height, 0x20, 96, (stbtt_bakedchar*)atlas->baked_chars);

    size_t i = 0;
    for (; i < atlas->width * atlas->height; i++)
        atlas->pixels32[i] = nmd_rgba(255, 255, 255, atlas->pixels8[i]);

    return true;
}

bool nmd_bake_font(const char* font_path, nmd_atlas* atlas, float size)
{
    bool ret = false;

#ifndef NMD_GRAPHICS_DISABLE_FILE_IO
    FILE* f = fopen(font_path, "rb");

    /* Get file size*/
    fseek(f, 0L, SEEK_END);
    const size_t file_size = ftell(f);
    fseek(f, 0L, SEEK_SET);

    /* Allocate and read file */
    void* font_data = NMD_MALLOC(file_size);    
    fread(font_data, 1, file_size, f);

    ret = nmd_bake_font_from_memory(font_data, atlas, size);
    
    /* Close and free file */
    fclose(f);
    NMD_FREE(font_data);
#endif

    return ret;
}

//bool nmd_bake_font(nmd_atlas* atlas)
//{
//
//    int i = 0;
//    void* tmp = 0;
//    size_t tmp_size, img_size;
//    struct nk_font* font_iter;
//    struct nk_font_baker* baker;
//
//#ifdef NK_INCLUDE_DEFAULT_FONT
//    /* no font added so just use default font */
//    if (!atlas->font_num)
//        atlas->default_font = nk_font_atlas_add_default(atlas, 13.0f, 0);
//#endif
//    NK_ASSERT(atlas->font_num);
//    if (!atlas->font_num) return 0;
//
//    /* allocate temporary baker memory required for the baking process */
//    nk_font_baker_memory(&tmp_size, &atlas->glyph_count, atlas->config, atlas->font_num);
//    tmp = atlas->temporary.alloc(atlas->temporary.userdata, 0, tmp_size);
//    NK_ASSERT(tmp);
//    if (!tmp) goto failed;
//    memset(tmp, 0, tmp_size);
//
//    /* allocate glyph memory for all fonts */
//    baker = nk_font_baker(tmp, atlas->glyph_count, atlas->font_num, &atlas->temporary);
//    atlas->glyphs = (struct nk_font_glyph*)atlas->permanent.alloc(
//        atlas->permanent.userdata, 0, sizeof(struct nk_font_glyph) *(size_t)atlas->glyph_count);
//    NK_ASSERT(atlas->glyphs);
//    if (!atlas->glyphs)
//        goto failed;
//
//    /* pack all glyphs into a tight fit space */
//    atlas->custom.w = (NK_CURSOR_DATA_W * 2) + 1;
//    atlas->custom.h = NK_CURSOR_DATA_H + 1;
//    if (!nk_font_bake_pack(baker, &img_size, width, height, &atlas->custom,
//        atlas->config, atlas->font_num, &atlas->temporary))
//        goto failed;
//
//    /* allocate memory for the baked image font atlas */
//    atlas->pixel = atlas->temporary.alloc(atlas->temporary.userdata, 0, img_size);
//    NK_ASSERT(atlas->pixel);
//    if (!atlas->pixel)
//        goto failed;
//
//    /* bake glyphs and custom white pixel into image */
//    nk_font_bake(baker, atlas->pixel, *width, *height,
//        atlas->glyphs, atlas->glyph_count, atlas->config, atlas->font_num);
//    nk_font_bake_custom_data(atlas->pixel, *width, *height, atlas->custom,
//        nk_custom_cursor_data, NK_CURSOR_DATA_W, NK_CURSOR_DATA_H, '.', 'X');
//
//    if (fmt == NK_FONT_ATLAS_RGBA32) {
//        /* convert alpha8 image into rgba32 image */
//        void* img_rgba = atlas->temporary.alloc(atlas->temporary.userdata, 0,
//            (size_t)(*width * *height * 4));
//        NK_ASSERT(img_rgba);
//        if (!img_rgba) goto failed;
//        nk_font_bake_convert(img_rgba, *width, *height, atlas->pixel);
//        atlas->temporary.free(atlas->temporary.userdata, atlas->pixel);
//        atlas->pixel = img_rgba;
//    }
//    atlas->tex_width = *width;
//    atlas->tex_height = *height;
//
//    /* initialize each font */
//    for (font_iter = atlas->fonts; font_iter; font_iter = font_iter->next) {
//        struct nk_font* font = font_iter;
//        struct nk_font_config* config = font->config;
//        nk_font_init(font, config->size, config->fallback_glyph, atlas->glyphs,
//            config->font, nk_handle_ptr(0));
//    }
//
//failed:
//    /* error so cleanup all memory */
//    if (tmp) atlas->temporary.free(atlas->temporary.userdata, tmp);
//    if (atlas->glyphs) {
//        atlas->permanent.free(atlas->permanent.userdata, atlas->glyphs);
//        atlas->glyphs = 0;
//    }
//    if (atlas->pixel) {
//        atlas->temporary.free(atlas->temporary.userdata, atlas->pixel);
//        atlas->pixel = 0;
//    }
//    return 0;
//}
