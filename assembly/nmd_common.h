#ifndef NMD_INTERNAL_H
#define NMD_INTERNAL_H

#include "nmd_assembly.h"

/* Four high-order bits of an opcode to index a row of the opcode table */
#define NMD_R(b) ((b) >> 4)

/* Four low-order bits to index a column of the table */
#define NMD_C(b) ((b) & 0xF)

#define NMD_NUM_ELEMENTS(arr) (sizeof(arr) / sizeof((arr)[0]))

#define NMD_IS_LOWERCASE(c) (c >= 'a' && c <= 'z')
#define NMD_IS_DECIMAL_NUMBER(c) (c >= '0' && c <= '9')

const char* const reg8[8];
const char* const reg8_x64[8];
const char* const reg16[8];
const char* const reg32[8];
const char* const reg64[8];
const char* const regrx[8];
const char* const segmentReg[6];

const char* const conditionSuffixes[16];

const char* const op1OpcodeMapMnemonics[8];
const char* const opcodeExtensionsGrp1[8];
const char* const opcodeExtensionsGrp2[8];
const char* const opcodeExtensionsGrp3[8];
const char* const opcodeExtensionsGrp5[7];
const char* const opcodeExtensionsGrp6[6];
const char* const opcodeExtensionsGrp7[8];
const char* const opcodeExtensionsGrp7reg0[6];
const char* const opcodeExtensionsGrp7reg1[8];
const char* const opcodeExtensionsGrp7reg2[8];
const char* const opcodeExtensionsGrp7reg3[8];
const char* const opcodeExtensionsGrp7reg7[6];

const char* const escapeOpcodesD8[8];
const char* const escapeOpcodesD9[8];
const char* const escapeOpcodesDA_DE[8];
const char* const escapeOpcodesDB[8];
const char* const escapeOpcodesDC[8];
const char* const escapeOpcodesDD[8];
const char* const escapeOpcodesDF[8];
const char* const* escapeOpcodes[8];

bool nmd_findByte(const uint8_t* arr, const size_t N, const uint8_t x);

/* Returns a pointer to the first occurrence of 'c' in 's', or a null pointer if 'c' is not present. */
const char* nmd_strchr(const char* s, char c);

/* Returns a pointer to the last occurrence of 'c' in 's', or a null pointer if 'c' is not present. */
const char* nmd_reverse_strchr(const char* s, char c);

/* Returns a pointer to the first occurrence of 's2' in 's', or a null pointer if 's2' is not present. */
const char* nmd_strstr(const char* s, const char* s2);

/* Returns a pointer to the first occurrence of 's2' in 's', or a null pointer if 's2' is not present. If 's3_opt' is not null it receives the address of the next byte in 's'. */
const char* nmd_strstr_ex(const char* s, const char* s2, const char** s3_opt);

/* Inserts 'c' at 's'. */
void nmd_insert_char(const char* s, char c);

/* Returns true if there is only a number between 's1' and 's2', false otherwise. */
bool nmd_is_number(const char* s1, const char* s2);

/* Returns a pointer to the first occurence of a number between 's1' and 's2', zero otherwise. */
const char* nmd_find_number(const char* s1, const char* s2);

/* Returns true if s1 matches s2 exactly. */
bool nmd_strcmp(const char* s1, const char* s2);

size_t nmd_getBitNumber(uint32_t mask);

#endif /* NMD_INTERNAL_H */