#ifndef NMD_COMMON_H
#define NMD_COMMON_H

#include "nmd_assembly.h"

/* Four high-order bits of an opcode to index a row of the opcode table */
#define NMD_R(b) ((b) >> 4)

/* Four low-order bits to index a column of the table */
#define NMD_C(b) ((b) & 0xF)

#define _NMD_NUM_ELEMENTS(arr) (sizeof(arr) / sizeof((arr)[0]))

#define _NMD_IS_UPPERCASE(c) (c >= 'A' && c <= 'Z')
#define _NMD_IS_LOWERCASE(c) (c >= 'a' && c <= 'z')
#define _NMD_TOLOWER(c) (_NMD_IS_UPPERCASE(c) ? c + 0x20 : c)
#define _NMD_IS_DECIMAL_NUMBER(c) (c >= '0' && c <= '9')

const char* const _nmd_reg8[8];
const char* const _nmd_reg8_x64[8];
const char* const _nmd_reg16[8];
const char* const _nmd_reg32[8];
const char* const _nmd_reg64[8];
const char* const _nmd_regrx[8];
const char* const _nmd_regrxd[8];
const char* const _nmd_regrxw[8];
const char* const _nmd_regrxb[8];
const char* const _nmd_segmentReg[6];

const char* const _nmd_conditionSuffixes[16];

const char* const _nmd_op1OpcodeMapMnemonics[8];
const char* const _nmd_opcodeExtensionsGrp1[8];
const char* const _nmd_opcodeExtensionsGrp2[8];
const char* const _nmd_opcodeExtensionsGrp3[8];
const char* const _nmd_opcodeExtensionsGrp5[7];
const char* const _nmd_opcodeExtensionsGrp6[6];
const char* const _nmd_opcodeExtensionsGrp7[8];
const char* const _nmd_opcodeExtensionsGrp7reg0[6];
const char* const _nmd_opcodeExtensionsGrp7reg1[8];
const char* const _nmd_opcodeExtensionsGrp7reg2[8];
const char* const _nmd_opcodeExtensionsGrp7reg3[8];
const char* const _nmd_opcodeExtensionsGrp7reg7[6];

const char* const _nmd_escapeOpcodesD8[8];
const char* const _nmd_escapeOpcodesD9[8];
const char* const _nmd_escapeOpcodesDA_DE[8];
const char* const _nmd_escapeOpcodesDB[8];
const char* const _nmd_escapeOpcodesDC[8];
const char* const _nmd_escapeOpcodesDD[8];
const char* const _nmd_escapeOpcodesDF[8];
const char* const* _nmd_escapeOpcodes[8];

const uint8_t _nmd_op1modrm[16];
const uint8_t _nmd_op1imm8[13];
const uint8_t _nmd_op1imm32[7];
const uint8_t _nmd_invalid2op[7];
const uint8_t _nmd_twoOpcodes[6];
const uint8_t _nmd_valid3DNowOpcodes[24];

bool _nmd_findByte(const uint8_t* arr, const size_t N, const uint8_t x);

/* Returns a pointer to the first occurrence of 'c' in 's', or a null pointer if 'c' is not present. */
const char* _nmd_strchr(const char* s, char c);

/* Returns a pointer to the last occurrence of 'c' in 's', or a null pointer if 'c' is not present. */
const char* _nmd_reverse_strchr(const char* s, char c);

/* Returns a pointer to the first occurrence of 's2' in 's', or a null pointer if 's2' is not present. */
const char* _nmd_strstr(const char* s, const char* s2);

/* Returns a pointer to the first occurrence of 's2' in 's', or a null pointer if 's2' is not present. If 's3_opt' is not null it receives the address of the next byte in 's'. */
const char* _nmd_strstr_ex(const char* s, const char* s2, const char** s3_opt);

/* Inserts 'c' at 's'. */
void _nmd_insert_char(const char* s, char c);

/* Returns true if there is only a number between 's1' and 's2', false otherwise. */
bool _nmd_is_number(const char* s1, const char* s2);

/* Returns a pointer to the first occurence of a number between 's1' and 's2', zero otherwise. */
const char* _nmd_find_number(const char* s1, const char* s2);

/* Returns true if s1 matches s2 exactly. */
bool _nmd_strcmp(const char* s1, const char* s2);

size_t _nmd_get_bit_index(uint32_t mask);

#endif /* NMD_COMMON_H */