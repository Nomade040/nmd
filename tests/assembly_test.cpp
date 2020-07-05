#include <gtest\gtest.h>
#include <string>

#define NMD_ASSEMBLY_IMPLEMENTATION
#include "nmd_assembly.h"

struct TestInstruction
{
	const char* s;
	NMD_X86Instruction i;
};

#define CPUFLAG(x) {!!((x)&(1<<0)),!!((x)&(1<<1)),!!((x)&(1<<2)),!!((x)&(1<<3)),!!((x)&(1<<4)),!!((x)&(1<<5)),!!((x)&(1<<6)),!!((x)&(1<<7)),!!((x)&(1<<8)),!!((x)&(1<<9)),!!((x)&(1<<10)),!!((x)&(1<<11)),!!((x)&(1<<13)),!!((x)&(1<<14)),!!((x)&(1<<15)),!!((x)&(1<<16)),!!((x)&(1<<17)),!!((x)&(1<<18)),!!((x)&(1<<19)),!!((x)&(1<<20)),!!((x)&(1<<21)),!!((x)&(1<<22)),!!((x)&(1<<23)),!!((x)&(1<<24)),!!((x)&(1<<25)),!!((x)&(1<<26)),!!((x)&(1<<27)),!!((x)&(1<<28)),!!((x)&(1<<29)),!!((x)&(1<<30)),!!((x)&(1<<31))}

static const NMD_X86InstructionFlags validFlags = {true, false, false, false, false, false};
static const NMD_X86InstructionFlags invalidFlags = {false, false, false, false, false, false};

static const TestInstruction instructions[] = {
	/*string     flags         mode             length  opcode  opcodeSize  id                          prefixes                                 numPrefixes  numOperands  group           fullInstruction  operands      modrm  sib    immMask           dispMask           immediate  displacement  opcodeMap                   encoding                             vex  modifiedFlags                                                                                                testedFlags                                     setFlags    clearedFlags                                    undefinedFlags                                                                          rex  segmentOverride  simdPrefix */
	{"int3",    {validFlags,   NMD_X86_MODE_32, 1,      0xcc,   1,          NMD_X86_INSTRUCTION_INT3,   NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_INT,  {0xcc},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(NMD_X86_EFLAGS_IF | NMD_X86_EFLAGS_NT | NMD_X86_EFLAGS_VM | NMD_X86_EFLAGS_AC | NMD_X86_EFLAGS_VIF), CPUFLAG(NMD_X86_EFLAGS_NT | NMD_X86_EFLAGS_VM), CPUFLAG(0), CPUFLAG(NMD_X86_EFLAGS_TF | NMD_X86_EFLAGS_RF), CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"nop",     {validFlags,   NMD_X86_MODE_32, 1,      0x90,   1,          NMD_X86_INSTRUCTION_NOP,    NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x90},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"ret",     {validFlags,   NMD_X86_MODE_32, 1,      0xc3,   1,          NMD_X86_INSTRUCTION_RET,    NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_RET,  {0xc3},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"pushf",   {validFlags,   NMD_X86_MODE_16, 1,      0x9c,   1,          NMD_X86_INSTRUCTION_PUSHF,  NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x9c},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"pushf",   {validFlags,   NMD_X86_MODE_32, 2,      0x9c,   1,          NMD_X86_INSTRUCTION_PUSHF,  NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE,  1,           0,           NMD_GROUP_NONE, {0x66, 0x9c},    {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE }},
	{"pushf",   {validFlags,   NMD_X86_MODE_64, 2,      0x9c,   1,          NMD_X86_INSTRUCTION_PUSHF,  NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE,  1,           0,           NMD_GROUP_NONE, {0x66, 0x9c},    {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE }},
	{"pushfd",  {validFlags,   NMD_X86_MODE_16, 2,      0x9c,   1,          NMD_X86_INSTRUCTION_PUSHFD, NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE,  1,           0,           NMD_GROUP_NONE, {0x66, 0x9c},    {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE }},
	{"pushfd",  {validFlags,   NMD_X86_MODE_32, 1,      0x9c,   1,          NMD_X86_INSTRUCTION_PUSHFD, NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x9c},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"pushfq",  {validFlags,   NMD_X86_MODE_64, 1,      0x9c,   1,          NMD_X86_INSTRUCTION_PUSHFQ, NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x9c},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"popf",    {validFlags,   NMD_X86_MODE_16, 1,      0x9d,   1,          NMD_X86_INSTRUCTION_POPF,   NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x9d},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"popf",    {validFlags,   NMD_X86_MODE_32, 2,      0x9d,   1,          NMD_X86_INSTRUCTION_POPF,   NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE,  1,           0,           NMD_GROUP_NONE, {0x66, 0x9d},    {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE }},
	{"popf",    {validFlags,   NMD_X86_MODE_64, 2,      0x9d,   1,          NMD_X86_INSTRUCTION_POPF,   NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE,  1,           0,           NMD_GROUP_NONE, {0x66, 0x9d},    {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE }},
	{"popfd",   {validFlags,   NMD_X86_MODE_16, 2,      0x9d,   1,          NMD_X86_INSTRUCTION_POPFD,  NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE,  1,           0,           NMD_GROUP_NONE, {0x66, 0x9d},    {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE }},
	{"popfd",   {validFlags,   NMD_X86_MODE_32, 1,      0x9d,   1,          NMD_X86_INSTRUCTION_POPFD,  NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x9d},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"popfq",   {validFlags,   NMD_X86_MODE_64, 1,      0x9d,   1,          NMD_X86_INSTRUCTION_POPFQ,  NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x9d},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"retf",    {validFlags,   NMD_X86_MODE_32, 1,      0xcb,   1,          NMD_X86_INSTRUCTION_RETF,   NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_RET,  {0xcb},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"retf",    {validFlags,   NMD_X86_MODE_64, 1,      0xcb,   1,          NMD_X86_INSTRUCTION_RETF,   NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_RET,  {0xcb},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"leave",   {validFlags,   NMD_X86_MODE_32, 1,      0xc9,   1,          NMD_X86_INSTRUCTION_LEAVE,  NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0xc9},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"int1",    {validFlags,   NMD_X86_MODE_32, 1,      0xf1,   1,          NMD_X86_INSTRUCTION_INT1,   NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_INT,  {0xf1},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"push es", {validFlags,   NMD_X86_MODE_32, 1,      0x06,   1,          NMD_X86_INSTRUCTION_PUSH,   NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x06},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"push es", {invalidFlags, NMD_X86_MODE_64, 1,      0x06,   1,          NMD_X86_INSTRUCTION_PUSH,   NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x06},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"push ss", {validFlags,   NMD_X86_MODE_32, 1,      0x16,   1,          NMD_X86_INSTRUCTION_PUSH,   NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x16},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"push ss", {invalidFlags, NMD_X86_MODE_64, 1,      0x16,   1,          NMD_X86_INSTRUCTION_PUSH,   NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x16},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"push ds", {validFlags,   NMD_X86_MODE_32, 1,      0x1e,   1,          NMD_X86_INSTRUCTION_PUSH,   NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x1e},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"push ds", {invalidFlags, NMD_X86_MODE_64, 1,      0x1e,   1,          NMD_X86_INSTRUCTION_PUSH,   NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x1e},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"push cs", {validFlags,   NMD_X86_MODE_32, 1,      0x0e,   1,          NMD_X86_INSTRUCTION_PUSH,   NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x0e},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"push cs", {invalidFlags, NMD_X86_MODE_64, 1,      0x0e,   1,          NMD_X86_INSTRUCTION_PUSH,   NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x0e},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"pop es",  {validFlags,   NMD_X86_MODE_32, 1,      0x07,   1,          NMD_X86_INSTRUCTION_POP,    NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x07},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"pop es",  {invalidFlags, NMD_X86_MODE_64, 1,      0x07,   1,          NMD_X86_INSTRUCTION_POP,    NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x07},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"pop ds",  {validFlags,   NMD_X86_MODE_32, 1,      0x1f,   1,          NMD_X86_INSTRUCTION_POP,    NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x1f},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"pop ds",  {invalidFlags, NMD_X86_MODE_64, 1,      0x1f,   1,          NMD_X86_INSTRUCTION_POP,    NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x1f},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(0),                                                                                                  CPUFLAG(0),                                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(0),                                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"daa",     {validFlags,   NMD_X86_MODE_32, 1,      0x27,   1,          NMD_X86_INSTRUCTION_DAA,    NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x27},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_PF),  CPUFLAG(NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_AF), CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(NMD_X86_EFLAGS_OF),                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"daa",     {invalidFlags, NMD_X86_MODE_64, 1,      0x27,   1,          NMD_X86_INSTRUCTION_DAA,    NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x27},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_PF),  CPUFLAG(NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_AF), CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(NMD_X86_EFLAGS_OF),                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"aaa",     {validFlags,   NMD_X86_MODE_32, 1,      0x37,   1,          NMD_X86_INSTRUCTION_AAA,    NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x37},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_AF),                                                              CPUFLAG(NMD_X86_EFLAGS_AF),                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_PF), 0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"aaa",     {invalidFlags, NMD_X86_MODE_64, 1,      0x37,   1,          NMD_X86_INSTRUCTION_AAA,    NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x37},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_AF),                                                              CPUFLAG(NMD_X86_EFLAGS_AF),                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_PF), 0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"das",     {validFlags,   NMD_X86_MODE_32, 1,      0x2f,   1,          NMD_X86_INSTRUCTION_DAS,    NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x2f},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_PF),  CPUFLAG(NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_AF), CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(NMD_X86_EFLAGS_OF),                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"das",     {invalidFlags, NMD_X86_MODE_64, 1,      0x2f,   1,          NMD_X86_INSTRUCTION_DAS,    NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x2f},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_PF),  CPUFLAG(NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_AF), CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(NMD_X86_EFLAGS_OF),                                                             0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"aas",     {validFlags,   NMD_X86_MODE_32, 1,      0x3f,   1,          NMD_X86_INSTRUCTION_AAS,    NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x3f},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_AF),                                                              CPUFLAG(NMD_X86_EFLAGS_AF),                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_PF), 0,   0,               NMD_X86_PREFIXES_NONE                  }},
	{"aas",     {invalidFlags, NMD_X86_MODE_64, 1,      0x3f,   1,          NMD_X86_INSTRUCTION_AAS,    NMD_X86_PREFIXES_NONE,                   0,           0,           NMD_GROUP_NONE, {0x3f},          {0, 0, 0, 0}, {0},   {0},   NMD_X86_IMM_NONE, NMD_X86_DISP_NONE, 0,         0,            NMD_X86_OPCODE_MAP_DEFAULT, NMD_X86_INSTRUCTION_ENCODING_LEGACY, {0}, CPUFLAG(NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_AF),                                                              CPUFLAG(NMD_X86_EFLAGS_AF),                     CPUFLAG(0), CPUFLAG(0),                                     CPUFLAG(NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_PF), 0,   0,               NMD_X86_PREFIXES_NONE                  }},
};

TEST(FirstTest, TestAllInstructions)
{	
	NMD_X86Instruction instruction;
	char formattedInstruction[128];
	uint8_t buffer[NMD_X86_MAXIMUM_INSTRUCTION_LENGTH];

	// Test all instructions in the array above.
	for (size_t i = 0; i < NMD_NUM_ELEMENTS(instructions); i++)
	{
		SCOPED_TRACE("INDEX=" + std::to_string(i));

		// Test decoder
		EXPECT_EQ(nmd_x86_decode_buffer(&instructions[i].i.buffer, instructions[i].i.length, &instruction, (NMD_X86_MODE)instructions[i].i.mode, NMD_X86_FEATURE_FLAGS_ALL), instructions[i].i.flags.fields.valid);
		if (instructions[i].i.flags.fields.valid)
		{
			// Test decoder
			EXPECT_EQ(memcmp(&instructions[i].i, &instruction, sizeof(NMD_X86Instruction)), 0);
			
			// Test formatter
			nmd_x86_format_instruction(&instruction, formattedInstruction, NMD_X86_INVALID_RUNTIME_ADDRESS, NMD_X86_FORMAT_FLAGS_DEFAULT);
			EXPECT_EQ(strcmp(formattedInstruction, instructions[i].s), 0);
			
			// Test assembler
			memset(buffer, 0x00, sizeof(buffer));
			EXPECT_EQ(nmd_x86_assemble(formattedInstruction, buffer, sizeof(buffer), NMD_X86_INVALID_RUNTIME_ADDRESS, (NMD_X86_MODE)instructions[i].i.mode, 0), instructions[i].i.length);
			EXPECT_EQ(memcmp(buffer, instructions[i].i.buffer, sizeof(buffer)), 0);
		}
	}

	//const uint8_t oneByteInstructions[] = {0xd7,0x9b,0xf4,0xf5,0x9e,0x9f,0xce,0xcf,0x98,0x99,0xd6,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd};
	//const uint8_t twoByteInstructions[] = { 0x05,0x06,0x07,0x08,0x09,0x0b,0x0e,0x30,0x31,0x32,0x33,0x34,0x35,0x37,0x77,0xa0,0xa1,0xa2,0xa8,0xa9,0xaa };
}

int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}