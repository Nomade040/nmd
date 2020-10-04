#include "nmd_common.h"

/* Four high-order bits of an opcode to index a row of the opcode table */
#define NMD_R(b) ((b) >> 4)

/* Four low-order bits to index a column of the table */
#define NMD_C(b) ((b) & 0xF)

#define _NMD_NUM_ELEMENTS(arr) (sizeof(arr) / sizeof((arr)[0]))

#define _NMD_IS_UPPERCASE(c) (c >= 'A' && c <= 'Z')
#define _NMD_IS_LOWERCASE(c) (c >= 'a' && c <= 'z')
#define _NMD_TOLOWER(c) (_NMD_IS_UPPERCASE(c) ? c + 0x20 : c)
#define _NMD_IS_DECIMAL_NUMBER(c) (c >= '0' && c <= '9')

NMD_ASSEMBLY_API const char* const _nmd_reg8[] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
NMD_ASSEMBLY_API const char* const _nmd_reg8_x64[] = { "al", "cl", "dl", "bl", "spl", "bpl", "sil", "dil" };
NMD_ASSEMBLY_API const char* const _nmd_reg16[] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" };
NMD_ASSEMBLY_API const char* const _nmd_reg32[] = { "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi" };
NMD_ASSEMBLY_API const char* const _nmd_reg64[] = { "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi" };
NMD_ASSEMBLY_API const char* const _nmd_regrx[] = { "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15" };
NMD_ASSEMBLY_API const char* const _nmd_regrxd[] = { "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d" };
NMD_ASSEMBLY_API const char* const _nmd_regrxw[] = { "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w" };
NMD_ASSEMBLY_API const char* const _nmd_regrxb[] = { "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b" };
NMD_ASSEMBLY_API const char* const _nmd_segment_reg[] = { "es", "cs", "ss", "ds", "fs", "gs" };

NMD_ASSEMBLY_API const char* const _nmd_condition_suffixes[] = { "o", "no", "b", "ae", "e", "ne", "be", "a", "s", "ns", "p", "np", "l", "ge", "le", "g" };

NMD_ASSEMBLY_API const char* const _nmd_op1_opcode_map_mnemonics[] = { "add", "adc", "and", "xor", "or", "sbb", "sub", "cmp" };
NMD_ASSEMBLY_API const char* const _nmd_opcode_extensions_grp1[] = { "add", "or", "adc", "sbb", "and", "sub", "xor", "cmp" };
NMD_ASSEMBLY_API const char* const _nmd_opcode_extensions_grp2[] = { "rol", "ror", "rcl", "rcr", "shl", "shr", "shl", "sar" };
NMD_ASSEMBLY_API const char* const _nmd_opcode_extensions_grp3[] = { "test", "test", "not", "neg", "mul", "imul", "div", "idiv" };
NMD_ASSEMBLY_API const char* const _nmd_opcode_extensions_grp5[] = { "inc", "dec", "call", "call far", "jmp", "jmp far", "push" };
NMD_ASSEMBLY_API const char* const _nmd_opcode_extensions_grp6[] = { "sldt", "str", "lldt", "ltr", "verr", "verw" };
NMD_ASSEMBLY_API const char* const _nmd_opcode_extensions_grp7[] = { "sgdt", "sidt", "lgdt", "lidt", "smsw", 0, "lmsw", "invlpg" };
NMD_ASSEMBLY_API const char* const _nmd_opcode_extensions_grp7_reg0[] = { "enclv", "vmcall", "vmlaunch", "vmresume", "vmxoff", "pconfig" };
NMD_ASSEMBLY_API const char* const _nmd_opcode_extensions_grp7_reg1[] = { "monitor", "mwait", "clac", "stac", 0, 0, 0, "encls" };
NMD_ASSEMBLY_API const char* const _nmd_opcode_extensions_grp7_reg2[] = { "xgetbv", "xsetbv", 0, 0, "vmfunc", "xend", "xtest", "enclu" };
NMD_ASSEMBLY_API const char* const _nmd_opcode_extensions_grp7_reg3[] = { "vmrun ", "vmmcall", "vmload ", "vmsave", "stgi", "clgi", "skinit eax", "invlpga " };
NMD_ASSEMBLY_API const char* const _nmd_opcode_extensions_grp7_reg7[] = { "swapgs", "rdtscp", "monitorx", "mwaitx", "clzero ", "rdpru" };

NMD_ASSEMBLY_API const char* const _nmd_escape_opcodesD8[] = { "add", "mul", "com", "comp", "sub", "subr", "div", "divr" };
NMD_ASSEMBLY_API const char* const _nmd_escape_opcodesD9[] = { "ld", 0, "st", "stp", "ldenv", "ldcw", "nstenv", "nstcw" };
NMD_ASSEMBLY_API const char* const _nmd_escape_opcodesDA_DE[] = { "iadd", "imul", "icom", "icomp", "isub", "isubr", "idiv", "idivr" };
NMD_ASSEMBLY_API const char* const _nmd_escape_opcodesDB[] = { "ild", "isttp", "ist", "istp", 0, "ld", 0, "stp" };
NMD_ASSEMBLY_API const char* const _nmd_escape_opcodesDC[] = { "add", "mul", "com", "comp", "sub", "subr", "div", "divr" };
NMD_ASSEMBLY_API const char* const _nmd_escape_opcodesDD[] = { "ld", "isttp", "st", "stp", "rstor", 0, "nsave", "nstsw" };
NMD_ASSEMBLY_API const char* const _nmd_escape_opcodesDF[] = { "ild", "isttp", "ist", "istp", "bld", "ild", "bstp", "istp" };
NMD_ASSEMBLY_API const char* const* _nmd_escape_opcodes[] = { _nmd_escape_opcodesD8, _nmd_escape_opcodesD9, _nmd_escape_opcodesDA_DE, _nmd_escape_opcodesDB, _nmd_escape_opcodesDC, _nmd_escape_opcodesDD, _nmd_escape_opcodesDA_DE, _nmd_escape_opcodesDF };

NMD_ASSEMBLY_API const uint8_t _nmd_op1_modrm[] = { 0xFF, 0x63, 0x69, 0x6B, 0xC0, 0xC1, 0xC6, 0xC7, 0xD0, 0xD1, 0xD2, 0xD3, 0xF6, 0xF7, 0xFE };
NMD_ASSEMBLY_API const uint8_t _nmd_op1_imm8[] = { 0x6A, 0x6B, 0x80, 0x82, 0x83, 0xA8, 0xC0, 0xC1, 0xC6, 0xCD, 0xD4, 0xD5, 0xEB };
NMD_ASSEMBLY_API const uint8_t _nmd_op1_imm32[] = { 0xE8, 0xE9, 0x68, 0x81, 0x69, 0xA9, 0xC7 };
NMD_ASSEMBLY_API const uint8_t _nmd_invalid_op2[] = { 0x04, 0x0a, 0x0c, 0x7a, 0x7b, 0x36, 0x39 };
NMD_ASSEMBLY_API const uint8_t _nmd_two_opcodes[] = { 0xb0, 0xb1, 0xb3, 0xbb, 0xc0, 0xc1 };
NMD_ASSEMBLY_API const uint8_t _nmd_valid_3DNow_opcodes[] = { 0x0c, 0x0d, 0x1c, 0x1d, 0x8a, 0x8e, 0x90, 0x94, 0x96, 0x97, 0x9a, 0x9e, 0xa0, 0xa4, 0xa6, 0xa7, 0xaa, 0xae, 0xb0, 0xb4, 0xb6, 0xb7, 0xbb, 0xbf };

NMD_ASSEMBLY_API bool _nmd_find_byte(const uint8_t* arr, const size_t N, const uint8_t x)
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
NMD_ASSEMBLY_API const char* _nmd_strchr(const char* s, char c)
{
	for (; *s; s++)
	{
		if (*s == c)
			return s;
	}

	return 0;
}

/* Returns a pointer to the last occurrence of 'c' in 's', or a null pointer if 'c' is not present. */
NMD_ASSEMBLY_API const char* _nmd_reverse_strchr(const char* s, char c)
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
NMD_ASSEMBLY_API const char* _nmd_strstr(const char* s, const char* s2)
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
NMD_ASSEMBLY_API const char* _nmd_strstr_ex(const char* s, const char* s2, const char** s3_opt)
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
NMD_ASSEMBLY_API void _nmd_insert_char(const char* s, char c)
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
NMD_ASSEMBLY_API bool _nmd_is_number(const char* s1, const char* s2)
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
NMD_ASSEMBLY_API const char* _nmd_find_number(const char* s1, const char* s2)
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
NMD_ASSEMBLY_API bool _nmd_strcmp(const char* s1, const char* s2)
{
	for (; *s1 && *s2; s1++, s2++)
	{
		if (*s1 != *s2)
			return false;
	}

	return !*s1 && !*s2;
}

NMD_ASSEMBLY_API size_t _nmd_get_bit_index(uint32_t mask)
{
	size_t i = 0;
	while (!(mask & (1 << i)))
		i++;

	return i;
}