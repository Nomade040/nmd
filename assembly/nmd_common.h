#ifndef NMD_COMMON_H
#define NMD_COMMON_H

#include "nmd_assembly.h"

/* Four high-order bits of an opcode to index a row of the opcode table */
#define _NMD_R(b) ((b) >> 4)

/* Four low-order bits to index a column of the table */
#define _NMD_C(b) ((b) & 0xF)

#define _NMD_NUM_ELEMENTS(arr) (sizeof(arr) / sizeof((arr)[0]))

#define _NMD_IS_UPPERCASE(c) (c >= 'A' && c <= 'Z')
#define _NMD_IS_LOWERCASE(c) (c >= 'a' && c <= 'z')
#define _NMD_TOLOWER(c) (_NMD_IS_UPPERCASE(c) ? c + 0x20 : c)
#define _NMD_IS_DECIMAL_NUMBER(c) (c >= '0' && c <= '9')
#define _NMD_MIN(a, b) ((a)<(b)?(a):(b))
#define _NMD_MAX(a, b) ((a)>(b)?(a):(b))

#define _NMD_SET_REG_OPERAND(operand, _is_implicit, _action, _reg) {operand.type = NMD_X86_OPERAND_TYPE_REGISTER; operand.is_implicit = _is_implicit; operand.action = _action; operand.fields.reg = _reg;}
#define _NMD_SET_IMM_OPERAND(operand, _is_implicit, _action, _imm) {operand.type = NMD_X86_OPERAND_TYPE_IMMEDIATE; operand.is_implicit = _is_implicit; operand.action = _action; operand.fields.imm = _imm;}
#define _NMD_SET_MEM_OPERAND(operand, _is_implicit, _action, _segment, _base, _index, _scale, _disp) {operand.type = NMD_X86_OPERAND_TYPE_MEMORY; operand.is_implicit = _is_implicit; operand.action = _action; operand.fields.mem.segment = _segment; operand.fields.mem.base = _base; operand.fields.mem.index = _index; operand.fields.mem.scale = _scale; operand.fields.mem.disp = _disp;}
#define _NMD_GET_GPR(reg) (reg + (instruction->mode>>2)*8) /* reg(16),reg(32),reg(64). e.g. ax,eax,rax */
#define _NMD_GET_IP() (NMD_X86_REG_IP + (instruction->mode>>2)) /* ip,eip,rip */					
#define _NMD_GET_BY_MODE_OPSZPRFX(mode, opszprfx, _16, _32) ((mode) == NMD_X86_MODE_16 ? ((opszprfx) ? (_32) : (_16)) : ((opszprfx) ? (_16) : (_32))) /* Get something based on mode and operand size prefix. Used for instructions where the the 64-bit mode variant does not exist or is the same as the one for 32-bit mode */
#define _NMD_GET_BY_MODE_OPSZPRFX_W64(mode, opszprfx, rex_w_prefix, _16, _32, _64) ((mode) == NMD_X86_MODE_16 ? ((opszprfx) ? (_32) : (_16)) : ((opszprfx) ? (_16) : ((rex_w_prefix) ? (_64) : (_32)))) /* Get something based on mode and operand size prefix. The 64-bit version is accessed with the REX.W prefix */
#define _NMD_GET_BY_MODE_OPSZPRFX_D64(mode, opszprfx, _16, _32, _64) ((mode) == NMD_X86_MODE_16 ? ((opszprfx) ? (_32) : (_16)) : ((opszprfx) ? (_16) : ((mode) == NMD_X86_MODE_64 ? (_64) : (_32)))) /* Get something based on mode and operand size prefix. The 64-bit version is accessed by default when mode is NMD_X86_MODE_64 and there's no operand size override prefix. */
#define _NMD_GET_BY_MODE_OPSZPRFX_F64(mode, opszprfx, _16, _32, _64) ((mode) == NMD_X86_MODE_64 ? (_64) : ((mode) == NMD_X86_MODE_16 ? ((opszprfx) ? (_32) : (_16)) : ((opszprfx) ? (_16) : (_32)))) /* Get something based on mode and operand size prefix. The 64-bit version is accessed when mode is NMD_X86_MODE_64 independent of an operand size override prefix. */

/* Make sure we can read a byte, read a byte, increment the buffer and decrement the buffer's size */
#define _NMD_READ_BYTE(buffer_, buffer_size_, var_) { if ((buffer_size_) < sizeof(uint8_t)) { return false; } var_ = *((uint8_t*)(buffer_)); buffer_ = ((uint8_t*)(buffer_)) + sizeof(uint8_t); (buffer_size_) -= sizeof(uint8_t); }

NMD_ASSEMBLY_API const char* const _nmd_reg8[8];
NMD_ASSEMBLY_API const char* const _nmd_reg8_x64[8];
NMD_ASSEMBLY_API const char* const _nmd_reg16[8];
NMD_ASSEMBLY_API const char* const _nmd_reg32[8];
NMD_ASSEMBLY_API const char* const _nmd_reg64[8];
NMD_ASSEMBLY_API const char* const _nmd_regrxb[8];
NMD_ASSEMBLY_API const char* const _nmd_regrxw[8];
NMD_ASSEMBLY_API const char* const _nmd_regrxd[8];
NMD_ASSEMBLY_API const char* const _nmd_regrx[8];
NMD_ASSEMBLY_API const char* const _nmd_segment_reg[6];

NMD_ASSEMBLY_API const char* const _nmd_condition_suffixes[16];

NMD_ASSEMBLY_API const char* const _nmd_op1_opcode_map_mnemonics[8];
NMD_ASSEMBLY_API const char* const _nmd_opcode_extensions_grp1[8];
NMD_ASSEMBLY_API const char* const _nmd_opcode_extensions_grp2[8];
NMD_ASSEMBLY_API const char* const _nmd_opcode_extensions_grp3[8];
NMD_ASSEMBLY_API const char* const _nmd_opcode_extensions_grp5[7];
NMD_ASSEMBLY_API const char* const _nmd_opcode_extensions_grp6[6];
NMD_ASSEMBLY_API const char* const _nmd_opcode_extensions_grp7[8];
NMD_ASSEMBLY_API const char* const _nmd_opcode_extensions_grp7_reg0[6];
NMD_ASSEMBLY_API const char* const _nmd_opcode_extensions_grp7_reg1[8];
NMD_ASSEMBLY_API const char* const _nmd_opcode_extensions_grp7_reg2[8];
NMD_ASSEMBLY_API const char* const _nmd_opcode_extensions_grp7_reg3[8];
NMD_ASSEMBLY_API const char* const _nmd_opcode_extensions_grp7_reg7[6];

NMD_ASSEMBLY_API const char* const _nmd_escape_opcodesD8[8];
NMD_ASSEMBLY_API const char* const _nmd_escape_opcodesD9[8];
NMD_ASSEMBLY_API const char* const _nmd_escape_opcodesDA_DE[8];
NMD_ASSEMBLY_API const char* const _nmd_escape_opcodesDB[8];
NMD_ASSEMBLY_API const char* const _nmd_escape_opcodesDC[8];
NMD_ASSEMBLY_API const char* const _nmd_escape_opcodesDD[8];
NMD_ASSEMBLY_API const char* const _nmd_escape_opcodesDF[8];
NMD_ASSEMBLY_API const char* const* _nmd_escape_opcodes[8];

NMD_ASSEMBLY_API const uint8_t _nmd_op1_modrm[16];
NMD_ASSEMBLY_API const uint8_t _nmd_op1_imm8[13];
NMD_ASSEMBLY_API const uint8_t _nmd_op1_imm32[7];
NMD_ASSEMBLY_API const uint8_t _nmd_invalid_op2[7];
NMD_ASSEMBLY_API const uint8_t _nmd_two_opcodes[6];
NMD_ASSEMBLY_API const uint8_t _nmd_valid_3DNow_opcodes[24];

NMD_ASSEMBLY_API bool _nmd_find_byte(const uint8_t* arr, const size_t N, const uint8_t x);

/* Returns a pointer to the first occurrence of 'c' in 's', or a null pointer if 'c' is not present. */
NMD_ASSEMBLY_API const char* _nmd_strchr(const char* s, char c);

/* Returns a pointer to the last occurrence of 'c' in 's', or a null pointer if 'c' is not present. */
NMD_ASSEMBLY_API const char* _nmd_reverse_strchr(const char* s, char c);

/* Returns a pointer to the first occurrence of 's2' in 's', or a null pointer if 's2' is not present. */
NMD_ASSEMBLY_API const char* _nmd_strstr(const char* s, const char* s2);

/* Returns a pointer to the first occurrence of 's2' in 's', or a null pointer if 's2' is not present. If 's3_opt' is not null it receives the address of the next byte in 's'. */
NMD_ASSEMBLY_API const char* _nmd_strstr_ex(const char* s, const char* s2, const char** s3_opt);

/* Inserts 'c' at 's'. */
NMD_ASSEMBLY_API void _nmd_insert_char(const char* s, char c);

/* Returns true if there is only a number between 's1' and 's2', false otherwise. */
NMD_ASSEMBLY_API bool _nmd_is_number(const char* s1, const char* s2);

/* Returns a pointer to the first occurence of a number between 's1' and 's2', zero otherwise. */
NMD_ASSEMBLY_API const char* _nmd_find_number(const char* s1, const char* s2);

/* Returns true if s1 matches s2 exactly. */
NMD_ASSEMBLY_API bool _nmd_strcmp(const char* s1, const char* s2);

NMD_ASSEMBLY_API size_t _nmd_get_bit_index(uint32_t mask);

NMD_ASSEMBLY_API size_t _nmd_assembly_get_num_digits_hex(uint64_t n);

NMD_ASSEMBLY_API size_t _nmd_assembly_get_num_digits(uint64_t n);

#endif /* NMD_COMMON_H */