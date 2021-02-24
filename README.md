![Build nmd_assembly.h](https://github.com/Nomade040/nmd/workflows/Build%20nmd_assembly.h/badge.svg) ![Test nmd_assembly.h](https://github.com/Nomade040/nmd/workflows/Test%20nmd_assembly.h/badge.svg) [![Discord](https://img.shields.io/badge/chat-on%20Discord-green.svg)](https://discord.gg/VF4rVxA)
<a href="https://scan.coverity.com/projects/nomade040-nmd"><img alt="Coverity Scan Build Status" src="https://scan.coverity.com/projects/21763/badge.svg"/></a>

# nmd
set of single-header libraries for C/C++

# Notable
- C89 x86 assembler and disassembler: [nmd_assembly.h](nmd_assembly.h)
- C89 2D graphics library: [nmd_graphics.h](nmd_graphics.h)

# General information
 - **Each library's documentation is at the start of the file.**
 - **The end user should use the single-header libraries in the root directory.** The code in the folders(e.g. `/assembly`, `/graphics`) is for development only.

# Showcase(listed in no particular order)
 - [RTTI Finder Dumper](https://github.com/theluc4s/RTTI-Finder-Dumper)
 - [CVEAC-2020](https://github.com/thesecretclub/CVEAC-2020)

# Code guidelines
 - Ensure C89 compatibility.
 - Every identifier uses snake case.
 - Enums and macros are uppercase, every other identifier is lowercase.
 - Non-internal identifiers start with the `NMD_` prefix. e.g. `nmd_x86_decode()`.
 - Internal identifiers start with the `_NMD_` prefix. e.g. `_nmd_append_string()`.

Repository inspired by [nothings/stb](https://github.com/nothings/stb).
