#include "nmd_common.hpp"

namespace nmd
{
#ifdef NMD_GRAPHICS_D3D9
    static LPDIRECT3DDEVICE9 g_pD3D9Device = nullptr;
    static LPDIRECT3DVERTEXBUFFER9 g_pD3D9VertexBuffer = nullptr;
    static LPDIRECT3DINDEXBUFFER9 g_pD3D9IndexBuffer = nullptr;
    static LPDIRECT3DTEXTURE9 g_pFontTexture = nullptr;
    static size_t g_D3D9VertexBufferSize, g_D3D9IndexBufferSize;
    static D3DMATRIX projectionMatrix;

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

    void D3D9Resize(int width, int height)
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
        memcpy(&projectionMatrix, matrix, sizeof(matrix));
    }

    bool D3D9CreateFontTexture()
    {
        if (g_pFontTexture)
            g_pFontTexture->Release();

        //if (g_pD3D9Device->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_pFontTexture, NULL) < 0)
        //    return false;
        //
        //D3DLOCKED_RECT lockedRect;
        //if (g_pFontTexture->LockRect(0, &lockedRect, NULL, 0) != D3D_OK)
        //    return false;
        //
        //for (int y = 0; y < height; y++)
        //    memcpy((uint8_t*)lockedRect.pBits + lockedRect.Pitch * y, pixels + (width * bpp) * y, (width * bpp));
        //
        //g_pFontTexture->UnlockRect(0);

        return true;
    }

    void D3D9Render()
    {
        const Context& context = GetContext();

        // Create/recreate vertex buffer if it doesn't exist or more space is needed.
        if (!g_pD3D9VertexBuffer || g_D3D9VertexBufferSize < context.drawList.vertices.size())
        {
            if (g_pD3D9VertexBuffer)
                g_pD3D9VertexBuffer->Release(), g_pD3D9VertexBuffer = NULL;

            g_D3D9VertexBufferSize = context.drawList.vertices.size() + 5000;
            if (g_pD3D9Device->CreateVertexBuffer(static_cast<UINT>(g_D3D9VertexBufferSize * sizeof(CustomVertex)), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, CustomVertex::FVF, D3DPOOL_DEFAULT, &g_pD3D9VertexBuffer, NULL) != D3D_OK)
                return;
        }

        // Create/recreate index buffer if it doesn't exist or more space is needed.
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
        D3DMATRIX lastProjectionMatrix;
        if (g_pD3D9Device->CreateStateBlock(D3DSBT_ALL, &stateBlock) != D3D_OK ||
            g_pD3D9Device->GetTransform(D3DTS_PROJECTION, &lastProjectionMatrix) != D3D_OK)
            return;
        
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
        g_pD3D9Device->SetTransform(D3DTS_PROJECTION, &projectionMatrix);
        
        // Issue draw calls.
        size_t vertexBufferOffset = 0, indexBufferOffset = 0;
        for (auto& drawCommand : context.drawList.drawCommands)
        {
            g_pD3D9Device->SetTexture(0, reinterpret_cast<LPDIRECT3DTEXTURE9>(drawCommand.userTextureId));
            g_pD3D9Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, static_cast<UINT>(drawCommand.numVertices), static_cast<UINT>(indexBufferOffset), static_cast<UINT>(drawCommand.numIndices / 3));
            vertexBufferOffset += drawCommand.numVertices, indexBufferOffset += drawCommand.numIndices;
        }

        // Restore render state.
        g_pD3D9Device->SetTransform(D3DTS_PROJECTION, &lastProjectionMatrix);
        stateBlock->Apply();
        stateBlock->Release();
    }
#endif // NMD_GRAPHICS_D3D9
} // namespace nmd
