![Build nmd_assembly.h](https://github.com/Nomade040/nmd/workflows/Build%20nmd_assembly.h/badge.svg)
![Test nmd_assembly.h](https://github.com/Nomade040/nmd/workflows/Test%20nmd_assembly.h/badge.svg)

# nmd
set of single-header libraries for C/C++

# Notable
- C89 x86 disassembler and length disassembler: [nmd_assembly.h](nmd_assembly.h)
- C89 2D graphics library: [nmd_graphics.h](nmd_graphics.h)
- C89 network library(sockets, SOCKS4/5): [nmd_network.h](nmd_network.h)

# General information
 - **Each library's documentation is at the start of the file.**
 - **The end user should use the single-header libraries in the root directory.** The code in the folders(e.g. `/assembly`, `/graphics`) is for development only.

# Showcase(listed in no particular order)
 - [RTTI Finder Dumper](https://github.com/theluc4s/RTTI-Finder-Dumper)
 - [CVEAC-2020](https://github.com/thesecretclub/CVEAC-2020)

# Code guidelines
 - Every identifier uses snake case.
 - Enums and macros are uppercase, every other identifier is lowercase.
 - Non-internal identifiers start with the 'NMD_' prefix. e.g. `nmd_x86_decode()`.
 - Internal identifiers start with the '_NMD_' prefix. e.g. `_nmd_append_string()`.

Repository inspired by [nothings/stb](https://github.com/nothings/stb).
