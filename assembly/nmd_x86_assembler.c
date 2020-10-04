#include "nmd_common.h"

typedef struct _nmd_assemble_info
{
	const char* s; /* string */
	uint8_t* b; /* buffer */
	NMD_X86_MODE mode;
	uint64_t runtime_address;
} _nmd_assemble_info;

enum _NMD_NUMBER_BASE
{
	_NMD_NUMBER_BASE_NONE = 0,
	_NMD_NUMBER_BASE_DECIMAL = 10,
	_NMD_NUMBER_BASE_HEXADECIMAL = 16,
	_NMD_NUMBER_BASE_BINARY = 2
};

NMD_ASSEMBLY_API size_t _nmd_assemble_reg(_nmd_assemble_info* ai, uint8_t base_byte)
{
	uint8_t i = 0;
	if (ai->mode == NMD_X86_MODE_64)
	{
		for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_reg64); i++)
		{
			if (_nmd_strcmp(ai->s, _nmd_reg64[i]))
			{
				ai->b[0] = base_byte + i;
				return 1;
			}
		}

		for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_regrx); i++)
		{
			if (_nmd_strcmp(ai->s, _nmd_regrx[i]))
			{
				ai->b[0] = 0x41;
				ai->b[1] = base_byte + i;
				return 2;
			}
		}
	}
	else if (ai->mode == NMD_X86_MODE_32)
	{
		for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_reg32); i++)
		{
			if (_nmd_strcmp(ai->s, _nmd_reg32[i]))
			{
				ai->b[0] = base_byte + i;
				return 1;
			}
		}
	}	

	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_reg16); i++)
	{
		if (_nmd_strcmp(ai->s, _nmd_reg16[i]))
		{
			ai->b[0] = 0x66;
			ai->b[1] = base_byte + i;
			return 2;
		}
	}

	return 0;
}

NMD_ASSEMBLY_API uint8_t _nmd_encode_segment_reg(NMD_X86_REG segment_reg)
{
	switch (segment_reg)
	{
	case NMD_X86_REG_ES: return 0x26;
	case NMD_X86_REG_CS: return 0x2e;
	case NMD_X86_REG_SS: return 0x36;
	case NMD_X86_REG_DS: return 0x3e;
	case NMD_X86_REG_FS: return 0x64;
	case NMD_X86_REG_GS: return 0x65;
	default: return 0;
	}
}

NMD_ASSEMBLY_API bool _nmd_parse_number(const char* string, int64_t* p_num, size_t* p_num_digits)
{
	if (*string == '\0')
		return false;

	/* Assume decimal base. */
	uint8_t base = _NMD_NUMBER_BASE_DECIMAL;
	size_t i;
	const char* s = string;
	bool is_negative = false;

	if (s[0] == '-')
	{
		is_negative = true;
		s++;
	}

	if (s[0] == '0')
	{
		if (s[1] == 'x')
		{
			s += 2;
			base = _NMD_NUMBER_BASE_HEXADECIMAL;
		}
		else if (s[1] == 'b')
		{
			s += 2;
			base = _NMD_NUMBER_BASE_BINARY;
		}
	}

	for (i = 0; s[i]; i++)
	{
		const char c = s[i];

		if (base == _NMD_NUMBER_BASE_DECIMAL)
		{
			if (c >= 'a' && c <= 'f')
			{
				base = _NMD_NUMBER_BASE_HEXADECIMAL;
				continue;
			}
			else if(!(c >= '0' && c <= '9'))
				break;
		}
		else if (base == _NMD_NUMBER_BASE_HEXADECIMAL)
		{
			if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')))
				break;
		}
		else if (c != '0' && c != '1') /* _NMD_NUMBER_BASE_BINARY */
			break;
	}

	size_t num_digits = i;

	int64_t num = 0;
	for (i = 0; i < num_digits; i++)
	{
		const char c = s[i];
		num += (c <= '9') ? (c - '0') : (10 + c - 'a');
		if (i < num_digits - 1)
		{
			/* Return false if number is greater than 2^64-1 */
			if ( num_digits > 16 && i >= 15)
			{
				if ((base == _NMD_NUMBER_BASE_DECIMAL && (uint64_t)num >= (uint64_t)1844674407370955162) || /* ceiling((2^64-1) / 10) */
					(base == _NMD_NUMBER_BASE_HEXADECIMAL && (uint64_t)num >= (uint64_t)1152921504606846976) || /* *ceiling((2^64-1) / 16) */
					(base == _NMD_NUMBER_BASE_BINARY && (uint64_t)num >= (uint64_t)9223372036854775808)) /* ceiling((2^64-1) / 2) */
				{
					return false;
				}

			}
			num *= base;
		}
	}

	if (s != string) /* There's either a "0x" or "0b" prefix. */
		num_digits = (size_t)((ptrdiff_t)(s + i) - (ptrdiff_t)string);
	
	if (is_negative)
		num *= -1;

	*p_num = num;

	if(p_num_digits)
		*p_num_digits = num_digits;

	return true;
}

NMD_ASSEMBLY_API size_t _nmd_append_prefix_by_reg_size(uint8_t* b, const char* s, size_t* num_prefixes, size_t* index)
{
	size_t i;
	
	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_reg32); i++)
	{
		if (_nmd_strcmp(s, _nmd_reg32[i]))
		{
			*num_prefixes = 0;
			*index = i;
			return 4;
		}
	}

	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_reg8); i++)
	{
		if (_nmd_strcmp(s, _nmd_reg8[i]))
		{
			*num_prefixes = 0;
			*index = i;
			return 1;
		}
	}

	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_reg16); i++)
	{
		if (_nmd_strcmp(s, _nmd_reg16[i]))
		{
			b[0] = 0x66;
			*num_prefixes = 1;
			*index = i;
			return 2;
		}
	}	

	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_reg64); i++)
	{
		if (_nmd_strcmp(s, _nmd_reg64[i]))
		{
			b[0] = 0x48;
			*num_prefixes = 1;
			*index = i;
			return 8;
		}
	}

	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_regrx); i++)
	{
		if (_nmd_strcmp(s, _nmd_regrx[i]))
		{
			b[0] = 0x49;
			*num_prefixes = 1;
			*index = i;
			return 8;
		}
	}

	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_regrxd); i++)
	{
		if (_nmd_strcmp(s, _nmd_regrxd[i]))
		{
			b[0] = 0x41;
			*num_prefixes = 1;
			*index = i;
			return 4;
		}
	}

	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_regrxw); i++)
	{
		if (_nmd_strcmp(s, _nmd_regrxw[i]))
		{
			b[0] = 0x66;
			b[1] = 0x41;
			*num_prefixes = 2;
			*index = i;
			return 2;
		}
	}

	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_regrxb); i++)
	{
		if (_nmd_strcmp(s, _nmd_regrxb[i]))
		{
			b[0] = 0x41;
			*num_prefixes = 1;
			*index = i;
			return 1;
		}
	}

	return 0;
}

/* 
Parses a memory operand in the format: '[exp]'
string: a pointer to the string that represents the memory operand. The string is modified to point to the character after the memory operand.
operand[out]: Describes the memory operand.
size[out]: The size of the pointer. byte ptr:1, dword ptr:4, etc.. or zero if unknown.
Return value: True if success, false otherwise.
*/
NMD_ASSEMBLY_API bool _nmd_parse_memory_operand(const char** string, nmd_x86_memory_operand* operand, size_t* size)
{
	const char* s = *string;
	size_t num_bytes = 0;
	if (_nmd_strstr(s, "byte") == s)
		num_bytes = 1;
	else if (_nmd_strstr(s, "word") == s)
		num_bytes = 2;
	else if (_nmd_strstr(s, "dword") == s)
		num_bytes = 4;
	else if (_nmd_strstr(s, "qword") == s)
		num_bytes = 8;
	else
		num_bytes = 0;
	*size = num_bytes;

	if (num_bytes > 0)
	{
		s += num_bytes >= 4 ? 5 : 4;

		if (s[0] == ' ' && s[1] == 'p' && s[2] == 't' && s[3] == 'r')
			s += 4;

		if (s[0] == ' ')
			s++;
		else if (s[0] != '[')
			return false;
	}

	size_t i = 0;
	operand->segment = NMD_X86_REG_NONE;
	for (; i < _NMD_NUM_ELEMENTS(_nmd_segment_reg); i++)
	{
		if (_nmd_strstr(s, _nmd_segment_reg[i]) == s)
		{
			if (s[2] != ':')
				return false;

			s += 3;
			operand->segment = (uint8_t)(NMD_X86_REG_ES + i);
			break;
		}
	}

	if (s[0] == '[')
		s++;
	else
		return false;
	
	operand->base = 0;
	operand->index = 0;
	operand->scale = 0;
	operand->disp = 0;
	bool add = false;
	bool sub = false;
	bool multiply = false;
	bool is_register = false;
	while (true)
	{
		bool parsed_element = false;
		if (!sub && !multiply)
		{
			for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_reg32); i++)
			{
				const char* tmp;
				if (_nmd_strstr_ex(s, _nmd_reg32[i], &tmp) == s)
				{
					s = tmp;
					if (add)
					{
						operand->index = (uint8_t)(NMD_X86_REG_EAX + i);
						operand->scale = 1;
						add = false;
					}
					else
						operand->base = (uint8_t)(NMD_X86_REG_EAX + i);
					parsed_element = true;
					is_register = true;
					break;
				}
			}
		}

		int64_t num;
		size_t num_digits;
		if (!parsed_element && _nmd_parse_number(s, &num, &num_digits))
		{
			s += num_digits;

			if (add)
			{
				operand->disp += num;
				add = false;
			}
			else if (sub)
			{
				operand->disp -= num;
				sub = false;
			}
			else if (multiply)
			{
				if (!is_register || (num != 1 && num != 2 && num != 4 && num != 8))
					return false;

				operand->scale = (uint8_t)num;
			}
			else
				operand->disp = num;

			parsed_element = true;
		}

		if (!parsed_element)
			return false;

		if (s[0] == '+')
			add = true;
		else if (s[0] == '-')
			sub = true;
		else if (s[0] == '*')
		{
			/* There cannot be more than one '*' operator. */
			if (multiply)
				return false;

			multiply = true;
		}
		else if (s[0] == ']')
		{
			break;
		}
		else
			return false;

		s++;
	}

	*string = s + 1;
	return true;
}

NMD_ASSEMBLY_API size_t _nmd_assemble_single(_nmd_assemble_info* ai)
{
	size_t i = 0;

	/* Parse prefixes */
	bool lock_prefix = false, repeat_prefix = false, repeat_zero_prefix = false, repeat_not_zero_prefix = false;
	if (_nmd_strstr(ai->s, "lock ") == ai->s)
		lock_prefix = true, ai->s += 5;
	else if (_nmd_strstr(ai->s, "rep ") == ai->s)
		repeat_prefix = true, ai->s += 4;
	else if (_nmd_strstr(ai->s, "repe ") == ai->s || _nmd_strstr(ai->s, "repz ") == ai->s)
		repeat_zero_prefix = true, ai->s += 5;
	else if (_nmd_strstr(ai->s, "repne ") == ai->s || _nmd_strstr(ai->s, "repnz ") == ai->s)
		repeat_not_zero_prefix = true, ai->s += 6;
	
	if (_nmd_strstr(ai->s, "xacquire ") == ai->s)
	{

	}
	else if (_nmd_strstr(ai->s, "xrelease ") == ai->s)
	{

	}

	/* Parse opcodes */
	if (ai->mode == NMD_X86_MODE_64) /* Only x86-64. */
	{
		if (_nmd_strcmp(ai->s, "xchg r8,rax") || _nmd_strcmp(ai->s, "xchg rax,r8"))
		{
			ai->b[0] = 0x49;
			ai->b[1] = 0x90;
			return 2;
		}
		else if (_nmd_strcmp(ai->s, "xchg r8d,eax") || _nmd_strcmp(ai->s, "xchg eax,r8d"))
		{
			ai->b[0] = 0x41;
			ai->b[1] = 0x90;
			return 2;
		}
		else if (_nmd_strcmp(ai->s, "pushfq"))
		{
			ai->b[0] = 0x9c;
			return 1;
		}
		else if (_nmd_strcmp(ai->s, "popfq"))
		{
			ai->b[0] = 0x9d;
			return 1;
		}
		else if (_nmd_strcmp(ai->s, "iretq"))
		{
			ai->b[0] = 0x48;
			ai->b[1] = 0xcf;
			return 2;
		}
		else if (_nmd_strcmp(ai->s, "cdqe"))
		{
			ai->b[0] = 0x48;
			ai->b[1] = 0x98;
			return 2;
		}
		else if (_nmd_strcmp(ai->s, "cqo"))
		{
			ai->b[0] = 0x48;
			ai->b[1] = 0x99;
			return 2;
		}
	}
	else /* x86-16 / x86-32 */
	{
		if (_nmd_strcmp(ai->s, "pushad"))
		{
			ai->b[0] = 0x60;
			return 1;
		}
		else if (_nmd_strcmp(ai->s, "pusha"))
		{
			ai->b[0] = 0x66;
			ai->b[1] = 0x60;
			return 2;
		}
		else if (_nmd_strcmp(ai->s, "popad"))
		{
			ai->b[0] = 0x61;
			return 1;
		}
		else if (_nmd_strcmp(ai->s, "popa"))
		{
			ai->b[0] = 0x66;
			ai->b[1] = 0x62;
			return 2;
		}
		else if (_nmd_strcmp(ai->s, "pushfd"))
		{
			if (ai->mode == NMD_X86_MODE_16)
			{
				ai->b[0] = 0x66;
				ai->b[1] = 0x9c;
				return 2;
			}
			else
			{
				ai->b[0] = 0x9c;
				return 1;
			}
		}
		else if (_nmd_strcmp(ai->s, "popfd"))
		{
			if (ai->mode == NMD_X86_MODE_16)
			{
				ai->b[0] = 0x66;
				ai->b[1] = 0x9d;
				return 2;
			}
			else
			{
				ai->b[0] = 0x9d;
				return 1;
			}
		}
	}

	typedef struct _nmd_string_byte_pair { const char* s; uint8_t b; } _nmd_string_byte_pair;

	const _nmd_string_byte_pair single_byte_op1[] = {
		{ "int3",    0xcc },
		{ "nop",     0x90 },
		{ "ret",     0xc3 },
		{ "retf",    0xcb },
		{ "ret far", 0xcb },
		{ "leave",   0xc9 },
		{ "int1",    0xf1 },
		{ "push es", 0x06 },
		{ "push ss", 0x16 },
		{ "push ds", 0x1e },
		{ "push cs", 0x0e },
		{ "pop es",  0x07 },
		{ "pop ss",  0x17 },
		{ "pop ds",  0x1f },
		{ "daa",     0x27 },
		{ "aaa",     0x37 },
		{ "das",     0x2f },
		{ "aas",     0x3f },
		{ "xlat",    0xd7 },
		{ "fwait",   0x9b },
		{ "hlt",     0xf4 },
		{ "cmc",     0xf5 },
		{ "sahf",    0x9e },
		{ "lahf",    0x9f },
		{ "into",    0xce },
		{ "cwde",    0x98 },
		{ "cdq",     0x99 },
		{ "salc",    0xd6 },
		{ "slc",     0xf8 },
		{ "stc",     0xf9 },
		{ "cli",     0xfa },
		{ "sti",     0xfb },
		{ "cld",     0xfc },
		{ "std",     0xfd },
	};
	for (i = 0; i < _NMD_NUM_ELEMENTS(single_byte_op1); i++)
	{
		if (_nmd_strcmp(ai->s, single_byte_op1[i].s))
		{
			ai->b[0] = single_byte_op1[i].b;
			return 1;
		}
	}

	const _nmd_string_byte_pair single_byte_op2[] = {
		{ "syscall",  0x05 },
		{ "clts",     0x06 },
		{ "sysret",   0x07 },
		{ "invd",     0x08 },
		{ "wbinvd",   0x09 },
		{ "ud2",      0x0b },
		{ "femms",    0x0e },
		{ "wrmsr",    0x30 },
		{ "rdtsc",    0x31 },
		{ "rdmsr",    0x32 },
		{ "rdpmc",    0x33 },
		{ "sysenter", 0x34 },
		{ "sysexit",  0x35 },
		{ "getsec",   0x37 },
		{ "emms",     0x77 },
		{ "push fs",  0xa0 },
		{ "pop fs",   0xa1 },
		{ "cpuid",    0xa2 },
		{ "push gs",  0xa8 },
		{ "pop gs",   0xa9 },
		{ "rsm",      0xaa }
	};
	for (i = 0; i < _NMD_NUM_ELEMENTS(single_byte_op2); i++)
	{
		if (_nmd_strcmp(ai->s, single_byte_op2[i].s))
		{
			ai->b[0] = 0x0f;
			ai->b[1] = single_byte_op2[i].b;
			return 2;
		}
	}
	
	if (ai->s[0] == 'j')
	{
		const char* s = 0;
		for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_condition_suffixes); i++)
		{
			if (_nmd_strstr_ex(ai->s + 1, _nmd_condition_suffixes[i], &s) == ai->s + 1)
			{
				if (s[0] != ' ')
					return 0;

				int64_t num;
				size_t num_digits;
				if (!_nmd_parse_number(s + 1, &num, &num_digits))
					return 0;

				const int64_t delta = num - ai->runtime_address;
				if (delta >= -(1 << 7) + 2 && delta <= (1 << 7) - 1 + 2)
				{
					ai->b[0] = 0x70 + (uint8_t)i;
					*(int8_t*)(ai->b + 1) = (int8_t)(delta - 2);
					return 2;
				}
				else if (delta >= -(1 << 31) + 6 && delta <= ((size_t)(1) << 31) - 1 + 6)
				{
					ai->b[0] = 0x0f;
					ai->b[1] = 0x80 + (uint8_t)i;
					*(int32_t*)(ai->b + 2) = (int32_t)(delta - 6);
					return 6;
				}
				else
					return 0;
			}
		}
	}
	else if (_nmd_strstr(ai->s, "inc ") == ai->s || _nmd_strstr(ai->s, "dec ") == ai->s)
	{
		const char* tmp = ai->s + 4;
		nmd_x86_memory_operand memory_operand;
		size_t size;
		if (_nmd_parse_memory_operand(&tmp, &memory_operand, &size))
		{
			size_t offset = 0;
			if (memory_operand.segment && memory_operand.segment != ((memory_operand.base == NMD_X86_REG_ESP || memory_operand.index == NMD_X86_REG_ESP) ? NMD_X86_REG_SS : NMD_X86_REG_DS))
				ai->b[offset++] = _nmd_encode_segment_reg((NMD_X86_REG)memory_operand.segment);

			ai->b[offset++] = size == 1 ? 0xfe : 0xff;

			nmd_x86_modrm modrm;
			modrm.fields.reg = ai->s[0] == 'i' ? 0 : 8;
			modrm.fields.mod = 0;

			if (memory_operand.index != NMD_X86_REG_NONE && memory_operand.base != NMD_X86_REG_NONE)
			{
				modrm.fields.rm = 0b100;
				nmd_x86_sib sib;
				sib.fields.scale = (uint8_t)_nmd_get_bit_index(memory_operand.scale);
				sib.fields.base = memory_operand.base - NMD_X86_REG_EAX;
				sib.fields.index = memory_operand.index - NMD_X86_REG_EAX;

				const size_t nextOffset = offset;
				if (memory_operand.disp != 0)
				{
					if (memory_operand.disp >= -128 && memory_operand.disp <= 127)
					{
						modrm.fields.mod = 1;
						*(int8_t*)(ai->b + offset + 2) = (int8_t)memory_operand.disp;
						offset++;
					}
					else
					{
						modrm.fields.mod = 2;
						*(int32_t*)(ai->b + offset + 2) = (int32_t)memory_operand.disp;
						offset += 4;
					}
				}

				ai->b[nextOffset] = modrm.modrm;
				ai->b[nextOffset + 1] = sib.sib;
				offset += 2;

				return offset;
			}
			else if (memory_operand.base != NMD_X86_REG_NONE)
			{
				modrm.fields.rm = memory_operand.base - NMD_X86_REG_EAX;
				const size_t nextOffset = offset;
				if (memory_operand.disp != 0)
				{
					if (memory_operand.disp >= -128 && memory_operand.disp <= 127)
					{
						modrm.fields.mod = 1;
						*(int8_t*)(ai->b + offset + 1) = (int8_t)memory_operand.disp;
						offset++;
					}
					else
					{
						modrm.fields.mod = 2;
						*(int32_t*)(ai->b + offset + 1) = (int32_t)memory_operand.disp;
						offset += 4;
					}
				}
				ai->b[nextOffset] = modrm.modrm;
				offset++;
			}
			else
			{
				modrm.fields.rm = 0b101;
				ai->b[offset++] = modrm.modrm;
				*(int32_t*)(ai->b + offset) = (int32_t)memory_operand.disp;
				offset += 4;
			}

			return offset;
		}

		size_t num_prefixes, index;
		size = _nmd_append_prefix_by_reg_size(ai->b, ai->s + 4, &num_prefixes, &index);
		if (size > 0)
		{
			if (ai->mode == NMD_X86_MODE_64)
			{
				ai->b[num_prefixes + 0] = size == 1 ? 0xfe : 0xff;
				ai->b[num_prefixes + 1] = 0xc0 + (ai->s[0] == 'i' ? 0 : 8) + (uint8_t)index;
				return num_prefixes + 2;
			}
			else
			{
				if (size == 1)
				{
					ai->b[0] = 0xfe;
					ai->b[1] = 0xc0 + (ai->s[0] == 'i' ? 0 : 8) + (uint8_t)index;
					return 2;
				}
				else
				{
					ai->b[num_prefixes + 0] = (ai->s[0] == 'i' ? 0x40 : 0x48) + (uint8_t)index;
					return num_prefixes + 1;
				}
			}
		}
	}
	else if (_nmd_strstr(ai->s, "push ") == ai->s)
	{
		size_t num_digits = 0;
		int64_t num = 0;
		if (_nmd_parse_number(ai->s + 5, &num, &num_digits))
		{
			if (*(ai->s + num_digits) != '\0' || !(num >= -(1 << 31) && num <= ((int64_t)1 << 31) - 1))
				return 0;

			if (num >= -(1 << 7) && num <= (1 << 7) - 1)
			{
				ai->b[0] = 0x6a;
				*(int8_t*)(ai->b + 1) = (int8_t)num;
				return 2;
			}
			else
			{
				ai->b[0] = 0x68;
				*(int32_t*)(ai->b + 1) = (int32_t)num;
				return 5;
			}
		}

		size_t n = _nmd_assemble_reg(ai, 0x50);
		if (n > 0)
			return n;
	}
	else if (_nmd_strstr(ai->s, "pop ") == ai->s)
	{
		ai->s += 3;
		return _nmd_assemble_reg(ai, 0x58);
	}
	else if (_nmd_strstr(ai->s, "emit ") == ai->s)
	{
		int64_t num = 0;
		size_t num_digits = 0;
		size_t offset = 5;
		while (_nmd_parse_number(ai->s + offset, &num, &num_digits))
		{
			if (num < 0 || num > 0xff)
				return 0;

			ai->b[i++] = (uint8_t)num;

			offset += num_digits;
			if (ai->s[offset] == ' ')
				offset++;
		}
		return i;
	}
	else if (_nmd_strcmp(ai->s, "pushf"))
	{
		if (ai->mode == NMD_X86_MODE_16)
		{
			ai->b[0] = 0x9c;
			return 1;
		}
		else
		{
			ai->b[0] = 0x66;
			ai->b[1] = 0x9c;
			return 2;
		}
	}
	else if (_nmd_strcmp(ai->s, "popf"))
	{
		if (ai->mode == NMD_X86_MODE_16)
		{
			ai->b[0] = 0x9d;
			return 1;
		}
		else
		{
			ai->b[0] = 0x66;
			ai->b[1] = 0x9d;
			return 2;
		}
	}
	else if (_nmd_strcmp(ai->s, "pause"))
	{
		ai->b[0] = 0xf3;
		ai->b[1] = 0x90;
		return 2;
	}
	else if (_nmd_strcmp(ai->s, "iret"))
	{
		if (ai->mode == NMD_X86_MODE_16)
		{
			ai->b[0] = 0xcf;
			return 1;
		}
		else
		{
			ai->b[0] = 0x66;
			ai->b[1] = 0xcf;
			return 2;
		}
	}
	else if (_nmd_strcmp(ai->s, "iretd"))
	{
		if (ai->mode == NMD_X86_MODE_16)
		{
			ai->b[0] = 0x66;
			ai->b[1] = 0xcf;
			return 2;
		}
		else
		{
			ai->b[0] = 0xcf;
			return 1;
		}
	}
	else if (_nmd_strcmp(ai->s, "cbw"))
	{
		ai->b[0] = 0x66;
		ai->b[1] = 0x98;
		return 2;
	}
	else if (_nmd_strcmp(ai->s, "cwd"))
	{
		ai->b[0] = 0x66;
		ai->b[1] = 0x99;
		return 2;
	}
	else if (_nmd_strcmp(ai->s, "pushf"))
	{
		ai->b[0] = 0x66;
		ai->b[1] = 0x9c;
		return 2;
	}
	else if (_nmd_strcmp(ai->s, "popf"))
	{
		ai->b[0] = 0x66;
		ai->b[1] = 0x9d;
		return 2;
	}
	
	return 0;
}

/*
Assembles an instruction from a string. Returns the number of bytes written to the buffer on success, zero otherwise. Instructions can be separated using the '\n'(new line) character.
Parameters:
 - string          [in]         A pointer to a string that represents one or more instructions in assembly language.
 - buffer          [out]        A pointer to a buffer that receives the encoded instructions.
 - buffer_size     [in]         The size of the buffer in bytes.
 - runtime_address [in]         The instruction's runtime address. You may use 'NMD_X86_INVALID_RUNTIME_ADDRESS'.
 - mode            [in]         The architecture mode. 'NMD_X86_MODE_32', 'NMD_X86_MODE_64' or 'NMD_X86_MODE_16'.
 - count           [in/out/opt] A pointer to a variable that on input is the maximum number of instructions that can be parsed(or zero for unlimited instructions), and on output is the number of instructions parsed. This parameter may be 0(zero).
*/
NMD_ASSEMBLY_API size_t nmd_x86_assemble(const char* string, void* buffer, size_t buffer_size, uint64_t runtime_address, NMD_X86_MODE mode, size_t* count)
{
	char parsed_string[128];
	const uint8_t* const buffer_end = (uint8_t*)buffer + buffer_size;
	uint8_t* b = (uint8_t*)buffer;
	size_t remaining_size;

	uint8_t temp_buffer[NMD_X86_MAXIMUM_INSTRUCTION_LENGTH];

	_nmd_assemble_info ai;
	ai.s = parsed_string;
	ai.mode = mode;
	ai.runtime_address = runtime_address;
	ai.b = temp_buffer;

	size_t num_instructions = 0;
	const size_t num_max_instructions = (count && *count != 0) ? *count : (size_t)(-1);

	while (string[0] != '\0' && num_instructions < num_max_instructions)
	{
		remaining_size = buffer_end - b;

		/* Copy 'string' to 'buffer' converting it to lowercase and removing unwanted spaces. If the instruction separator character ';' and '\n' is found, stop. */
		size_t length = 0;
		bool allow_space = false;
		for (; *string; string++)
		{
			char c = *string;
			if (c == '\n')
				break;
			else if (c == ' ' && !allow_space)
				continue;

			if (length >= 128)
				return 0;

			c = _NMD_TOLOWER(c);
			parsed_string[length++] = c;
			allow_space = (_NMD_IS_LOWERCASE(c) || _NMD_IS_DECIMAL_NUMBER(c)) && (_NMD_IS_LOWERCASE(string[2]) || _NMD_IS_DECIMAL_NUMBER(string[2]));
		}

		if (*string != '\0')
			string++;

		/* If the last character is a ' '(space), remove it. */
		if (length > 0 && parsed_string[length - 1] == ' ')
			length--;

		/* After all of the string manipulation, place the null character. */
		parsed_string[length] = '\0';

		const size_t num_bytes = _nmd_assemble_single(&ai);
		if (num_bytes == 0 || num_bytes > remaining_size)
			return 0;

		/* Copy bytes from 'temp_buffer' to the buffer provided by the user. */
		size_t i = 0;
		for (; i < num_bytes; i++)
			b[i] = temp_buffer[i];

		b += num_bytes;

		num_instructions++;
	}

	if (count)
		*count = num_instructions;

	return (size_t)(b - (uint8_t*)buffer);
}