#include "nmd_common.h"

typedef struct _nmd_assemble_info
{
	char* s; /* string */
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

NMD_ASSEMBLY_API size_t _nmd_parse_number(const char* string, int64_t* p_num)
{
	if (*string == '\0')
		return 0;

	/* Assume decimal base. */
	uint8_t base = _NMD_NUMBER_BASE_DECIMAL;
	size_t i;
	const char* s = string;
	bool is_negative = false;
	bool force_positive = false;
	bool h_suffix = false;
    bool assume_hex = false;

	if (s[0] == '-')
	{
		is_negative = true;
		s++;
	}
	else if (s[0] == '+')
	{
		force_positive = true;
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
			if ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
			{
				base = _NMD_NUMBER_BASE_HEXADECIMAL;
                assume_hex = true;
				continue;
			}
			else if (!(c >= '0' && c <= '9'))
				break;
		}
		else if (base == _NMD_NUMBER_BASE_HEXADECIMAL)
		{
			if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
				break;
		}
		else if (c != '0' && c != '1') /* _NMD_NUMBER_BASE_BINARY */
			break;
	}

	if (s[i] == 'h')
	{
		base = _NMD_NUMBER_BASE_HEXADECIMAL;
		h_suffix = true;
	}

	const size_t num_digits = i;
	if (num_digits == 0)
		return 0;

	int64_t num = 0;
	for (i = 0; i < num_digits; i++)
	{
		const char c = s[i];
		int n = c % 16;
		if (c >= 'A')
			n += 9;

		num += n;
		if (i < num_digits - 1)
		{
			/* Return false if number is greater than 2^64-1 */
			if ( num_digits > 16 && i >= 15)
			{
				if ((base == _NMD_NUMBER_BASE_DECIMAL && (uint64_t)num >= (uint64_t)1844674407370955162) || /* ceiling((2^64-1) / 10) */
					(base == _NMD_NUMBER_BASE_HEXADECIMAL && (uint64_t)num >= (uint64_t)1152921504606846976) || /* *ceiling((2^64-1) / 16) */
					(base == _NMD_NUMBER_BASE_BINARY && (uint64_t)num >= (uint64_t)9223372036854775808U)) /* ceiling((2^64-1) / 2) */
				{
					return 0;
				}

			}
			num *= base;
		}
	}

	if (is_negative)
		num *= -1;

	*p_num = num;

	size_t offset = 0;

	if (is_negative || force_positive)
		offset += 1;

	if (h_suffix)
		offset += 1;
	else if ((base == _NMD_NUMBER_BASE_HEXADECIMAL && !assume_hex) || base == _NMD_NUMBER_BASE_BINARY) /* 0x / 0b*/
        offset += 2;
	
	return offset + num_digits;
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

NMD_ASSEMBLY_API NMD_X86_REG _nmd_parse_reg8(const char** string)
{
	const char* s = *string;
	size_t i;
	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_reg8); i++)
	{
		if (_nmd_strstr_ex(s, _nmd_reg8[i], string) == s)
			return (NMD_X86_REG)(NMD_X86_REG_AL + i);
	}
	return NMD_X86_REG_NONE;
}

NMD_ASSEMBLY_API NMD_X86_REG _nmd_parse_reg16(const char** string)
{
	const char* s = *string;
	size_t i;
	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_reg16); i++)
	{
		if (_nmd_strstr_ex(s, _nmd_reg16[i], string) == s)
			return (NMD_X86_REG)(NMD_X86_REG_AX + i);
	}
	return NMD_X86_REG_NONE;
}

NMD_ASSEMBLY_API NMD_X86_REG _nmd_parse_reg32(const char** string)
{
	const char* s = *string;
	size_t i;
	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_reg32); i++)
	{
		if (_nmd_strstr_ex(s, _nmd_reg32[i], string) == s)
			return (NMD_X86_REG)(NMD_X86_REG_EAX + i);
	}
	return NMD_X86_REG_NONE;
}

NMD_ASSEMBLY_API NMD_X86_REG _nmd_parse_reg64(const char** string)
{
	const char* s = *string;
	size_t i;
	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_reg64); i++)
	{
		if (_nmd_strstr_ex(s, _nmd_reg64[i], string) == s)
			return (NMD_X86_REG)(NMD_X86_REG_RAX + i);
	}
	return NMD_X86_REG_NONE;
}

NMD_ASSEMBLY_API NMD_X86_REG _nmd_parse_regrxb(const char** string)
{
	const char* s = *string;
	size_t i;
	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_regrxb); i++)
	{
		if (_nmd_strstr_ex(s, _nmd_regrxb[i], string) == s)
			return (NMD_X86_REG)(NMD_X86_REG_R8B + i);
	}
	return NMD_X86_REG_NONE;
}

NMD_ASSEMBLY_API NMD_X86_REG _nmd_parse_regrxw(const char** string)
{
	const char* s = *string;
	size_t i;
	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_regrxw); i++)
	{
		if (_nmd_strstr_ex(s, _nmd_regrxw[i], string) == s)
			return (NMD_X86_REG)(NMD_X86_REG_R8W + i);
	}
	return NMD_X86_REG_NONE;
}

NMD_ASSEMBLY_API NMD_X86_REG _nmd_parse_regrxd(const char** string)
{
	const char* s = *string;
	size_t i;
	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_regrxd); i++)
	{
		if (_nmd_strstr_ex(s, _nmd_regrxd[i], string) == s)
			return (NMD_X86_REG)(NMD_X86_REG_R8D + i);
	}
	return NMD_X86_REG_NONE;
}

NMD_ASSEMBLY_API NMD_X86_REG _nmd_parse_regrx(const char** string)
{
	const char* s = *string;
	size_t i;
	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_regrx); i++)
	{
		if (_nmd_strstr_ex(s, _nmd_regrx[i], string) == s)
			return (NMD_X86_REG)(NMD_X86_REG_R8 + i);
	}
	return NMD_X86_REG_NONE;
}

/* Parses a register */
NMD_ASSEMBLY_API NMD_X86_REG _nmd_parse_reg(const char** string)
{
    NMD_X86_REG reg;
    if(!(reg = _nmd_parse_reg8(string)) && !(reg = _nmd_parse_reg16(string)) && !(reg = _nmd_parse_reg32(string)) && !(reg = _nmd_parse_reg64(string)) &&
    !(reg = _nmd_parse_regrxb(string)) && !(reg = _nmd_parse_regrxw(string)) && !(reg = _nmd_parse_regrxd(string)) && !(reg = _nmd_parse_regrx(string)))
    { }
    
    return reg;
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
	/* Check for pointer size */
	const char* s = *string;
	size_t num_bytes;
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

	/* Check for the "ptr" keyword. It should only be present if a pointer size was specified */
	if (num_bytes > 0)
	{
		s += num_bytes >= 4 ? 5 : 4;

		/* " ptr" */
		if (s[0] == ' ' && s[1] == 'p' && s[2] == 't' && s[3] == 'r')
			s += 4;

		if (s[0] == ' ')
			s++;
		else if (s[0] != '[')
			return false;
	}

	/* Check for a segment register */
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

	/* Check for the actual memory operand expression. If this check fails, this is not a memory operand */
	if (s[0] == '[')
		s++;
	else
		return false;
	
	/* 
	Parse the memory operand expression.
	Even though formally there's no support for subtraction, if the expression has such operation between
	two numeric operands, it'll be resolved to a single number(the same applies for the other operations).
	*/
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
		/* 
		Check for the base/index register. The previous math operation must not be subtration
		nor multiplication because these are not valid for registers(only addition is).
		*/
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
		if (!parsed_element && (num_digits = _nmd_parse_number(s, &num)))
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
			break;
		else
			return false;

		s++;
	}

	*string = s + 1;
	return true;
}

NMD_ASSEMBLY_API size_t _nmd_assemble_mem_reg(uint8_t* buffer, nmd_x86_memory_operand* mem, uint8_t opcode, uint8_t modrm_reg)
{
	size_t offset = 0;
	
	/* Assemble segment register if required */
	if (mem->segment && mem->segment != ((mem->base == NMD_X86_REG_ESP || mem->index == NMD_X86_REG_ESP) ? NMD_X86_REG_SS : NMD_X86_REG_DS))
		buffer[offset++] = _nmd_encode_segment_reg((NMD_X86_REG)mem->segment);

	buffer[offset++] = opcode;

	nmd_x86_modrm modrm;
	modrm.fields.reg = modrm_reg;
	modrm.fields.mod = 0;

	if (mem->index != NMD_X86_REG_NONE && mem->base != NMD_X86_REG_NONE)
	{
		modrm.fields.rm = 0b100;
		nmd_x86_sib sib;
		sib.fields.scale = (uint8_t)_nmd_get_bit_index(mem->scale);
		sib.fields.base = mem->base - NMD_X86_REG_EAX;
		sib.fields.index = mem->index - NMD_X86_REG_EAX;

		const size_t next_offset = offset;
		if (mem->disp != 0)
		{
			if (mem->disp >= -128 && mem->disp <= 127)
			{
				modrm.fields.mod = 1;
				*(int8_t*)(buffer + offset + 2) = (int8_t)mem->disp;
				offset++;
			}
			else
			{
				modrm.fields.mod = 2;
				*(int32_t*)(buffer + offset + 2) = (int32_t)mem->disp;
				offset += 4;
			}
		}

		buffer[next_offset] = modrm.modrm;
		buffer[next_offset + 1] = sib.sib;
		offset += 2;

		return offset;
	}
	else if (mem->base != NMD_X86_REG_NONE)
	{
		modrm.fields.rm = mem->base - NMD_X86_REG_EAX;
		const size_t next_offset = offset;
		if (mem->disp != 0)
		{
			if (mem->disp >= -128 && mem->disp <= 127)
			{
				modrm.fields.mod = 1;
				*(int8_t*)(buffer + offset + 1) = (int8_t)mem->disp;
				offset++;
			}
			else
			{
				modrm.fields.mod = 2;
				*(int32_t*)(buffer + offset + 1) = (int32_t)mem->disp;
				offset += 4;
			}
		}
		buffer[next_offset] = modrm.modrm;
		offset++;
	}
	else
	{
		modrm.fields.rm = 0b101;
		buffer[offset++] = modrm.modrm;
		*(int32_t*)(buffer + offset) = (int32_t)mem->disp;
		offset += 4;
	}

	return offset;
}

NMD_ASSEMBLY_API size_t _nmd_assemble_single(_nmd_assemble_info* ai)
{
    const char* s;
    int64_t num;
    size_t num_digits;
    NMD_X86_REG reg, reg2;
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
		if (_nmd_strstr(ai->s, "mov "))
		{
			const char* s = ai->s + 4;
			if ((reg = _nmd_parse_regrxb((const char**)&s)))
			{
				ai->b[0] = 0x41;
				ai->b[1] = 0xb0 + (reg - NMD_X86_REG_R8B);

				if (*s++ != ',')
					return 0;

				if ((num_digits = _nmd_parse_number(s, &num)))
				{
					ai->b[2] = (uint8_t)num;
					return 3;
				}
			}
		}
		else if (_nmd_strstr(ai->s, "push ") == ai->s || _nmd_strstr(ai->s, "pop ") == ai->s)
		{
			const bool is_push = ai->s[1] == 'u';
			s = ai->s + (is_push ? 5 : 4);
			if (((reg = _nmd_parse_reg64(&s))) && !*s)
			{
				ai->b[0] = (is_push ? 0x50 : 0x58) + reg % 8;
				return 1;
			}
			else if ((reg = _nmd_parse_regrxw(&s)) && !*s)
			{
				ai->b[0] = 0x66;
				ai->b[1] = 0x41;
				ai->b[2] = (is_push ? 0x50 : 0x58) + reg % 8;
				return 3;
			}
			else if (((reg = _nmd_parse_regrx(&s))) && !*s)
			{
				ai->b[0] = 0x41;
				ai->b[1] = (is_push ? 0x50 : 0x58) + reg % 8;
				return 2;
			}
		}
		else if (_nmd_strstr(ai->s, "mov ") == ai->s)
		{
			ai->s += 4;
			if ((reg = _nmd_parse_reg8((const char**)&ai->s)))
			{
				ai->b[0] = 0xb0 + reg % 8;

				if (*ai->s++ != ',')
					return 0;

				if ((num_digits = _nmd_parse_number(ai->s, &num)))
				{
					ai->b[1] = (uint8_t)num;
					return 2;
				}
			}
		}
		else if (_nmd_strcmp(ai->s, "xchg r8,rax") || _nmd_strcmp(ai->s, "xchg rax,r8"))
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
		if (_nmd_strstr(ai->s, "inc ") || _nmd_strstr(ai->s, "dec "))
		{
			const bool is_inc = ai->s[0] == 'i';
			s = ai->s + 4;
			int offset = 0;
			if (((reg = _nmd_parse_reg32(&s))) && !*s)
			{
				if (ai->mode == NMD_X86_MODE_16)
					ai->b[offset++] = 0x66;
			}
			else if (((reg = _nmd_parse_reg16(&s))) && !*s)
			{
				if (ai->mode == NMD_X86_MODE_32)
					ai->b[offset++] = 0x66;
			}
			else
				return 0;

			ai->b[offset++] = (is_inc ? 0x40 : 0x48) + reg % 8;
			return offset;
		}
		else if (_nmd_strstr(ai->s, "push ") == ai->s || _nmd_strstr(ai->s, "pop ") == ai->s)
		{
			const bool is_push = ai->s[1] == 'u';
			s = ai->s + (is_push ? 5 : 4);
			if (((reg = _nmd_parse_reg32(&s))) && !*s)
			{
				if (ai->mode == NMD_X86_MODE_16)
				{
					ai->b[0] = 0x66;
					ai->b[1] = (is_push ? 0x50 : 0x58) + reg % 8;
					return 2;
				}
				else
				{
					ai->b[0] = (is_push ? 0x50 : 0x58) + reg % 8;
					return 1;
				}
			}
		}
		else if (_nmd_strcmp(ai->s, "pushad"))
		{
			if (ai->mode == NMD_X86_MODE_16)
			{
				ai->b[0] = 0x66;
				ai->b[1] = 0x60;
				return 2;
			}
			else
			{
				ai->b[0] = 0x60;
				return 1;
			}
		}
		else if (_nmd_strcmp(ai->s, "pusha"))
		{
			if (ai->mode == NMD_X86_MODE_16)
			{
				ai->b[0] = 0x60;
				return 1;
			}
			else
			{
				ai->b[0] = 0x66;
				ai->b[1] = 0x60;
				return 2;
			}
		}
		else if (_nmd_strcmp(ai->s, "popad"))
		{
			if (ai->mode == NMD_X86_MODE_16)
			{
				ai->b[0] = 0x66;
				ai->b[1] = 0x61;
				return 2;
			}
			else
			{
				ai->b[0] = 0x61;
				return 1;
			}
		}
		else if (_nmd_strcmp(ai->s, "popa"))
		{
			if (ai->mode == NMD_X86_MODE_16)
			{
				ai->b[0] = 0x61;
				return 1;
			}
			else
			{
				ai->b[0] = 0x66;
				ai->b[1] = 0x61;
				return 2;
			}
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
		{ "clc",     0xf8 },
		{ "sahf",    0x9e },
		{ "lahf",    0x9f },
		{ "into",    0xce },
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
	
	/* Parse 'add', 'adc', 'and', 'xor', 'or', 'sbb', 'sub' and 'cmp' . Opcodes in first "4 rows"/[80, 83] */
	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_op1_opcode_map_mnemonics); i++)
	{
		if (_nmd_strstr(ai->s, _nmd_op1_opcode_map_mnemonics[i]) == ai->s)
		{
			const uint8_t base_opcode = (i % 4) * 0x10 + (i >= 4 ? 8 : 0);
			ai->s += 4;
            
			nmd_x86_memory_operand memory_operand;
			size_t pointer_size;
			if (_nmd_parse_memory_operand((const char**)&ai->s, &memory_operand, &pointer_size)) /* Colum 00,01,08,09 */
			{                
				if (*ai->s++ != ',' || !(reg = _nmd_parse_reg((const char**)&ai->s)))
					return 0;
                
				return _nmd_assemble_mem_reg(ai->b, &memory_operand, base_opcode, reg % 8);

				/*
				size_t offset = 0;
				if (memory_operand.segment && memory_operand.segment != ((memory_operand.base == NMD_X86_REG_ESP || memory_operand.index == NMD_X86_REG_ESP) ? NMD_X86_REG_SS : NMD_X86_REG_DS))
					ai->b[offset++] = _nmd_encode_segment_reg((NMD_X86_REG)memory_operand.segment);
				
				if (pointer_size == 1)
				{
					ai->b[offset++] = base_opcode + 0;
				
					if (*ai->s++ != ',')
						return 0;
				
					NMD_X86_REG reg = _nmd_parse_reg(&ai->s);
				
					nmd_x86_modrm modrm;
					modrm.fields.mod = 0;
					modrm.fields.reg = (reg - 1) % 8;
					modrm.fields.rm = (memory_operand.base - 1) % 8;
				
					ai->b[offset++] = modrm.modrm;
				
					return offset;
				}
				*/
			}
            else if (_nmd_strstr_ex(ai->s, "al,", &s) == ai->s && (num_digits = _nmd_parse_number(s, &num)) && *(s + num_digits) == '\0') /* column 04,0C */
			{
				if (num < -0x80 || num > 0xff)
					return 0;

				ai->b[0] = base_opcode + 4;
				ai->b[1] = (int8_t)num;
				return 2;
			}
			else if ((_nmd_strstr_ex(ai->s, "eax,", &s) == ai->s || _nmd_strstr_ex(ai->s, "ax,", &s) == ai->s || _nmd_strstr_ex(ai->s, "rax,", &s) == ai->s) && _nmd_parse_number(s, &num)) /* column 05,0D */
			{
				const bool is_eax = ai->s[0] == 'e';
				const bool is_ax = ai->s[1] == 'x';

				if (is_eax)
				{
					if (num < -(int64_t)0x80000000 || num > 0xffffffff)
						return 0;

					ai->b[0] = base_opcode + 5;
					*(int32_t*)(ai->b + 1) = (int32_t)num;
					return 5;
				}
				else if (!is_eax)
				{
					if (ai->mode != NMD_X86_MODE_64 && !is_ax)
						return 0;

					ai->b[0] = is_ax ? 0x66 : 0x48;
					ai->b[1] = base_opcode + 5;

					if (is_ax)
					{
						if (num < -0x8000 || num > 0xffff)
							return 0;

						*(int16_t*)(ai->b + 2) = (int16_t)num;
						return 4;
					}
					else
					{
						if (num < -(int64_t)0x80000000 || num > 0xffffffff)
							return 0;

						*(int32_t*)(ai->b + 2) = (int32_t)num;
						return 6;
					}
				}
			}
            else if ((reg = _nmd_parse_reg((const char**)&ai->s)) && *ai->s++ == ',') /* column 00-04,08-0B */
            {
                if(_nmd_parse_memory_operand((const char**)&ai->s, &memory_operand, &pointer_size)) /* column 02,03,0A,0B */
                {
                    return 0;
                }
                else /* 00,01,08,09 */
                {
                    if(!(reg2 = _nmd_parse_reg((const char**)&ai->s)))
                        return 0;
                    
                    ai->b[0] = base_opcode;
                    
                    /* mod = 0b11, reg = reg2, rm = reg */
                    ai->b[1] = 0b11000000 | ((reg2 % 8) << 3) | (reg % 8);
                    
                    return 2;
                }
            }

			return 0;
		}
	}

	if (_nmd_strstr(ai->s, "jmp ") == ai->s)
	{
		if (!(num_digits = _nmd_parse_number(ai->s + 4, &num)) && ai->s[4 + num_digits] == '\0')
			return 0;

		size_t offset = 0;
		if (ai->mode == NMD_X86_MODE_16)
			ai->b[offset++] = 0x66;
		ai->b[offset++] = 0xe9;
		const int64_t size = (int64_t)offset + 4;
		*(uint32_t*)(ai->b + offset) = (uint32_t)((ai->runtime_address == NMD_X86_INVALID_RUNTIME_ADDRESS) ? (num - size) : (ai->runtime_address + size + num));
		return size;
	}
	else if (ai->s[0] == 'j')
	{
		char* s = ai->s;
		while (true)
		{
			if (*s == ' ')
			{
				*s = '\0';
				break;
			}
			else if (*s == '\0')
				return 0;

			s++;
		}

		for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_condition_suffixes); i++)
		{
			if (_nmd_strcmp(ai->s + 1, _nmd_condition_suffixes[i]))
			{
				if (!(num_digits = _nmd_parse_number(s + 1, &num)))
					return 0;

				const int64_t delta = (ai->runtime_address == NMD_X86_INVALID_RUNTIME_ADDRESS ? num : num - ai->runtime_address);
				if (delta >= -(1 << 7) + 2 && delta <= (1 << 7) - 1 + 2)
				{
					ai->b[0] = 0x70 + (uint8_t)i;
					*(int8_t*)(ai->b + 1) = (int8_t)(delta - 2);
					return 2;
				}
				else if (delta >= -((int64_t)1 << 31) + 6 && delta <= ((int64_t)1 << 31) - 1 + 6)
				{
					size_t offset = 0;
					if (ai->mode == NMD_X86_MODE_16)
						ai->b[offset++] = 0x66;

					ai->b[offset++] = 0x0f;
					ai->b[offset++] = 0x80 + (uint8_t)i;
					*(int32_t*)(ai->b + offset) = (int32_t)(delta - (offset+4));
					return offset + 4;
				}
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

				const size_t next_offset = offset;
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

				ai->b[next_offset] = modrm.modrm;
				ai->b[next_offset + 1] = sib.sib;
				offset += 2;

				return offset;
			}
			else if (memory_operand.base != NMD_X86_REG_NONE)
			{
				modrm.fields.rm = memory_operand.base - NMD_X86_REG_EAX;
				const size_t next_offset = offset;
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
				ai->b[next_offset] = modrm.modrm;
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
	else if (_nmd_strstr(ai->s, "call ") == ai->s)
	{
		if ((num_digits = _nmd_parse_number(ai->s + 5, &num)))
		{
			ai->b[0] = 0xe8;
			if(ai->runtime_address == NMD_X86_INVALID_RUNTIME_ADDRESS)
				*(int32_t*)(ai->b + 1) = (int32_t)(num - 5);
			else
				*(int32_t*)(ai->b + 1) = (int32_t)(num - (ai->runtime_address + 5));
			return 5;
		}
	}
	else if (_nmd_strstr(ai->s, "push ") == ai->s || _nmd_strstr(ai->s, "pop ") == ai->s)
	{
		const bool is_push = ai->s[1] == 'u';
		ai->s += is_push ? 5 : 4;
		NMD_X86_REG reg;
		if ((reg = _nmd_parse_reg16((const char**)&ai->s)))
		{
			if (ai->mode == NMD_X86_MODE_16)
			{
				ai->b[0] = (is_push ? 0x50 : 0x58) + reg % 8;
				return 1;
			}
			else
			{
				ai->b[0] = 0x66;
				ai->b[1] = (is_push ? 0x50 : 0x58) + reg % 8;
				return 2;
			}
		}
		else if (is_push && (num_digits = _nmd_parse_number(ai->s, &num)) && ai->s[num_digits] == '\0')
		{
			if (num >= -(1 << 7) && num <= (1 << 7) - 1)
			{
				ai->b[0] = 0x6a;
				*(int8_t*)(ai->b + 1) = (int8_t)num;
				return 2;
			}
			else
			{
				size_t offset = 0;
				if (ai->mode == NMD_X86_MODE_16)
					ai->b[offset++] = 0x66;
				ai->b[offset++] = 0x68;
				*(int32_t*)(ai->b + offset) = (int32_t)num;
				return offset + 4;
			}
		}
	}
	else if (_nmd_strstr(ai->s, "mov ") == ai->s)
	{
		ai->s += 4;
		const NMD_X86_REG reg = _nmd_parse_reg8((const char**)&ai->s);
		if (reg)
		{
			ai->b[0] = 0xb0 + (reg - NMD_X86_REG_AL);
			
			if (*ai->s++ != ',')
				return 0;
            
			if ((num_digits = _nmd_parse_number(ai->s, &num)))
			{
				ai->b[1] = (uint8_t)num;
				return 2;
			}
		}
	}
	else if (_nmd_strstr(ai->s, "ret ") == ai->s || _nmd_strstr(ai->s, "retf ") == ai->s)
	{
		const bool is_far = ai->s[3] == 'f';
		ai->s += is_far ? 5 : 4;
        
		if ((num_digits = _nmd_parse_number(ai->s, &num)) && ai->s[num_digits] == '\0')
		{
			ai->b[0] = is_far ? 0xca : 0xc2;
			*(uint16_t*)(ai->b + 1) = (uint16_t)num;
			return 3;
		}
	}
	else if (_nmd_strstr(ai->s, "emit ") == ai->s)
	{
		size_t offset = 5;
		while ((num_digits = _nmd_parse_number(ai->s + offset, &num)))
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
	else if (_nmd_strcmp(ai->s, "cwde"))
	{
		int offset = 0;
		if (ai->mode == NMD_X86_MODE_16)
			ai->b[offset++] = 0x66;
		ai->b[offset++] = 0x98;
		return offset;
	}
	else if (_nmd_strstr(ai->s, "int ") == ai->s)
	{
		ai->s += 4;
		if ((num_digits = _nmd_parse_number(ai->s, &num)) && ai->s[num_digits] == '\0')
		{
			ai->b[0] = 0xcd;
			ai->b[1] = (uint8_t)num;
			return 2;
		}
	}
	else if (_nmd_strcmp(ai->s, "cbw"))
	{
		int offset = 0;
		if (ai->mode != NMD_X86_MODE_16)
			ai->b[offset++] = 0x66;
		ai->b[offset++] = 0x98;
		return offset;
	}
	else if (_nmd_strcmp(ai->s, "cdq"))
	{
		int offset = 0;
		if (ai->mode == NMD_X86_MODE_16)
			ai->b[offset++] = 0x66;
		ai->b[offset++] = 0x99;
		return offset;
	}
	else if (_nmd_strcmp(ai->s, "cwd"))
	{
		int offset = 0;
		if (ai->mode != NMD_X86_MODE_16)
			ai->b[offset++] = 0x66;
		ai->b[offset++] = 0x99;
		return offset;
	}

	return 0;
}

/*
Assembles one or more instructions from a string. Returns the number of bytes written to the buffer on success, zero otherwise. Instructions can be separated using the '\n'(new line) character.
Parameters:
 - string          [in]         A pointer to a string that represents one or more instructions in assembly language.
 - buffer          [out]        A pointer to a buffer that receives the encoded instructions.
 - buffer_size     [in]         The size of the buffer in bytes.
 - runtime_address [in]         The instruction's runtime address. You may use 'NMD_X86_INVALID_RUNTIME_ADDRESS'.
 - mode            [in]         The architecture mode. 'NMD_X86_MODE_32', 'NMD_X86_MODE_64' or 'NMD_X86_MODE_16'.
 - count           [in/out/opt] A pointer to a variable that on input is the maximum number of instructions that can be parsed, and on output the number of instructions parsed. This parameter may be null.
*/
NMD_ASSEMBLY_API size_t nmd_x86_assemble(const char* string, void* buffer, size_t buffer_size, uint64_t runtime_address, NMD_X86_MODE mode, size_t* count)
{
	if (!*string)
		return 0;

	const uint8_t* const buffer_end = (uint8_t*)buffer + buffer_size;
	const size_t num_max_instructions = count ? *count : (size_t)(-1);
	size_t num_instructions = 0;
	char parsed_string[256];
	uint8_t temp_buffer[15]; /* The assembling takes place on this buffer instead of the user's buffer because the assembler doesn't check the buffer size. If it assembled directly to the user's buffer it could access bad memory */
	uint8_t* b = (uint8_t*)buffer;
	size_t remaining_size;
	size_t length = 0;

	_nmd_assemble_info ai;
	ai.s = parsed_string;
	ai.mode = mode;
	ai.runtime_address = runtime_address;
	ai.b = temp_buffer;
	
	/* Parse the first character of the string because the loop just ahead accesses `string-1` which could be bad memory if we didn't do this */
	if (*string == ' ')
		string++;
	else
	{
		parsed_string[0] = *string++;
		length++;
	}

	while (*string && num_instructions < num_max_instructions)
	{
		remaining_size = buffer_end - b;
				
		/* Copy 'string' to 'parsed_string' converting it to lowercase and removing unwanted spaces. If the instruction separator character '\n' is found, stop. */
		char c = *string; /* Current character */
		char prev_c = *(string - 1); /* Previous character */
		while (c && c != '\n')
		{
			/* Ignore(skip) the current character if it's a space and the previous character is one of the following: ' ', '+', '*', '[' */
			if (c == ' ' && (prev_c == ' ' || prev_c == '+' || prev_c == '*' || prev_c == '['))
			{
				c = *++string;
				continue;
			}

			/* Append character */
			parsed_string[length++] = _NMD_TOLOWER(c);

			/* The maximum length is 255 */
			if (length >= 256)
				return 0;

			/* Set previous character */
			prev_c = c;

			/* Get the next character */
			c = *++string;
		}

		/* This check is only ever true if *string == '\n', that is the instruction separator character */
		if (*string /* == '\n' */)
			string++; /* Go forward by one character so 'string' points to the next instruction */

		/* If the last character is a space, remove it. */
		if (length > 0 && parsed_string[length - 1] == ' ')
			length--;

		/* After all of the string manipulation, place the null character */
		parsed_string[length] = '\0';

		/* Try to assemble the instruction */
		const size_t num_bytes = _nmd_assemble_single(&ai);
		if (num_bytes == 0 || num_bytes > remaining_size)
			return 0;

		/* Copy bytes from 'temp_buffer' to 'buffer' */
		size_t i = 0;
		for (; i < num_bytes; i++)
			b[i] = temp_buffer[i];
		b += num_bytes;

		num_instructions++;

		/* Reset length in case there's another instruction */
		length = 0;
	}

	if (count)
		*count = num_instructions;

	/* Return the number of bytes written to the buffer */
	return (size_t)((ptrdiff_t)b - (ptrdiff_t)buffer);
}
