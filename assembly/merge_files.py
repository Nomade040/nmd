file_names = [
    # Header file
    'nmd_assembly.h',

    # Implementation files
    'nmd_common.c', # common macros, functions, structs...
    'nmd_x86_assembler.c',
    'nmd_x86_decoder.c',
    'nmd_x86_formatter.c',
    'nmd_x86_emulator.c'
]

file_contents = []

with open('../nmd_assembly.h', 'w') as out:
    for file_name in file_names:
        with open(file_name, 'r') as file:
            # Read file's content
            content = file.read()

            # Remove these include statements
            content = content.replace('#include "nmd_common.h"', '')

            # Remove any '\n'(new line) character at the start of the file
            while content[0] == '\n':
                content = content[1:]

            # Append the file to 'file_contents'
            file_contents.append(content)

    # Write the header
    out.write(file_contents[0])
    file_contents.pop(0)

    out.write('\n\n')

    # Write the implementation
    out.write('#ifdef NMD_ASSEMBLY_IMPLEMENTATION\n\n')

    for file_content in file_contents:
        out.write(file_content)
        out.write('\n\n')

    out.write('#endif /* NMD_ASSEMBLY_IMPLEMENTATION */')
