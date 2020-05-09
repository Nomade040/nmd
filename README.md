[![buddy pipeline](https://app.buddy.works/nomade040/nmd/pipelines/pipeline/255609/badge.svg?token=2fb17a3ff8ce8e4793c03168be95a073eb3a1be6606e720a560806ca0bf9ce8a "buddy pipeline")](https://app.buddy.works/nomade040/nmd/pipelines/pipeline/255609)

# nmd
set of single-header libraries for C/C++

- x86 disassembler: [nmd_assembly.h](nmd_assembly.h)
- length disassembler: [nmd_ldisasm.h](nmd_ldisasm.h)
- memory library for windows: [nmd_memory.hpp](nmd_memory.hpp)
- retained mode gui: [nmd_gui.hpp](nmd_gui.hpp)
- web gui editor: [nmd_gui_editor.html](nmd_gui_editor.html)

All libraries require that you define the macro NMD_{LIBRARY_NAME}_IMPLEMENTATION in one source file.

The first few lines of each library are comments describing how the library works, you should read it.

Showcase:
![Image of disassembler](https://i.imgur.com/Zw2l94k.png)

Currently there's no gui, it can only draw basic shapes :/

![Gif of an application using nmd_gui.hpp](https://media.giphy.com/media/KzoJfuFCt9vxyfYuFz/giphy.gif)

I wanted to make this reposity in the same style as https://github.com/nothings/stb/ because it seemed practical.
