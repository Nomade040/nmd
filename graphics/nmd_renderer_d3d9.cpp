#include "nmd_common.h"

#ifdef NMD_GRAPHICS_D3D9
#pragma comment(lib, "d3d9.lib")
struct
{
    LPDIRECT3DDEVICE9 device = 0;
    LPDIRECT3DVERTEXBUFFER9 vb = 0; /* vertex buffer */
    LPDIRECT3DINDEXBUFFER9 ib = 0; /* index buffer*/
    int vb_size, ib_size; /* The number of vertices and indices respectively. */
    D3DMATRIX proj;
    D3DVIEWPORT9 viewport;
} _nmd_d3d9;

typedef struct
{
    float pos[3];
    D3DCOLOR color;
    float uv[2];
} _nmd_d3d9_custom_vertex;

#define _NMD_D3D9_CUSTOM_VERTEX_FVF (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)

void nmd_d3d9_set_device(LPDIRECT3DDEVICE9 p_d3d9_device)
{
    _nmd_d3d9.device = p_d3d9_device;

    _nmd_d3d9.viewport.X;
    _nmd_d3d9.viewport.Y = 0;
    _nmd_d3d9.viewport.MinZ = 0.0f;
    _nmd_d3d9.viewport.MaxZ = 1.0f;

    int width = 16, height = 16;
    unsigned char* pixels = (unsigned char*)NMD_MALLOC(width * height * 4);
    
    NMD_MEMSET(pixels, 0xff, width * height * 4);

    _nmd_context.draw_list.default_atlas.font_id = nmd_d3d9_create_texture(pixels, width, height);
}

nmd_tex_id nmd_d3d9_create_texture(void* pixels, int width, int height)
{
    IDirect3DTexture9* texture;
    if (_nmd_d3d9.device->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture, NULL) != D3D_OK)
        return 0;

    D3DLOCKED_RECT tex_locked_rect;
    if (texture->LockRect(0, &tex_locked_rect, NULL, 0) != D3D_OK)
        return 0;

    for (int y = 0; y < height; y++)
        NMD_MEMCPY((unsigned char*)tex_locked_rect.pBits + tex_locked_rect.Pitch * y, (unsigned char*)pixels + (width * 4) * y, (width * 4));
   
    texture->UnlockRect(0);
    
    return (nmd_tex_id)texture;
}

void nmd_d3d9_resize(int width, int height)
{
    const float L = 0.0f;
    const float R = (float)width + 0.0f;
    const float T = 0.0f;
    const float B = (float)height + 0.0f;
    float matrix[4][4] = {
        {    2.0f / (R - L),              0.0f, 0.0f, 0.0f },
        {              0.0f,    2.0f / (T - B), 0.0f, 0.0f },
        {              0.0f,              0.0f, 0.0f, 0.0f },
        { (R + L) / (L - R), (T + B) / (B - T), 0.0f, 1.0f },
    };
    NMD_MEMCPY(&_nmd_d3d9.proj, matrix, sizeof(matrix));

    _nmd_d3d9.viewport.Width = width;
    _nmd_d3d9.viewport.Height = height;
}

void _nmd_d3d9_set_render_state()
{
    _nmd_d3d9.device->SetStreamSource(0, _nmd_d3d9.vb, 0, sizeof(_nmd_d3d9_custom_vertex));
    _nmd_d3d9.device->SetIndices(_nmd_d3d9.ib);
    _nmd_d3d9.device->SetFVF(_NMD_D3D9_CUSTOM_VERTEX_FVF);
    _nmd_d3d9.device->SetTransform(D3DTS_PROJECTION, &_nmd_d3d9.proj);
    _nmd_d3d9.device->SetViewport(&_nmd_d3d9.viewport);
    _nmd_d3d9.device->SetPixelShader(NULL);
    _nmd_d3d9.device->SetVertexShader(NULL);
    _nmd_d3d9.device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    _nmd_d3d9.device->SetRenderState(D3DRS_LIGHTING, false);
    _nmd_d3d9.device->SetRenderState(D3DRS_ZENABLE, false);
    _nmd_d3d9.device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
    _nmd_d3d9.device->SetRenderState(D3DRS_ALPHATESTENABLE, false);
    _nmd_d3d9.device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    _nmd_d3d9.device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    _nmd_d3d9.device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    _nmd_d3d9.device->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
    _nmd_d3d9.device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    _nmd_d3d9.device->SetRenderState(D3DRS_FOGENABLE, false);
    _nmd_d3d9.device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    _nmd_d3d9.device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    _nmd_d3d9.device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    _nmd_d3d9.device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    _nmd_d3d9.device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    _nmd_d3d9.device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    _nmd_d3d9.device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    _nmd_d3d9.device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
}

void nmd_d3d9_render()
{
    /* Create/recreate vertex buffer if it doesn't exist or more space is needed */
    if (!_nmd_d3d9.vb || _nmd_d3d9.vb_size < _nmd_context.draw_list.num_vertices)
    {
        if (_nmd_d3d9.vb)
        {
            _nmd_d3d9.vb->Release();
            _nmd_d3d9.vb = 0;
        }

        _nmd_d3d9.vb_size = _nmd_context.draw_list.num_vertices + NMD_VERTEX_BUFFER_INITIAL_SIZE;
        if (_nmd_d3d9.device->CreateVertexBuffer(_nmd_d3d9.vb_size * sizeof(_nmd_d3d9_custom_vertex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, _NMD_D3D9_CUSTOM_VERTEX_FVF, D3DPOOL_DEFAULT, &_nmd_d3d9.vb, NULL) != D3D_OK)
            return;

#ifdef NMD_GRAPHICS_D3D9_OPTIMIZE_RENDER_STATE
        _nmd_d3d9_set_render_state();
#endif /* NMD_GRAPHICS_D3D9_OPTIMIZE_RENDER_STATE */
    }

    /* Create/recreate index buffer if it doesn't exist or more space is needed */
    if (!_nmd_d3d9.ib || _nmd_d3d9.ib_size < _nmd_context.draw_list.num_indices)
    {
        if (_nmd_d3d9.ib)
        {
            _nmd_d3d9.ib->Release();
            _nmd_d3d9.ib = 0;
        }

        _nmd_d3d9.ib_size = _nmd_context.draw_list.num_indices + NMD_INDEX_BUFFER_INITIAL_SIZE;
        if (_nmd_d3d9.device->CreateIndexBuffer(_nmd_d3d9.ib_size * sizeof(nmd_index), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(nmd_index) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &_nmd_d3d9.ib, NULL) < 0)
            return;

#ifdef NMD_GRAPHICS_D3D9_OPTIMIZE_RENDER_STATE
        _nmd_d3d9_set_render_state();
#endif /* NMD_GRAPHICS_D3D9_OPTIMIZE_RENDER_STATE */
    }

    /* Copy vertices to the gpu */
    _nmd_d3d9_custom_vertex* p_vertices = 0;
    if (_nmd_d3d9.vb->Lock(0, (UINT)(_nmd_context.draw_list.num_vertices * sizeof(_nmd_d3d9_custom_vertex)), (void**)&p_vertices, D3DLOCK_DISCARD) != D3D_OK)
        return;
    size_t i = 0;
    for (; i < _nmd_context.draw_list.num_vertices; i++)
    {
        p_vertices[i].pos[0] = _nmd_context.draw_list.vertices[i].pos.x;
        p_vertices[i].pos[1] = _nmd_context.draw_list.vertices[i].pos.y;
        p_vertices[i].pos[2] = 0.0f;

        p_vertices[i].uv[0] = _nmd_context.draw_list.vertices[i].uv.x;
        p_vertices[i].uv[1] = _nmd_context.draw_list.vertices[i].uv.y;

        const nmd_color color = _nmd_context.draw_list.vertices[i].color;
        p_vertices[i].color = D3DCOLOR_RGBA(color.r, color.g, color.b, color.a);
    }
    _nmd_d3d9.vb->Unlock();

    /* Copy indices to the gpu */
    nmd_index* p_indices = 0;
    if (_nmd_d3d9.ib->Lock(0, (UINT)(_nmd_context.draw_list.num_indices * sizeof(nmd_index)), (void**)&p_indices, D3DLOCK_DISCARD) != D3D_OK)
        return;
    NMD_MEMCPY(p_indices, _nmd_context.draw_list.indices, _nmd_context.draw_list.num_indices * sizeof(nmd_index));
    _nmd_d3d9.ib->Unlock();
    
#ifndef NMD_GRAPHICS_D3D9_DONT_BACKUP_RENDER_STATE
    /* Backup the current render state */
    IDirect3DStateBlock9* d3d9_state_block = NULL;
    if (_nmd_d3d9.device->CreateStateBlock(D3DSBT_ALL, &d3d9_state_block) < 0)
        return;
    D3DMATRIX last_world, last_view, last_projection;
    _nmd_d3d9.device->GetTransform(D3DTS_WORLD, &last_world);
    _nmd_d3d9.device->GetTransform(D3DTS_VIEW, &last_view);
    _nmd_d3d9.device->GetTransform(D3DTS_PROJECTION, &last_projection);
#endif /* NMD_GRAPHICS_D3D9_DONT_BACKUP_RENDER_STATE */

#ifndef NMD_GRAPHICS_D3D9_OPTIMIZE_RENDER_STATE
    /* Set render state */
    _nmd_d3d9_set_render_state();
#endif /* NMD_GRAPHICS_D3D9_OPTIMIZE_RENDER_STATE */
    
    /* Render draw commands */
    size_t index_offset = 0;
    for (i = 0; i < _nmd_context.draw_list.num_draw_commands; i++)
    {
        /* Apply scissor rectangle */
        RECT r;
        if (_nmd_context.draw_list.draw_commands[i].rect.p1.x == -1.0f)
            r = { (LONG)_nmd_d3d9.viewport.X, (LONG)_nmd_d3d9.viewport.Y, (LONG)_nmd_d3d9.viewport.Width, (LONG)_nmd_d3d9.viewport.Height };
        else
            r = { (LONG)_nmd_context.draw_list.draw_commands[i].rect.p0.x, (LONG)_nmd_context.draw_list.draw_commands[i].rect.p0.y, (LONG)_nmd_context.draw_list.draw_commands[i].rect.p1.x, (LONG)_nmd_context.draw_list.draw_commands[i].rect.p1.y };
        _nmd_d3d9.device->SetScissorRect(&r);
        
        /* Set texture */
        const LPDIRECT3DTEXTURE9 texture = (LPDIRECT3DTEXTURE9)_nmd_context.draw_list.draw_commands[i].user_texture_id;
        _nmd_d3d9.device->SetTexture(0, texture);

        /* Issue draw calls */
        _nmd_d3d9.device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, (UINT)_nmd_context.draw_list.draw_commands[i].num_vertices, index_offset, _nmd_context.draw_list.draw_commands[i].num_indices / 3);
        
        /* Update offsets */
        index_offset += _nmd_context.draw_list.draw_commands[i].num_indices;
    }

#ifndef NMD_GRAPHICS_D3D9_DONT_BACKUP_RENDER_STATE
    /* Restore previous render state */
    _nmd_d3d9.device->SetTransform(D3DTS_WORLD, &last_world);
    _nmd_d3d9.device->SetTransform(D3DTS_VIEW, &last_view);
    _nmd_d3d9.device->SetTransform(D3DTS_PROJECTION, &last_projection);
    d3d9_state_block->Apply();
    d3d9_state_block->Release();
#endif /* NMD_GRAPHICS_D3D9_DONT_BACKUP_RENDER_STATE */
}
#endif /* NMD_GRAPHICS_D3D9 */
