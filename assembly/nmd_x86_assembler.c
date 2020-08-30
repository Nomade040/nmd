#include "nmd_common.h"

typedef struct _nmd_assemble_info
{
	const char* s; /* string */
	uint8_t* b; /* buffer */
	NMD_X86_MODE mode;
	uint64_t runtimeAddress;
} _nmd_assemble_info;

size_t _nmd_assemble_reg(_nmd_assemble_info* ai, uint8_t baseByte)
{
	uint8_t i = 0;
	if (ai->mode == NMD_X86_MODE_64)
	{
		for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_reg64); i++)
		{
			if (_nmd_strcmp(ai->s, _nmd_reg64[i]))
			{
				ai->b[0] = baseByte + i;
				return 1;
			}
		}

		for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_regrx); i++)
		{
			if (_nmd_strcmp(ai->s, _nmd_regrx[i]))
			{
				ai->b[0] = 0x41;
				ai->b[1] = baseByte + i;
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
				ai->b[0] = baseByte + i;
				return 1;
			}
		}
	}	

	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_reg16); i++)
	{
		if (_nmd_strcmp(ai->s, _nmd_reg16[i]))
		{
			ai->b[0] = 0x66;
			ai->b[1] = baseByte + i;
			return 2;
		}
	}

	return 0;
}

uint8_t _nmd_encode_segment_reg(NMD_X86_REG segmentReg)
{
	switch (segmentReg)
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

enum _NMD_NUMBER_BASE
{
	_NMD_NUMBER_BASE_NONE        = 0,
	_NMD_NUMBER_BASE_DECIMAL     = 10,
	_NMD_NUMBER_BASE_HEXADECIMAL = 16,
	_NMD_NUMBER_BASE_BINARY      = 2
};

bool _nmd_parse_number(const char* string, int64_t* pNum, size_t* pNumDigits)
{
	if (*string == '\0')
		return false;

	/* Assume decimal base. */
	uint8_t base = _NMD_NUMBER_BASE_DECIMAL;
	size_t i;
	const char* s = string;
	bool isNegative = false;

	if (s[0] == '-')
	{
		isNegative = true;
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

	size_t numDigits = i;

	int64_t num = 0;
	for (i = 0; i < numDigits; i++)
	{
		const char c = s[i];
		num += (c <= '9') ? (c - '0') : (10 + c - 'a');
		if (i < numDigits - 1)
		{
			/* Return false if number is greater than 2^64-1 */
			if ( numDigits > 16 && i >= 15)
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
		numDigits = (size_t)((ptrdiff_t)(s + i) - (ptrdiff_t)string);
	
	if (isNegative)
		num *= -1;

	*pNum = num;

	if(pNumDigits)
		*pNumDigits = numDigits;

	return true;
}

size_t _nmd_append_prefix_by_reg_size(uint8_t* b, const char* s, size_t* numPrefixes, size_t* index)
{
	size_t i;
	
	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_reg32); i++)
	{
		if (_nmd_strcmp(s, _nmd_reg32[i]))
		{
			*numPrefixes = 0;
			*index = i;
			return 4;
		}
	}

	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_reg8); i++)
	{
		if (_nmd_strcmp(s, _nmd_reg8[i]))
		{
			*numPrefixes = 0;
			*index = i;
			return 1;
		}
	}

	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_reg16); i++)
	{
		if (_nmd_strcmp(s, _nmd_reg16[i]))
		{
			b[0] = 0x66;
			*numPrefixes = 1;
			*index = i;
			return 2;
		}
	}	

	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_reg64); i++)
	{
		if (_nmd_strcmp(s, _nmd_reg64[i]))
		{
			b[0] = 0x48;
			*numPrefixes = 1;
			*index = i;
			return 8;
		}
	}

	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_regrx); i++)
	{
		if (_nmd_strcmp(s, _nmd_regrx[i]))
		{
			b[0] = 0x49;
			*numPrefixes = 1;
			*index = i;
			return 8;
		}
	}

	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_regrxd); i++)
	{
		if (_nmd_strcmp(s, _nmd_regrxd[i]))
		{
			b[0] = 0x41;
			*numPrefixes = 1;
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
			*numPrefixes = 2;
			*index = i;
			return 2;
		}
	}

	for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_regrxb); i++)
	{
		if (_nmd_strcmp(s, _nmd_regrxb[i]))
		{
			b[0] = 0x41;
			*numPrefixes = 1;
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
bool _nmd_parse_memory_operand(const char** string, nmd_x86_memory_operand* operand, size_t* size)
{
	const char* s = *string;
	size_t numBytes = 0;
	if (_nmd_strstr(s, "byte") == s)
		numBytes = 1;
	else if (_nmd_strstr(s, "word") == s)
		numBytes = 2;
	else if (_nmd_strstr(s, "dword") == s)
		numBytes = 4;
	else if (_nmd_strstr(s, "qword") == s)
		numBytes = 8;
	else
		numBytes = 0;
	*size = numBytes;

	if (numBytes > 0)
	{
		s += numBytes >= 4 ? 5 : 4;

		if (s[0] == ' ' && s[1] == 'p' && s[2] == 't' && s[3] == 'r')
			s += 4;

		if (s[0] == ' ')
			s++;
		else if (s[0] != '[')
			return false;
	}

	size_t i = 0;
	operand->segment = NMD_X86_REG_NONE;
	for (; i < _NMD_NUM_ELEMENTS(_nmd_segmentReg); i++)
	{
		if (_nmd_strstr(s, _nmd_segmentReg[i]) == s)
		{
			if (s[2] != ':')
				return false;

			s += 3;
			operand->segment = (NMD_X86_REG)(NMD_X86_REG_ES + i);
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
	bool isRegister = false;
	while (true)
	{
		bool parsedElement = false;
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
						operand->index = (NMD_X86_REG)(NMD_X86_REG_EAX + i);
						operand->scale = 1;
						add = false;
					}
					else
						operand->base = (NMD_X86_REG)(NMD_X86_REG_EAX + i);
					parsedElement = true;
					isRegister = true;
					break;
				}
			}
		}

		int64_t num;
		size_t numDigits;
		if (!parsedElement && _nmd_parse_number(s, &num, &numDigits))
		{
			s += numDigits;

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
				if (!isRegister || (num != 1 && num != 2 && num != 4 && num != 8))
					return false;

				operand->scale = num;
			}
			else
				operand->disp = num;

			parsedElement = true;
		}

		if (!parsedElement)
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

size_t _nmd_assemble_single(_nmd_assemble_info* ai)
{
	size_t i = 0;

	/* Parse prefixes */
	bool lockPrefix = false, repeatPrefix = false, repeatZeroPrefix = false, repeatNotZeroPrefix = false;
	if (_nmd_strstr(ai->s, "lock ") == ai->s)
		lockPrefix = true, ai->s += 5;
	else if (_nmd_strstr(ai->s, "rep ") == ai->s)
		repeatPrefix = true, ai->s += 4;
	else if (_nmd_strstr(ai->s, "repe ") == ai->s || _nmd_strstr(ai->s, "repz ") == ai->s)
		repeatZeroPrefix = true, ai->s += 5;
	else if (_nmd_strstr(ai->s, "repne ") == ai->s || _nmd_strstr(ai->s, "repnz ") == ai->s)
		repeatNotZeroPrefix = true, ai->s += 6;
	
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

	typedef struct NMD_StringBytePair { const char* s; uint8_t b; } NMD_StringBytePair;

	const NMD_StringBytePair op1SingleByte[] = {
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
	for (i = 0; i < _NMD_NUM_ELEMENTS(op1SingleByte); i++)
	{
		if (_nmd_strcmp(ai->s, op1SingleByte[i].s))
		{
			ai->b[0] = op1SingleByte[i].b;
			return 1;
		}
	}

	const NMD_StringBytePair op2SingleByte[] = {
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
	for (i = 0; i < _NMD_NUM_ELEMENTS(op2SingleByte); i++)
	{
		if (_nmd_strcmp(ai->s, op2SingleByte[i].s))
		{
			ai->b[0] = 0x0f;
			ai->b[1] = op2SingleByte[i].b;
			return 2;
		}
	}
	
	if (ai->s[0] == 'j')
	{
		const char* s = 0;
		for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_conditionSuffixes); i++)
		{
			if (_nmd_strstr_ex(ai->s + 1, _nmd_conditionSuffixes[i], &s) == ai->s + 1)
			{
				if (s[0] != ' ')
					return 0;


				int64_t num;
				size_t numDigits;
				if (!_nmd_parse_number(s + 1, &num, &numDigits))
					return 0;

				const int64_t delta = num - ai->runtimeAddress;
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
		nmd_x86_memory_operand memoryOperand;
		size_t size;
		if (_nmd_parse_memory_operand(&tmp, &memoryOperand, &size))
		{
			size_t offset = 0;
			if (memoryOperand.segment && memoryOperand.segment != ((memoryOperand.base == NMD_X86_REG_ESP || memoryOperand.index == NMD_X86_REG_ESP) ? NMD_X86_REG_SS : NMD_X86_REG_DS))
				ai->b[offset++] = _nmd_encode_segment_reg((NMD_X86_REG)memoryOperand.segment);

			ai->b[offset++] = size == 1 ? 0xfe : 0xff;

			nmd_x86_modrm modrm;
			modrm.fields.reg = ai->s[0] == 'i' ? 0 : 8;
			modrm.fields.mod = 0;

			if (memoryOperand.index != NMD_X86_REG_NONE && memoryOperand.base != NMD_X86_REG_NONE)
			{
				modrm.fields.rm = 0b100;
				nmd_x86_sib sib;
				sib.fields.scale = _nmd_get_bit_index(memoryOperand.scale);
				sib.fields.base = memoryOperand.base - NMD_X86_REG_EAX;
				sib.fields.index = memoryOperand.index - NMD_X86_REG_EAX;

				const size_t nextOffset = offset;
				if (memoryOperand.disp != 0)
				{
					if (memoryOperand.disp >= -128 && memoryOperand.disp <= 127)
					{
						modrm.fields.mod = 1;
						*(int8_t*)(ai->b + offset + 2) = (int8_t)memoryOperand.disp;
						offset++;
					}
					else
					{
						modrm.fields.mod = 2;
						*(int32_t*)(ai->b + offset + 2) = (int32_t)memoryOperand.disp;
						offset += 4;
					}
				}

				ai->b[nextOffset] = modrm.modrm;
				ai->b[nextOffset + 1] = sib.sib;
				offset += 2;

				return offset;
			}
			else if (memoryOperand.base != NMD_X86_REG_NONE)
			{
				modrm.fields.rm = memoryOperand.base - NMD_X86_REG_EAX;
				const size_t nextOffset = offset;
				if (memoryOperand.disp != 0)
				{
					if (memoryOperand.disp >= -128 && memoryOperand.disp <= 127)
					{
						modrm.fields.mod = 1;
						*(int8_t*)(ai->b + offset + 1) = (int8_t)memoryOperand.disp;
						offset++;
					}
					else
					{
						modrm.fields.mod = 2;
						*(int32_t*)(ai->b + offset + 1) = (int32_t)memoryOperand.disp;
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
				*(int32_t*)(ai->b + offset) = (int32_t)memoryOperand.disp;
				offset += 4;
			}

			return offset;
		}

		size_t numPrefixes, index;
		size = _nmd_append_prefix_by_reg_size(ai->b, ai->s + 4, &numPrefixes, &index);
		if (size > 0)
		{
			if (ai->mode == NMD_X86_MODE_64)
			{
				ai->b[numPrefixes + 0] = size == 1 ? 0xfe : 0xff;
				ai->b[numPrefixes + 1] = 0xc0 + (ai->s[0] == 'i' ? 0 : 8) + (uint8_t)index;
				return numPrefixes + 2;
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
					ai->b[numPrefixes + 0] = (ai->s[0] == 'i' ? 0x40 : 0x48) + (uint8_t)index;
					return numPrefixes + 1;
				}
			}
		}
	}
	else if (_nmd_strstr(ai->s, "push ") == ai->s)
	{
		size_t numDigits = 0;
		int64_t num = 0;
		if (_nmd_parse_number(ai->s + 5, &num, &numDigits))
		{
			if (*(ai->s + numDigits) != '\0' || !(num >= -(1 << 31) && num <= ((int64_t)1 << 31) - 1))
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
		size_t numDigits = 0;
		size_t offset = 5;
		while (_nmd_parse_number(ai->s + offset, &num, &numDigits))
		{
			if (num < 0 || num > 0xff)
				return 0;

			ai->b[i++] = (uint8_t)num;

			offset += numDigits;
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
Assembles an instruction from a string. Returns the number of bytes written to the buffer on success, zero otherwise. Instructions can be separated using either the ';' or '\n' character.
Parameters:
 - string         [in]         A pointer to a string that represents one or more instructions in assembly language.
 - buffer         [out]        A pointer to a buffer that receives the encoded instructions.
 - bufferSize     [in]         The size of the buffer in bytes.
 - runtimeAddress [in]         The instruction's runtime address. You may use 'NMD_X86_INVALID_RUNTIME_ADDRESS'.
 - mode           [in]         The architecture mode. 'NMD_X86_MODE_32', 'NMD_X86_MODE_64' or 'NMD_X86_MODE_16'.
 - count          [in/out/opt] A pointer to a variable that on input is the maximum number of instructions that can be parsed(or zero for unlimited instructions), and on output is the number of instructions parsed. This parameter may be 0(zero).
*/
size_t nmd_x86_assemble(const char* string, void* buffer, size_t bufferSize, uint64_t runtimeAddress, NMD_X86_MODE mode, size_t* count)
{
	char parsedString[128];
	const uint8_t* const bufferEnd = (uint8_t*)buffer + bufferSize;
	uint8_t* b = (uint8_t*)buffer;
	size_t remainingSize;

	uint8_t tempBuffer[NMD_X86_MAXIMUM_INSTRUCTION_LENGTH];

	_nmd_assemble_info ai;
	ai.s = parsedString;
	ai.mode = mode;
	ai.runtimeAddress = runtimeAddress;
	ai.b = tempBuffer;

	size_t numInstructions = 0;
	const size_t numMaxInstructions = (count && *count != 0) ? *count : (size_t)(-1);

	while (string[0] != '\0' && numInstructions < numMaxInstructions)
	{
		remainingSize = bufferEnd - b;

		/* Copy 'string' to 'buffer' converting it to lowercase and removing unwanted spaces. If the instruction separator character ';' and '\n' is found, stop. */
		size_t length = 0;
		bool allowSpace = false;
		for (; *string; string++)
		{
			char c = *string;
			if (c == ';' || c == '\n')
				break;
			else if (c == ' ' && !allowSpace)
				continue;

			if (length >= 128)
				return 0;

			c = _NMD_TOLOWER(c);
			parsedString[length++] = c;
			allowSpace = (_NMD_IS_LOWERCASE(c) || _NMD_IS_DECIMAL_NUMBER(c)) && (_NMD_IS_LOWERCASE(string[2]) || _NMD_IS_DECIMAL_NUMBER(string[2]));
		}

		if (*string != '\0')
			string++;

		/* If the last character is a ' '(space), remove it. */
		if (length > 0 && parsedString[length - 1] == ' ')
			length--;

		/* After all of the string manipulation, place the null character. */
		parsedString[length] = '\0';

		const size_t numBytes = _nmd_assemble_single(&ai);
		if (numBytes == 0 || numBytes > remainingSize)
			return 0;

		/* Copy bytes from 'tempBuffer' to the buffer provided by the user. */
		size_t i = 0;
		for (; i < numBytes; i++)
			b[i] = tempBuffer[i];

		b += numBytes;

		numInstructions++;
	}

	if (count)
		*count = numInstructions;

	return (size_t)(b - (uint8_t*)buffer);
}