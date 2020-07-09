#include "nmd_common.hpp"

namespace nmd
{

#ifdef NMD_GRAPHICS_D3D11
    static ID3D11Device* g_pD3D11Device = NULL;
    static ID3D11DeviceContext* g_pD3D11DeviceContext = NULL;
    static ID3D11Buffer* g_pD3D11VertexBuffer = NULL;
    static ID3D11Buffer* g_pD3D11IndexBuffer = NULL;
    static size_t g_D3D11VertexBufferSize = 0, g_D3D11IndexBufferSize = 0;
    static ID3DBlob* g_pD3D11ShaderBlob = NULL;
    static ID3D11VertexShader* g_pD3D11VertexShader = NULL;
    static ID3D11PixelShader* g_pD3D11PixelShader = NULL;
    static ID3D11SamplerState* g_pD3D11SamplerState = NULL;
    static ID3D11InputLayout* g_pD3D11InputLayout = NULL;
    static ID3D11Buffer* g_pD3D11VertexConstantBuffer = NULL;
    static ID3D11BlendState* g_pD3D11BlendState = NULL;
    static ID3D11RasterizerState* g_pD3D11RasterizerState = NULL;
    static ID3D11DepthStencilState* g_pD3D11DepthStencilState = NULL;
    static ID3D11ShaderResourceView* g_pD3D11FontTextureView = NULL;

    struct D3D11_RENDER_STATE
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
        ID3D11ClassInstance* PSInstances[256], * VSInstances[256], * GSInstances[256];
        D3D11_PRIMITIVE_TOPOLOGY PrimitiveTopology;
        ID3D11Buffer* IndexBuffer, * VertexBuffer, * VSConstantBuffer;
        UINT IndexBufferOffset, VertexBufferStride, VertexBufferOffset;
        DXGI_FORMAT IndexBufferFormat;
        ID3D11InputLayout* InputLayout;
    };

    struct VERTEX_CONSTANT_BUFFER { float mvp[4][4]; };

    void D3D11SetDeviceContext(ID3D11DeviceContext* pD3D11DeviceContext)
    {
        g_pD3D11DeviceContext = pD3D11DeviceContext;
        g_pD3D11DeviceContext->GetDevice(&g_pD3D11Device);
    }

    void D3D11Render()
    {
        Context& context = GetContext();

        //Don't render if screen is minimized.
        //if (context.io.displaySize.x <= 0.0f || context.io.displaySize.y <= 0.0f)
        //    return;

        //If not initialized, create pixel shader, vertex shader, input layout, etc..
        if (!g_pD3D11VertexShader)
        {
            const char* const pixelShaderCode = "\
            struct PS_INPUT { float4 pos : SV_POSITION; float4 color : COLOR0; };\
            float4 main(PS_INPUT ps_input) : SV_TARGET\
            {\
                return ps_input.color;\
            }";

            if (D3DCompile(pixelShaderCode, strlen(pixelShaderCode), NULL, NULL, NULL, "main", "ps_4_0", 0, 0, &g_pD3D11ShaderBlob, NULL) != S_OK)
                return;

            if (g_pD3D11Device->CreatePixelShader(g_pD3D11ShaderBlob->GetBufferPointer(), g_pD3D11ShaderBlob->GetBufferSize(), NULL, &g_pD3D11PixelShader) != S_OK)
                return;

            const char* const vertexShaderCode = "\
            cbuffer vertexBuffer : register(b0) { float4x4 projectionMatrix; };\
            struct VS_INPUT { float2 pos : POSITION; float4 color : COLOR0; };\
            struct PS_INPUT { float4 pos : SV_POSITION; float4 color : COLOR0; };\
            PS_INPUT main(VS_INPUT vs_input)\
            {\
                PS_INPUT ps_input;\
                ps_input.pos = mul(projectionMatrix, float4(vs_input.pos.xy, 0.0f, 1.0f));\
                ps_input.color = vs_input.color;\
                return ps_input;\
            }";

            if (D3DCompile(vertexShaderCode, strlen(vertexShaderCode), NULL, NULL, NULL, "main", "vs_4_0", 0, 0, &g_pD3D11ShaderBlob, NULL) != S_OK)
                return;

            if (g_pD3D11Device->CreateVertexShader(g_pD3D11ShaderBlob->GetBufferPointer(), g_pD3D11ShaderBlob->GetBufferSize(), NULL, &g_pD3D11VertexShader) != S_OK)
                return;

            D3D11_INPUT_ELEMENT_DESC inputs[] = {
                { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };
            if (g_pD3D11Device->CreateInputLayout(inputs, 2, g_pD3D11ShaderBlob->GetBufferPointer(), g_pD3D11ShaderBlob->GetBufferSize(), &g_pD3D11InputLayout) != S_OK)
                return;

            g_pD3D11ShaderBlob->Release();

            D3D11_BUFFER_DESC desc;
            desc.ByteWidth = sizeof(VERTEX_CONSTANT_BUFFER);
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;
            g_pD3D11Device->CreateBuffer(&desc, NULL, &g_pD3D11VertexConstantBuffer);
        }


        //Create/recreate vertex/index buffer if it doesn't exist or more space is needed.
        {
            if (!g_pD3D11VertexBuffer || g_D3D11VertexBufferSize < context.drawList.vertices.size())
            {
                if (g_pD3D11VertexBuffer)
                    g_pD3D11VertexBuffer->Release(), g_pD3D11VertexBuffer = NULL;

                g_D3D11VertexBufferSize = context.drawList.vertices.size() + 5000;

                D3D11_BUFFER_DESC desc;
                desc.Usage = D3D11_USAGE_DYNAMIC;
                desc.ByteWidth = static_cast<UINT>(g_D3D11VertexBufferSize * sizeof(Vertex));
                desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                desc.MiscFlags = 0;

                if (g_pD3D11Device->CreateBuffer(&desc, NULL, &g_pD3D11VertexBuffer) != S_OK)
                    return;
            }

            if (!g_pD3D11IndexBuffer || g_D3D11IndexBufferSize < context.drawList.indices.size())
            {
                if (g_pD3D11IndexBuffer)
                    g_pD3D11IndexBuffer->Release(), g_pD3D11IndexBuffer = NULL;

                g_D3D11IndexBufferSize = context.drawList.indices.size() + 10000;

                D3D11_BUFFER_DESC desc;
                desc.Usage = D3D11_USAGE_DYNAMIC;
                desc.ByteWidth = static_cast<UINT>(g_D3D11IndexBufferSize * sizeof(IndexType));
                desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
                desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                desc.MiscFlags = 0;

                if (g_pD3D11Device->CreateBuffer(&desc, NULL, &g_pD3D11IndexBuffer) != S_OK)
                    return;
            }
        }

        //Copy data to GPU.
        {
            D3D11_MAPPED_SUBRESOURCE mappedResource = { NULL, 0, 0 };

            //Copy vertices.
            if (g_pD3D11DeviceContext->Map(g_pD3D11VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource) != S_OK)
                return;
            size_t i = 0;
            for (auto& vertex : context.drawList.vertices)
                memcpy(reinterpret_cast<uint8_t*>(mappedResource.pData) + i * 12, &vertex, 12), i++;
            g_pD3D11DeviceContext->Unmap(g_pD3D11VertexBuffer, 0);

            //Copy indices.
            if (g_pD3D11DeviceContext->Map(g_pD3D11IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource) != S_OK)
                return;
            memcpy(mappedResource.pData, context.drawList.indices.data(), sizeof(IndexType) * context.drawList.indices.size());
            g_pD3D11DeviceContext->Unmap(g_pD3D11IndexBuffer, 0);

            //Copy matrix allowing us to specify pixel coordinates(e.g. (250, 250)) instead of normalized coordinates([-1, +1]).
            if (g_pD3D11DeviceContext->Map(g_pD3D11VertexConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource) != S_OK)
                return;
            const float L = 0.0f;
            const float R = 800; // 0.0f + context.io.displaySize.x;
            const float T = 0.0f;
            const float B = 600; // 0.0f + context.io.displaySize.y;
            const float mvp[4][4] =
            {
                { 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
                { 0.0f,         2.0f / (T - B),     0.0f,       0.0f },
                { 0.0f,         0.0f,           0.5f,       0.0f },
                { (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
            };
            memcpy(mappedResource.pData, mvp, sizeof(mvp));
            g_pD3D11DeviceContext->Unmap(g_pD3D11VertexConstantBuffer, 0);
        }

        //Backup current render state.
        D3D11_RENDER_STATE renderState;
        {
            renderState.ScissorRectsCount = renderState.ViewportsCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
            g_pD3D11DeviceContext->RSGetScissorRects(&renderState.ScissorRectsCount, renderState.ScissorRects);
            g_pD3D11DeviceContext->RSGetViewports(&renderState.ViewportsCount, renderState.Viewports);
            g_pD3D11DeviceContext->RSGetState(&renderState.RS);
            g_pD3D11DeviceContext->OMGetBlendState(&renderState.BlendState, renderState.BlendFactor, &renderState.SampleMask);
            g_pD3D11DeviceContext->OMGetDepthStencilState(&renderState.DepthStencilState, &renderState.StencilRef);
            g_pD3D11DeviceContext->PSGetShaderResources(0, 1, &renderState.PSShaderResource);
            g_pD3D11DeviceContext->PSGetSamplers(0, 1, &renderState.PSSampler);
            renderState.PSInstancesCount = renderState.VSInstancesCount = renderState.GSInstancesCount = 256;
            g_pD3D11DeviceContext->PSGetShader(&renderState.PS, renderState.PSInstances, &renderState.PSInstancesCount);
            g_pD3D11DeviceContext->VSGetShader(&renderState.VS, renderState.VSInstances, &renderState.VSInstancesCount);
            g_pD3D11DeviceContext->VSGetConstantBuffers(0, 1, &renderState.VSConstantBuffer);
            g_pD3D11DeviceContext->GSGetShader(&renderState.GS, renderState.GSInstances, &renderState.GSInstancesCount);
            g_pD3D11DeviceContext->IAGetPrimitiveTopology(&renderState.PrimitiveTopology);
            g_pD3D11DeviceContext->IAGetIndexBuffer(&renderState.IndexBuffer, &renderState.IndexBufferFormat, &renderState.IndexBufferOffset);
            g_pD3D11DeviceContext->IAGetVertexBuffers(0, 1, &renderState.VertexBuffer, &renderState.VertexBufferStride, &renderState.VertexBufferOffset);
            g_pD3D11DeviceContext->IAGetInputLayout(&renderState.InputLayout);
        }

        //Set our render state
        {
            D3D11_VIEWPORT vp;
            vp.Width = 800;// context.io.displaySize.x;
            vp.Height = 600;// context.io.displaySize.y;
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;
            vp.TopLeftX = vp.TopLeftY = 0;
            g_pD3D11DeviceContext->RSSetViewports(1, &vp);

            const UINT stride = 12, offset = 0;
            g_pD3D11DeviceContext->IASetInputLayout(g_pD3D11InputLayout);
            g_pD3D11DeviceContext->IASetVertexBuffers(0, 1, &g_pD3D11VertexBuffer, &stride, &offset);
            g_pD3D11DeviceContext->IASetIndexBuffer(g_pD3D11IndexBuffer, sizeof(IndexType) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
            g_pD3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            g_pD3D11DeviceContext->VSSetShader(g_pD3D11VertexShader, NULL, 0);
            g_pD3D11DeviceContext->PSSetShader(g_pD3D11PixelShader, NULL, 0);
            g_pD3D11DeviceContext->VSSetConstantBuffers(0, 1, &g_pD3D11VertexConstantBuffer);
        }

        //Issue draw calls to the GPU.
        {
            size_t vertexBufferOffset = 0, indexBufferOffset = 0;
            for (auto& drawCommand : context.drawList.drawCommands)
            {
                //ID3D11ShaderResourceView* pD3D11Texture = reinterpret_cast<ID3D11ShaderResourceView*>(drawCommand.userTextureId);
                //g_pD3D11DeviceContext->PSSetShaderResources(0, 1, &pD3D11Texture);
                g_pD3D11DeviceContext->DrawIndexed(static_cast<UINT>(drawCommand.numIndices), static_cast<UINT>(indexBufferOffset), 0/*static_cast<INT>(vertexBufferOffset)*/);
                vertexBufferOffset += drawCommand.numVertices, indexBufferOffset += drawCommand.numIndices;
            }
        }

        //Restore render state.
        {
            g_pD3D11DeviceContext->RSSetScissorRects(renderState.ScissorRectsCount, renderState.ScissorRects);
            g_pD3D11DeviceContext->RSSetViewports(renderState.ViewportsCount, renderState.Viewports);
            g_pD3D11DeviceContext->RSSetState(renderState.RS);
            g_pD3D11DeviceContext->OMSetBlendState(renderState.BlendState, renderState.BlendFactor, renderState.SampleMask);
            g_pD3D11DeviceContext->OMSetDepthStencilState(renderState.DepthStencilState, renderState.StencilRef);
            g_pD3D11DeviceContext->PSSetShaderResources(0, 1, &renderState.PSShaderResource);
            g_pD3D11DeviceContext->PSSetSamplers(0, 1, &renderState.PSSampler);
            g_pD3D11DeviceContext->PSSetShader(renderState.PS, renderState.PSInstances, renderState.PSInstancesCount);
            g_pD3D11DeviceContext->VSSetShader(renderState.VS, renderState.VSInstances, renderState.VSInstancesCount);
            g_pD3D11DeviceContext->VSSetConstantBuffers(0, 1, &renderState.VSConstantBuffer);
            g_pD3D11DeviceContext->IASetPrimitiveTopology(renderState.PrimitiveTopology);
            g_pD3D11DeviceContext->IASetIndexBuffer(renderState.IndexBuffer, renderState.IndexBufferFormat, renderState.IndexBufferOffset);
            g_pD3D11DeviceContext->IASetVertexBuffers(0, 1, &renderState.VertexBuffer, &renderState.VertexBufferStride, &renderState.VertexBufferOffset);
            g_pD3D11DeviceContext->IASetInputLayout(renderState.InputLayout);
        }
    }
#endif // NMD_GRAPHICS_D3D11

} // namespace nmd
