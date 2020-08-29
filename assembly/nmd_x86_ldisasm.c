#include "nmd_common.h"

bool _nmd_ldisasm_parse_modrm(const uint8_t** b, bool addressPrefix, NMD_X86_MODE mode, nmd_x86_modrm* const pModrm, size_t remainingSize)
{
	if (remainingSize == 0)
		return false;

	const nmd_x86_modrm modrm = *(nmd_x86_modrm*)(++*b);
	*pModrm = modrm;
	bool hasSIB = false;
	size_t dispSize = 0;

	if (mode == NMD_X86_MODE_16)
	{
		if (modrm.fields.mod != 0b11)
		{
			if (modrm.fields.mod == 0b00)
			{
				if (modrm.fields.rm == 0b110)
					dispSize = 2;
			}
			else
				dispSize = modrm.fields.mod == 0b01 ? 1 : 2;
		}
	}
	else
	{
		if (addressPrefix && mode == NMD_X86_MODE_32)
		{
			if ((modrm.fields.mod == 0b00 && modrm.fields.rm == 0b110) || modrm.fields.mod == 0b10)
				dispSize = 2;
			else if (modrm.fields.mod == 0b01)
				dispSize = 1;
		}
		else
		{
			/* Check for SIB byte */
			uint8_t sib = 0;
			if (modrm.modrm < 0xC0 && modrm.fields.rm == 0b100 && (!addressPrefix || (addressPrefix && mode == NMD_X86_MODE_64)))
			{
				if (remainingSize < 2)
					return false;

				hasSIB = true;
				sib = *++*b;
			}

			if (modrm.fields.mod == 0b01) /* disp8 (ModR/M) */
				dispSize = 1;
			else if ((modrm.fields.mod == 0b00 && modrm.fields.rm == 0b101) || modrm.fields.mod == 0b10) /* disp16,32 (ModR/M) */
				dispSize = (addressPrefix && !(mode == NMD_X86_MODE_64 && addressPrefix) ? 2 : 4);
			else if (hasSIB && (sib & 0b111) == 0b101) /* disp8,32 (SIB) */
				dispSize = (modrm.fields.mod == 0b01 ? 1 : 4);
		}
	}

	*b += dispSize;

	return remainingSize - (hasSIB ? 2 : 1) >= dispSize;
}

/*
Returns the instruction's length if it's valid, zero otherwise.
Parameters:
 - buffer     [in] A pointer to a buffer containing a encoded instruction.
 - bufferSize [in] The buffer's size in bytes.
 - mode       [in] The architecture mode. 'NMD_X86_MODE_32', 'NMD_X86_MODE_64' or 'NMD_X86_MODE_16'.
*/
size_t nmd_x86_ldisasm(const void* buffer, size_t bufferSize, NMD_X86_MODE mode)
{
	const uint8_t* b = (const uint8_t*)(buffer);

	bool operandPrefix = false;
	bool addressPrefix = false;
	bool repeatPrefix = false;
	bool repeatNotZeroPrefix = false;
	bool rexW = false;
	bool lockPrefix = false;
	uint16_t simdPrefix = NMD_X86_PREFIXES_NONE;
	uint8_t op = 0;
	uint8_t opcodeSize = 0;

	bool hasModrm = false;
	nmd_x86_modrm modrm = { 0,0,0 };

	size_t offset = 0;

	/* Parse legacy prefixes & REX prefixes. */
	size_t i = 0;
	for (; i < NMD_X86_MAXIMUM_INSTRUCTION_LENGTH; i++, b++)
	{
		switch (*b)
		{
		case 0xF0: lockPrefix = true; continue;
		case 0xF2: repeatNotZeroPrefix = true, simdPrefix = NMD_X86_PREFIXES_REPEAT_NOT_ZERO; continue;
		case 0xF3: repeatPrefix = true, simdPrefix = NMD_X86_PREFIXES_REPEAT; continue;
		case 0x2E: continue;
		case 0x36: continue;
		case 0x3E: continue;
		case 0x26: continue;
		case 0x64: continue;
		case 0x65: continue;
		case 0x66: operandPrefix = true, simdPrefix = NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE; continue;
		case 0x67: addressPrefix = true; continue;
		default:
			if (mode == NMD_X86_MODE_64 && NMD_R(*b) == 4) /* [40,4f[ */
			{
				if(NMD_C(*b) & 0b1000)
					rexW = true;
				continue;
			}
		}

		break;
	}

	const size_t numPrefixes = (uint8_t)((ptrdiff_t)(b)-(ptrdiff_t)(buffer));

	const size_t remainingValidBytes = (NMD_X86_MAXIMUM_INSTRUCTION_LENGTH - numPrefixes);
	if (remainingValidBytes == 0)
		return 0;

	const size_t remainingBufferSize = bufferSize - numPrefixes;
	if (remainingBufferSize == 0)
		return 0;

	const size_t remainingSize = remainingValidBytes < remainingBufferSize ? remainingValidBytes : remainingBufferSize;

	/* Parse opcode. */
	if (*b == 0x0F) /* 2 or 3 byte opcode. */
	{
		if (remainingSize == 1)
			return false;

		b++;

		if (*b == 0x38 || *b == 0x3A) /* 3 byte opcode. */
		{

			if (remainingSize < 4)
				return false;

			const bool isOpcodeMap38 = *b == 0x38;
			op = *++b;
			modrm = *(nmd_x86_modrm*)(b + 1);
			opcodeSize = 3;
			hasModrm = true;
			if (!_nmd_ldisasm_parse_modrm(&b, addressPrefix, mode, &modrm, remainingSize - 3))
				return 0;

			if (isOpcodeMap38)
			{
#ifndef NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK
				/* Check if the instruction is invalid. */
				if (op == 0x36)
				{
					return 0;
				}
				else if (op <= 0xb || (op >= 0x1c && op <= 0x1e))
				{
					if (simdPrefix == NMD_X86_PREFIXES_REPEAT || simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
						return 0;
				}
				else if (op >= 0xc8 && op <= 0xcd)
				{
					if (simdPrefix)
						return 0;
				}
				else if (op == 0x10 || op == 0x14 || op == 0x15 || op == 0x17 || (op >= 0x20 && op <= 0x25) || op == 0x28 || op == 0x29 || op == 0x2b || NMD_R(op) == 3 || op == 0x40 || op == 0x41 || op == 0xcf || (op >= 0xdb && op <= 0xdf))
				{
					if (simdPrefix != NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
						return 0;
				}
				else if (op == 0x2a || (op >= 0x80 && op <= 0x82))
				{
					if (modrm.fields.mod == 0b11 || simdPrefix != NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
						return 0;
				}
				else if (op == 0xf0 || op == 0xf1)
				{
					if (modrm.fields.mod == 0b11 && (simdPrefix == NMD_X86_PREFIXES_NONE || simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE))
						return 0;
					else if (simdPrefix == NMD_X86_PREFIXES_REPEAT)
						return 0;
				}
				else if (op == 0xf5 || op == 0xf8)
				{
					if (simdPrefix != NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE || modrm.fields.mod == 0b11)
						return 0;
				}
				else if (op == 0xf6)
				{
					if (simdPrefix == NMD_X86_PREFIXES_NONE && modrm.fields.mod == 0b11)
						return 0;
					else if (simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
						return 0;
				}
				else if (op == 0xf9)
				{
					if (simdPrefix != NMD_X86_PREFIXES_NONE || modrm.fields.mod == 0b11)
						return 0;
				}
				else
					return 0;
#endif /* NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK */
			}
			else /* 0x3a */
			{
				if (remainingSize < 5)
					return false;

				offset++;

#ifndef NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK
				/* Check if the instruction is invalid. */
				if ((op >= 0x8 && op <= 0xe) || (op >= 0x14 && op <= 0x17) || (op >= 0x20 && op <= 0x22) || (op >= 0x40 && op <= 0x42) || op == 0x44 || (op >= 0x60 && op <= 0x63) || op == 0xdf || op == 0xce || op == 0xcf)
				{
					if (simdPrefix != NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
						return 0;
				}
				else if (op == 0x0f || op == 0xcc)
				{
					if (simdPrefix)
						return 0;
				}
				else
					return 0;
#endif /* NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK */
			}
		}
		else if (*b == 0x0f) /* 3DNow! opcode map*/
		{
#ifndef NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_3DNOW
				if (remainingSize < 5)
					return false;

				/*
				if (!_nmd_decode_modrm(&b, instruction, remainingSize - 2))
					return false;
				
				instruction->encoding = NMD_X86_ENCODING_3DNOW;
				instruction->opcode = 0x0f;
				instruction->immMask = NMD_X86_IMM8; 
				instruction->immediate = *(b + 1);
				*/

#ifndef NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK
				/*if (!_nmd_findByte(_nmd_valid3DNowOpcodes, sizeof(_nmd_valid3DNowOpcodes), (uint8_t)instruction->immediate))
					return false;*/
#endif /* NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK */
#else /* NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_3DNOW */
			return false;
#endif /* NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_3DNOW */
		}
		else /* 2 byte opcode. */
		{
			op = *b;
			opcodeSize = 2;

			/* Check for ModR/M, SIB and displacement. */
			if (op >= 0x20 && op <= 0x23 && remainingSize == 2)
				hasModrm = true, modrm.modrm = *++b;
			else if (op < 4 || (NMD_R(op) != 3 && NMD_R(op) > 0 && NMD_R(op) < 7) || (op >= 0xD0 && op != 0xFF) || (NMD_R(op) == 7 && NMD_C(op) != 7) || NMD_R(op) == 9 || NMD_R(op) == 0xB || (NMD_R(op) == 0xC && NMD_C(op) < 8) || (NMD_R(op) == 0xA && (op % 8) >= 3) || op == 0x0ff || op == 0x00 || op == 0x0d)
			{
				if (!_nmd_ldisasm_parse_modrm(&b, addressPrefix, mode, &modrm, remainingSize - 2))
					return 0;
				hasModrm = true;
			}

#ifndef NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK
			/* Check if the instruction is invalid. */
			if (_nmd_findByte(_nmd_invalid2op, sizeof(_nmd_invalid2op), op))
				return 0;
			else if (op == 0xc7)
			{
				if ((!simdPrefix && (modrm.fields.mod == 0b11 ? modrm.fields.reg <= 0b101 : modrm.fields.reg == 0b000 || modrm.fields.reg == 0b010)) || (simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO && (modrm.fields.mod == 0b11 || modrm.fields.reg != 0b001)) || ((simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE || simdPrefix == NMD_X86_PREFIXES_REPEAT) && (modrm.fields.mod == 0b11 ? modrm.fields.reg <= (simdPrefix == NMD_X86_PREFIXES_REPEAT ? 0b110 : 0b101) : (modrm.fields.reg != 0b001 && modrm.fields.reg != 0b110))))
					return 0;
			}
			else if (op == 0x00)
			{
				if (modrm.fields.reg >= 0b110)
					return 0;
			}
			else if (op == 0x01)
			{
				if ((modrm.fields.mod == 0b11 ? (( (simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE || simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO || simdPrefix == NMD_X86_PREFIXES_REPEAT) && ((modrm.modrm >= 0xc0 && modrm.modrm <= 0xc5) || (modrm.modrm >= 0xc8 && modrm.modrm <= 0xcb) || (modrm.modrm >= 0xcf && modrm.modrm <= 0xd1) || (modrm.modrm >= 0xd4 && modrm.modrm <= 0xd7) || modrm.modrm == 0xee || modrm.modrm == 0xef || modrm.modrm == 0xfa || modrm.modrm == 0xfb)) || (modrm.fields.reg == 0b000 && modrm.fields.rm >= 0b110) || (modrm.fields.reg == 0b001 && modrm.fields.rm >= 0b100 && modrm.fields.rm <= 0b110) || (modrm.fields.reg == 0b010 && (modrm.fields.rm == 0b010 || modrm.fields.rm == 0b011)) || (modrm.fields.reg == 0b101 && modrm.fields.rm < 0b110 && (!repeatPrefix || (simdPrefix == NMD_X86_PREFIXES_REPEAT && (modrm.fields.rm != 0b000 && modrm.fields.rm != 0b010)))) || (modrm.fields.reg == 0b111 && (modrm.fields.rm > 0b101 || (mode != NMD_X86_MODE_64 && modrm.fields.rm == 0b000)))) : (!repeatPrefix && modrm.fields.reg == 0b101)))
					return 0;
			}
			else if (op == 0x1A || op == 0x1B)
			{
				if (modrm.fields.mod == 0b11)
					return 0;
			}
			else if (op == 0x20 || op == 0x22)
			{
				if (modrm.fields.reg == 0b001 || modrm.fields.reg >= 0b101)
					return 0;
			}
			else if (op >= 0x24 && op <= 0x27)
				return 0;
			else if (op >= 0x3b && op <= 0x3f)
				return 0;
			else if (NMD_R(op) == 5)
			{
				if ((op == 0x50 && modrm.fields.mod != 0b11) || (simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && (op == 0x52 || op == 0x53)) || (simdPrefix == NMD_X86_PREFIXES_REPEAT && (op == 0x50 || (op >= 0x54 && op <= 0x57))) || (repeatNotZeroPrefix && (op == 0x50 || (op >= 0x52 && op <= 0x57) || op == 0x5b)))
					return 0;
			}
			else if (NMD_R(op) == 6)
			{
				if ((!(simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE || simdPrefix == NMD_X86_PREFIXES_REPEAT || simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO) && (op == 0x6c || op == 0x6d)) || (simdPrefix == NMD_X86_PREFIXES_REPEAT && op != 0x6f) || repeatNotZeroPrefix)
					return 0;
			}
			else if (op == 0x78 || op == 0x79)
			{
				if ((((simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && op == 0x78) && !(modrm.fields.mod == 0b11 && modrm.fields.reg == 0b000)) || ((simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE || simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO) && modrm.fields.mod != 0b11)) || (simdPrefix == NMD_X86_PREFIXES_REPEAT))
					return 0;
			}
			else if (op == 0x7c || op == 0x7d)
			{
				if (simdPrefix == NMD_X86_PREFIXES_REPEAT || !(simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE || simdPrefix == NMD_X86_PREFIXES_REPEAT || simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO))
					return 0;
			}
			else if (op == 0x7e || op == 0x7f)
			{
				if (repeatNotZeroPrefix)
					return 0;
			}
			else if (op >= 0x71 && op <= 0x73)
			{
				if ((simdPrefix == NMD_X86_PREFIXES_REPEAT || simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO) || modrm.modrm <= 0xcf || (modrm.modrm >= 0xe8 && modrm.modrm <= 0xef))
					return 0;
			}
			else if (op == 0x73)
			{
				if (modrm.modrm >= 0xe0 && modrm.modrm <= 0xe8)
					return 0;
			}
			else if (op == 0xa6)
			{
				if (modrm.modrm != 0xc0 && modrm.modrm != 0xc8 && modrm.modrm != 0xd0)
					return 0;
			}
			else if (op == 0xa7)
			{
				if (!(modrm.fields.mod == 0b11 && modrm.fields.reg <= 0b101 && modrm.fields.rm == 0b000))
					return 0;
			}
			else if (op == 0xae)
			{
				if (((!simdPrefix && modrm.fields.mod == 0b11 && modrm.fields.reg <= 0b100) || (simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO && !(modrm.fields.mod == 0b11 && modrm.fields.reg == 0b110)) || (simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && (modrm.fields.reg < 0b110 || (modrm.fields.mod == 0b11 && modrm.fields.reg == 0b111))) || (simdPrefix == NMD_X86_PREFIXES_REPEAT && (modrm.fields.reg != 0b100 && modrm.fields.reg != 0b110) && !(modrm.fields.mod == 0b11 && modrm.fields.reg == 0b101))))
					return 0;
			}
			else if (op == 0xb8)
			{
				if (!repeatPrefix)
					return 0;
			}
			else if (op == 0xba)
			{
				if (modrm.fields.reg <= 0b011)
					return 0;
			}
			else if (op == 0xd0)
			{
				if (!simdPrefix || simdPrefix == NMD_X86_PREFIXES_REPEAT)
					return 0;
			}
			else if (op == 0xe0)
			{
				if (simdPrefix == NMD_X86_PREFIXES_REPEAT || simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
					return 0;
			}
			else if (op == 0xf0)
			{
				if (simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? modrm.fields.mod == 0b11 : true)
					return 0;
			}
			else if (simdPrefix == NMD_X86_PREFIXES_REPEAT || simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
			{
				if ((op >= 0x13 && op <= 0x17 && !(op == 0x16 && simdPrefix == NMD_X86_PREFIXES_REPEAT)) || op == 0x28 || op == 0x29 || op == 0x2e || op == 0x2f || (op <= 0x76 && op >= 0x74))
					return 0;
			}
			else if (op == 0x71 || op == 0x72 || (op == 0x73 && !(simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)))
			{
				if ((modrm.modrm >= 0xd8 && modrm.modrm <= 0xdf) || modrm.modrm >= 0xf8)
					return 0;
			}
			else if (op >= 0xc3 && op <= 0xc6)
			{
				if ((op == 0xc5 && modrm.fields.mod != 0b11) || (simdPrefix == NMD_X86_PREFIXES_REPEAT || simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO) || (op == 0xc3 && simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE))
					return 0;
			}
			else if (NMD_R(op) >= 0xd && NMD_C(op) != 0 && op != 0xff && ((NMD_C(op) == 6 && NMD_R(op) != 0xf) ? (!simdPrefix || (NMD_R(op) == 0xD && (simdPrefix == NMD_X86_PREFIXES_REPEAT || simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO) ? modrm.fields.mod != 0b11 : false)) : (simdPrefix == NMD_X86_PREFIXES_REPEAT || simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO || ((NMD_C(op) == 7 && NMD_R(op) != 0xe) ? modrm.fields.mod != 0b11 : false))))
				return 0;
			else if (hasModrm && modrm.fields.mod == 0b11)
			{
				if (op == 0xb2 || op == 0xb4 || op == 0xb5 || op == 0xc3 || op == 0xe7 || op == 0x2b || (simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && (op == 0x12 || op == 0x16)) || (!(simdPrefix == NMD_X86_PREFIXES_REPEAT || simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO) && (op == 0x13 || op == 0x17)))
					return 0;
			}
#endif /* NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK */

			if (NMD_R(op) == 8) /* imm32 */
				offset += (operandPrefix ? 2 : 4);
			else if ((NMD_R(op) == 7 && NMD_C(op) < 4) || op == 0xA4 || op == 0xC2 || (op > 0xC3 && op <= 0xC6) || op == 0xBA || op == 0xAC) /* imm8 */
				offset++;
			else if (op == 0x78 && (repeatNotZeroPrefix || operandPrefix)) /* imm8 + imm8 = "imm16" */
				offset += 2;
		}
	}
	else /* 1 byte opcode */
	{
		op = *b;
		opcodeSize = 1;

		/* Check for ModR/M, SIB and displacement. */
		if (NMD_R(op) == 8 || _nmd_findByte(_nmd_op1modrm, sizeof(_nmd_op1modrm), op) || (NMD_R(op) < 4 && (NMD_C(op) < 4 || (NMD_C(op) >= 8 && NMD_C(op) < 0xC))) || (NMD_R(op) == 0xD && NMD_C(op) >= 8) || ((op == 0xc4 || op == 0xc5) && remainingSize > 1 && ((nmd_x86_modrm*)(b + 1))->fields.mod != 0b11))
		{
			if (!_nmd_ldisasm_parse_modrm(&b, addressPrefix, mode, &modrm, remainingSize - 1))
				return 0;
			hasModrm = true;
		}

#ifndef NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK
		if (op == 0xC6 || op == 0xC7)
		{
			if ((modrm.fields.reg != 0b000 && modrm.fields.reg != 0b111) || (modrm.fields.reg == 0b111 && (modrm.fields.mod != 0b11 || modrm.fields.rm != 0b000)))
				return 0;
		}
		else if (op == 0x8f)
		{
			if (modrm.fields.reg != 0b000)
				return 0;
		}
		else if (op == 0xfe)
		{
			if (modrm.fields.reg >= 0b010)
				return 0;
		}
		else if (op == 0xff)
		{
			if (modrm.fields.reg == 0b111 || (modrm.fields.mod == 0b11 && (modrm.fields.reg == 0b011 || modrm.fields.reg == 0b101)))
				return 0;
		}
		else if (op == 0x8c)
		{
			if (modrm.fields.reg >= 0b110)
				return 0;
		}
		else if (op == 0x8e)
		{
			if (modrm.fields.reg == 0b001 || modrm.fields.reg >= 0b110)
				return 0;
		}
		else if (op == 0x62)
		{
			if (mode == NMD_X86_MODE_64)
				return 0;
		}
		else if (op == 0x8d)
		{
			if (modrm.fields.mod == 0b11)
				return 0;
		}
		else if (op == 0xc4 || op == 0xc5)
		{
			if (mode == NMD_X86_MODE_64 && hasModrm && modrm.fields.mod != 0b11)
				return 0;
		}
		else if (op >= 0xd8 && op <= 0xdf)
		{
			switch (op)
			{
			case 0xd9:
				if ((modrm.fields.reg == 0b001 && modrm.fields.mod != 0b11) || (modrm.modrm > 0xd0 && modrm.modrm < 0xd8) || modrm.modrm == 0xe2 || modrm.modrm == 0xe3 || modrm.modrm == 0xe6 || modrm.modrm == 0xe7 || modrm.modrm == 0xef)
					return 0;
				break;
			case 0xda:
				if (modrm.modrm >= 0xe0 && modrm.modrm != 0xe9)
					return 0;
				break;
			case 0xdb:
				if (((modrm.fields.reg == 0b100 || modrm.fields.reg == 0b110) && modrm.fields.mod != 0b11) || (modrm.modrm >= 0xe5 && modrm.modrm <= 0xe7) || modrm.modrm >= 0xf8)
					return 0;
				break;
			case 0xdd:
				if ((modrm.fields.reg == 0b101 && modrm.fields.mod != 0b11) || NMD_R(modrm.modrm) == 0xf)
					return 0;
				break;
			case 0xde:
				if (modrm.modrm == 0xd8 || (modrm.modrm >= 0xda && modrm.modrm <= 0xdf))
					return 0;
				break;
			case 0xdf:
				if ((modrm.modrm >= 0xe1 && modrm.modrm <= 0xe7) || modrm.modrm >= 0xf8)
					return 0;
				break;
			}
		}
		else if (mode == NMD_X86_MODE_64)
		{
			if (op == 0x6 || op == 0x7 || op == 0xe || op == 0x16 || op == 0x17 || op == 0x1e || op == 0x1f || op == 0x27 || op == 0x2f || op == 0x37 || op == 0x3f || (op >= 0x60 && op <= 0x62) || op == 0x82 || op == 0xce || (op >= 0xd4 && op <= 0xd6))
				return 0;
		}
#endif /* NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK */

#ifndef NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VEX
		/* Check if instruction is VEX. */
		if ((op == 0xc4 || op == 0xc5) && !hasModrm)
		{
			const uint8_t byte0 = op;
			if (remainingSize < 4)
				return 0;

			if (byte0 == 0xc4)
			{
				b += 3;
				op = *b;

				if (op == 0x0c || op == 0x0d || op == 0x40 || op == 0x41 || op == 0x17 || op == 0x21 || op == 0x42)
					offset++;
			}
			else /* 0xc5 */
			{
				b += 2;
				op = *b;
			}

			if (!_nmd_ldisasm_parse_modrm(&b, addressPrefix, mode, &modrm, remainingSize - (byte0 == 0xc4 ? 4 : 3)))
				return false;
			hasModrm = true;
		}
		else
#endif /* NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VEX */

		{
			/* Check for immediate */
			if (_nmd_findByte(_nmd_op1imm32, sizeof(_nmd_op1imm32), op) || (NMD_R(op) < 4 && (NMD_C(op) == 5 || NMD_C(op) == 0xD)) || (NMD_R(op) == 0xB && NMD_C(op) >= 8) || (op == 0xF7 && modrm.fields.reg == 0b000)) /* imm32,16 */
			{
				if (NMD_R(op) == 0xB && NMD_C(op) >= 8)
					offset += rexW ? 8 : (operandPrefix || (mode == NMD_X86_MODE_16 && !operandPrefix) ? 2 : 4);
				else
				{
					if ((mode == NMD_X86_MODE_16 && operandPrefix) || (mode != NMD_X86_MODE_16 && !operandPrefix))
						offset += NMD_X86_IMM32;
					else
						offset += NMD_X86_IMM16;
				}
			}
			else if (NMD_R(op) == 7 || (NMD_R(op) == 0xE && NMD_C(op) < 8) || (NMD_R(op) == 0xB && NMD_C(op) < 8) || (NMD_R(op) < 4 && (NMD_C(op) == 4 || NMD_C(op) == 0xC)) || (op == 0xF6 && modrm.fields.reg <= 0b001) || _nmd_findByte(_nmd_op1imm8, sizeof(_nmd_op1imm8), op)) /* imm8 */
				offset++;
			else if (NMD_R(op) == 0xA && NMD_C(op) < 4)
				offset += (mode == NMD_X86_MODE_64) ? (addressPrefix ? 4 : 8) : (addressPrefix ? 2 : 4);
			else if (op == 0xEA || op == 0x9A) /* imm32,48 */
			{
				if (mode == NMD_X86_MODE_64)
					return 0;
				offset += (operandPrefix ? 4 : 6);
			}
			else if (op == 0xC2 || op == 0xCA) /* imm16 */
				offset += 2;
			else if (op == 0xC8) /* imm16 + imm8 */
				offset += 3;
		}
	}

	if (lockPrefix)
	{
		if (!(hasModrm && modrm.fields.mod != 0b11 &&
			((opcodeSize == 1 && (op == 0x86 || op == 0x87 || (NMD_R(op) < 4 && (op % 8) < 2 && op < 0x38) || ((op >= 0x80 && op <= 0x83) && modrm.fields.reg != 0b111) || (op >= 0xfe && modrm.fields.reg < 2) || ((op == 0xf6 || op == 0xf7) && (modrm.fields.reg == 0b010 || modrm.fields.reg == 0b011)))) ||
				(opcodeSize == 2 && (_nmd_findByte(_nmd_twoOpcodes, sizeof(_nmd_twoOpcodes), op) || op == 0xab || (op == 0xba && modrm.fields.reg != 0b100) || (op == 0xc7 && modrm.fields.reg == 0b001))))))
			return 0;
	}

	return (size_t)((ptrdiff_t)(++b + offset) - (ptrdiff_t)(buffer));
}