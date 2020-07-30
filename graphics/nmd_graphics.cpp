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

/* Starts a new empty scene. Internally this function clears all vertices, indices and command buffers. */
void nmd_begin()
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
        _nmd_context.drawList.path = (nmd_vec2*)NMD_MALLOC(NMD_INITIAL_PATH_BUFFER_SIZE);
        _nmd_context.drawList.pathCapacity = NMD_INITIAL_PATH_BUFFER_SIZE;

        _nmd_context.drawList.vertices = (nmd_vertex*)NMD_MALLOC(NMD_INITIAL_VERTICES_BUFFER_SIZE);
        _nmd_context.drawList.verticesCapacity = NMD_INITIAL_VERTICES_BUFFER_SIZE;

        _nmd_context.drawList.indices = (nmd_index*)NMD_MALLOC(NMD_INITIAL_INDICES_BUFFER_SIZE);
        _nmd_context.drawList.indicesCapacity = NMD_INITIAL_INDICES_BUFFER_SIZE;

        _nmd_context.drawList.drawCommands = (nmd_draw_command*)NMD_MALLOC(NMD_INITIAL_DRAW_COMMANDS_BUFFER_SIZE);
        _nmd_context.drawList.drawCommandsCapacity = NMD_INITIAL_DRAW_COMMANDS_BUFFER_SIZE;
    }

    _nmd_context.drawList.numVertices = 0;
    _nmd_context.drawList.numIndices = 0;
    _nmd_context.drawList.numDrawCommands = 0;
}

/* Ends a scene, so it can be rendered. Internally this functions creates the remaining draw commands. */
void nmd_end()
{
    nmd_push_draw_command(0);
}