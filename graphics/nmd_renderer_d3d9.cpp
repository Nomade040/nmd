#include "nmd_common.h"

#ifdef NMD_GRAPHICS_D3D9
#pragma comment(lib, "d3d9.lib")
static LPDIRECT3DDEVICE9 _nmd_d3d9_device = 0;
static LPDIRECT3DVERTEXBUFFER9 _nmd_d3d9_vb = 0; /* vertex buffer */
static LPDIRECT3DINDEXBUFFER9 _nmd_d3d9_ib = 0; /* index buffer*/
static LPDIRECT3DTEXTURE9 _nmd_d3d9_font = 0;
static size_t _nmd_d3d9_vb_size, _nmd_d3d9_ib_size;
static D3DMATRIX _nmd_d3d9_proj;

typedef struct
{
    nmd_vec3 pos;
    nmd_vec2 uv;
    D3DCOLOR color;
} _nmd_d3d9_custom_vertex;

#define _NMD_D3D9_CUSTOM_VERTEX_FVF (D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_DIFFUSE)

#define _NMD_RGBA_TO_ARGB(color) (((color&0xff)<<24)|(color>>8))

void nmd_d3d9_set_device(LPDIRECT3DDEVICE9 pD3D9Device) { _nmd_d3d9_device = pD3D9Device; }

void nmd_d3d9_resize(int width, int height)
{
    const float L = 0.5f;
    const float R = (float)width + 0.5f;
    const float T = 0.5f;
    const float B = (float)height + 0.5f;
    float matrix[4][4] = {
        {    2.0f / (R - L),              0.0f, 0.0f, 0.0f },
        {              0.0f,    2.0f / (T - B), 0.0f, 0.0f },
        {              0.0f,              0.0f, 0.0f, 0.0f },
        { (R + L) / (L - R), (T + B) / (B - T), 0.0f, 1.0f },
    };
    memcpy(&_nmd_d3d9_proj, matrix, sizeof(matrix));
}

bool _nmd_d3d9_create_font_texture()
{
    /*
    if (g_pFontTexture)
        g_pFontTexture->Release();

    
    if (_nmd_d3d9_device->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_pFontTexture, NULL) < 0)
        return false;
    
    D3DLOCKED_RECT lockedRect;
    if (g_pFontTexture->LockRect(0, &lockedRect, NULL, 0) != D3D_OK)
        return false;
    
    for (int y = 0; y < height; y++)
        memcpy((uint8_t*)lockedRect.pBits + lockedRect.Pitch * y, pixels + (width * bpp) * y, (width * bpp));
    
    g_pFontTexture->UnlockRect(0);
    */
    return true;
}

void nmd_d3d9_render()
{
    /* Create/recreate vertex buffer if it doesn't exist or more space is needed. */
    if (!_nmd_d3d9_vb || _nmd_d3d9_vb_size < _nmd_context.drawList.numVertices)
    {
        if (_nmd_d3d9_vb)
        {
            _nmd_d3d9_vb->Release();
            _nmd_d3d9_vb = 0;
        }

        _nmd_d3d9_vb_size = _nmd_context.drawList.numVertices + 5000;
        if (_nmd_d3d9_device->CreateVertexBuffer((UINT)(_nmd_d3d9_vb_size * sizeof(_nmd_d3d9_custom_vertex)), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, _NMD_D3D9_CUSTOM_VERTEX_FVF, D3DPOOL_DEFAULT, &_nmd_d3d9_vb, NULL) != D3D_OK)
            return;
    }

    /* Create/recreate index buffer if it doesn't exist or more space is needed. */
    if (!_nmd_d3d9_ib || _nmd_d3d9_ib_size < _nmd_context.drawList.numIndices)
    {
        if (_nmd_d3d9_ib)
        {
            _nmd_d3d9_ib->Release();
            _nmd_d3d9_ib = 0;
        }

        _nmd_d3d9_ib_size = _nmd_context.drawList.numIndices + 10000;
        if (_nmd_d3d9_device->CreateIndexBuffer((UINT)(_nmd_d3d9_ib_size * sizeof(nmd_index)), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(nmd_index) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &_nmd_d3d9_ib, NULL) < 0)
            return;
    }
    

    /* Copy vertices to the gpu */
    _nmd_d3d9_custom_vertex* pVertices = 0;
    if (_nmd_d3d9_vb->Lock(0, (UINT)(_nmd_context.drawList.numVertices * sizeof(_nmd_d3d9_custom_vertex)), (void**)&pVertices, D3DLOCK_DISCARD) != D3D_OK)
        return;
    size_t i = 0;
    for (; i < _nmd_context.drawList.numVertices; i++)
    {
        pVertices[i].pos.x = _nmd_context.drawList.vertices[i].pos.x;
        pVertices[i].pos.y = _nmd_context.drawList.vertices[i].pos.y;
        pVertices[i].pos.z = 0.0f;

        pVertices[i].uv = _nmd_context.drawList.vertices[i].uv;
        pVertices[i].color = _NMD_RGBA_TO_ARGB(_nmd_context.drawList.vertices[i].color);
    }
    _nmd_d3d9_vb->Unlock();

    /* Copy indices to the gpu. */
    nmd_index* pIndices = 0;
    if (_nmd_d3d9_ib->Lock(0, (UINT)(_nmd_context.drawList.numIndices * sizeof(nmd_index)), (void**)&pIndices, D3DLOCK_DISCARD) != D3D_OK)
        return;
    memcpy(pIndices, _nmd_context.drawList.indices, _nmd_context.drawList.numIndices * sizeof(nmd_index));
    _nmd_d3d9_ib->Unlock();
    
    /* Backup current render state. */
    IDirect3DStateBlock9* stateBlock;
    D3DMATRIX lastProjectionMatrix;
    if (_nmd_d3d9_device->CreateStateBlock(D3DSBT_ALL, &stateBlock) != D3D_OK ||
        _nmd_d3d9_device->GetTransform(D3DTS_PROJECTION, &lastProjectionMatrix) != D3D_OK)
        return;
    
    /* Set render state. */
    _nmd_d3d9_device->SetStreamSource(0, _nmd_d3d9_vb, 0, sizeof(_nmd_d3d9_custom_vertex));
    _nmd_d3d9_device->SetIndices(_nmd_d3d9_ib);
    _nmd_d3d9_device->SetFVF(_NMD_D3D9_CUSTOM_VERTEX_FVF);
    _nmd_d3d9_device->SetPixelShader(NULL);
    _nmd_d3d9_device->SetVertexShader(NULL);
    _nmd_d3d9_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    _nmd_d3d9_device->SetRenderState(D3DRS_LIGHTING, false);
    _nmd_d3d9_device->SetRenderState(D3DRS_ZENABLE, false);
    _nmd_d3d9_device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
    _nmd_d3d9_device->SetRenderState(D3DRS_ALPHATESTENABLE, false);
    _nmd_d3d9_device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    _nmd_d3d9_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    _nmd_d3d9_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    _nmd_d3d9_device->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
    _nmd_d3d9_device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    _nmd_d3d9_device->SetRenderState(D3DRS_FOGENABLE, false);
    _nmd_d3d9_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    _nmd_d3d9_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    _nmd_d3d9_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    _nmd_d3d9_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    _nmd_d3d9_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    _nmd_d3d9_device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    _nmd_d3d9_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    _nmd_d3d9_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    _nmd_d3d9_device->SetTransform(D3DTS_PROJECTION, &_nmd_d3d9_proj);
    
    /* Issue draw calls. */
    size_t vertexBufferOffset = 0, indexBufferOffset = 0;
    for (i = 0; i < _nmd_context.drawList.numDrawCommands; i++)
    {
        _nmd_d3d9_device->SetTexture(0, (LPDIRECT3DTEXTURE9)_nmd_context.drawList.drawCommands[i].userTextureId);
        _nmd_d3d9_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, (UINT)_nmd_context.drawList.drawCommands[i].numVertices, (UINT)indexBufferOffset, (UINT)(_nmd_context.drawList.drawCommands[i].numIndices / 3));
        vertexBufferOffset += _nmd_context.drawList.drawCommands[i].numVertices;
        indexBufferOffset += _nmd_context.drawList.drawCommands[i].numIndices;
    }

    /* Restore render state. */
    _nmd_d3d9_device->SetTransform(D3DTS_PROJECTION, &lastProjectionMatrix);
    stateBlock->Apply();
    stateBlock->Release();
}
#endif /* NMD_GRAPHICS_D3D9 */
