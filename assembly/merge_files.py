file_names = [
    # Header file
    'nmd_assembly.h',

    # Implementation files
    'nmd_common.c', # common macros, functions, structs...
    'nmd_x86_assembler.c',
    'nmd_x86_decoder.c',
    'nmd_x86_ldisasm.c',
    'nmd_x86_formatter.c',
]

file_contents = []

import sys
if sys.version_info < (3, 0):
    # If we are running Python 2, ignore the 'newline' arg.
    def wrap(opn):
        def open_py2(*args, **kwargs):
            if 'newline' in kwargs:
                del kwargs['newline'] 
            return opn(*args, **kwargs)
        return open_py2
    open = wrap(open)


with open('../nmd_assembly.h', 'w', newline='') as out:
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
    out.write('#ifdef NMD_ASSEMBLY_IMPLEMENTATION\n\n')

    for file_content in file_contents:
        out.write(file_content)
        out.write('\n\n')

    out.write('#endif /* NMD_ASSEMBLY_IMPLEMENTATION */\n')
