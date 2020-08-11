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
 clipRect [opt/in] A pointer to a rect that specifies the clip area. This parameter can be null.
*/
void nmd_push_draw_command(const nmd_rect* clipRect)
{
    size_t numAccountedVertices = 0, numAccountedIndices = 0;
    size_t i = 0;
    for (; i < _nmd_context.drawList.numDrawCommands; i++)
    {
        numAccountedVertices += _nmd_context.drawList.drawCommands[i].numVertices;
        numAccountedIndices += _nmd_context.drawList.drawCommands[i].numIndices;
    }

    size_t numUnaccountedIndices = _nmd_context.drawList.numIndices - numAccountedIndices;

    while (numUnaccountedIndices > 0)
    {
        /* If the number of unaccounted indices is less than the maximum number of indices that can be hold by 'nmd_index'(usually 2^16). */
        if (numUnaccountedIndices <= (1 << (8 * sizeof(nmd_index))))
        {
            /* Add draw command */
            _nmd_context.drawList.drawCommands[_nmd_context.drawList.numDrawCommands].numVertices = _nmd_context.drawList.numVertices - numAccountedVertices;
            _nmd_context.drawList.drawCommands[_nmd_context.drawList.numDrawCommands].numIndices = numUnaccountedIndices;
            _nmd_context.drawList.drawCommands[_nmd_context.drawList.numDrawCommands].userTextureId = _nmd_context.drawList.font;
            if (clipRect)
                _nmd_context.drawList.drawCommands[_nmd_context.drawList.numDrawCommands].rect = *clipRect;
            else
                _nmd_context.drawList.drawCommands[_nmd_context.drawList.numDrawCommands].rect.p1.x = -1.0f;
            _nmd_context.drawList.numDrawCommands++;
            return;
        }
        else
        {
            size_t numIndices = (2 << (8 * sizeof(nmd_index) - 1)) - 1;
            nmd_index lastIndex = _nmd_context.drawList.indices[numIndices - 1];

            bool isLastIndexReferenced = false;
            do
            {
                for (size_t i = numIndices; i < numUnaccountedIndices; i++)
                {
                    if (_nmd_context.drawList.indices[i] == lastIndex)
                    {
                        isLastIndexReferenced = true;
                        numIndices -= 3;
                        lastIndex = _nmd_context.drawList.indices[numIndices - 1];
                        break;
                    }
                }
            } while (isLastIndexReferenced);

            _nmd_context.drawList.drawCommands[_nmd_context.drawList.numDrawCommands].numVertices = lastIndex + 1;
            _nmd_context.drawList.drawCommands[_nmd_context.drawList.numDrawCommands].numIndices = numIndices;
            _nmd_context.drawList.drawCommands[_nmd_context.drawList.numDrawCommands].userTextureId = 0;
            _nmd_context.drawList.numDrawCommands++;
            
            numUnaccountedIndices -= numIndices;
        }
    }
}

void nmd_push_texture_draw_command(nmd_tex_id userTextureId, const nmd_rect* clipRect)
{
    size_t numAccountedVertices = 0, numAccountedIndices = 0;
    size_t i = 0;
    for (; i < _nmd_context.drawList.numDrawCommands; i++)
    {
        numAccountedVertices += _nmd_context.drawList.drawCommands[i].numVertices;
        numAccountedIndices += _nmd_context.drawList.drawCommands[i].numIndices;
    }

    const size_t numUnaccountedIndices = _nmd_context.drawList.numIndices - numAccountedIndices;

    _nmd_context.drawList.drawCommands[_nmd_context.drawList.numDrawCommands].numVertices = _nmd_context.drawList.numVertices - numAccountedVertices;
    _nmd_context.drawList.drawCommands[_nmd_context.drawList.numDrawCommands].numIndices = numUnaccountedIndices;
    _nmd_context.drawList.drawCommands[_nmd_context.drawList.numDrawCommands].userTextureId = userTextureId;
    if (clipRect)
        _nmd_context.drawList.drawCommands[_nmd_context.drawList.numDrawCommands].rect = *clipRect;
    else
        _nmd_context.drawList.drawCommands[_nmd_context.drawList.numDrawCommands].rect.p1.x = -1.0f;
    _nmd_context.drawList.numDrawCommands++;
}

void _nmd_calculate_circle_segments(float maxError)
{
    for (size_t i = 0; i < 64; i++)
    {
        const uint8_t segment_count = NMD_CIRCLE_AUTO_SEGMENT_CALC(i + 1.0f, maxError);
        _nmd_context.drawList.cachedCircleSegmentCounts64[i] = NMD_MIN(segment_count, 255);
    }
}

/* Starts a new empty scene/frame. Internally this function clears all vertices, indices and command buffers. */
void nmd_new_frame()
{
    if (!_nmd_initialized)
    {
        _nmd_initialized = true;

        _nmd_context.drawList.lineAntiAliasing = true;
        _nmd_context.drawList.fillAntiAliasing = true;

        for (size_t i = 0; i < 12; i++)
        {
            const float angle = (i / 12.0f) * NMD_2PI;
            _nmd_context.drawList.cachedCircleVertices12[i].x = NMD_COS(angle);
            _nmd_context.drawList.cachedCircleVertices12[i].y = NMD_SIN(angle);
        }
        
        _nmd_calculate_circle_segments(1.6f);

        /* Allocate buffers */
        _nmd_context.drawList.path = (nmd_vec2*)NMD_ALLOC(NMD_PATH_BUFFER_INITIAL_SIZE * sizeof(nmd_vec2));
        _nmd_context.drawList.pathCapacity = NMD_PATH_BUFFER_INITIAL_SIZE * sizeof(nmd_vec2);

        _nmd_context.drawList.vertices = (nmd_vertex*)NMD_ALLOC(NMD_VERTEX_BUFFER_INITIAL_SIZE * sizeof(nmd_vertex));
        _nmd_context.drawList.verticesCapacity = NMD_VERTEX_BUFFER_INITIAL_SIZE * sizeof(nmd_vertex);

        _nmd_context.drawList.indices = (nmd_index*)NMD_ALLOC(NMD_INDEX_BUFFER_INITIAL_SIZE * sizeof(nmd_index));
        _nmd_context.drawList.indicesCapacity = NMD_INDEX_BUFFER_INITIAL_SIZE * sizeof(nmd_index);

        _nmd_context.drawList.drawCommands = (nmd_draw_command*)NMD_ALLOC(NMD_DRAW_COMMANDS_BUFFER_INITIAL_SIZE * sizeof(nmd_draw_command));
        _nmd_context.drawList.drawCommandsCapacity = NMD_DRAW_COMMANDS_BUFFER_INITIAL_SIZE * sizeof(nmd_draw_command);
    }

    _nmd_context.drawList.numVertices = 0;
    _nmd_context.drawList.numIndices = 0;
    _nmd_context.drawList.numDrawCommands = 0;
}

/* "Ends" a frame. Wrapper around nmd_push_draw_command(). */
void nmd_end_frame()
{
    nmd_push_draw_command(0);
}

bool nmd_bake_font(const char* fontPath, nmd_atlas* atlas, float size)
{
    atlas->width = 512;
    atlas->height = 512;

    FILE* f = fopen(fontPath, "rb");
    fseek(f, 0L, SEEK_END);
    const size_t fileSize = ftell(f);
    fseek(f, 0L, SEEK_SET);
    atlas->fontData = NMD_ALLOC(fileSize);
    atlas->pixels8 = (uint8_t*)NMD_ALLOC(atlas->width * atlas->height);
    atlas->pixels32 = (nmd_color*)NMD_ALLOC(atlas->width * atlas->height * 4);
    fread(atlas->fontData, 1, fileSize, f);
    atlas->bakedChars = NMD_ALLOC(sizeof(stbtt_bakedchar) * 96);
    stbtt_BakeFontBitmap((unsigned char*)atlas->fontData, 0, size, atlas->pixels8 , atlas->width, atlas->height, 0x20, 96, (stbtt_bakedchar*)atlas->bakedChars);
    fclose(f);

    size_t i = 0;
    for (; i < atlas->width * atlas->height; i++)
        atlas->pixels32[i] = nmd_rgba(255, 255, 255, atlas->pixels8[i]);

    return true;
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