#include "nmd_common.hpp"
#include <assert.h>

namespace nmd
{
    static GLuint g_vbo = 0, g_vao, g_ebo;
    static GLuint g_vs, g_fs, g_shader;
    static GLuint g_AttribLocationTexure, g_AttribLocationProjectionMatrix, g_AttribLocationPos, g_AttribLocationUV, g_AttribLocationColor;

#ifdef NMD_GRAPHICS_OPENGL

    void OpenGLCreateObjects()
    {
        if (g_vbo)
            return;

        const char* vertexShaderSource = "#version 330 core\n"
            "uniform mat4 projection;\n"
            "layout (location = 0) in vec2 pos;\n"
            "layout (location = 1) in vec4 color;\n"
            "out vec4 fragColor;\n"
            "void main() { fragColor = color; gl_Position = projection * vec4(pos.xy, 0.0, 1.0); }";

        const char* fragmentShaderSource = "#version 330 core\n"
            "in vec4 fragColor;\n"
            "out vec4 outColor;\n"
            "void main() { outColor = fragColor; }";

        int success = 0;
        
        g_vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(g_vs, 1, &vertexShaderSource, NULL);
        glCompileShader(g_vs);
        glGetShaderiv(g_vs, GL_COMPILE_STATUS, &success);
        assert(success);

        g_fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(g_fs, 1, &fragmentShaderSource, NULL);
        glCompileShader(g_fs);
        glGetShaderiv(g_fs, GL_COMPILE_STATUS, &success);
        assert(success);

        g_shader = glCreateProgram();
        glAttachShader(g_shader, g_vs);
        glAttachShader(g_shader, g_fs);
        glLinkProgram(g_shader);
        glGetProgramiv(g_shader, GL_LINK_STATUS, &success);
        assert(success);

        glDeleteShader(g_vs);
        glDeleteShader(g_fs);

        
        glGenVertexArrays(1, &g_vao);
        glGenBuffers(1, &g_vbo);
        glGenBuffers(1, &g_ebo);
    }

    void OpenGLResize(int width, int height)
    {
        OpenGLCreateObjects();

        glViewport(0, 0, (GLsizei)width, (GLsizei)height);
        float L = 0;
        float R = 0 + 640;
        float T = 0;
        float B = 0 + 480;
        const float ortho_projection[4][4] =
        {
            { 2.0f / (R - L),   0.0f,         0.0f,   0.0f },
            { 0.0f,         2.0f / (T - B),   0.0f,   0.0f },
            { 0.0f,         0.0f,        -1.0f,   0.0f },
            { (R + L) / (L - R),  (T + B) / (B - T),  0.0f,   1.0f },
        };
        glUseProgram(g_shader);
        glUniformMatrix4fv(glGetUniformLocation(g_shader, "projection"), 1, GL_FALSE, &ortho_projection[0][0]);
    }

    void OpenGLRender()
    {
        const auto& drawList = GetContext().drawList;

        OpenGLCreateObjects();

        glUseProgram(g_shader);

        glBindVertexArray(g_vao);

        glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
        glBufferData(GL_ARRAY_BUFFER, drawList.vertices.size() * sizeof(Vertex), drawList.vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)8);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, drawList.indices.size() * sizeof(IndexType), drawList.indices.data(), GL_STATIC_DRAW);

        size_t offset = 0;
        for (auto& cmd : drawList.drawCommands)
        {
            glDrawElements(GL_TRIANGLES, cmd.numIndices, sizeof(IndexType) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)offset);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

#endif // NMD_GRAPHICS_OPENGL

} // namespace nmd