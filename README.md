# nmd
set of single-header libraries for C/C++

- x86 assembler & disassembler: [nmd_assembly.h](nmd_assembly.h)
- length disassembler: [nmd_ldisasm.h](nmd_ldisasm.h)
- memory library for windows: [nmd_memory.hpp](nmd_memory.hpp)
- retained mode gui: [nmd_gui.hpp](nmd_gui.hpp)
- web gui editor: [nmd_gui_editor.html](nmd_gui_editor.html)

All libraries require that you define the macro NMD_{LIBRARY_NAME}_IMPLEMENTATION in one source file. See the first lines of the library for more details.

TODO:
 - *.[h/hpp]
   - Add usage example as comment blocks in the beginning of the file.

 - nmd_assembly.h
   - Implement Assembler(only the initial parsing is done).
   - implement instruction set extensions to the disassembler: VEX, EVEX, MVEX, 3DNOW, XOP.
   - Check for bugs using fuzzers.
   
 - nmd_ldisasm.h
   - when the disassembler is updated it's a good idea to copy the logic to ldisasm().
   
 - nmd_memory.hpp
   - Finish ValueScan()
   - Make MemEx::Call() work in a wow64 process(target) from a 64 bit process(this process).
   
 - nmd_gui.hpp
   - Embed font rasterizer: https://github.com/nothings/stb/blob/master/stb_truetype.h.
   - Add AddText() method to DrawList.
   - Implement default widgets.
   - Add support for the remaining Rendering APIs: Direct3D 12, OpenGL and Vulkan.
   
 - nmd_gui_editor.html
   - Make the whole thing(only the basic structure is done) ;P

I wanted to make this reposity in the same style as https://github.com/nothings/stb/ because it seemed practical.
