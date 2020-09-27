file_names = [
    # Header file
    'nmd_graphics.h',

    # Implementation files
    'nmd_common.c',
    'nmd_stb_truetype.c',
    'nmd_default_font.c',
    'stb_truetype.h',
    'nmd_graphics.c',
    'nmd_drawlist.c',
    'nmd_gui.c',
    'nmd_renderer_d3d9.cpp',
    'nmd_renderer_d3d11.cpp',
    'nmd_renderer_opengl.c'
]

file_contents = []

with open('../nmd_graphics.h', 'w') as out:
    for file_name in file_names:
        with open(file_name, 'r') as file:
            # Read file's content
            content = file.read()

            # Remove these include statements
            content = content.replace('#include "nmd_common.h"', '')

            # Remove all empty lines at the start of the file
            while content[0] == '\n':
                content = content[1:]

            # Append the file to 'file_contents'
            file_contents.append(content)

    # Write the header
    out.write(file_contents[0])
    file_contents.pop(0)

    out.write('\n\n')

    # Write the implementation
    out.write('#ifdef NMD_GRAPHICS_IMPLEMENTATION\n\n')

    for file_content in file_contents:
        out.write(file_content)
        out.write('\n\n')

    out.write('#endif // NMD_GRAPHICS_IMPLEMENTATION\n')
