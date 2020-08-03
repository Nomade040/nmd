#include "nmd_common.h"

/* Four high-order bits of an opcode to index a row of the opcode table */
#define NMD_R(b) ((b) >> 4)

/* Four low-order bits to index a column of the table */
#define NMD_C(b) ((b) & 0xF)

#define _NMD_NUM_ELEMENTS(arr) (sizeof(arr) / sizeof((arr)[0]))

#define _NMD_IS_LOWERCASE(c) (c >= 'a' && c <= 'z')
#define _NMD_IS_DECIMAL_NUMBER(c) (c >= '0' && c <= '9')

const char* const _nmd_reg8[] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
const char* const _nmd_reg8_x64[] = { "al", "cl", "dl", "bl", "spl", "bpl", "sil", "dil" };
const char* const _nmd_reg16[] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" };
const char* const _nmd_reg32[] = { "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi" };
const char* const _nmd_reg64[] = { "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi" };
const char* const _nmd_regrx[] = { "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15" };
const char* const _nmd_segmentReg[] = { "es", "cs", "ss", "ds", "fs", "gs" };

const char* const _nmd_conditionSuffixes[] = { "o", "no", "b", "ae", "e", "ne", "be", "a", "s", "ns", "p", "np", "l", "ge", "le", "g" };

const char* const _nmd_op1OpcodeMapMnemonics[] = { "add", "adc", "and", "xor", "or", "sbb", "sub", "cmp" };
const char* const _nmd_opcodeExtensionsGrp1[] = { "add", "or", "adc", "sbb", "and", "sub", "xor", "cmp" };
const char* const _nmd_opcodeExtensionsGrp2[] = { "rol", "ror", "rcl", "rcr", "shl", "shr", "shl", "sar" };
const char* const _nmd_opcodeExtensionsGrp3[] = { "test", "test", "not", "neg", "mul", "imul", "div", "idiv" };
const char* const _nmd_opcodeExtensionsGrp5[] = { "inc", "dec", "call", "call far", "jmp", "jmp far", "push" };
const char* const _nmd_opcodeExtensionsGrp6[] = { "sldt", "str", "lldt", "ltr", "verr", "verw" };
const char* const _nmd_opcodeExtensionsGrp7[] = { "sgdt", "sidt", "lgdt", "lidt", "smsw", 0, "lmsw", "invlpg" };
const char* const _nmd_opcodeExtensionsGrp7reg0[] = { "enclv", "vmcall", "vmlaunch", "vmresume", "vmxoff", "pconfig" };
const char* const _nmd_opcodeExtensionsGrp7reg1[] = { "monitor", "mwait", "clac", "stac", 0, 0, 0, "encls" };
const char* const _nmd_opcodeExtensionsGrp7reg2[] = { "xgetbv", "xsetbv", 0, 0, "vmfunc", "xend", "xtest", "enclu" };
const char* const _nmd_opcodeExtensionsGrp7reg3[] = { "vmrun ", "vmmcall", "vmload ", "vmsave", "stgi", "clgi", "skinit eax", "invlpga " };
const char* const _nmd_opcodeExtensionsGrp7reg7[] = { "swapgs", "rdtscp", "monitorx", "mwaitx", "clzero ", "rdpru" };

const char* const _nmd_escapeOpcodesD8[] = { "add", "mul", "com", "comp", "sub", "subr", "div", "divr" };
const char* const _nmd_escapeOpcodesD9[] = { "ld", 0, "st", "stp", "ldenv", "ldcw", "nstenv", "nstcw" };
const char* const _nmd_escapeOpcodesDA_DE[] = { "iadd", "imul", "icom", "icomp", "isub", "isubr", "idiv", "idivr" };
const char* const _nmd_escapeOpcodesDB[] = { "ild", "isttp", "ist", "istp", 0, "ld", 0, "stp" };
const char* const _nmd_escapeOpcodesDC[] = { "add", "mul", "com", "comp", "sub", "subr", "div", "divr" };
const char* const _nmd_escapeOpcodesDD[] = { "ld", "isttp", "st", "stp", "rstor", 0, "nsave", "nstsw" };
const char* const _nmd_escapeOpcodesDF[] = { "ild", "isttp", "ist", "istp", "bld", "ild", "bstp", "istp" };
const char* const* _nmd_escapeOpcodes[] = { _nmd_escapeOpcodesD8, _nmd_escapeOpcodesD9, _nmd_escapeOpcodesDA_DE, _nmd_escapeOpcodesDB, _nmd_escapeOpcodesDC, _nmd_escapeOpcodesDD, _nmd_escapeOpcodesDA_DE, _nmd_escapeOpcodesDF };

const uint8_t _nmd_op1modrm[] = { 0xFF, 0x63, 0x69, 0x6B, 0xC0, 0xC1, 0xC6, 0xC7, 0xD0, 0xD1, 0xD2, 0xD3, 0xF6, 0xF7, 0xFE };
const uint8_t _nmd_op1imm8[] = { 0x6A, 0x6B, 0x80, 0x82, 0x83, 0xA8, 0xC0, 0xC1, 0xC6, 0xCD, 0xD4, 0xD5, 0xEB };
const uint8_t _nmd_op1imm32[] = { 0xE8, 0xE9, 0x68, 0x81, 0x69, 0xA9, 0xC7 };
const uint8_t _nmd_invalid2op[] = { 0x04, 0x0a, 0x0c, 0x7a, 0x7b, 0x36, 0x39 };
const uint8_t _nmd_twoOpcodes[] = { 0xb0, 0xb1, 0xb3, 0xbb, 0xc0, 0xc1 };
const uint8_t _nmd_valid3DNowOpcodes[] = { 0x0c, 0x0d, 0x1c, 0x1d, 0x8a, 0x8e, 0x90, 0x94, 0x96, 0x97, 0x9a, 0x9e, 0xa0, 0xa4, 0xa6, 0xa7, 0xaa, 0xae, 0xb0, 0xb4, 0xb6, 0xb7, 0xbb, 0xbf };

bool _nmd_findByte(const uint8_t* arr, const size_t N, const uint8_t x)
{
	size_t i = 0;
	for (; i < N; i++)
	{
		if (arr[i] == x)
			return true;
	}; 
	
	return false;
}

/* Returns a pointer to the first occurrence of 'c' in 's', or a null pointer if 'c' is not present. */
const char* _nmd_strchr(const char* s, char c)
{
	for (; *s; s++)
	{
		if (*s == c)
			return s;
	}

	return 0;
}

/* Returns a pointer to the last occurrence of 'c' in 's', or a null pointer if 'c' is not present. */
const char* _nmd_reverse_strchr(const char* s, char c)
{
	const char* end = s;
	while (*end)
		end++;

	for (; end > s; end--)
	{
		if (*end == c)
			return end;
	}

	return 0;
}

/* Returns a pointer to the first occurrence of 's2' in 's', or a null pointer if 's2' is not present. */
const char* _nmd_strstr(const char* s, const char* s2)
{
	size_t i = 0;
	for (; *s; s++)
	{
		if (s2[i] == '\0')
			return s - i;

		if (*s != s2[i])
			i = 0;

		if (*s == s2[i])
			i++;
	}

	return 0;
}

/* Returns a pointer to the first occurrence of 's2' in 's', or a null pointer if 's2' is not present. If 's3_opt' is not null it receives the address of the next byte in 's'. */
const char* _nmd_strstr_ex(const char* s, const char* s2, const char** s3_opt)
{
	size_t i = 0;
	for (; *s; s++)
	{
		if (s2[i] == '\0')
		{
			if (s3_opt)
				*s3_opt = s;
			return s - i;
		}

		if (*s != s2[i])
			i = 0;

		if (*s == s2[i])
			i++;
	}

	return 0;
}

/* Inserts 'c' at 's'. */
void _nmd_insert_char(const char* s, char c)
{
	char* end = (char*)s;
	while (*end)
		end++;

	*(end + 1) = '\0';

	for (; end > s; end--)
		*end = *(end - 1);

	*end = c;
}

/* Returns true if there is only a number between 's1' and 's2', false otherwise. */
bool _nmd_is_number(const char* s1, const char* s2)
{
	const char* s = s1;
	for (; s < s2; s++)
	{
		if (!(*s >= '0' && *s <= '9') && !(*s >= 'a' && *s <= 'f') && !(*s >= 'A' && *s <= 'F'))
		{
			if ((s == s1 + 1 && *s1 == '0' && (*s == 'x' || *s == 'X')) || (s == s2 - 1 && (*s == 'h' || *s == 'H')))
				continue;

			return false;
		}
	}

	return true;
}

/* Returns a pointer to the first occurence of a number between 's1' and 's2', zero otherwise. */
const char* _nmd_find_number(const char* s1, const char* s2)
{
	const char* s = s1;
	for (; s < s2; s++)
	{
		if ((*s >= '0' && *s <= '9') || (*s >= 'a' && *s <= 'f') || (*s >= 'A' && *s <= 'F'))
			return s;
	}

	return 0;
}

/* Returns true if s1 matches s2 exactly. */
bool _nmd_strcmp(const char* s1, const char* s2)
{
	for (; *s1 && *s2; s1++, s2++)
	{
		if (*s1 != *s2)
			return false;
	}

	return !*s1 && !*s2;
}

size_t _nmd_getBitNumber(uint32_t mask)
{
	size_t i = 0;
	while (!(mask & (1 << i)))
		i++;

	return i;
}