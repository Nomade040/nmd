#include "nmd_common.h"

NMD_ASSEMBLY_API bool _nmd_ldisasm_parse_modrm(const uint8_t** b, bool address_prefix, NMD_X86_MODE mode, nmd_x86_modrm* const p_modrm, size_t remaining_size)
{
	if (remaining_size == 0)
		return false;

	const nmd_x86_modrm modrm = *(nmd_x86_modrm*)(++*b);
	*p_modrm = modrm;
	bool has_sib = false;
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
		if (address_prefix && mode == NMD_X86_MODE_32)
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
			if (modrm.modrm < 0xC0 && modrm.fields.rm == 0b100 && (!address_prefix || (address_prefix && mode == NMD_X86_MODE_64)))
			{
				if (remaining_size < 2)
					return false;

				has_sib = true;
				sib = *++*b;
			}

			if (modrm.fields.mod == 0b01) /* disp8 (ModR/M) */
				dispSize = 1;
			else if ((modrm.fields.mod == 0b00 && modrm.fields.rm == 0b101) || modrm.fields.mod == 0b10) /* disp16,32 (ModR/M) */
				dispSize = (address_prefix && !(mode == NMD_X86_MODE_64 && address_prefix) ? 2 : 4);
			else if (has_sib && (sib & 0b111) == 0b101) /* disp8,32 (SIB) */
				dispSize = (modrm.fields.mod == 0b01 ? 1 : 4);
		}
	}

	*b += dispSize;

	return remaining_size - (has_sib ? 2 : 1) >= dispSize;
}

/*
Returns the length of the instruction if it is valid, zero otherwise.
Parameters:
 - buffer      [in] A pointer to a buffer containing an encoded instruction.
 - buffer_size [in] The size of the buffer in bytes.
 - mode        [in] The architecture mode. 'NMD_X86_MODE_32', 'NMD_X86_MODE_64' or 'NMD_X86_MODE_16'.
*/
NMD_ASSEMBLY_API size_t nmd_x86_ldisasm(const void* buffer, size_t buffer_size, NMD_X86_MODE mode)
{
	const uint8_t* b = (const uint8_t*)(buffer);

	bool operand_prefix = false;
	bool address_prefix = false;
	bool repeat_prefix = false;
	bool repeat_not_zero_prefix = false;
	bool rexW = false;
	bool lock_prefix = false;
	uint16_t simd_prefix = NMD_X86_PREFIXES_NONE;
	uint8_t op = 0;
	uint8_t opcode_size = 0;

	bool has_modrm = false;
	nmd_x86_modrm modrm = { 0,0,0 };

	size_t offset = 0;

	/* Parse legacy prefixes & REX prefixes. */
	size_t i = 0;
	for (; i < NMD_X86_MAXIMUM_INSTRUCTION_LENGTH; i++, b++)
	{
		switch (*b)
		{
		case 0xF0: lock_prefix = true; continue;
		case 0xF2: repeat_not_zero_prefix = true, simd_prefix = NMD_X86_PREFIXES_REPEAT_NOT_ZERO; continue;
		case 0xF3: repeat_prefix = true, simd_prefix = NMD_X86_PREFIXES_REPEAT; continue;
		case 0x2E: continue;
		case 0x36: continue;
		case 0x3E: continue;
		case 0x26: continue;
		case 0x64: continue;
		case 0x65: continue;
		case 0x66: operand_prefix = true, simd_prefix = NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE; continue;
		case 0x67: address_prefix = true; continue;
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

	const size_t num_prefixes = (uint8_t)((ptrdiff_t)(b)-(ptrdiff_t)(buffer));

	const size_t remaining_valid_bytes = (NMD_X86_MAXIMUM_INSTRUCTION_LENGTH - num_prefixes);
	if (remaining_valid_bytes == 0)
		return 0;

	const size_t remaining_buffer_size = buffer_size - num_prefixes;
	if (remaining_buffer_size == 0)
		return 0;

	const size_t remaining_size = remaining_valid_bytes < remaining_buffer_size ? remaining_valid_bytes : remaining_buffer_size;

	/* Parse opcode. */
	if (*b == 0x0F) /* 2 or 3 byte opcode. */
	{
		if (remaining_size == 1)
			return false;

		b++;

		if (*b == 0x38 || *b == 0x3A) /* 3 byte opcode. */
		{

			if (remaining_size < 4)
				return false;

			const bool is_opcode_map38 = *b == 0x38;
			op = *++b;
			modrm = *(nmd_x86_modrm*)(b + 1);
			opcode_size = 3;
			has_modrm = true;
			if (!_nmd_ldisasm_parse_modrm(&b, address_prefix, mode, &modrm, remaining_size - 3))
				return 0;

			if (is_opcode_map38)
			{
#ifndef NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK
				/* Check if the instruction is invalid. */
				if (op == 0x36)
				{
					return 0;
				}
				else if (op <= 0xb || (op >= 0x1c && op <= 0x1e))
				{
					if (simd_prefix == NMD_X86_PREFIXES_REPEAT || simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
						return 0;
				}
				else if (op >= 0xc8 && op <= 0xcd)
				{
					if (simd_prefix)
						return 0;
				}
				else if (op == 0x10 || op == 0x14 || op == 0x15 || op == 0x17 || (op >= 0x20 && op <= 0x25) || op == 0x28 || op == 0x29 || op == 0x2b || NMD_R(op) == 3 || op == 0x40 || op == 0x41 || op == 0xcf || (op >= 0xdb && op <= 0xdf))
				{
					if (simd_prefix != NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
						return 0;
				}
				else if (op == 0x2a || (op >= 0x80 && op <= 0x82))
				{
					if (modrm.fields.mod == 0b11 || simd_prefix != NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
						return 0;
				}
				else if (op == 0xf0 || op == 0xf1)
				{
					if (modrm.fields.mod == 0b11 && (simd_prefix == NMD_X86_PREFIXES_NONE || simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE))
						return 0;
					else if (simd_prefix == NMD_X86_PREFIXES_REPEAT)
						return 0;
				}
				else if (op == 0xf5 || op == 0xf8)
				{
					if (simd_prefix != NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE || modrm.fields.mod == 0b11)
						return 0;
				}
				else if (op == 0xf6)
				{
					if (simd_prefix == NMD_X86_PREFIXES_NONE && modrm.fields.mod == 0b11)
						return 0;
					else if (simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
						return 0;
				}
				else if (op == 0xf9)
				{
					if (simd_prefix != NMD_X86_PREFIXES_NONE || modrm.fields.mod == 0b11)
						return 0;
				}
				else
					return 0;
#endif /* NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK */
			}
			else /* 0x3a */
			{
				if (remaining_size < 5)
					return false;

				offset++;

#ifndef NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK
				/* Check if the instruction is invalid. */
				if ((op >= 0x8 && op <= 0xe) || (op >= 0x14 && op <= 0x17) || (op >= 0x20 && op <= 0x22) || (op >= 0x40 && op <= 0x42) || op == 0x44 || (op >= 0x60 && op <= 0x63) || op == 0xdf || op == 0xce || op == 0xcf)
				{
					if (simd_prefix != NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
						return 0;
				}
				else if (op == 0x0f || op == 0xcc)
				{
					if (simd_prefix)
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
				if (remaining_size < 5)
					return false;

				/*
				if (!_nmd_decode_modrm(&b, instruction, remaining_size - 2))
					return false;
				
				instruction->encoding = NMD_X86_ENCODING_3DNOW;
				instruction->opcode = 0x0f;
				instruction->imm_mask = NMD_X86_IMM8; 
				instruction->immediate = *(b + 1);
				*/

#ifndef NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK
				/*if (!_nmd_find_byte(_nmd_valid_3DNow_opcodes, sizeof(_nmd_valid_3DNow_opcodes), (uint8_t)instruction->immediate))
					return false;*/
#endif /* NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK */
#else /* NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_3DNOW */
			return false;
#endif /* NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_3DNOW */
		}
		else /* 2 byte opcode. */
		{
			op = *b;
			opcode_size = 2;

			/* Check for ModR/M, SIB and displacement. */
			if (op >= 0x20 && op <= 0x23 && remaining_size == 2)
				has_modrm = true, modrm.modrm = *++b;
			else if (op < 4 || (NMD_R(op) != 3 && NMD_R(op) > 0 && NMD_R(op) < 7) || (op >= 0xD0 && op != 0xFF) || (NMD_R(op) == 7 && NMD_C(op) != 7) || NMD_R(op) == 9 || NMD_R(op) == 0xB || (NMD_R(op) == 0xC && NMD_C(op) < 8) || (NMD_R(op) == 0xA && (op % 8) >= 3) || op == 0x0ff || op == 0x00 || op == 0x0d)
			{
				if (!_nmd_ldisasm_parse_modrm(&b, address_prefix, mode, &modrm, remaining_size - 2))
					return 0;
				has_modrm = true;
			}

#ifndef NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK
			/* Check if the instruction is invalid. */
			if (_nmd_find_byte(_nmd_invalid_op2, sizeof(_nmd_invalid_op2), op))
				return 0;
			else if (op == 0xc7)
			{
				if ((!simd_prefix && (modrm.fields.mod == 0b11 ? modrm.fields.reg <= 0b101 : modrm.fields.reg == 0b000 || modrm.fields.reg == 0b010)) || (simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO && (modrm.fields.mod == 0b11 || modrm.fields.reg != 0b001)) || ((simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE || simd_prefix == NMD_X86_PREFIXES_REPEAT) && (modrm.fields.mod == 0b11 ? modrm.fields.reg <= (simd_prefix == NMD_X86_PREFIXES_REPEAT ? 0b110 : 0b101) : (modrm.fields.reg != 0b001 && modrm.fields.reg != 0b110))))
					return 0;
			}
			else if (op == 0x00)
			{
				if (modrm.fields.reg >= 0b110)
					return 0;
			}
			else if (op == 0x01)
			{
				if ((modrm.fields.mod == 0b11 ? (( (simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE || simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO || simd_prefix == NMD_X86_PREFIXES_REPEAT) && ((modrm.modrm >= 0xc0 && modrm.modrm <= 0xc5) || (modrm.modrm >= 0xc8 && modrm.modrm <= 0xcb) || (modrm.modrm >= 0xcf && modrm.modrm <= 0xd1) || (modrm.modrm >= 0xd4 && modrm.modrm <= 0xd7) || modrm.modrm == 0xee || modrm.modrm == 0xef || modrm.modrm == 0xfa || modrm.modrm == 0xfb)) || (modrm.fields.reg == 0b000 && modrm.fields.rm >= 0b110) || (modrm.fields.reg == 0b001 && modrm.fields.rm >= 0b100 && modrm.fields.rm <= 0b110) || (modrm.fields.reg == 0b010 && (modrm.fields.rm == 0b010 || modrm.fields.rm == 0b011)) || (modrm.fields.reg == 0b101 && modrm.fields.rm < 0b110 && (!repeat_prefix || (simd_prefix == NMD_X86_PREFIXES_REPEAT && (modrm.fields.rm != 0b000 && modrm.fields.rm != 0b010)))) || (modrm.fields.reg == 0b111 && (modrm.fields.rm > 0b101 || (mode != NMD_X86_MODE_64 && modrm.fields.rm == 0b000)))) : (!repeat_prefix && modrm.fields.reg == 0b101)))
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
				if ((op == 0x50 && modrm.fields.mod != 0b11) || (simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && (op == 0x52 || op == 0x53)) || (simd_prefix == NMD_X86_PREFIXES_REPEAT && (op == 0x50 || (op >= 0x54 && op <= 0x57))) || (repeat_not_zero_prefix && (op == 0x50 || (op >= 0x52 && op <= 0x57) || op == 0x5b)))
					return 0;
			}
			else if (NMD_R(op) == 6)
			{
				if ((!(simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE || simd_prefix == NMD_X86_PREFIXES_REPEAT || simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO) && (op == 0x6c || op == 0x6d)) || (simd_prefix == NMD_X86_PREFIXES_REPEAT && op != 0x6f) || repeat_not_zero_prefix)
					return 0;
			}
			else if (op == 0x78 || op == 0x79)
			{
				if ((((simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && op == 0x78) && !(modrm.fields.mod == 0b11 && modrm.fields.reg == 0b000)) || ((simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE || simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO) && modrm.fields.mod != 0b11)) || (simd_prefix == NMD_X86_PREFIXES_REPEAT))
					return 0;
			}
			else if (op == 0x7c || op == 0x7d)
			{
				if (simd_prefix == NMD_X86_PREFIXES_REPEAT || !(simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE || simd_prefix == NMD_X86_PREFIXES_REPEAT || simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO))
					return 0;
			}
			else if (op == 0x7e || op == 0x7f)
			{
				if (repeat_not_zero_prefix)
					return 0;
			}
			else if (op >= 0x71 && op <= 0x73)
			{
				if ((simd_prefix == NMD_X86_PREFIXES_REPEAT || simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO) || modrm.modrm <= 0xcf || (modrm.modrm >= 0xe8 && modrm.modrm <= 0xef))
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
				if (((!simd_prefix && modrm.fields.mod == 0b11 && modrm.fields.reg <= 0b100) || (simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO && !(modrm.fields.mod == 0b11 && modrm.fields.reg == 0b110)) || (simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && (modrm.fields.reg < 0b110 || (modrm.fields.mod == 0b11 && modrm.fields.reg == 0b111))) || (simd_prefix == NMD_X86_PREFIXES_REPEAT && (modrm.fields.reg != 0b100 && modrm.fields.reg != 0b110) && !(modrm.fields.mod == 0b11 && modrm.fields.reg == 0b101))))
					return 0;
			}
			else if (op == 0xb8)
			{
				if (!repeat_prefix)
					return 0;
			}
			else if (op == 0xba)
			{
				if (modrm.fields.reg <= 0b011)
					return 0;
			}
			else if (op == 0xd0)
			{
				if (!simd_prefix || simd_prefix == NMD_X86_PREFIXES_REPEAT)
					return 0;
			}
			else if (op == 0xe0)
			{
				if (simd_prefix == NMD_X86_PREFIXES_REPEAT || simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
					return 0;
			}
			else if (op == 0xf0)
			{
				if (simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? modrm.fields.mod == 0b11 : true)
					return 0;
			}
			else if (simd_prefix == NMD_X86_PREFIXES_REPEAT || simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
			{
				if ((op >= 0x13 && op <= 0x17 && !(op == 0x16 && simd_prefix == NMD_X86_PREFIXES_REPEAT)) || op == 0x28 || op == 0x29 || op == 0x2e || op == 0x2f || (op <= 0x76 && op >= 0x74))
					return 0;
			}
			else if (op == 0x71 || op == 0x72 || (op == 0x73 && !(simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)))
			{
				if ((modrm.modrm >= 0xd8 && modrm.modrm <= 0xdf) || modrm.modrm >= 0xf8)
					return 0;
			}
			else if (op >= 0xc3 && op <= 0xc6)
			{
				if ((op == 0xc5 && modrm.fields.mod != 0b11) || (simd_prefix == NMD_X86_PREFIXES_REPEAT || simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO) || (op == 0xc3 && simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE))
					return 0;
			}
			else if (NMD_R(op) >= 0xd && NMD_C(op) != 0 && op != 0xff && ((NMD_C(op) == 6 && NMD_R(op) != 0xf) ? (!simd_prefix || (NMD_R(op) == 0xD && (simd_prefix == NMD_X86_PREFIXES_REPEAT || simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO) ? modrm.fields.mod != 0b11 : false)) : (simd_prefix == NMD_X86_PREFIXES_REPEAT || simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO || ((NMD_C(op) == 7 && NMD_R(op) != 0xe) ? modrm.fields.mod != 0b11 : false))))
				return 0;
			else if (has_modrm && modrm.fields.mod == 0b11)
			{
				if (op == 0xb2 || op == 0xb4 || op == 0xb5 || op == 0xc3 || op == 0xe7 || op == 0x2b || (simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && (op == 0x12 || op == 0x16)) || (!(simd_prefix == NMD_X86_PREFIXES_REPEAT || simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO) && (op == 0x13 || op == 0x17)))
					return 0;
			}
#endif /* NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK */

			if (NMD_R(op) == 8) /* imm32 */
				offset += (operand_prefix ? 2 : 4);
			else if ((NMD_R(op) == 7 && NMD_C(op) < 4) || op == 0xA4 || op == 0xC2 || (op > 0xC3 && op <= 0xC6) || op == 0xBA || op == 0xAC) /* imm8 */
				offset++;
			else if (op == 0x78 && (repeat_not_zero_prefix || operand_prefix)) /* imm8 + imm8 = "imm16" */
				offset += 2;
		}
	}
	else /* 1 byte opcode */
	{
		op = *b;
		opcode_size = 1;

		/* Check for ModR/M, SIB and displacement. */
		if (NMD_R(op) == 8 || _nmd_find_byte(_nmd_op1_modrm, sizeof(_nmd_op1_modrm), op) || (NMD_R(op) < 4 && (NMD_C(op) < 4 || (NMD_C(op) >= 8 && NMD_C(op) < 0xC))) || (NMD_R(op) == 0xD && NMD_C(op) >= 8) || ((op == 0xc4 || op == 0xc5) && remaining_size > 1 && ((nmd_x86_modrm*)(b + 1))->fields.mod != 0b11))
		{
			if (!_nmd_ldisasm_parse_modrm(&b, address_prefix, mode, &modrm, remaining_size - 1))
				return 0;
			has_modrm = true;
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
			if (mode == NMD_X86_MODE_64 && has_modrm && modrm.fields.mod != 0b11)
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
		if ((op == 0xc4 || op == 0xc5) && !has_modrm)
		{
			const uint8_t byte0 = op;
			if (remaining_size < 4)
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

			if (!_nmd_ldisasm_parse_modrm(&b, address_prefix, mode, &modrm, remaining_size - (byte0 == 0xc4 ? 4 : 3)))
				return false;
			has_modrm = true;
		}
		else
#endif /* NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VEX */

		{
			/* Check for immediate */
			if (_nmd_find_byte(_nmd_op1_imm32, sizeof(_nmd_op1_imm32), op) || (NMD_R(op) < 4 && (NMD_C(op) == 5 || NMD_C(op) == 0xD)) || (NMD_R(op) == 0xB && NMD_C(op) >= 8) || (op == 0xF7 && modrm.fields.reg == 0b000)) /* imm32,16 */
			{
				if (NMD_R(op) == 0xB && NMD_C(op) >= 8)
					offset += rexW ? 8 : (operand_prefix || (mode == NMD_X86_MODE_16 && !operand_prefix) ? 2 : 4);
				else
				{
					if ((mode == NMD_X86_MODE_16 && operand_prefix) || (mode != NMD_X86_MODE_16 && !operand_prefix))
						offset += NMD_X86_IMM32;
					else
						offset += NMD_X86_IMM16;
				}
			}
			else if (NMD_R(op) == 7 || (NMD_R(op) == 0xE && NMD_C(op) < 8) || (NMD_R(op) == 0xB && NMD_C(op) < 8) || (NMD_R(op) < 4 && (NMD_C(op) == 4 || NMD_C(op) == 0xC)) || (op == 0xF6 && modrm.fields.reg <= 0b001) || _nmd_find_byte(_nmd_op1_imm8, sizeof(_nmd_op1_imm8), op)) /* imm8 */
				offset++;
			else if (NMD_R(op) == 0xA && NMD_C(op) < 4)
				offset += (mode == NMD_X86_MODE_64) ? (address_prefix ? 4 : 8) : (address_prefix ? 2 : 4);
			else if (op == 0xEA || op == 0x9A) /* imm32,48 */
			{
				if (mode == NMD_X86_MODE_64)
					return 0;
				offset += (operand_prefix ? 4 : 6);
			}
			else if (op == 0xC2 || op == 0xCA) /* imm16 */
				offset += 2;
			else if (op == 0xC8) /* imm16 + imm8 */
				offset += 3;
		}
	}

	if (lock_prefix)
	{
		if (!(has_modrm && modrm.fields.mod != 0b11 &&
			((opcode_size == 1 && (op == 0x86 || op == 0x87 || (NMD_R(op) < 4 && (op % 8) < 2 && op < 0x38) || ((op >= 0x80 && op <= 0x83) && modrm.fields.reg != 0b111) || (op >= 0xfe && modrm.fields.reg < 2) || ((op == 0xf6 || op == 0xf7) && (modrm.fields.reg == 0b010 || modrm.fields.reg == 0b011)))) ||
				(opcode_size == 2 && (_nmd_find_byte(_nmd_two_opcodes, sizeof(_nmd_two_opcodes), op) || op == 0xab || (op == 0xba && modrm.fields.reg != 0b100) || (op == 0xc7 && modrm.fields.reg == 0b001))))))
			return 0;
	}

	return (size_t)((ptrdiff_t)(++b + offset) - (ptrdiff_t)(buffer));
}