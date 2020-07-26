#include "nmd_common.h"
#include "glad/glad.h"

static GLuint _nmd_vbo = 0, _nmd_vao, _nmd_ebo;
static GLuint _nmd_vs, _nmd_fs, _nmd_program;
static GLuint _nmd_uniform_tex, _nmd_uniform_proj, _nmd_attrib_pos, _nmd_attrib_uv, _nmd_attrib_color;

#ifdef NMD_GRAPHICS_OPENGL

    bool _nmd_opengl_create_objects()
    {
        if (_nmd_vbo)
            return true;

        const char* vertexShaderSource = "#version 330 core\n"
            "uniform mat4 projection;\n"
            "layout (location = 0) in vec2 pos;\n"
            "layout (location = 1) in vec2 uv;\n"
            "layout (location = 2) in vec4 color;\n"
            "out vec4 fragColor;\n"
            "void main() { fragColor = color; gl_Position = projection * vec4(pos.xy, 0.0, 1.0); }";

        const char* fragmentShaderSource = "#version 330 core\n"
            "in vec4 fragColor;\n"
            "out vec4 outColor;\n"
            "void main() { outColor = fragColor; }";

        int success = 0;

        _nmd_vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(_nmd_vs, 1, &vertexShaderSource, NULL);
        glCompileShader(_nmd_vs);
        glGetShaderiv(_nmd_vs, GL_COMPILE_STATUS, &success);
        if (!success)
            return false;

        _nmd_fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(_nmd_fs, 1, &fragmentShaderSource, NULL);
        glCompileShader(_nmd_fs);
        glGetShaderiv(_nmd_fs, GL_COMPILE_STATUS, &success);
        if (!success)
            return false;

        _nmd_program = glCreateProgram();
        glAttachShader(_nmd_program, _nmd_vs);
        glAttachShader(_nmd_program, _nmd_fs);
        glLinkProgram(_nmd_program);
        glGetProgramiv(_nmd_program, GL_LINK_STATUS, &success);
        if(!success)
            return false;

        glDeleteShader(_nmd_vs);
        glDeleteShader(_nmd_fs);

        glGenVertexArrays(1, &_nmd_vao);
        glGenBuffers(1, &_nmd_vbo);
        glGenBuffers(1, &_nmd_ebo);

        return true;
    }

    bool nmd_opengl_resize(int width, int height)
    {
        if (!_nmd_opengl_create_objects())
            return false;

        glViewport(0, 0, (GLsizei)width, (GLsizei)height);
        GLfloat ortho[4][4] = {
        {2.0f, 0.0f, 0.0f, 0.0f},
        {0.0f,-2.0f, 0.0f, 0.0f},
        {0.0f, 0.0f,-1.0f, 0.0f},
        {-1.0f,1.0f, 0.0f, 1.0f},
        };
        ortho[0][0] /= (GLfloat)width;
        ortho[1][1] /= (GLfloat)height;
        glUseProgram(_nmd_program);
        glUniformMatrix4fv(glGetUniformLocation(_nmd_program, "projection"), 1, GL_FALSE, &ortho[0][0]);

        return true;
    }

    bool nmd_opengl_render()
    {
        if (!_nmd_opengl_create_objects())
            return false;

        glUseProgram(_nmd_program);

        glBindVertexArray(_nmd_vao);

        glBindBuffer(GL_ARRAY_BUFFER, _nmd_vbo);
        glBufferData(GL_ARRAY_BUFFER, _nmd_context.drawList.numVertices * sizeof(nmd_vertex), _nmd_context.drawList.vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(nmd_vertex), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(nmd_vertex), (void*)8);
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(nmd_vertex), (void*)16);
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _nmd_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, _nmd_context.drawList.numIndices * sizeof(IndexType), _nmd_context.drawList.indices, GL_STATIC_DRAW);

        size_t offset = 0;
        size_t i = 0;
        for (; i < _nmd_context.drawList.numDrawCommands; i++)
        {
            glDrawElements(GL_TRIANGLES, _nmd_context.drawList.drawCommands[i].numIndices, sizeof(IndexType) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)offset);
            offset += _nmd_context.drawList.drawCommands[i].numIndices;
        }

        return true;
    }

#endif /* NMD_GRAPHICS_OPENGL */
