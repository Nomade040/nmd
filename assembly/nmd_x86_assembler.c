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

enum _NMD_NUMBER_BASE
{
	_NMD_NUMBER_BASE_NONE        = 0,
	_NMD_NUMBER_BASE_DECIMAL     = 10,
	_NMD_NUMBER_BASE_HEXADECIMAL = 16,
	_NMD_NUMBER_BASE_BINARY      = 2
};

bool _nmd_parse_number(const char* string, int64_t* num, size_t* numDigits)
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

	*numDigits = i;

	int64_t numTemp = 0;
	for (i = 0; i < *numDigits; i++)
	{
		const char c = s[i];
		numTemp += (c <= '9') ? (c - '0') : (10 + c - 'a');
		if (i < *numDigits - 1)
		{
			/* Return false if number is greater than 2^64-1 */
			if (*numDigits > 16 && i >= 15)
			{
				if ((base == _NMD_NUMBER_BASE_DECIMAL && (uint64_t)numTemp >= (uint64_t)1844674407370955162) || /* ceiling((2^64-1) / 10) */
					(base == _NMD_NUMBER_BASE_HEXADECIMAL && (uint64_t)numTemp >= (uint64_t)1152921504606846976) || /* *ceiling((2^64-1) / 16) */
					(base == _NMD_NUMBER_BASE_BINARY && (uint64_t)numTemp >= (uint64_t)9223372036854775808)) /* ceiling((2^64-1) / 2) */
				{
					return false;
				}

			}
			numTemp *= base;
		}
	}

	if (s != string) /* There's either a "0x" or "0b" prefix. */
		*numDigits = (size_t)((ptrdiff_t)(s + i) - (ptrdiff_t)string);
	
	if (isNegative)
		numTemp *= -1;

	*num = numTemp;

	return true;
}

size_t _nmd_assemble_single(_nmd_assemble_info* ai)
{
	size_t i = 0;

	bool lockPrefix = false, repeatPrefix = false, repeatZeroPrefix = false , repeatNotZeroPrefix = false;

	if (_nmd_strstr(ai->s, "emit ") == (const char*)ai->s)
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

	/* Parse prefixes */
	if (_nmd_strstr(ai->s, "lock ") == (const char*)ai->s)
		lockPrefix = true, ai->s += 5;
	else if (_nmd_strstr(ai->s, "rep ") == (const char*)ai->s)
		repeatPrefix = true, ai->s += 4;
	else if (_nmd_strstr(ai->s, "repe ") == (const char*)ai->s || _nmd_strstr(ai->s, "repz ") == (const char*)ai->s)
		repeatZeroPrefix = true, ai->s += 5;
	else if (_nmd_strstr(ai->s, "repne ") == (const char*)ai->s || _nmd_strstr(ai->s, "repnz ") == (const char*)ai->s)
		repeatNotZeroPrefix = true, ai->s += 6;

	if (_nmd_strstr(ai->s, "xacquire ") == (const char*)ai->b)
	{
	}
	else if (_nmd_strstr(ai->s, "xrelease ") == (const char*)ai->b)
	{
	}

	/* Parse opcode */
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
	else /* Only x86-16 or x86-32. */
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
		else if (_nmd_strstr(ai->s, "inc ") == ai->s || _nmd_strstr(ai->s, "dec ") == ai->s)
		{
			ai->s += 4;
			for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_reg32); i++)
			{
				if (_nmd_strcmp(ai->s, _nmd_reg32[i]))
				{
					ai->b[0] = (*(ai->s - 4) == 'i' ? 0x40 : 0x48) + (uint8_t)i;
					return 1;
				}
			}

			for (i = 0; i < _NMD_NUM_ELEMENTS(_nmd_reg16); i++)
			{
				if (_nmd_strcmp(ai->s, _nmd_reg16[i]))
				{
					ai->b[0] = 0x66;
					ai->b[1] = (*(ai->s - 4) == 'i' ? 0x40 : 0x48) + (uint8_t)i;
					return 2;
				}
			}
		}
	}

	if (_nmd_strcmp(ai->s, "pushf"))
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

	for (i = 0; i < _NMD_NUM_ELEMENTS(op1SingleByte); i++)
	{
		if (_nmd_strcmp(ai->s, op1SingleByte[i].s))
		{
			ai->b[0] = op1SingleByte[i].b;
			return 1;
		}
	}

	for (i = 0; i < _NMD_NUM_ELEMENTS(op2SingleByte); i++)
	{
		if (_nmd_strcmp(ai->s, op2SingleByte[i].s))
		{
			ai->b[0] = 0x0f;
			ai->b[1] = op2SingleByte[i].b;
			return 2;
		}
	}

	if (_nmd_strcmp(ai->s, "pause"))
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

	/* try to parse 00 00*/
	if (_nmd_strstr("add", ai->s) == ai->s)
	{

	}

	return 0;
}

/*
Assembles an instruction from a string. Returns the number of bytes written to the buffer on success, zero otherwise. Instructions can be separated using either the ';' or '\n' character.
Parameters:
 - string         [in]         A pointer to a string that represents a instruction in assembly language.
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

	while (string[0] != '\0')
	{
		if (numInstructions == numMaxInstructions)
			break;

		remainingSize = bufferEnd - b;

		/* Copy 'string' to 'buffer' converting it to lowercase and removing unwanted spaces. If the instruction separator character ';' and '\n' is found, stop. */
		size_t length = 0;
		bool allowSpace = false;
		for (; *string; string++)
		{
			const char c = *string;
			if (c == ';' || c == '\n')
				break;
			else if (c == ' ' && !allowSpace)
				continue;

			if (length >= 128)
				return 0;

			const char newChar = (c >= 'A' && c <= 'Z') ? c + 0x20 : c;
			parsedString[length++] = newChar;
			allowSpace = (_NMD_IS_LOWERCASE(newChar) || _NMD_IS_DECIMAL_NUMBER(newChar)) && (_NMD_IS_LOWERCASE(*(string + 2)) || _NMD_IS_DECIMAL_NUMBER(*(string + 2)));
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