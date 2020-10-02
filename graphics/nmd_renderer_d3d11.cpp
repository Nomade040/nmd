#include "nmd_common.h"

#ifdef NMD_GRAPHICS_D3D11

#pragma comment(lib, "d3d11")
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler")

struct
{
    ID3D11Device* device;
    ID3D11DeviceContext* device_context;
    ID3D11Buffer* vertex_buffer;
    ID3D11Buffer* index_buffer;
    int vertex_buffer_size = 5000, index_buffer_size = 10000; /*The number of vertices and indices respectively. */
    ID3D11VertexShader* vertex_shader;
    ID3D11PixelShader* pixel_shader;
    ID3D11InputLayout* input_layout;
    D3D11_VIEWPORT viewport;
    ID3D11Buffer* const_buffer;
    ID3D11SamplerState* font_sampler = NULL;
    ID3D11RasterizerState* rasterizer_state;
    ID3D11BlendState* blend_state;
    ID3D11DepthStencilState* depth_stencil_state;
} _nmd_d3d11;

void nmd_d3d11_set_device_context(ID3D11DeviceContext* device_context)
{
    _nmd_d3d11.device_context = device_context;
    _nmd_d3d11.device_context->GetDevice(&_nmd_d3d11.device);
}

nmd_tex_id nmd_d3d11_create_texture(void* pixels, int width, int height)
{
    if (!_nmd_d3d11.device)
        return 0;

    D3D11_TEXTURE2D_DESC texDesc;
    memset(&texDesc, 0, sizeof(texDesc));
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;

    ID3D11Texture2D* pTexture = NULL;
    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = pixels;
    subResource.SysMemPitch = texDesc.Width * 4;
    subResource.SysMemSlicePitch = 0;
    if(FAILED(_nmd_d3d11.device->CreateTexture2D(&texDesc, &subResource, &pTexture)))
        return 0;

    /* Create texture view */
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    ID3D11ShaderResourceView* shaderResourceView;
    const bool failed = FAILED(_nmd_d3d11.device->CreateShaderResourceView(pTexture, &srvDesc, &shaderResourceView));
    pTexture->Release();

    return failed ? 0 : (nmd_tex_id)shaderResourceView;
}

bool _nmd_d3d11_create_objects()
{
    _nmd_d3d11.viewport.MinDepth = 0.0f;
    _nmd_d3d11.viewport.MaxDepth = 1.0f;
    _nmd_d3d11.viewport.TopLeftX = 0.0f;
    _nmd_d3d11.viewport.TopLeftY = 0;

    static const char* vertexShader =
        "cbuffer vertexBuffer : register(b0) \
        {\
          float4x4 ProjectionMatrix; \
        };\
        struct VS_INPUT\
        {\
          float2 pos : POSITION;\
          float4 col : COLOR0;\
          float2 uv  : TEXCOORD0;\
        };\
        \
        struct PS_INPUT\
        {\
          float4 pos : SV_POSITION;\
          float4 col : COLOR0;\
          float2 uv  : TEXCOORD0;\
        };\
        \
        PS_INPUT main(VS_INPUT input)\
        {\
          PS_INPUT output;\
          output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\
          output.col = input.col;\
          output.uv  = input.uv;\
          return output;\
        }";

    ID3DBlob* vertexShaderBlob;
    if (FAILED(D3DCompile(vertexShader, strlen(vertexShader), NULL, NULL, NULL, "main", "vs_4_0", 0, 0, &vertexShaderBlob, NULL)))
        return false;

    if (FAILED(_nmd_d3d11.device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), NULL, &_nmd_d3d11.vertex_shader)))
    {
        vertexShaderBlob->Release();
        return false;
    }

    /* Create the input layout */
    D3D11_INPUT_ELEMENT_DESC local_layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (UINT)_NMD_OFFSETOF(nmd_vertex, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (UINT)_NMD_OFFSETOF(nmd_vertex, uv),  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (UINT)_NMD_OFFSETOF(nmd_vertex, color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    if (FAILED(_nmd_d3d11.device->CreateInputLayout(local_layout, 3, vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &_nmd_d3d11.input_layout)))
    {
        vertexShaderBlob->Release();
        return false;
    }

    vertexShaderBlob->Release();

    /* Create the constant buffer */
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.ByteWidth = sizeof(float) * 4 * 4;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;
    _nmd_d3d11.device->CreateBuffer(&bufferDesc, NULL, &_nmd_d3d11.const_buffer);

    /* Create the pixel shader */
    static const char* pixelShader =
        "struct PS_INPUT\
        {\
        float4 pos : SV_POSITION;\
        float4 col : COLOR0;\
        float2 uv  : TEXCOORD0;\
        };\
        sampler sampler0;\
        Texture2D texture0;\
        \
        float4 main(PS_INPUT input) : SV_Target\
        {\
        float4 out_col = input.col * texture0.Sample(sampler0, input.uv); \
        return out_col; \
        }";

    ID3DBlob* pixelShaderBlob;
    if (FAILED(D3DCompile(pixelShader, strlen(pixelShader), NULL, NULL, NULL, "main", "ps_4_0", 0, 0, &pixelShaderBlob, NULL)))
        return false;
    if (_nmd_d3d11.device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), NULL, &_nmd_d3d11.pixel_shader) != S_OK)
    {
        pixelShaderBlob->Release();
        return false;
    }
    pixelShaderBlob->Release();

    /* Create the blending setup */
    D3D11_BLEND_DESC blendDesc;
    memset(&blendDesc, 0, sizeof(blendDesc));
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    _nmd_d3d11.device->CreateBlendState(&blendDesc, &_nmd_d3d11.blend_state);

    /* Create the rasterizer state */
    D3D11_RASTERIZER_DESC rasterizerDesc;
    memset(&rasterizerDesc, 0, sizeof(rasterizerDesc));
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.ScissorEnable = true;
    rasterizerDesc.DepthClipEnable = true;
    _nmd_d3d11.device->CreateRasterizerState(&rasterizerDesc, &_nmd_d3d11.rasterizer_state);

    /* Create depth-stencil State */
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    memset(&depthStencilDesc, 0, sizeof(depthStencilDesc));
    depthStencilDesc.DepthEnable = false;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    depthStencilDesc.StencilEnable = false;
    depthStencilDesc.FrontFace.StencilFailOp = depthStencilDesc.FrontFace.StencilDepthFailOp = depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    depthStencilDesc.BackFace = depthStencilDesc.FrontFace;
    _nmd_d3d11.device->CreateDepthStencilState(&depthStencilDesc, &_nmd_d3d11.depth_stencil_state);

    if (!_nmd_context.drawList.default_atlas.font_id)
    {
        if (!nmd_bake_font_from_memory(nmd_karla_ttf_regular, &_nmd_context.drawList.default_atlas, 14.0f))
            return false;

        /* Upload texture to graphics system */
        if (!(_nmd_context.drawList.default_atlas.font_id = nmd_d3d11_create_texture(_nmd_context.drawList.default_atlas.pixels32, 512, 512)))
            return false;

        uint32_t tmp = 0xffffffff;
        if (!(_nmd_context.drawList.blank_tex_id = nmd_d3d11_create_texture(&tmp, 1, 1)))
            return false;
    }
    
    /* Create texture sampler */
    D3D11_SAMPLER_DESC samplerDesc;
    memset(&samplerDesc, 0, sizeof(samplerDesc));
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.f;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.MinLOD = 0.f;
    samplerDesc.MaxLOD = 0.f;
    _nmd_d3d11.device->CreateSamplerState(&samplerDesc, &_nmd_d3d11.font_sampler);

    return true;
}

void nmd_d3d11_delete_objects()
{
    _nmd_d3d11.vertex_shader->Release();
    _nmd_d3d11.vertex_shader = 0;

    _nmd_d3d11.pixel_shader->Release();
    _nmd_d3d11.pixel_shader = 0;

    _nmd_d3d11.input_layout->Release();
    _nmd_d3d11.input_layout = 0;

    _nmd_d3d11.const_buffer->Release();
    _nmd_d3d11.const_buffer = 0;

    _nmd_d3d11.font_sampler->Release();
    _nmd_d3d11.font_sampler = 0;

    _nmd_d3d11.rasterizer_state->Release();
    _nmd_d3d11.rasterizer_state = 0;

    _nmd_d3d11.blend_state->Release();
    _nmd_d3d11.blend_state = 0;

    _nmd_d3d11.depth_stencil_state = 0;

    _nmd_d3d11.vertex_buffer_size = 5000;
    _nmd_d3d11.index_buffer_size = 10000;

    _nmd_d3d11.vertex_buffer->Release();
    _nmd_d3d11.vertex_buffer = 0;

    _nmd_d3d11.index_buffer->Release();
    _nmd_d3d11.index_buffer = 0;

    _nmd_d3d11.device_context = 0;
    _nmd_d3d11.device->Release();

    _nmd_d3d11.device = 0;
}

bool nmd_d3d11_resize(int width, int height)
{
    if (!_nmd_d3d11.font_sampler && !_nmd_d3d11_create_objects())
        return false;

    D3D11_MAPPED_SUBRESOURCE mapped_resource;
    if (_nmd_d3d11.device_context->Map(_nmd_d3d11.const_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource) != S_OK)
        return false;

    float L = 0;
    float R = width;
    float T = 0;
    float B = height;
    float mvp[4][4] =
    {
        { 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
        { 0.0f,         2.0f / (T - B),     0.0f,       0.0f },
        { 0.0f,         0.0f,           0.5f,       0.0f },
        { (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
    };
    memcpy(mapped_resource.pData, mvp, sizeof(mvp));
    _nmd_d3d11.device_context->Unmap(_nmd_d3d11.const_buffer, 0);

    /* Setup viewport */
    _nmd_d3d11.viewport.Width = width;
    _nmd_d3d11.viewport.Height = height;

    return true;
}

void _nmd_d3d11_set_render_state()
{
    if (!_nmd_d3d11.font_sampler && !_nmd_d3d11_create_objects())
        return;

    unsigned int stride = sizeof(nmd_vertex);
    unsigned int offset = 0;
    _nmd_d3d11.device_context->IASetInputLayout(_nmd_d3d11.input_layout);
    _nmd_d3d11.device_context->IASetVertexBuffers(0, 1, &_nmd_d3d11.vertex_buffer, &stride, &offset);
    _nmd_d3d11.device_context->IASetIndexBuffer(_nmd_d3d11.index_buffer, sizeof(nmd_index) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
    _nmd_d3d11.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _nmd_d3d11.device_context->VSSetShader(_nmd_d3d11.vertex_shader, NULL, 0);
    _nmd_d3d11.device_context->VSSetConstantBuffers(0, 1, &_nmd_d3d11.const_buffer);
    _nmd_d3d11.device_context->PSSetShader(_nmd_d3d11.pixel_shader, NULL, 0);
    _nmd_d3d11.device_context->PSSetSamplers(0, 1, &_nmd_d3d11.font_sampler);
    _nmd_d3d11.device_context->GSSetShader(NULL, NULL, 0);
    _nmd_d3d11.device_context->HSSetShader(NULL, NULL, 0);
    _nmd_d3d11.device_context->DSSetShader(NULL, NULL, 0);
    _nmd_d3d11.device_context->CSSetShader(NULL, NULL, 0);
    _nmd_d3d11.device_context->RSSetViewports(1, &_nmd_d3d11.viewport);
    const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
    _nmd_d3d11.device_context->OMSetBlendState(_nmd_d3d11.blend_state, blend_factor, 0xffffffff);
    _nmd_d3d11.device_context->OMSetDepthStencilState(_nmd_d3d11.depth_stencil_state, 0);
    _nmd_d3d11.device_context->RSSetState(_nmd_d3d11.rasterizer_state);
}

void nmd_d3d11_render()
{
    if (!_nmd_d3d11.font_sampler && !_nmd_d3d11_create_objects())
        return;

    /* Create/Recreate vertex/index buffers if needed */
    if (!_nmd_d3d11.vertex_buffer || _nmd_d3d11.vertex_buffer_size < _nmd_context.drawList.numVertices)
    {
        if (_nmd_d3d11.vertex_buffer)
            _nmd_d3d11.vertex_buffer->Release();

        _nmd_d3d11.vertex_buffer_size = _nmd_context.drawList.numVertices + NMD_VERTEX_BUFFER_INITIAL_SIZE;

        D3D11_BUFFER_DESC desc;
        memset(&desc, 0, sizeof(desc));
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = _nmd_d3d11.vertex_buffer_size * sizeof(nmd_vertex);
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        if (FAILED(_nmd_d3d11.device->CreateBuffer(&desc, NULL, &_nmd_d3d11.vertex_buffer)))
            return;

#ifdef NMD_GRAPHICS_D3D11_OPTIMIZE_RENDER_STATE
        _nmd_d3d11_set_render_state();
#endif /* NMD_GRAPHICS_D3D11_OPTIMIZE_RENDER_STATE */
    }

    if (!_nmd_d3d11.index_buffer || _nmd_d3d11.index_buffer_size < _nmd_context.drawList.numIndices)
    {
        if (_nmd_d3d11.index_buffer)
            _nmd_d3d11.index_buffer->Release();

        _nmd_d3d11.index_buffer_size = _nmd_context.drawList.numIndices + NMD_INDEX_BUFFER_INITIAL_SIZE;

        D3D11_BUFFER_DESC desc;
        memset(&desc, 0, sizeof(desc));
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = _nmd_d3d11.index_buffer_size * sizeof(nmd_index);
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        if (FAILED(_nmd_d3d11.device->CreateBuffer(&desc, NULL, &_nmd_d3d11.index_buffer)))
            return;

#ifdef NMD_GRAPHICS_D3D11_OPTIMIZE_RENDER_STATE
        _nmd_d3d11_set_render_state();
#endif /* NMD_GRAPHICS_D3D11_OPTIMIZE_RENDER_STATE */
    }

    /* Copy vertices and indices and to the GPU */
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    if (_nmd_d3d11.device_context->Map(_nmd_d3d11.vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource) != S_OK)
        return;
    memcpy(mappedResource.pData, _nmd_context.drawList.vertices, _nmd_context.drawList.numVertices * sizeof(nmd_vertex));
    _nmd_d3d11.device_context->Unmap(_nmd_d3d11.vertex_buffer, 0);

    if (_nmd_d3d11.device_context->Map(_nmd_d3d11.index_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource) != S_OK)
        return;
    memcpy(mappedResource.pData, _nmd_context.drawList.indices, _nmd_context.drawList.numIndices * sizeof(nmd_index));
    _nmd_d3d11.device_context->Unmap(_nmd_d3d11.index_buffer, 0);

#ifndef NMD_GRAPHICS_D3D11_DONT_BACKUP_RENDER_STATE
    /* Backup the current render state */
    struct
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
        ID3D11ClassInstance* PSInstances[256], *VSInstances[256], *GSInstances[256];
        D3D11_PRIMITIVE_TOPOLOGY PrimitiveTopology;
        ID3D11Buffer* IndexBuffer, *VertexBuffer, *VSConstantBuffer;
        UINT IndexBufferOffset, VertexBufferStride, VertexBufferOffset;
        DXGI_FORMAT IndexBufferFormat;
        ID3D11InputLayout* InputLayout;
    } old;
    old.ScissorRectsCount = old.ViewportsCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    _nmd_d3d11.device_context->RSGetScissorRects(&old.ScissorRectsCount, old.ScissorRects);
    _nmd_d3d11.device_context->RSGetViewports(&old.ViewportsCount, old.Viewports);
    _nmd_d3d11.device_context->RSGetState(&old.RS);
    _nmd_d3d11.device_context->OMGetBlendState(&old.BlendState, old.BlendFactor, &old.SampleMask);
    _nmd_d3d11.device_context->OMGetDepthStencilState(&old.DepthStencilState, &old.StencilRef);
    _nmd_d3d11.device_context->PSGetShaderResources(0, 1, &old.PSShaderResource);
    _nmd_d3d11.device_context->PSGetSamplers(0, 1, &old.PSSampler);
    old.PSInstancesCount = old.VSInstancesCount = old.GSInstancesCount = 256;
    _nmd_d3d11.device_context->PSGetShader(&old.PS, old.PSInstances, &old.PSInstancesCount);
    _nmd_d3d11.device_context->VSGetShader(&old.VS, old.VSInstances, &old.VSInstancesCount);
    _nmd_d3d11.device_context->VSGetConstantBuffers(0, 1, &old.VSConstantBuffer);
    _nmd_d3d11.device_context->GSGetShader(&old.GS, old.GSInstances, &old.GSInstancesCount);
    _nmd_d3d11.device_context->IAGetPrimitiveTopology(&old.PrimitiveTopology);
    _nmd_d3d11.device_context->IAGetIndexBuffer(&old.IndexBuffer, &old.IndexBufferFormat, &old.IndexBufferOffset);
    _nmd_d3d11.device_context->IAGetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset);
    _nmd_d3d11.device_context->IAGetInputLayout(&old.InputLayout);
#endif /* NMD_GRAPHICS_D3D11_DONT_BACKUP_RENDER_STATE */

#ifndef NMD_GRAPHICS_D3D11_OPTIMIZE_RENDER_STATE
    /* Set render state */
    _nmd_d3d11_set_render_state();
#endif /* NMD_GRAPHICS_D3D11_OPTIMIZE_RENDER_STATE */

    /* Render draw commands */
    size_t indexOffset = 0;
    for (int i = 0; i < _nmd_context.drawList.numDrawCommands; i++)
    {
        /* Apply scissor rectangle */
        D3D11_RECT r;
        if (_nmd_context.drawList.drawCommands[i].rect.p1.x == -1.0f)
            r = { (LONG)_nmd_d3d11.viewport.TopLeftX, (LONG)_nmd_d3d11.viewport.TopLeftY, (LONG)_nmd_d3d11.viewport.Width, (LONG)_nmd_d3d11.viewport.Height };
        else
            r = { (LONG)_nmd_context.drawList.drawCommands[i].rect.p0.x, (LONG)_nmd_context.drawList.drawCommands[i].rect.p0.y, (LONG)_nmd_context.drawList.drawCommands[i].rect.p1.x, (LONG)_nmd_context.drawList.drawCommands[i].rect.p1.y };
        _nmd_d3d11.device_context->RSSetScissorRects(1, &r);

        /* Set texture */
        ID3D11ShaderResourceView* texture_srv = (ID3D11ShaderResourceView*)_nmd_context.drawList.drawCommands[i].userTextureId;
        _nmd_d3d11.device_context->PSSetShaderResources(0, 1, &texture_srv);

        /* Issue draw call */
        _nmd_d3d11.device_context->DrawIndexed(_nmd_context.drawList.drawCommands[i].numIndices, indexOffset, 0);

        /* Update offset */
        indexOffset += _nmd_context.drawList.drawCommands[i].numIndices;
    }

#ifndef NMD_GRAPHICS_D3D11_DONT_BACKUP_RENDER_STATE
    /* Restore previous render state */
    _nmd_d3d11.device_context->RSSetScissorRects(old.ScissorRectsCount, old.ScissorRects);
    _nmd_d3d11.device_context->RSSetViewports(old.ViewportsCount, old.Viewports);
    _nmd_d3d11.device_context->RSSetState(old.RS); if (old.RS) old.RS->Release();
    _nmd_d3d11.device_context->OMSetBlendState(old.BlendState, old.BlendFactor, old.SampleMask); if (old.BlendState) old.BlendState->Release();
    _nmd_d3d11.device_context->OMSetDepthStencilState(old.DepthStencilState, old.StencilRef); if (old.DepthStencilState) old.DepthStencilState->Release();
    _nmd_d3d11.device_context->PSSetShaderResources(0, 1, &old.PSShaderResource); if (old.PSShaderResource) old.PSShaderResource->Release();
    _nmd_d3d11.device_context->PSSetSamplers(0, 1, &old.PSSampler); if (old.PSSampler) old.PSSampler->Release();
    _nmd_d3d11.device_context->PSSetShader(old.PS, old.PSInstances, old.PSInstancesCount); if (old.PS) old.PS->Release();
    for (UINT i = 0; i < old.PSInstancesCount; i++) if (old.PSInstances[i]) old.PSInstances[i]->Release();
    _nmd_d3d11.device_context->VSSetShader(old.VS, old.VSInstances, old.VSInstancesCount); if (old.VS) old.VS->Release();
    _nmd_d3d11.device_context->VSSetConstantBuffers(0, 1, &old.VSConstantBuffer); if (old.VSConstantBuffer) old.VSConstantBuffer->Release();
    _nmd_d3d11.device_context->GSSetShader(old.GS, old.GSInstances, old.GSInstancesCount); if (old.GS) old.GS->Release();
    for (UINT i = 0; i < old.VSInstancesCount; i++) if (old.VSInstances[i]) old.VSInstances[i]->Release();
    _nmd_d3d11.device_context->IASetPrimitiveTopology(old.PrimitiveTopology);
    _nmd_d3d11.device_context->IASetIndexBuffer(old.IndexBuffer, old.IndexBufferFormat, old.IndexBufferOffset); if (old.IndexBuffer) old.IndexBuffer->Release();
    _nmd_d3d11.device_context->IASetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset); if (old.VertexBuffer) old.VertexBuffer->Release();
    _nmd_d3d11.device_context->IASetInputLayout(old.InputLayout); if (old.InputLayout) old.InputLayout->Release();
#endif /* NMD_GRAPHICS_D3D11_DONT_BACKUP_RENDER_STATE */
}

#endif /* NMD_GRAPHICS_D3D11 */
