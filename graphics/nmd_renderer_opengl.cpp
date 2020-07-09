#include "nmd_common.hpp"

//#ifdef NMD_GUI_OPENGL
//
//    void OpenGLRender()
//    {
//        //Setup render state
//        glEnable(GL_BLEND);
//        glBlendEquation(GL_FUNC_ADD);
//        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//        glDisable(GL_CULL_FACE);
//        glDisable(GL_DEPTH_TEST);
//        glEnable(GL_SCISSOR_TEST);
//
//        glViewport(0, 0, static_cast<GLsizei>(g_context.io.displaySize.x), static_cast<GLsizei>(g_context.io.displaySize.y));
//        float L = 0.0f;
//        float R = 0.0f + g_context.io.displaySize.x;
//        float T = 0.0f;
//        float B = 0.0f + g_context.io.displaySize.y;
//        const float ortho_projection[4][4] =
//        {
//            { 2.0f / (R - L),   0.0f,         0.0f,   0.0f },
//            { 0.0f,         2.0f / (T - B),   0.0f,   0.0f },
//            { 0.0f,         0.0f,        -1.0f,   0.0f },
//            { (R + L) / (L - R),  (T + B) / (B - T),  0.0f,   1.0f },
//        };
//        glUseProgram(g_ShaderHandle);
//        glUniform1i(g_AttribLocationTex, 0);
//        glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
//
//        size_t vertexBufferOffset = 0, indexBufferOffset = 0;
//        for (auto& drawCommand : g_context.drawList.drawCommands)
//        {
//            glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(drawCommand.numVertices) * sizeof(Vertex), reinterpret_cast<const GLvoid*>(g_context.drawList.vertices.data() + vertexBufferOffset), GL_STREAM_DRAW);
//            glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(drawCommand.numIndices) * sizeof(IndexType), reinterpret_cast<const GLvoid*>(g_context.drawList.indices.data() + indexBufferOffset), GL_STREAM_DRAW);
//
//            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(drawCommand.numIndices / 3), sizeof(IndexType) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, reinterpret_cast<void*>(static_cast<intptr_t>(indexBufferOffset * sizeof(IndexType))));
//        }
//    }
//
//#endif // NMD_GRAPHICS_OPENGL