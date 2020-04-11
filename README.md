# nmd
set of single-header libraries for C/C++

- x86 assembler & disassembler: [nmd_assembly.h](nmd_assembly.h)
- length disassembler: [nmd_ldisasm.h](nmd_ldisasm.h)
- memory library for windows: [nmd_memory.hpp](nmd_memory.hpp)
- retained mode gui: [nmd_gui.hpp](nmd_gui.hpp)
- web gui editor: [nmd_gui_editor.html](nmd_gui_editor.html)

All libraries require that you define the macro NMD_{LIBRARY_NAME}_IMPLEMENTATION in one source file. See the first lines of the library for more details.

The first few lines of each library are comments describing how the library works, you should read it.

Showcase:
![Image of disassembler](https://i.imgur.com/Zw2l94k.png)

![Gif of an application using nmd_gui.hpp](https://media.giphy.com/media/KzoJfuFCt9vxyfYuFz/giphy.gif)

I wanted to make this reposity in the same style as https://github.com/nothings/stb/ because it seemed practical.
