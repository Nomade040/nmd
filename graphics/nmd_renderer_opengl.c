#include "nmd_common.h"
#ifdef NMD_GRAPHICS_OPENGL
struct
{
    GLuint vbo, vao, ebo;
    GLuint vs, fs, program;
    GLuint uniform_tex, uniform_proj, attrib_pos, attrib_uv, attrib_color;
    GLsizei width, height;
    GLfloat ortho[4][4];
} _nmd_opengl;
bool _nmd_opengl_initialized = false;

nmd_tex_id nmd_opengl_create_texture(void* pixels, int width, int height)
{
    GLint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    GLuint texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

#ifdef GL_UNPACK_ROW_LENGTH
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glBindTexture(GL_TEXTURE_2D, last_texture);

    return texture;
}

#ifdef __APPLE__
    #define _NMD_OPENGL_SHADER_VERSION "#version 150\n"
#else
    #define _NMD_OPENGL_SHADER_VERSION "#version 300 es\n"
#endif

bool _nmd_opengl_create_objects()
{
    if (_nmd_opengl_initialized)
        return true;

    _nmd_opengl_initialized = true;

    GLfloat ortho[4][4] = {
    {2.0f, 0.0f, 0.0f, 0.0f},
    {0.0f,-2.0f, 0.0f, 0.0f},
    {0.0f, 0.0f,-1.0f, 0.0f},
    {-1.0f,1.0f, 0.0f, 1.0f},
    };
    NMD_MEMCPY(_nmd_opengl.ortho, ortho, sizeof(GLfloat) * 4 * 4);

    // Backup GL state
    GLint last_texture, last_array_buffer;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
#ifndef IMGUI_IMPL_OPENGL_ES2
    GLint last_vertex_array;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
#endif

    const GLchar* vertex_shader =
        "#version 300 es\n"
        "precision mediump float;\n"
        "layout (location = 0) in vec2 Position;\n"
        "layout (location = 1) in vec2 UV;\n"
        "layout (location = 2) in vec4 Color;\n"
        "uniform mat4 ProjMtx;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "    Frag_UV = UV;\n"
        "    Frag_Color = Color;\n"
        "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
        "}\n";
    const GLchar* fragment_shader =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "layout (location = 0) out vec4 Out_Color;\n"
        "void main()\n"
        "{\n"
        "    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
        "}\n";
    GLint status;

    _nmd_opengl.vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(_nmd_opengl.vs, 1, &vertex_shader, 0);
    glCompileShader(_nmd_opengl.vs);
    glGetShaderiv(_nmd_opengl.vs, GL_COMPILE_STATUS, &status);
    if(status != GL_TRUE)
        return false;

    _nmd_opengl.fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(_nmd_opengl.fs, 1, &fragment_shader, 0);
    glCompileShader(_nmd_opengl.fs);
    glGetShaderiv(_nmd_opengl.fs, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
        return false;

    _nmd_opengl.program = glCreateProgram();
    glAttachShader(_nmd_opengl.program, _nmd_opengl.vs);
    glAttachShader(_nmd_opengl.program, _nmd_opengl.fs);
    glLinkProgram(_nmd_opengl.program);
    glGetProgramiv(_nmd_opengl.program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE)
        return false;

    _nmd_opengl.uniform_tex = glGetUniformLocation(_nmd_opengl.program, "Texture");
    _nmd_opengl.uniform_proj = glGetUniformLocation(_nmd_opengl.program, "ProjMtx");
    _nmd_opengl.attrib_pos = (GLuint)glGetAttribLocation(_nmd_opengl.program, "Position");
    _nmd_opengl.attrib_uv = (GLuint)glGetAttribLocation(_nmd_opengl.program, "UV");
    _nmd_opengl.attrib_color = (GLuint)glGetAttribLocation(_nmd_opengl.program, "Color");

    /* Create buffers */
    glGenBuffers(1, &_nmd_opengl.vbo);
    glGenBuffers(1, &_nmd_opengl.ebo);

    int width = 16, height = 16;
    char* pixels = malloc(width * height * 4);
    NMD_MEMSET(pixels, 255, width * height * 4);
    _nmd_context.draw_list.font = nmd_opengl_create_texture(pixels, width, height);

    /* Restore modified GL state */
    glBindTexture(GL_TEXTURE_2D, last_texture);
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
#ifndef IMGUI_IMPL_OPENGL_ES2
    glBindVertexArray(last_vertex_array);
#endif

    return true;
}

bool nmd_opengl_resize(int width, int height)
{
    if (!_nmd_opengl_create_objects())
        return false;

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    _nmd_opengl.ortho[0][0] /= (GLfloat)width;
    _nmd_opengl.ortho[1][1] /= (GLfloat)height;

    _nmd_opengl.width = width;
    _nmd_opengl.height = height;

    return true;
}

void _nmd_opengl_set_render_state()
{
    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
#ifdef GL_POLYGON_MODE
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

    // Support for GL 4.5 rarely used glClipControl(GL_UPPER_LEFT)
    bool clip_origin_lower_left = true;
#if defined(GL_CLIP_ORIGIN) && !defined(__APPLE__)
    GLenum current_clip_origin = 0; glGetIntegerv(GL_CLIP_ORIGIN, (GLint*)&current_clip_origin);
    if (current_clip_origin == GL_UPPER_LEFT)
        clip_origin_lower_left = false;
#endif

    glViewport(0, 0, (GLsizei)_nmd_opengl.width, (GLsizei)_nmd_opengl.height);
    glUseProgram(_nmd_opengl.program);
    glUniform1i(_nmd_opengl.uniform_tex, 0);
    glUniformMatrix4fv(_nmd_opengl.uniform_proj, 1, GL_FALSE, &_nmd_opengl.ortho[0][0]);
#ifdef GL_SAMPLER_BINDING
    glBindSampler(0, 0); // We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.
#endif

//    (void)vertex_array_object;
//#ifndef IMGUI_IMPL_OPENGL_ES2
//    glBindVertexArray(vertex_array_object);
//#endif

    // Bind vertex/index buffers and setup attributes for ImDrawVert
    glBindBuffer(GL_ARRAY_BUFFER, _nmd_opengl.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _nmd_opengl.ebo);
    glEnableVertexAttribArray(_nmd_opengl.attrib_pos);
    glEnableVertexAttribArray(_nmd_opengl.attrib_uv);
    glEnableVertexAttribArray(_nmd_opengl.attrib_color);
    glVertexAttribPointer(_nmd_opengl.attrib_pos, 2, GL_FLOAT, GL_FALSE, sizeof(nmd_vertex), (GLvoid*)_NMD_OFFSETOF(nmd_vertex, pos));
    glVertexAttribPointer(_nmd_opengl.attrib_uv, 2, GL_FLOAT, GL_FALSE, sizeof(nmd_vertex), (GLvoid*)_NMD_OFFSETOF(nmd_vertex, uv));
    glVertexAttribPointer(_nmd_opengl.attrib_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(nmd_vertex), (GLvoid*)_NMD_OFFSETOF(nmd_vertex, color));
}

void nmd_opengl_render()
{
    if (!_nmd_opengl_create_objects())
        return false;

#ifndef NMD_GRAPHICS_OPENGL_DONT_BACKUP_RENDER_STATE
    /* Backup the current render state */
    GLenum last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
    glActiveTexture(GL_TEXTURE0);
    GLuint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&last_program);
    GLuint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&last_texture);
#ifdef GL_SAMPLER_BINDING
    GLuint last_sampler; glGetIntegerv(GL_SAMPLER_BINDING, (GLint*)&last_sampler);
#endif
    GLuint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (GLint*)&last_array_buffer);
#ifndef IMGUI_IMPL_OPENGL_ES2
    GLuint last_vertex_array_object; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint*)&last_vertex_array_object);
#endif
#ifdef GL_POLYGON_MODE
    GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
#endif
    GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
    GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
    GLenum last_blend_src_rgb; glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
    GLenum last_blend_dst_rgb; glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
    GLenum last_blend_src_alpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
    GLenum last_blend_dst_alpha; glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
    GLenum last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
    GLenum last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
    GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
    GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
    GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
    GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
#endif /* NMD_GRAPHICS_OPENGL_DONT_BACKUP_RENDER_STATE */

    /* Set render state */
    _nmd_opengl_set_render_state();

    /* Copy vertices and indices and to the GPU */
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)_nmd_context.draw_list.num_vertices * (int)sizeof(nmd_vertex), (const GLvoid*)_nmd_context.draw_list.vertices, GL_STREAM_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)_nmd_context.draw_list.num_indices * (int)sizeof(nmd_index), (const GLvoid*)_nmd_context.draw_list.indices, GL_STREAM_DRAW);

    /* Render command buffers */
    size_t i = 0;
    size_t index_offset = 0;
    for (; i < _nmd_context.draw_list.num_draw_commands; i++)
    {
        /* Apply scissor rectangle */
        if (_nmd_context.draw_list.draw_commands[i].rect.p1.x == -1.0f)
            glScissor(0, 0, (GLsizei)_nmd_opengl.width, (GLsizei)_nmd_opengl.height);
        else
            glScissor((GLint)_nmd_context.draw_list.draw_commands[i].rect.p0.x, (GLint)_nmd_context.draw_list.draw_commands[i].rect.p0.y, (GLsizei)_nmd_context.draw_list.draw_commands[i].rect.p1.x, (GLsizei)_nmd_context.draw_list.draw_commands[i].rect.p1.y);
        
        /* Set texture */
        glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)_nmd_context.draw_list.draw_commands[i].user_texture_id);

        /* Issue draw call */
        glDrawElements(GL_TRIANGLES, (GLsizei)_nmd_context.draw_list.draw_commands[i].num_indices, sizeof(nmd_index) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)(intptr_t)(index_offset * sizeof(nmd_index)));
        
        /* Update offset */
        index_offset += _nmd_context.draw_list.draw_commands[i].num_indices;
    }

#ifndef NMD_GRAPHICS_OPENGL_DONT_BACKUP_RENDER_STATE
    /* Restore previous render state */
    glUseProgram(last_program);
    glBindTexture(GL_TEXTURE_2D, last_texture);
#ifdef GL_SAMPLER_BINDING
    glBindSampler(0, last_sampler);
#endif
    glActiveTexture(last_active_texture);
#ifndef IMGUI_IMPL_OPENGL_ES2
    glBindVertexArray(last_vertex_array_object);
#endif
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
    glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
    if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
#ifdef GL_POLYGON_MODE
    glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
#endif
    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
    glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
#endif /* NMD_GRAPHICS_OPENGL_DONT_BACKUP_RENDER_STATE */
}

#endif /* NMD_GRAPHICS_OPENGL */
