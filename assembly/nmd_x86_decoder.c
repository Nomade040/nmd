#include "nmd_common.h"

NMD_ASSEMBLY_API void _nmd_decode_operand_segment_reg(const nmd_x86_instruction* instruction, nmd_x86_operand* operand)
{
	if (instruction->segment_override)
		operand->fields.reg = (uint8_t)(NMD_X86_REG_ES + _nmd_get_bit_index(instruction->segment_override));
	else
		operand->fields.reg = (uint8_t)(!(instruction->prefixes & NMD_X86_PREFIXES_REX_B) && (instruction->modrm.fields.rm == 0b100 || instruction->modrm.fields.rm == 0b101) ? NMD_X86_REG_SS : NMD_X86_REG_DS);
}

/* Decodes a memory operand. modrm is assumed to be in the range [00,BF] */
NMD_ASSEMBLY_API void _nmd_decode_modrm_upper32(const nmd_x86_instruction* instruction, nmd_x86_operand* operand)
{
    /* Set operand type */
	operand->type = NMD_X86_OPERAND_TYPE_MEMORY;

	if (instruction->has_sib) /* R/M is 0b100 */
	{
		if (instruction->sib.fields.base == 0b101) /* Check if there is displacement */
		{
			if (instruction->modrm.fields.mod != 0b00)
				operand->fields.mem.base = (uint8_t)(instruction->mode == NMD_X86_MODE_64 && !(instruction->prefixes & NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE) ? (instruction->prefixes & NMD_X86_PREFIXES_REX_B ? NMD_X86_REG_R13 : NMD_X86_REG_RBP) : NMD_X86_REG_EBP);
		}
		else
			operand->fields.mem.base = (uint8_t)((instruction->mode == NMD_X86_MODE_64 && !(instruction->prefixes & NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE) ? (instruction->prefixes & NMD_X86_PREFIXES_REX_B ? NMD_X86_REG_R8 : NMD_X86_REG_RAX) : NMD_X86_REG_EAX) + instruction->sib.fields.base);

		if (instruction->sib.fields.index != 0b100)
			operand->fields.mem.index = (uint8_t)((instruction->mode == NMD_X86_MODE_64 && !(instruction->prefixes & NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE) ? (instruction->prefixes & NMD_X86_PREFIXES_REX_X ? NMD_X86_REG_R8 : NMD_X86_REG_RAX) : NMD_X86_REG_EAX) + instruction->sib.fields.index);

		if (instruction->prefixes & NMD_X86_PREFIXES_REX_X && instruction->sib.fields.index == 0b100)
		{
			operand->fields.mem.index = (uint8_t)NMD_X86_REG_R12;
		}
        
		operand->fields.mem.scale = instruction->sib.fields.scale;
	}
	else if (!(instruction->modrm.fields.mod == 0b00 && instruction->modrm.fields.rm == 0b101))
	{
		if (instruction->mode == NMD_X86_MODE_16 && !(instruction->prefixes & NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE))
		{
			operand->fields.mem.base = NMD_X86_REG_BX;
			operand->fields.mem.index = NMD_X86_REG_SI;
		}
		else
		{
			if ((instruction->prefixes & (NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE | NMD_X86_PREFIXES_REX_B)) == (NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE | NMD_X86_PREFIXES_REX_B) && instruction->mode == NMD_X86_MODE_64)
				operand->fields.mem.base = (uint8_t)(NMD_X86_REG_R8D + instruction->modrm.fields.rm);
			else
				operand->fields.mem.base = (uint8_t)((instruction->mode == NMD_X86_MODE_64 && !(instruction->prefixes & NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE) ? (instruction->prefixes & NMD_X86_PREFIXES_REX_B ? NMD_X86_REG_R8 : NMD_X86_REG_RAX) : NMD_X86_REG_EAX) + instruction->modrm.fields.rm);
		}
	}

	_nmd_decode_operand_segment_reg(instruction, operand);

	operand->fields.mem.disp = instruction->displacement;
}

NMD_ASSEMBLY_API void _nmd_decode_memory_operand(const nmd_x86_instruction* instruction, nmd_x86_operand* operand, uint8_t mod11base_reg)
{
	if (instruction->modrm.fields.mod == 0b11)
	{
		operand->type = NMD_X86_OPERAND_TYPE_REGISTER;
		operand->fields.reg = mod11base_reg + instruction->modrm.fields.rm;
	}
	else
		_nmd_decode_modrm_upper32(instruction, operand);
}

NMD_ASSEMBLY_API void _nmd_decode_operand_Eb(const nmd_x86_instruction* instruction, nmd_x86_operand* operand)
{
	_nmd_decode_memory_operand(instruction, operand, NMD_X86_REG_AL);
}

NMD_ASSEMBLY_API void _nmd_decode_operand_Ew(const nmd_x86_instruction* instruction, nmd_x86_operand* operand)
{
	_nmd_decode_memory_operand(instruction, operand, NMD_X86_REG_AX);
}

NMD_ASSEMBLY_API void _nmd_decode_operand_Ev(const nmd_x86_instruction* instruction, nmd_x86_operand* operand)
{
	_nmd_decode_memory_operand(instruction, operand, (uint8_t)(instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_REG_AX : NMD_X86_REG_EAX));
}

NMD_ASSEMBLY_API void _nmd_decode_operand_Ey(const nmd_x86_instruction* instruction, nmd_x86_operand* operand)
{
	_nmd_decode_memory_operand(instruction, operand, (uint8_t)(instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_REG_AX : NMD_X86_REG_EAX));
}

NMD_ASSEMBLY_API void _nmd_decode_operand_Qq(const nmd_x86_instruction* instruction, nmd_x86_operand* operand)
{
	_nmd_decode_memory_operand(instruction, operand, NMD_X86_REG_MM0);
}

NMD_ASSEMBLY_API void _nmd_decode_operand_Wdq(const nmd_x86_instruction* instruction, nmd_x86_operand* operand)
{
	_nmd_decode_memory_operand(instruction, operand, NMD_X86_REG_XMM0);
}

NMD_ASSEMBLY_API void _nmd_decode_operand_Gb(const nmd_x86_instruction* instruction, nmd_x86_operand* operand)
{
	operand->type = NMD_X86_OPERAND_TYPE_REGISTER;
	operand->fields.reg = NMD_X86_REG_AL + instruction->modrm.fields.reg;
}

NMD_ASSEMBLY_API void _nmd_decode_operand_Gd(const nmd_x86_instruction* instruction, nmd_x86_operand* operand)
{
	operand->type = NMD_X86_OPERAND_TYPE_REGISTER;
	operand->fields.reg = NMD_X86_REG_EAX + instruction->modrm.fields.reg;
}

NMD_ASSEMBLY_API void _nmd_decode_operand_Gw(const nmd_x86_instruction* instruction, nmd_x86_operand* operand)
{
	operand->type = NMD_X86_OPERAND_TYPE_REGISTER;
	operand->fields.reg = NMD_X86_REG_AX + instruction->modrm.fields.reg;
}

NMD_ASSEMBLY_API void _nmd_decode_operand_Gv(const nmd_x86_instruction* instruction, nmd_x86_operand* operand)
{
	operand->type = NMD_X86_OPERAND_TYPE_REGISTER;
	if (instruction->prefixes & NMD_X86_PREFIXES_REX_B)
		operand->fields.reg = (uint8_t)((!(instruction->prefixes & NMD_X86_PREFIXES_REX_W) ? NMD_X86_REG_R8D : NMD_X86_REG_R8) + instruction->modrm.fields.reg);
	else
		operand->fields.reg = (uint8_t)((instruction->rex_w_prefix ? NMD_X86_REG_RAX : (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && instruction->mode != NMD_X86_MODE_16 ? NMD_X86_REG_AX : NMD_X86_REG_EAX)) + instruction->modrm.fields.reg);
}

NMD_ASSEMBLY_API void _nmd_decode_operand_Rv(const nmd_x86_instruction* instruction, nmd_x86_operand* operand)
{
	operand->type = NMD_X86_OPERAND_TYPE_REGISTER;
	if (instruction->prefixes & NMD_X86_PREFIXES_REX_R)
		operand->fields.reg = (uint8_t)((!(instruction->prefixes & NMD_X86_PREFIXES_REX_W) ? NMD_X86_REG_R8D : NMD_X86_REG_R8) + instruction->modrm.fields.rm);
	else
		operand->fields.reg = (uint8_t)((instruction->rex_w_prefix ? NMD_X86_REG_RAX : ((instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && instruction->mode != NMD_X86_MODE_16) || (instruction->mode == NMD_X86_MODE_16 && !(instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)) ? NMD_X86_REG_AX : NMD_X86_REG_EAX)) + instruction->modrm.fields.rm);
}

NMD_ASSEMBLY_API void _nmd_decode_operand_Gy(const nmd_x86_instruction* instruction, nmd_x86_operand* operand)
{
	operand->type = NMD_X86_OPERAND_TYPE_REGISTER;
	operand->fields.reg = (uint8_t)((instruction->mode == NMD_X86_MODE_64 ? NMD_X86_REG_RAX : NMD_X86_REG_EAX) + instruction->modrm.fields.reg);
}

NMD_ASSEMBLY_API void _nmd_decode_operand_Pq(const nmd_x86_instruction* instruction, nmd_x86_operand* operand)
{
	operand->type = NMD_X86_OPERAND_TYPE_REGISTER;
	operand->fields.reg = NMD_X86_REG_MM0 + instruction->modrm.fields.reg;
}

NMD_ASSEMBLY_API void _nmd_decode_operand_Nq(const nmd_x86_instruction* instruction, nmd_x86_operand* operand)
{
	operand->type = NMD_X86_OPERAND_TYPE_REGISTER;
	operand->fields.reg = NMD_X86_REG_MM0 + instruction->modrm.fields.rm;
}

NMD_ASSEMBLY_API void _nmd_decode_operand_Vdq(const nmd_x86_instruction* instruction, nmd_x86_operand* operand)
{
	operand->type = NMD_X86_OPERAND_TYPE_REGISTER;
	operand->fields.reg = NMD_X86_REG_XMM0 + instruction->modrm.fields.reg;
}

NMD_ASSEMBLY_API void _nmd_decode_operand_Udq(const nmd_x86_instruction* instruction, nmd_x86_operand* operand)
{
	operand->type = NMD_X86_OPERAND_TYPE_REGISTER;
	operand->fields.reg = NMD_X86_REG_XMM0 + instruction->modrm.fields.rm;
}

NMD_ASSEMBLY_API void _nmd_decode_conditional_flag(nmd_x86_instruction* instruction, const uint8_t condition)
{
	switch (condition)
	{
		case 0x0: instruction->tested_flags.fields.OF = 1; break;                                                                             /* Jump if overflow (OF=1) */
		case 0x1: instruction->tested_flags.fields.OF = 1; break;                                                                             /* Jump if not overflow (OF=0) */
		case 0x2: instruction->tested_flags.fields.CF = 1; break;                                                                             /* Jump if not above or equal (CF=1) */
		case 0x3: instruction->tested_flags.fields.CF = 1; break;                                                                             /* Jump if not below (CF=0) */
		case 0x4: instruction->tested_flags.fields.ZF = 1; break;                                                                             /* Jump if equal (ZF=1) */
		case 0x5: instruction->tested_flags.fields.ZF = 1; break;                                                                             /* Jump if not equal (ZF=0) */
		case 0x6: instruction->tested_flags.fields.CF = instruction->tested_flags.fields.ZF = 1; break;                                       /* Jump if not above (CF=1 or ZF=1) */
		case 0x7: instruction->tested_flags.fields.CF = instruction->tested_flags.fields.ZF = 1; break;                                       /* Jump if not below or equal (CF=0 and ZF=0) */
		case 0x8: instruction->tested_flags.fields.SF = 1; break;                                                                             /* Jump if sign (SF=1) */
		case 0x9: instruction->tested_flags.fields.SF = 1; break;                                                                             /* Jump if not sign (SF=0) */
		case 0xa: instruction->tested_flags.fields.PF = 1; break;                                                                             /* Jump if parity/parity even (PF=1) */
		case 0xb: instruction->tested_flags.fields.PF = 1; break;                                                                             /* Jump if parity odd (PF=0) */
		case 0xc: instruction->tested_flags.fields.SF = instruction->tested_flags.fields.OF = 1; break;                                       /* Jump if not greater or equal (SF != OF) */
		case 0xd: instruction->tested_flags.fields.SF = instruction->tested_flags.fields.OF = 1; break;                                       /* Jump if not less (SF=OF) */
		case 0xe: instruction->tested_flags.fields.ZF = instruction->tested_flags.fields.SF = instruction->tested_flags.fields.OF = 1; break; /* Jump if not greater (ZF=1 or SF != OF) */
		case 0xf: instruction->tested_flags.fields.ZF = instruction->tested_flags.fields.SF = instruction->tested_flags.fields.OF = 1; break; /* Jump if not less or equal (ZF=0 and SF=OF) */
	}
}

NMD_ASSEMBLY_API bool _nmd_decode_modrm(const uint8_t** p_buffer, size_t* p_buffer_size, nmd_x86_instruction* const instruction)
{
	instruction->has_modrm = true;
	_NMD_READ_BYTE(*p_buffer, *p_buffer_size, instruction->modrm.modrm);
	
	const bool address_prefix = (bool)(instruction->prefixes & NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE);

	/* Check for 16-Bit Addressing Form */
	if (instruction->mode == NMD_X86_MODE_16)
	{
		if (instruction->modrm.fields.mod != 0b11)
		{
			if (instruction->modrm.fields.mod == 0b00)
			{
				if (instruction->modrm.fields.rm == 0b110)
					instruction->disp_mask = NMD_X86_DISP16;
			}
			else
				instruction->disp_mask = (uint8_t)(instruction->modrm.fields.mod == 0b01 ? NMD_X86_DISP8 : NMD_X86_DISP16);
		}
	}
	else
	{
		/* Check for 16-Bit Addressing Form */
		if (address_prefix && instruction->mode == NMD_X86_MODE_32)
		{
			/* Check for displacement */
			if ((instruction->modrm.fields.mod == 0b00 && instruction->modrm.fields.rm == 0b110) || instruction->modrm.fields.mod == 0b10)
				instruction->disp_mask = NMD_X86_DISP16;
			else if (instruction->modrm.fields.mod == 0b01)
				instruction->disp_mask = NMD_X86_DISP8;
		}
		else /*if (!address_prefix || (address_prefix && **b >= 0x40) || (address_prefix && instruction->mode == NMD_X86_MODE_64)) */
		{
			/* Check for SIB byte */
			if (instruction->modrm.modrm < 0xC0 && instruction->modrm.fields.rm == 0b100 && (!address_prefix || (address_prefix && instruction->mode == NMD_X86_MODE_64)))
			{
				instruction->has_sib = true;
				_NMD_READ_BYTE(*p_buffer, *p_buffer_size, instruction->sib.sib);
			}

			/* Check for displacement */
			if (instruction->modrm.fields.mod == 0b01) /* disp8 (ModR/M) */
				instruction->disp_mask = NMD_X86_DISP8;
			else if ((instruction->modrm.fields.mod == 0b00 && instruction->modrm.fields.rm == 0b101) || instruction->modrm.fields.mod == 0b10) /* disp16,32 (ModR/M) */
				instruction->disp_mask = (uint8_t)(address_prefix && !(instruction->mode == NMD_X86_MODE_64 && instruction->prefixes & NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE) ? NMD_X86_DISP16 : NMD_X86_DISP32);
			else if (instruction->has_sib && instruction->sib.fields.base == 0b101) /* disp8,32 (SIB) */
				instruction->disp_mask = (uint8_t)(instruction->modrm.fields.mod == 0b01 ? NMD_X86_DISP8 : NMD_X86_DISP32);
		}
	}

	/* Make sure we can read 'instruction->disp_mask' bytes from the buffer */
	if (*p_buffer_size < instruction->disp_mask)
		return false;
	
	/* Copy 'instruction->disp_mask' bytes from the buffer */
	size_t i = 0;
	for (; i < (size_t)instruction->disp_mask; i++)
		((uint8_t*)(&instruction->displacement))[i] = (*p_buffer)[i];
	
	/* Increment the buffer and decrement the buffer's size */
	*p_buffer += instruction->disp_mask;
	*p_buffer_size -= instruction->disp_mask;

	return true;
}

/*
Decodes an instruction. Returns true if the instruction is valid, false otherwise.
Parameters:
 - buffer      [in]  A pointer to a buffer containing an encoded instruction.
 - buffer_size [in]  The size of the buffer in bytes.
 - instruction [out] A pointer to a variable of type 'nmd_x86_instruction' that receives information about the instruction.
 - mode        [in]  The architecture mode. 'NMD_X86_MODE_32', 'NMD_X86_MODE_64' or 'NMD_X86_MODE_16'.
 - flags       [in]  A mask of 'NMD_X86_DECODER_FLAGS_XXX' that specifies which features the decoder is allowed to use. If uncertain, use 'NMD_X86_DECODER_FLAGS_MINIMAL'.
*/
NMD_ASSEMBLY_API bool nmd_x86_decode(const void* const buffer, size_t buffer_size, nmd_x86_instruction* instruction, NMD_X86_MODE mode, uint32_t flags)
{
	/* Security considerations for memory safety:
	The contents of 'buffer' should be considered untrusted and decoded carefully.
	
	'buffer' should always point to the start of the buffer. We use the 'b'
	buffer iterator to read data from the buffer, however before accessing it
	make sure to check 'buffer_size' to see if we can safely access it. Then,
	after reading data from the buffer we increment 'b' and decrement 'buffer_size'.
	Helper macros: _NMD_READ_BYTE()
	*/
	
	/* Clear 'instruction' */
	size_t i = 0;
	for (; i < sizeof(nmd_x86_instruction); i++)
		((uint8_t*)(instruction))[i] = 0x00;

	/* Set mode */
	instruction->mode = (uint8_t)mode;

	/* Set buffer iterator */
	const uint8_t* b = (const uint8_t*)buffer;
	
	/*  Clamp 'buffer_size' to 15. We will only read up to 15 bytes(NMD_X86_MAXIMUM_INSTRUCTION_LENGTH) */
	if (buffer_size > 15)
		buffer_size = 15;
	
	/* Decode legacy and REX prefixes */
	for (; buffer_size > 0; b++, buffer_size--)
	{
		switch (*b)
		{
		case 0xF0: instruction->prefixes = (instruction->prefixes | (instruction->simd_prefix = NMD_X86_PREFIXES_LOCK)); continue;
		case 0xF2: instruction->prefixes = (instruction->prefixes | (instruction->simd_prefix = NMD_X86_PREFIXES_REPEAT_NOT_ZERO)), instruction->repeat_prefix = false; continue;
		case 0xF3: instruction->prefixes = (instruction->prefixes | (instruction->simd_prefix = NMD_X86_PREFIXES_REPEAT)), instruction->repeat_prefix = true; continue;
		case 0x2E: instruction->prefixes = (instruction->prefixes | (instruction->segment_override = NMD_X86_PREFIXES_CS_SEGMENT_OVERRIDE)); continue;
		case 0x36: instruction->prefixes = (instruction->prefixes | (instruction->segment_override = NMD_X86_PREFIXES_SS_SEGMENT_OVERRIDE)); continue;
		case 0x3E: instruction->prefixes = (instruction->prefixes | (instruction->segment_override = NMD_X86_PREFIXES_DS_SEGMENT_OVERRIDE)); continue;
		case 0x26: instruction->prefixes = (instruction->prefixes | (instruction->segment_override = NMD_X86_PREFIXES_ES_SEGMENT_OVERRIDE)); continue;
		case 0x64: instruction->prefixes = (instruction->prefixes | (instruction->segment_override = NMD_X86_PREFIXES_FS_SEGMENT_OVERRIDE)); continue;
		case 0x65: instruction->prefixes = (instruction->prefixes | (instruction->segment_override = NMD_X86_PREFIXES_GS_SEGMENT_OVERRIDE)); continue;
		case 0x66: instruction->prefixes = (instruction->prefixes | (instruction->simd_prefix = NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)), instruction->rex_w_prefix = false; continue;
		case 0x67: instruction->prefixes = (instruction->prefixes | NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE); continue;
		default:
			if (mode == NMD_X86_MODE_64 && _NMD_R(*b) == 4) /* REX prefixes [0x40,0x4f] */
			{
				instruction->has_rex = true;
				instruction->rex = *b;
				instruction->prefixes = (instruction->prefixes & ~(NMD_X86_PREFIXES_REX_B | NMD_X86_PREFIXES_REX_X | NMD_X86_PREFIXES_REX_R | NMD_X86_PREFIXES_REX_W));

				if (*b & 0b0001) /* Bit position 0 */
					instruction->prefixes = instruction->prefixes | NMD_X86_PREFIXES_REX_B;
				if (*b & 0b0010) /* Bit position 1 */
					instruction->prefixes = instruction->prefixes | NMD_X86_PREFIXES_REX_X;
				if (*b & 0b0100) /* Bit position 2 */
					instruction->prefixes = instruction->prefixes | NMD_X86_PREFIXES_REX_R;
				if (*b & 0b1000) /* Bit position 3 */
				{
					instruction->prefixes = instruction->prefixes | NMD_X86_PREFIXES_REX_W;
					instruction->rex_w_prefix = true;
				}

				continue;
			}
		}

		break;
	}

	/* Calculate the number of prefixes based on how much the iterator moved */
	instruction->num_prefixes = (uint8_t)((ptrdiff_t)(b)-(ptrdiff_t)(buffer));

	/* Assume the instruction uses legacy encoding. It is most likely the case */
	instruction->encoding = NMD_X86_ENCODING_LEGACY;

	/* Opcode byte. This variable is used because 'op' is simpler than 'instruction->opcode' */
	uint8_t op;
	_NMD_READ_BYTE(b, buffer_size, op);

	if (op == 0x0F) /* 2 or 3 byte opcode */
	{
		_NMD_READ_BYTE(b, buffer_size, op);

		if (op == 0x38 || op == 0x3A) /* 3 byte opcode */
		{
			instruction->opcode_size = 3;
			instruction->opcode_map = (uint8_t)(op == 0x38 ? NMD_X86_OPCODE_MAP_0F38 : NMD_X86_OPCODE_MAP_0F3A);
            
			_NMD_READ_BYTE(b, buffer_size, op);
			instruction->opcode = op;

			if (!_nmd_decode_modrm(&b, &buffer_size, instruction))
				return false;

			const nmd_x86_modrm modrm = instruction->modrm;
			if (instruction->opcode_map == NMD_X86_OPCODE_MAP_0F38)
			{
#ifndef NMD_ASSEMBLY_DISABLE_DECODER_VALIDITY_CHECK
				if (flags & NMD_X86_DECODER_FLAGS_VALIDITY_CHECK)
				{
					/* Check if the instruction is invalid. */
					if (op == 0x36)
					{
						return false;
					}
					else if (op <= 0xb || (op >= 0x1c && op <= 0x1e))
					{
						if (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT || instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
							return false;
					}
					else if (op >= 0xc8 && op <= 0xcd)
					{
						if (instruction->simd_prefix)
							return false;
					}
					else if (op == 0x10 || op == 0x14 || op == 0x15 || op == 0x17 || (op >= 0x20 && op <= 0x25) || op == 0x28 || op == 0x29 || op == 0x2b || _NMD_R(op) == 3 || op == 0x40 || op == 0x41 || op == 0xcf || (op >= 0xdb && op <= 0xdf))
					{
						if (instruction->simd_prefix != NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
							return false;
					}
					else if (op == 0x2a || (op >= 0x80 && op <= 0x82))
					{
						if (modrm.fields.mod == 0b11 || instruction->simd_prefix != NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
							return false;
					}
					else if (op == 0xf0 || op == 0xf1)
					{
						if (modrm.fields.mod == 0b11 && (instruction->simd_prefix == NMD_X86_PREFIXES_NONE || instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE))
							return false;
						else if (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT)
							return false;
					}
					else if (op == 0xf5 || op == 0xf8)
					{
						if (instruction->simd_prefix != NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE || modrm.fields.mod == 0b11)
							return false;
					}
					else if (op == 0xf6)
					{
						if (instruction->simd_prefix == NMD_X86_PREFIXES_NONE && modrm.fields.mod == 0b11)
							return false;
						else if (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
							return false;
					}
					else if (op == 0xf9)
					{
						if (instruction->simd_prefix != NMD_X86_PREFIXES_NONE || modrm.fields.mod == 0b11)
							return false;
					}
					else
						return false;
				}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_VALIDITY_CHECK */

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_INSTRUCTION_ID
				if (flags & NMD_X86_DECODER_FLAGS_INSTRUCTION_ID)
				{
					if (_NMD_R(op) == 0x00)
						instruction->id = NMD_X86_INSTRUCTION_PSHUFB + op;
					else if (op >= 0x1c && op <= 0x1e)
						instruction->id = NMD_X86_INSTRUCTION_PABSB + (op - 0x1c);
					else if (_NMD_R(op) == 2)
						instruction->id = NMD_X86_INSTRUCTION_PMOVSXBW + _NMD_C(op);
					else if (_NMD_R(op) == 3)
						instruction->id = NMD_X86_INSTRUCTION_PMOVZXBW + _NMD_C(op);
					else if (_NMD_R(op) == 8)
						instruction->id = NMD_X86_INSTRUCTION_INVEPT + _NMD_C(op);
					else if (_NMD_R(op) == 0xc)
						instruction->id = NMD_X86_INSTRUCTION_SHA1NEXTE + (_NMD_C(op) - 8);
					else if (_NMD_R(op) == 0xd)
						instruction->id = NMD_X86_INSTRUCTION_AESIMC + (_NMD_C(op) - 0xb);
					else
					{
						switch (op)
						{
						case 0x10: instruction->id = NMD_X86_INSTRUCTION_PBLENDVB; break;
						case 0x14: instruction->id = NMD_X86_INSTRUCTION_BLENDVPS; break;
						case 0x15: instruction->id = NMD_X86_INSTRUCTION_BLENDVPD; break;
						case 0x17: instruction->id = NMD_X86_INSTRUCTION_PTEST; break;
						case 0x40: instruction->id = NMD_X86_INSTRUCTION_PMULLD; break;
						case 0x41: instruction->id = NMD_X86_INSTRUCTION_PHMINPOSUW; break;
						case 0xf0: case 0xf1: instruction->id = (uint16_t)((instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE || instruction->simd_prefix == 0x00) ? NMD_X86_INSTRUCTION_MOVBE : NMD_X86_INSTRUCTION_CRC32); break;
						case 0xf6: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_ADCX : NMD_X86_INSTRUCTION_ADOX); break;
						}
					}
				}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_INSTRUCTION_ID */
				
#ifndef NMD_ASSEMBLY_DISABLE_DECODER_CPU_FLAGS
				if (flags & NMD_X86_DECODER_FLAGS_CPU_FLAGS)
				{
					if (op == 0x80 || op == 0x81) /* invept,invvpid */
					{
						instruction->modified_flags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_ZF;
						instruction->cleared_flags.eflags = NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_OF;
					}
					else if (op == 0xf6)
					{
						if (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE) /* adcx */
							instruction->modified_flags.eflags = instruction->tested_flags.eflags = NMD_X86_EFLAGS_CF;
						if (instruction->prefixes & NMD_X86_PREFIXES_REPEAT) /* adox */
							instruction->modified_flags.eflags = instruction->tested_flags.eflags = NMD_X86_EFLAGS_OF;
					}
				}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_CPU_FLAGS */

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_OPERANDS
				if (flags & NMD_X86_DECODER_FLAGS_OPERANDS)
				{
					instruction->num_operands = 2;
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READWRITE;
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;

					if (_NMD_R(op) == 0 || (op >= 0x1c && op <= 0x1e))
					{
						_nmd_decode_operand_Pq(instruction, &instruction->operands[0]);
						_nmd_decode_operand_Qq(instruction, &instruction->operands[1]);
					}
					else if (_NMD_R(op) == 8)
					{
						_nmd_decode_operand_Gy(instruction, &instruction->operands[0]);
						_nmd_decode_modrm_upper32(instruction, &instruction->operands[1]);
					}
					else if (_NMD_R(op) >= 1 && _NMD_R(op) <= 0xe)
					{
						_nmd_decode_operand_Vdq(instruction, &instruction->operands[0]);
						_nmd_decode_operand_Wdq(instruction, &instruction->operands[1]);
					}
					else if (op == 0xf6)
					{
						_nmd_decode_operand_Gy(instruction, &instruction->operands[!instruction->simd_prefix ? 1 : 0]);
						_nmd_decode_operand_Ey(instruction, &instruction->operands[!instruction->simd_prefix ? 0 : 1]);
					}
					else if (op == 0xf0 || op == 0xf1)
					{
						if (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO || (instruction->prefixes & (NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE | NMD_X86_PREFIXES_REPEAT_NOT_ZERO)) == (NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE | NMD_X86_PREFIXES_REPEAT_NOT_ZERO))
						{
							_nmd_decode_operand_Gd(instruction, &instruction->operands[0]);
							if (op == 0xf0)
								_nmd_decode_operand_Eb(instruction, &instruction->operands[1]);
							else if (instruction->prefixes == NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
								_nmd_decode_operand_Ey(instruction, &instruction->operands[1]);
							else
								_nmd_decode_operand_Ew(instruction, &instruction->operands[1]);
						}
						else
						{
							if (instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
								_nmd_decode_operand_Gw(instruction, &instruction->operands[op == 0xf0 ? 0 : 1]);
							else
								_nmd_decode_operand_Gy(instruction, &instruction->operands[op == 0xf0 ? 0 : 1]);

							_nmd_decode_memory_operand(instruction, &instruction->operands[op == 0xf0 ? 1 : 0], (uint8_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_REG_AX : (instruction->rex_w_prefix ? NMD_X86_REG_RAX : NMD_X86_REG_EAX)));
						}
					}
				}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_OPERANDS */
			}
			else /* 0x3a */
			{
				instruction->imm_mask = NMD_X86_IMM8;
                _NMD_READ_BYTE(b, buffer_size, instruction->immediate);

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_VALIDITY_CHECK
				if (flags & NMD_X86_DECODER_FLAGS_VALIDITY_CHECK)
				{
					/* Check if the instruction is invalid. */
					if ((op >= 0x8 && op <= 0xe) || (op >= 0x14 && op <= 0x17) || (op >= 0x20 && op <= 0x22) || (op >= 0x40 && op <= 0x42) || op == 0x44 || (op >= 0x60 && op <= 0x63) || op == 0xdf || op == 0xce || op == 0xcf)
					{
						if (instruction->simd_prefix != NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
							return false;
					}
					else if (op == 0x0f || op == 0xcc)
					{
						if (instruction->simd_prefix)
							return false;
					}
					else
						return false;
				}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_VALIDITY_CHECK */

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_INSTRUCTION_ID
				if (flags & NMD_X86_DECODER_FLAGS_INSTRUCTION_ID)
				{
					if (_NMD_R(op) == 0)
						instruction->id = NMD_X86_INSTRUCTION_ROUNDPS + (op - 8);
					else if (_NMD_R(op) == 4)
						instruction->id = NMD_X86_INSTRUCTION_DPPS + _NMD_C(op);
					else if (_NMD_R(op) == 6)
						instruction->id = NMD_X86_INSTRUCTION_PCMPESTRM + _NMD_C(op);
					else
					{
						switch (op)
						{
						case 0x14: instruction->id = NMD_X86_INSTRUCTION_PEXTRB; break;
						case 0x15: instruction->id = NMD_X86_INSTRUCTION_PEXTRW; break;
						case 0x16: instruction->id = (uint16_t)(instruction->prefixes & NMD_X86_PREFIXES_REX_W ? NMD_X86_INSTRUCTION_PEXTRQ : NMD_X86_INSTRUCTION_PEXTRD); break;
						case 0x17: instruction->id = NMD_X86_INSTRUCTION_EXTRACTPS; break;
						case 0x20: instruction->id = NMD_X86_INSTRUCTION_PINSRB; break;
						case 0x21: instruction->id = NMD_X86_INSTRUCTION_INSERTPS; break;
						case 0x22: instruction->id = (uint16_t)(instruction->prefixes & NMD_X86_PREFIXES_REX_W ? NMD_X86_INSTRUCTION_PINSRQ : NMD_X86_INSTRUCTION_PINSRD); break;
						case 0xcc: instruction->id = NMD_X86_INSTRUCTION_SHA1RNDS4; break;
						case 0xdf: instruction->id = NMD_X86_INSTRUCTION_AESKEYGENASSIST; break;
						}
					}
				}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_INSTRUCTION_ID */

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_OPERANDS
				if (flags & NMD_X86_DECODER_FLAGS_OPERANDS)
				{
					instruction->num_operands = 3;
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READWRITE;
					instruction->operands[1].action = instruction->operands[2].action = NMD_X86_OPERAND_ACTION_READ;
					instruction->operands[2].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;

					if (op == 0x0f && !instruction->simd_prefix)
					{
						_nmd_decode_operand_Pq(instruction, &instruction->operands[0]);
						_nmd_decode_operand_Qq(instruction, &instruction->operands[1]);
					}
					else if (_NMD_R(op) == 1)
					{
						_nmd_decode_memory_operand(instruction, &instruction->operands[0], NMD_X86_REG_EAX);
						_nmd_decode_operand_Vdq(instruction, &instruction->operands[1]);
					}
					else if (_NMD_R(op) == 2)
					{
						_nmd_decode_operand_Vdq(instruction, &instruction->operands[0]);
						_nmd_decode_memory_operand(instruction, &instruction->operands[1], (uint8_t)(_NMD_C(op) == 1 ? NMD_X86_REG_XMM0 : NMD_X86_REG_EAX));
					}
					else if (op == 0xcc || op == 0xdf || _NMD_R(op) == 4 || _NMD_R(op) == 6 || _NMD_R(op) == 0)
					{
						_nmd_decode_operand_Vdq(instruction, &instruction->operands[0]);
						_nmd_decode_operand_Wdq(instruction, &instruction->operands[1]);
					}
				}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_OPERANDS */
			}
		}
		else if (op == 0x0f) /* 3DNow! opcode map*/
		{
#ifndef NMD_ASSEMBLY_DISABLE_DECODER_3DNOW
			if (flags & NMD_X86_DECODER_FLAGS_3DNOW)
			{
				if (!_nmd_decode_modrm(&b, &buffer_size, instruction))
					return false;

				instruction->encoding = NMD_X86_ENCODING_3DNOW;
				instruction->opcode = 0x0f;
				instruction->imm_mask = NMD_X86_IMM8; /* The real opcode is encoded as the immediate byte. */
				_NMD_READ_BYTE(b, buffer_size, instruction->immediate);

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_VALIDITY_CHECK
				if (!_nmd_find_byte(_nmd_valid_3DNow_opcodes, sizeof(_nmd_valid_3DNow_opcodes), (uint8_t)instruction->immediate))
					return false;
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_VALIDITY_CHECK */

			}
			else
				return false;
#else /* NMD_ASSEMBLY_DISABLE_DECODER_3DNOW */
		return false;
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_3DNOW */
		}
		else /* 2 byte opcode. */
		{
			instruction->opcode_size = 2;
			instruction->opcode = op;
			instruction->opcode_map = NMD_X86_OPCODE_MAP_0F;
			
			/* Check for ModR/M, SIB and displacement */
			if (op >= 0x20 && op <= 0x23 && buffer_size == 2)
			{
				instruction->has_modrm = true;
				_NMD_READ_BYTE(b, buffer_size, instruction->modrm.modrm);
			}
			else if (op < 4 || (_NMD_R(op) != 3 && _NMD_R(op) > 0 && _NMD_R(op) < 7) || (op >= 0xD0 && op != 0xFF) || (_NMD_R(op) == 7 && _NMD_C(op) != 7) || _NMD_R(op) == 9 || _NMD_R(op) == 0xB || (_NMD_R(op) == 0xC && _NMD_C(op) < 8) || (_NMD_R(op) == 0xA && (op % 8) >= 3) || op == 0x0ff || op == 0x00 || op == 0x0d)
			{
				if (!_nmd_decode_modrm(&b, &buffer_size, instruction))
					return false;
			}

			const nmd_x86_modrm modrm = instruction->modrm;
#ifndef NMD_ASSEMBLY_DISABLE_DECODER_VALIDITY_CHECK
			if (flags & NMD_X86_DECODER_FLAGS_VALIDITY_CHECK)
			{
				/* Check if the instruction is invalid. */
				if (_nmd_find_byte(_nmd_invalid_op2, sizeof(_nmd_invalid_op2), op))
					return false;
				else if (op == 0xc7)
				{
					if ((!instruction->simd_prefix && (modrm.fields.mod == 0b11 ? modrm.fields.reg <= 0b101 : modrm.fields.reg == 0b000 || modrm.fields.reg == 0b010)) || (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO && (modrm.fields.mod == 0b11 || modrm.fields.reg != 0b001)) || ((instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE || instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT) && (modrm.fields.mod == 0b11 ? modrm.fields.reg <= (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT ? 0b110 : 0b101) : (modrm.fields.reg != 0b001 && modrm.fields.reg != 0b110))))
						return false;
				}
				else if (op == 0x00)
				{
					if (modrm.fields.reg >= 0b110)
						return false;
				}
				else if (op == 0x01)
				{
					if ((modrm.fields.mod == 0b11 ? ((instruction->simd_prefix & (NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE | NMD_X86_PREFIXES_REPEAT_NOT_ZERO | NMD_X86_PREFIXES_REPEAT) && ((modrm.modrm >= 0xc0 && modrm.modrm <= 0xc5) || (modrm.modrm >= 0xc8 && modrm.modrm <= 0xcb) || (modrm.modrm >= 0xcf && modrm.modrm <= 0xd1) || (modrm.modrm >= 0xd4 && modrm.modrm <= 0xd7) || modrm.modrm == 0xee || modrm.modrm == 0xef || modrm.modrm == 0xfa || modrm.modrm == 0xfb)) || (modrm.fields.reg == 0b000 && modrm.fields.rm >= 0b110) || (modrm.fields.reg == 0b001 && modrm.fields.rm >= 0b100 && modrm.fields.rm <= 0b110) || (modrm.fields.reg == 0b010 && (modrm.fields.rm == 0b010 || modrm.fields.rm == 0b011)) || (modrm.fields.reg == 0b101 && modrm.fields.rm < 0b110 && (!(instruction->simd_prefix & NMD_X86_PREFIXES_REPEAT) || (instruction->simd_prefix & NMD_X86_PREFIXES_REPEAT && (modrm.fields.rm != 0b000 && modrm.fields.rm != 0b010)))) || (modrm.fields.reg == 0b111 && (modrm.fields.rm > 0b101 || (mode != NMD_X86_MODE_64 && modrm.fields.rm == 0b000)))) : (!(instruction->simd_prefix & NMD_X86_PREFIXES_REPEAT) && modrm.fields.reg == 0b101)))
						return false;
				}
				else if (op == 0x1A || op == 0x1B)
				{
					if (modrm.fields.mod == 0b11)
						return false;
				}
				else if (op == 0x20 || op == 0x22)
				{
					if (modrm.fields.reg == 0b001 || modrm.fields.reg >= 0b101)
						return false;
				}
				else if (op >= 0x24 && op <= 0x27)
					return false;
				else if (op >= 0x3b && op <= 0x3f)
					return false;
				else if (_NMD_R(op) == 5)
				{
					if ((op == 0x50 && modrm.fields.mod != 0b11) || (instruction->simd_prefix & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && (op == 0x52 || op == 0x53)) || (instruction->simd_prefix & NMD_X86_PREFIXES_REPEAT && (op == 0x50 || (op >= 0x54 && op <= 0x57))) || (instruction->simd_prefix & NMD_X86_PREFIXES_REPEAT_NOT_ZERO && (op == 0x50 || (op >= 0x52 && op <= 0x57) || op == 0x5b)))
						return false;
				}
				else if (_NMD_R(op) == 6)
				{
					if ((!(instruction->simd_prefix & (NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE | NMD_X86_PREFIXES_REPEAT | NMD_X86_PREFIXES_REPEAT_NOT_ZERO)) && (op == 0x6c || op == 0x6d)) || (instruction->simd_prefix & NMD_X86_PREFIXES_REPEAT && op != 0x6f) || instruction->simd_prefix & NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
						return false;
				}
				else if (op == 0x78 || op == 0x79)
				{
					if ((((instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && op == 0x78) && !(modrm.fields.mod == 0b11 && modrm.fields.reg == 0b000)) || ((instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE || instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO) && modrm.fields.mod != 0b11)) || (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT))
						return false;
				}
				else if (op == 0x7c || op == 0x7d)
				{
					if (instruction->simd_prefix & NMD_X86_PREFIXES_REPEAT || !(instruction->simd_prefix & (NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE | NMD_X86_PREFIXES_REPEAT | NMD_X86_PREFIXES_REPEAT_NOT_ZERO)))
						return false;
				}
				else if (op == 0x7e || op == 0x7f)
				{
					if (instruction->simd_prefix & NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
						return false;
				}
				else if (op >= 0x71 && op <= 0x73)
				{
					if (instruction->simd_prefix & (NMD_X86_PREFIXES_REPEAT | NMD_X86_PREFIXES_REPEAT_NOT_ZERO) || modrm.modrm <= 0xcf || (modrm.modrm >= 0xe8 && modrm.modrm <= 0xef))
						return false;
				}
				else if (op == 0x73)
				{
					if (modrm.modrm >= 0xe0 && modrm.modrm <= 0xe8)
						return false;
				}
				else if (op == 0xa6)
				{
					if (modrm.modrm != 0xc0 && modrm.modrm != 0xc8 && modrm.modrm != 0xd0)
						return false;
				}
				else if (op == 0xa7)
				{
					if (!(modrm.fields.mod == 0b11 && modrm.fields.reg <= 0b101 && modrm.fields.rm == 0b000))
						return false;
				}
				else if (op == 0xae)
				{
					if (((!instruction->simd_prefix && modrm.fields.mod == 0b11 && modrm.fields.reg <= 0b100) || (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO && !(modrm.fields.mod == 0b11 && modrm.fields.reg == 0b110)) || (instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && (modrm.fields.reg < 0b110 || (modrm.fields.mod == 0b11 && modrm.fields.reg == 0b111))) || (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT && (modrm.fields.reg != 0b100 && modrm.fields.reg != 0b110) && !(modrm.fields.mod == 0b11 && modrm.fields.reg == 0b101))))
						return false;
				}
				else if (op == 0xb8)
				{
					if (!(instruction->simd_prefix & NMD_X86_PREFIXES_REPEAT))
						return false;
				}
				else if (op == 0xba)
				{
					if (modrm.fields.reg <= 0b011)
						return false;
				}
				else if (op == 0xd0)
				{
					if (!instruction->simd_prefix || instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT)
						return false;
				}
				else if (op == 0xe0)
				{
					if (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT || instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
						return false;
				}
				else if (op == 0xf0)
				{
					if (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? modrm.fields.mod == 0b11 : true)
						return false;
				}
				else if (instruction->simd_prefix & (NMD_X86_PREFIXES_REPEAT | NMD_X86_PREFIXES_REPEAT_NOT_ZERO))
				{
					if ((op >= 0x13 && op <= 0x17 && !(op == 0x16 && instruction->simd_prefix & NMD_X86_PREFIXES_REPEAT)) || op == 0x28 || op == 0x29 || op == 0x2e || op == 0x2f || (op <= 0x76 && op >= 0x74))
						return false;
				}
				else if (op == 0x71 || op == 0x72 || (op == 0x73 && !(instruction->simd_prefix & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)))
				{
					if ((modrm.modrm >= 0xd8 && modrm.modrm <= 0xdf) || modrm.modrm >= 0xf8)
						return false;
				}
				else if (op >= 0xc3 && op <= 0xc6)
				{
					if ((op == 0xc5 && modrm.fields.mod != 0b11) || (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT || instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO) || (op == 0xc3 && instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE))
						return false;
				}
				else if (_NMD_R(op) >= 0xd && _NMD_C(op) != 0 && op != 0xff && ((_NMD_C(op) == 6 && _NMD_R(op) != 0xf) ? (!instruction->simd_prefix || (_NMD_R(op) == 0xD && (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT || instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO) ? modrm.fields.mod != 0b11 : false)) : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT || instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO || ((_NMD_C(op) == 7 && _NMD_R(op) != 0xe) ? modrm.fields.mod != 0b11 : false))))
					return false;
				else if (modrm.fields.mod == 0b11)
				{
					if (op == 0xb2 || op == 0xb4 || op == 0xb5 || op == 0xc3 || op == 0xe7 || op == 0x2b || (instruction->simd_prefix & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && (op == 0x12 || op == 0x16)) || (!(instruction->simd_prefix & (NMD_X86_PREFIXES_REPEAT | NMD_X86_PREFIXES_REPEAT_NOT_ZERO)) && (op == 0x13 || op == 0x17)))
						return false;
				}
			}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_VALIDITY_CHECK */

			if (_NMD_R(op) == 8) /* imm32 */
				instruction->imm_mask = _NMD_GET_BY_MODE_OPSZPRFX_F64(mode, instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE, NMD_X86_IMM16, NMD_X86_IMM32, NMD_X86_IMM32);
			else if ((_NMD_R(op) == 7 && _NMD_C(op) < 4) || op == 0xA4 || op == 0xC2 || (op > 0xC3 && op <= 0xC6) || op == 0xBA || op == 0xAC) /* imm8 */
				instruction->imm_mask = NMD_X86_IMM8;
			else if (op == 0x78 && (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO || instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)) /* imm8 + imm8 = "imm16" */
				instruction->imm_mask = NMD_X86_IMM16;

			/* Make sure we can read 'instruction->imm_mask' bytes from the buffer */
			if (buffer_size < instruction->imm_mask)
				return false;
			
			/* Copy 'instruction->imm_mask' bytes from the buffer */
			for (i = 0; i < instruction->imm_mask; i++)
				((uint8_t*)(&instruction->immediate))[i] = b[i];
			
			/* Increment the buffer and decrement the buffer's size */
			b += instruction->imm_mask;
			buffer_size -= instruction->imm_mask;
			
			if (_NMD_R(op) == 8 && instruction->immediate & ((uint64_t)(1) << (instruction->imm_mask * 8 - 1)))
				instruction->immediate |= 0xffffffffffffffff << (instruction->imm_mask * 8);

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_INSTRUCTION_ID
			if (flags & NMD_X86_DECODER_FLAGS_INSTRUCTION_ID)
			{
				if (_NMD_R(op) == 8)
					instruction->id = NMD_X86_INSTRUCTION_JO + _NMD_C(op);
				else if (op >= 0xa2 && op <= 0xa5)
					instruction->id = NMD_X86_INSTRUCTION_CPUID + (op - 0xa2);
				else if (op == 0x05)
					instruction->id = NMD_X86_INSTRUCTION_SYSCALL;
				else if (_NMD_R(op) == 4)
					instruction->id = NMD_X86_INSTRUCTION_CMOVO + _NMD_C(op);
				else if (op == 0x00)
					instruction->id = NMD_X86_INSTRUCTION_SLDT + modrm.fields.reg;
				else if (op == 0x01)
				{
					if (modrm.fields.mod == 0b11)
					{
						switch (modrm.fields.reg)
						{
						case 0b000: instruction->id = NMD_X86_INSTRUCTION_VMCALL + modrm.fields.rm; break;
						case 0b001: instruction->id = NMD_X86_INSTRUCTION_MONITOR + modrm.fields.rm; break;
						case 0b010: instruction->id = NMD_X86_INSTRUCTION_XGETBV + modrm.fields.rm; break;
						case 0b011: instruction->id = NMD_X86_INSTRUCTION_VMRUN + modrm.fields.rm; break;
						case 0b100: instruction->id = NMD_X86_INSTRUCTION_SMSW; break;
						case 0b110: instruction->id = NMD_X86_INSTRUCTION_LMSW; break;
						case 0b111: instruction->id = (uint16_t)(modrm.fields.rm == 0b000 ? NMD_X86_INSTRUCTION_SWAPGS : NMD_X86_INSTRUCTION_RDTSCP); break;
						}
					}
					else
						instruction->id = NMD_X86_INSTRUCTION_SGDT + modrm.fields.reg;
				}
				else if (op <= 0x0b)
					instruction->id = NMD_X86_INSTRUCTION_LAR + (op - 2);
				else if (op == 0x19 || (op >= 0x1c && op <= 0x1f))
				{
					if (op == 0x1e && modrm.modrm == 0xfa)
						instruction->id = NMD_X86_INSTRUCTION_ENDBR64;
					else if (op == 0x1e && modrm.modrm == 0xfb)
						instruction->id = NMD_X86_INSTRUCTION_ENDBR32;
					else
						instruction->id = NMD_X86_INSTRUCTION_NOP;
				}
				else if (op >= 0x10 && op <= 0x17)
				{
					switch (instruction->simd_prefix)
					{
					case NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE: instruction->id = NMD_X86_INSTRUCTION_VMOVUPS + _NMD_C(op); break;
					case NMD_X86_PREFIXES_REPEAT: instruction->id = NMD_X86_INSTRUCTION_VMOVUPS + _NMD_C(op); break;
					case NMD_X86_PREFIXES_REPEAT_NOT_ZERO: instruction->id = NMD_X86_INSTRUCTION_VMOVUPS + _NMD_C(op); break;
					default: instruction->id = NMD_X86_INSTRUCTION_VMOVUPS + _NMD_C(op); break;
					}
				}
				else if (op >= 0x20 && op <= 0x23)
					instruction->id = NMD_X86_INSTRUCTION_MOV;
				else if (_NMD_R(op) == 3)
					instruction->id = NMD_X86_INSTRUCTION_WRMSR + _NMD_C(op);
				else if (_NMD_R(op) == 5)
				{
					switch (instruction->simd_prefix)
					{
					case NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE: instruction->id = NMD_X86_INSTRUCTION_MOVMSKPD + _NMD_C(op); break;
					case NMD_X86_PREFIXES_REPEAT: instruction->id = NMD_X86_INSTRUCTION_BNDMOV + _NMD_C(op); break;
					case NMD_X86_PREFIXES_REPEAT_NOT_ZERO: instruction->id = NMD_X86_INSTRUCTION_BNDCL + _NMD_C(op); break;
					default: instruction->id = NMD_X86_INSTRUCTION_MOVMSKPS + _NMD_C(op); break;
					}
				}
				else if (op >= 0x60 && op <= 0x6d)
					instruction->id = NMD_X86_INSTRUCTION_PUNPCKLBW + _NMD_C(op);
				else if (op >= 0x74 && op <= 0x76)
					instruction->id = NMD_X86_INSTRUCTION_PCMPEQB + (op - 0x74);
				else if (op >= 0xb2 && op <= 0xb5)
					instruction->id = NMD_X86_INSTRUCTION_LSS + (op - 0xb2);
				else if (op >= 0xc3 && op <= 0xc5)
					instruction->id = NMD_X86_INSTRUCTION_MOVNTI + (op - 0xc3);
				else if (op == 0xc7)
				{
					if (modrm.fields.reg == 0b001)
						instruction->id = (uint16_t)(instruction->rex_w_prefix ? NMD_X86_INSTRUCTION_CMPXCHG16B : NMD_X86_INSTRUCTION_CMPXCHG8B);
					else if (modrm.fields.reg == 0b111)
						instruction->id = (uint16_t)(modrm.fields.mod == 0b11 ? (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_RDPID : NMD_X86_INSTRUCTION_RDSEED) : NMD_X86_INSTRUCTION_VMPTRST);
					else
						instruction->id = (uint16_t)(modrm.fields.mod == 0b11 ? NMD_X86_INSTRUCTION_RDRAND : (instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_VMCLEAR : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_VMXON : NMD_X86_INSTRUCTION_VMPTRLD)));
				}
				else if (op >= 0xc8 && op <= 0xcf)
					instruction->id = NMD_X86_INSTRUCTION_BSWAP;
				else if (op == 0xa3)
					instruction->id = (uint16_t)((modrm.fields.mod == 0b11 ? NMD_X86_INSTRUCTION_RDFSBASE : NMD_X86_INSTRUCTION_FXSAVE) + modrm.fields.reg);
				else if (op >= 0xd1 && op <= 0xfe)
				{
					if (op == 0xd6)
						instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_MOVQ : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_MOVQ2DQ : NMD_X86_INSTRUCTION_MOVDQ2Q));
					else if (op == 0xe6)
						instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_CVTTPD2DQ : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_CVTDQ2PD : NMD_X86_INSTRUCTION_CVTPD2DQ));
					else if (op == 0xe7)
						instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_MOVNTDQ : NMD_X86_INSTRUCTION_MOVNTQ);
					else if (op == 0xf7)
						instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_MASKMOVDQU : NMD_X86_INSTRUCTION_MASKMOVQ);
					else
						instruction->id = NMD_X86_INSTRUCTION_PSRLW + (op - 0xd1);
				}
				else
				{
					switch (op)
					{
					case 0xa0: case 0xa8: instruction->id = NMD_X86_INSTRUCTION_PUSH; break;
					case 0xa1: case 0xa9: instruction->id = NMD_X86_INSTRUCTION_POP; break;
					case 0xaf: instruction->id = NMD_X86_INSTRUCTION_IMUL; break;
					case 0xb0: case 0xb1: instruction->id = NMD_X86_INSTRUCTION_CMPXCHG; break;
					case 0x10: case 0x11: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_MOVUPD : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_MOVSS : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? NMD_X86_INSTRUCTION_MOVSD : NMD_X86_INSTRUCTION_MOVUPD))); break;
					case 0x12: case 0x13: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_MOVLPD : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_MOVSLDUP : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? NMD_X86_INSTRUCTION_MOVDDUP : NMD_X86_INSTRUCTION_MOVLPS))); break;
					case 0x14: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_UNPCKLPD : NMD_X86_INSTRUCTION_UNPCKLPS); break;
					case 0x15: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_UNPCKHPD : NMD_X86_INSTRUCTION_UNPCKHPS); break;
					case 0x16: case 0x17: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_MOVHPD : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_MOVSHDUP : NMD_X86_INSTRUCTION_MOVHPS)); break;
					case 0x18: instruction->id = (uint16_t)(modrm.fields.reg >= 0b100 ? NMD_X86_INSTRUCTION_NOP : (modrm.fields.reg == 0b000 ? NMD_X86_INSTRUCTION_PREFETCHNTA : (modrm.fields.reg == 0b001 ? NMD_X86_INSTRUCTION_PREFETCHT0 : (modrm.fields.reg == 0b010 ? NMD_X86_INSTRUCTION_PREFETCHT1 : NMD_X86_INSTRUCTION_PREFETCHT2)))); break;
					case 0x1a: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_BNDMOV : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_BNDCL : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? NMD_X86_INSTRUCTION_BNDCU : NMD_X86_INSTRUCTION_BNDLDX))); break;
					case 0x1b: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_BNDMOV : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_BNDMK : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? NMD_X86_INSTRUCTION_BNDCN : NMD_X86_INSTRUCTION_BNDSTX))); break;
					case 0x28: case 0x29: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_MOVAPD : NMD_X86_INSTRUCTION_MOVAPS); break;
					case 0x2a: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_CVTPI2PD : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_CVTSI2SS : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? NMD_X86_INSTRUCTION_CVTSI2SD : NMD_X86_INSTRUCTION_CVTPI2PS))); break;
					case 0x2b: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_MOVNTPD : NMD_X86_INSTRUCTION_MOVNTPS); break;
					case 0x2c: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_CVTTPD2PI : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_CVTTSS2SI : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? NMD_X86_INSTRUCTION_CVTTSS2SI : NMD_X86_INSTRUCTION_CVTTPS2PI))); break;
					case 0x2d: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_CVTPD2PI : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_CVTSS2SI : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? NMD_X86_INSTRUCTION_CVTSS2SI : NMD_X86_INSTRUCTION_CVTPS2PI))); break;
					case 0x2e: case 0x2f: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_UCOMISD : NMD_X86_INSTRUCTION_UCOMISS); break;
					case 0x6e: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && !instruction->rex_w_prefix && (instruction->prefixes & NMD_X86_PREFIXES_REX_W) ? NMD_X86_INSTRUCTION_MOVQ : NMD_X86_INSTRUCTION_MOVD); break;
					case 0x6f: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_MOVDQA : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_MOVDQU : NMD_X86_INSTRUCTION_MOVQ)); break;
					case 0x70: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_PSHUFD : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_PSHUFHW : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? NMD_X86_INSTRUCTION_PSHUFLW : NMD_X86_INSTRUCTION_PSHUFW))); break;
					case 0x71: instruction->id = (uint16_t)(modrm.fields.reg == 0b101 ? NMD_X86_INSTRUCTION_PSRLQ : (modrm.fields.reg == 0b100 ? NMD_X86_INSTRUCTION_PSRAW : NMD_X86_INSTRUCTION_PSLLW)); break;
					case 0x72: instruction->id = (uint16_t)(modrm.fields.reg == 0b101 ? NMD_X86_INSTRUCTION_PSRLD : (modrm.fields.reg == 0b100 ? NMD_X86_INSTRUCTION_PSRAD : NMD_X86_INSTRUCTION_PSLLD)); break;
					case 0x73: instruction->id = (uint16_t)(modrm.fields.reg == 0b010 ? NMD_X86_INSTRUCTION_PSRLQ : (modrm.fields.reg == 0b011 ? NMD_X86_INSTRUCTION_PSRLDQ : (modrm.fields.reg == 0b110 ? NMD_X86_INSTRUCTION_PSLLQ : NMD_X86_INSTRUCTION_PSLLDQ))); break;
					case 0x78: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_EXTRQ : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? NMD_X86_INSTRUCTION_INSERTQ : NMD_X86_INSTRUCTION_VMREAD)); break;
					case 0x79: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_EXTRQ : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? NMD_X86_INSTRUCTION_INSERTQ : NMD_X86_INSTRUCTION_VMWRITE)); break;
					case 0x7c: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_HADDPD : NMD_X86_INSTRUCTION_HADDPS); break;
					case 0x7d: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_HSUBPD : NMD_X86_INSTRUCTION_HSUBPS); break;
					case 0x7e: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT || (instruction->rex_w_prefix && instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE) ? NMD_X86_INSTRUCTION_MOVQ : NMD_X86_INSTRUCTION_MOVD); break;
					case 0x7f: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_MOVDQA : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_MOVDQU : NMD_X86_INSTRUCTION_MOVQ)); break;
					case 0x77: instruction->id = NMD_X86_INSTRUCTION_EMMS; break;
					case 0x0e: instruction->id = NMD_X86_INSTRUCTION_FEMMS; break;
					case 0xa3: instruction->id = NMD_X86_INSTRUCTION_BT; break;
					case 0xa4: case 0xa5: instruction->id = NMD_X86_INSTRUCTION_SHLD; break;
					case 0xaa: instruction->id = NMD_X86_INSTRUCTION_RSM; break;
					case 0xab: instruction->id = NMD_X86_INSTRUCTION_BTS; break;
					case 0xac: case 0xad: instruction->id = NMD_X86_INSTRUCTION_SHRD; break;
					case 0xb6: case 0xb7: instruction->id = NMD_X86_INSTRUCTION_MOVZX; break;
					case 0xb8: instruction->id = NMD_X86_INSTRUCTION_POPCNT; break;
					case 0xb9: instruction->id = NMD_X86_INSTRUCTION_UD1; break;
					case 0xba: instruction->id = (uint16_t)(modrm.fields.reg == 0b100 ? NMD_X86_INSTRUCTION_BT : (modrm.fields.reg == 0b101 ? NMD_X86_INSTRUCTION_BTS : (modrm.fields.reg == 0b110 ? NMD_X86_INSTRUCTION_BTR : NMD_X86_INSTRUCTION_BTC))); break;
					case 0xbb: instruction->id = NMD_X86_INSTRUCTION_BTC; break;
					case 0xbc: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_BSF : NMD_X86_INSTRUCTION_TZCNT); break;
					case 0xbd: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_BSR : NMD_X86_INSTRUCTION_LZCNT); break;
					case 0xbe: case 0xbf: instruction->id = NMD_X86_INSTRUCTION_MOVSX; break;
					case 0xc0: case 0xc1: instruction->id = NMD_X86_INSTRUCTION_XADD; break;
					case 0xc2: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_CMPPD : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_CMPSS : (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? NMD_X86_INSTRUCTION_CMPSD : NMD_X86_INSTRUCTION_CMPPS))); break;
					case 0xd0: instruction->id = (uint16_t)(instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_ADDSUBPD : NMD_X86_INSTRUCTION_ADDSUBPS); break;
					case 0xff: instruction->id = NMD_X86_INSTRUCTION_UD0; break;
					}
				}
			}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_INSTRUCTION_ID */

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_CPU_FLAGS
			if (flags & NMD_X86_DECODER_FLAGS_CPU_FLAGS)
			{
				if (_NMD_R(op) == 4 || _NMD_R(op) == 8 || _NMD_R(op) == 9) /* Conditional Move (CMOVcc),Conditional jump(Jcc),Byte set on condition(SETcc) */
					_nmd_decode_conditional_flag(instruction, _NMD_C(op));
				else if (op == 0x05 || op == 0x07) /* syscall,sysret */
				{
					instruction->cleared_flags.eflags = op == 0x05 ? NMD_X86_EFLAGS_VM | NMD_X86_EFLAGS_RF : NMD_X86_EFLAGS_RF;
					instruction->modified_flags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_TF | NMD_X86_EFLAGS_IF | NMD_X86_EFLAGS_DF | NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_IOPL | NMD_X86_EFLAGS_NT | NMD_X86_EFLAGS_AC | NMD_X86_EFLAGS_VIF | NMD_X86_EFLAGS_VIP | NMD_X86_EFLAGS_ID;
				}
				else if (op == 0x34) /* sysenter */
					instruction->cleared_flags.eflags = NMD_X86_EFLAGS_IF | NMD_X86_EFLAGS_RF | NMD_X86_EFLAGS_VM;
				else if (op == 0xaa) /* rsm */
					instruction->modified_flags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_TF | NMD_X86_EFLAGS_IF | NMD_X86_EFLAGS_DF | NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_IOPL | NMD_X86_EFLAGS_NT | NMD_X86_EFLAGS_RF | NMD_X86_EFLAGS_VM | NMD_X86_EFLAGS_AC | NMD_X86_EFLAGS_VIF | NMD_X86_EFLAGS_VIP | NMD_X86_EFLAGS_ID;
				else if (op == 0xaf) /* mul */
				{
					instruction->modified_flags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_OF;
					instruction->undefined_flags.eflags = NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_SF;
				}
				else if (op == 0xb0 || op == 0xb1) /* cmpxchg */
					instruction->modified_flags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_OF;
				else if (op == 0xc0 || op == 0xc1) /* xadd */
					instruction->modified_flags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_OF;
				else if (op == 0x00 && (modrm.fields.reg == 0b100 || modrm.fields.reg == 0b101)) /* verr,verw*/
					instruction->modified_flags.eflags = NMD_X86_EFLAGS_OF;
				else if (op == 0x01 && modrm.fields.mod == 0b11)
				{
					if (modrm.fields.reg == 0b000)
					{
						if (modrm.fields.rm == 0b001 || modrm.fields.rm == 0b010 || modrm.fields.rm == 0b011) /* vmcall,vmlaunch,vmresume */
						{
							instruction->tested_flags.eflags = NMD_X86_EFLAGS_IOPL | NMD_X86_EFLAGS_VM;
							instruction->modified_flags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_TF | NMD_X86_EFLAGS_IF | NMD_X86_EFLAGS_DF | NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_IOPL | NMD_X86_EFLAGS_NT | NMD_X86_EFLAGS_RF | NMD_X86_EFLAGS_VM | NMD_X86_EFLAGS_AC | NMD_X86_EFLAGS_VIF | NMD_X86_EFLAGS_VIP | NMD_X86_EFLAGS_ID;
						}
					}
				}
				else if (op == 0x34)
					instruction->cleared_flags.eflags = NMD_X86_EFLAGS_VM | NMD_X86_EFLAGS_IF;
				else if (op == 0x78 || op == 0x79) /* vmread,vmwrite */
				{
					instruction->modified_flags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_ZF;
					instruction->cleared_flags.eflags = NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_OF;
				}
				else if (op == 0x02 || op == 0x03) /* lar,lsl */
					instruction->modified_flags.eflags = NMD_X86_EFLAGS_ZF;
				else if (op == 0xa3 || op == 0xab || op == 0xb3 || op == 0xba || op == 0xbb) /* bt,bts,btc */
				{
					instruction->modified_flags.eflags = NMD_X86_EFLAGS_CF;
					instruction->undefined_flags.eflags = NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_PF;
				}
				else if (op == 0xa4 || op == 0xa5 || op == 0xac || op == 0xad || op == 0xbc) /* shld,shrd */
				{
					instruction->modified_flags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_SF;
					instruction->undefined_flags.eflags = NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_OF;
				}
				else if (op == 0xaa) /* rsm */
					instruction->modified_flags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_TF | NMD_X86_EFLAGS_IF | NMD_X86_EFLAGS_DF | NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_IOPL | NMD_X86_EFLAGS_NT | NMD_X86_EFLAGS_RF | NMD_X86_EFLAGS_VM | NMD_X86_EFLAGS_AC | NMD_X86_EFLAGS_VIF | NMD_X86_EFLAGS_VIP | NMD_X86_EFLAGS_ID;
				else if ((op == 0xbc || op == 0xbd) && instruction->prefixes & NMD_X86_PREFIXES_REPEAT) /* tzcnt */
				{
					instruction->modified_flags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_ZF;
					instruction->undefined_flags.eflags = NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_OF;
				}
				else if (op == 0xbc || op == 0xbd) /* bsf */
				{
					instruction->modified_flags.eflags = NMD_X86_EFLAGS_ZF;
					instruction->undefined_flags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_OF;
				}
			}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_CPU_FLAGS */

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_GROUP
			/* Parse the instruction's group. */
			if (flags & NMD_X86_DECODER_FLAGS_GROUP)
			{
				if (_NMD_R(op) == 8)
					instruction->group = NMD_GROUP_JUMP | NMD_GROUP_CONDITIONAL_BRANCH | NMD_GROUP_RELATIVE_ADDRESSING;
				else if ((op == 0x01 && modrm.fields.rm == 0b111 && (modrm.fields.mod == 0b00 || modrm.modrm == 0xf8)) || op == 0x06 || op == 0x08 || op == 0x09 || op == 0x30 || op == 0x32 || op == 0x33 || op == 0x35 || op == 0x37)
					instruction->group = NMD_GROUP_PRIVILEGE;
			}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_GROUP */

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_OPERANDS
			if (flags & NMD_X86_DECODER_FLAGS_OPERANDS)
			{
				if (op == 0x2 || op == 0x3 || (op >= 0x10 && op <= 0x17) || _NMD_R(op) == 2 || (_NMD_R(op) >= 4 && _NMD_R(op) <= 7 && op != 0x77) || op == 0xa3 || op == 0xab || op == 0xaf || (_NMD_R(op) >= 0xc && op != 0xc7 && op != 0xff))
					instruction->num_operands = 2;
				else if (_NMD_R(op) == 9 || op == 0xc7)
					instruction->num_operands = 1;
				else if (op == 0xa4 || op == 0xa5 || op == 0xc2 || (op >= 0xc4 && op <= 0xc6))
					instruction->num_operands = 3;

				if (op == 0x05 || op == 0x07) /* syscall,sysret */
				{
					instruction->num_operands = 5;
					_NMD_SET_REG_OPERAND(instruction->operands[0], true, NMD_X86_OPERAND_ACTION_WRITE, _NMD_GET_IP());
					_NMD_SET_REG_OPERAND(instruction->operands[1], true, op == 0x05 ? NMD_X86_OPERAND_ACTION_WRITE : NMD_X86_OPERAND_ACTION_READ, NMD_X86_REG_RCX);
					_NMD_SET_REG_OPERAND(instruction->operands[2], true, op == 0x05 ? NMD_X86_OPERAND_ACTION_WRITE : NMD_X86_OPERAND_ACTION_READ, NMD_X86_REG_R11);
					_NMD_SET_REG_OPERAND(instruction->operands[3], true, NMD_X86_OPERAND_ACTION_WRITE, NMD_X86_REG_CS);
					_NMD_SET_REG_OPERAND(instruction->operands[4], true, NMD_X86_OPERAND_ACTION_WRITE, NMD_X86_REG_SS);
				}
				else if (op == 0x34 || op == 0x35) /* sysenter,sysexit */
				{
					instruction->num_operands = op == 0x34 ? 2 : 4;
					_NMD_SET_REG_OPERAND(instruction->operands[0], true, NMD_X86_OPERAND_ACTION_WRITE, _NMD_GET_IP());
					_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_WRITE, _NMD_GET_GPR(NMD_X86_REG_SP));
					if (op == 0x35)
					{
						_NMD_SET_REG_OPERAND(instruction->operands[2], true, NMD_X86_OPERAND_ACTION_READ, _NMD_GET_GPR(NMD_X86_REG_CX));
						_NMD_SET_REG_OPERAND(instruction->operands[3], true, NMD_X86_OPERAND_ACTION_READ, _NMD_GET_GPR(NMD_X86_REG_DX));
					}
				}
				else if (_NMD_R(op) == 8) /* jCC rel32 */
				{
					instruction->num_operands = 2;
					_NMD_SET_IMM_OPERAND(instruction->operands[0], false, NMD_X86_OPERAND_ACTION_READ, instruction->immediate);
					_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_READWRITE, _NMD_GET_IP());
				}
				else if (op == 0x31) /* rdtsc */
				{
					instruction->num_operands = 2;
					_NMD_SET_REG_OPERAND(instruction->operands[0], true, NMD_X86_OPERAND_ACTION_WRITE, NMD_X86_REG_EAX);
					_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_WRITE, NMD_X86_REG_EDX);
				}
				else if (op == 0xa2) /* cpuid */
				{
					instruction->num_operands = 4;
					_NMD_SET_REG_OPERAND(instruction->operands[0], true, NMD_X86_OPERAND_ACTION_READWRITE, NMD_X86_REG_EAX);
					_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_WRITE, NMD_X86_REG_EBX);
					_NMD_SET_REG_OPERAND(instruction->operands[2], true, NMD_X86_OPERAND_ACTION_WRITE | NMD_X86_OPERAND_ACTION_CONDREAD, NMD_X86_REG_ECX);
					_NMD_SET_REG_OPERAND(instruction->operands[3], true, NMD_X86_OPERAND_ACTION_WRITE, NMD_X86_REG_EDX);
				}
				else if (op == 0xa0 || op == 0xa8) /* push fs,push gs */
				{
					instruction->num_operands = 3;
					_NMD_SET_REG_OPERAND(instruction->operands[0], false, NMD_X86_OPERAND_ACTION_READ, op == 0xa0 ? NMD_X86_REG_FS : NMD_X86_REG_GS);
					_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_READWRITE, _NMD_GET_GPR(NMD_X86_REG_SP));
					_NMD_SET_MEM_OPERAND(instruction->operands[2], true, NMD_X86_OPERAND_ACTION_WRITE, NMD_X86_REG_SS, _NMD_GET_GPR(NMD_X86_REG_SP), NMD_X86_REG_NONE, 0, 0);
				}
				else if (op == 0x30 || op == 0x32 || op == 0x33) /* wrmsr,rdmsr,rdpmc */
				{
					instruction->num_operands = 3;
					_NMD_SET_REG_OPERAND(instruction->operands[0], true, op == 0x30 ? NMD_X86_OPERAND_ACTION_READ : NMD_X86_OPERAND_ACTION_WRITE, NMD_X86_REG_EAX);
					_NMD_SET_REG_OPERAND(instruction->operands[1], true, op == 0x30 ? NMD_X86_OPERAND_ACTION_READ : NMD_X86_OPERAND_ACTION_WRITE, NMD_X86_REG_EDX);
					_NMD_SET_REG_OPERAND(instruction->operands[2], true, NMD_X86_OPERAND_ACTION_READ, NMD_X86_REG_ECX);
				}
				else if (op == 0xa1 || op == 0xa9) /* pop fs,pop gs */
				{
					instruction->num_operands = 3;
					_NMD_SET_REG_OPERAND(instruction->operands[0], false, NMD_X86_OPERAND_ACTION_WRITE, op == 0xa1 ? NMD_X86_REG_FS : NMD_X86_REG_GS);
					_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_READWRITE, _NMD_GET_GPR(NMD_X86_REG_SP));
					_NMD_SET_MEM_OPERAND(instruction->operands[2], true, NMD_X86_OPERAND_ACTION_READ, NMD_X86_REG_SS, _NMD_GET_GPR(NMD_X86_REG_SP), NMD_X86_REG_NONE, 0, 0);
				}
				else if (op == 0x37) /* getsec */
				{
					instruction->num_operands = 2;
					_NMD_SET_REG_OPERAND(instruction->operands[0], true, NMD_X86_OPERAND_ACTION_READ | NMD_X86_OPERAND_ACTION_CONDWRITE, NMD_X86_REG_EAX);
					_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_READ, NMD_X86_REG_EBX);
				}
				else if (op == 0xaa) /* rsm */
				{
					instruction->num_operands = 1;
					_NMD_SET_REG_OPERAND(instruction->operands[0], true, NMD_X86_OPERAND_ACTION_WRITE, _NMD_GET_IP());
				}
				else if (op == 0x00)
				{
					if (instruction->modrm.fields.reg >= 0b010)
						_nmd_decode_operand_Ew(instruction, &instruction->operands[0]);
					else
						_nmd_decode_operand_Ev(instruction, &instruction->operands[0]);

					instruction->operands[0].action = (uint8_t)(instruction->modrm.fields.reg >= 0b010 ? NMD_X86_OPERAND_ACTION_READ : NMD_X86_OPERAND_ACTION_WRITE);
				}
				else if (op == 0x01)
				{
					if (instruction->modrm.fields.mod != 0b11)
					{
						_nmd_decode_modrm_upper32(instruction, &instruction->operands[0]);
						instruction->operands[0].action = (uint8_t)(instruction->modrm.fields.reg >= 0b010 ? NMD_X86_OPERAND_ACTION_READ : NMD_X86_OPERAND_ACTION_WRITE);
					}
					else if (instruction->modrm.fields.reg == 0b100)
						_nmd_decode_operand_Rv(instruction, &instruction->operands[0]);
					else if (instruction->modrm.fields.reg == 0b110)
					{
						_nmd_decode_operand_Ew(instruction, &instruction->operands[0]);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READ;
					}

					if (instruction->modrm.fields.reg == 0b100)
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
				}
				else if (op == 0x02 || op == 0x03)
				{
					_nmd_decode_operand_Gv(instruction, &instruction->operands[0]);
					_nmd_decode_operand_Ew(instruction, &instruction->operands[1]);
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
				}
				else if (op == 0x0d)
				{
					_nmd_decode_operand_Ev(instruction, &instruction->operands[0]);
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READ;
				}
				else if (_NMD_R(op) == 0x8)
				{
					instruction->operands[0].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
				}
				else if (_NMD_R(op) == 9)
				{
					_nmd_decode_operand_Eb(instruction, &instruction->operands[0]);
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
				}
				else if (op == 0x17)
				{
					_nmd_decode_modrm_upper32(instruction, &instruction->operands[0]);
					_nmd_decode_operand_Vdq(instruction, &instruction->operands[1]);
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
				}
				else if (op >= 0x20 && op <= 0x23)
				{
					instruction->operands[0].type = instruction->operands[1].type = NMD_X86_OPERAND_TYPE_REGISTER;
					instruction->operands[op < 0x22 ? 0 : 1].fields.reg = NMD_X86_REG_EAX + instruction->modrm.fields.rm;
					instruction->operands[op < 0x22 ? 1 : 0].fields.reg = (uint8_t)((op % 2 == 0 ? NMD_X86_REG_CR0 : NMD_X86_REG_DR0) + instruction->modrm.fields.reg);
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
				}
				else if (op == 0x29 || op == 0x2b || (op == 0x7f && instruction->simd_prefix))
				{
					_nmd_decode_operand_Wdq(instruction, &instruction->operands[0]);
					_nmd_decode_operand_Vdq(instruction, &instruction->operands[1]);
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
				}
				else if (op == 0x2a || op == 0x2c || op == 0x2d)
				{
					if (op == 0x2a)
						_nmd_decode_operand_Vdq(instruction, &instruction->operands[0]);
					else if (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT || instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
						_nmd_decode_operand_Gy(instruction, &instruction->operands[0]);
					else if (op == 0x2d && instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
						_nmd_decode_operand_Qq(instruction, &instruction->operands[0]);
					else
						_nmd_decode_operand_Pq(instruction, &instruction->operands[0]);

					if (op == 0x2a)
					{
						if (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT || instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
							_nmd_decode_operand_Ey(instruction, &instruction->operands[1]);
						else
							_nmd_decode_operand_Qq(instruction, &instruction->operands[1]);
					}
					else
						_nmd_decode_operand_Wdq(instruction, &instruction->operands[1]);
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
				}
				else if (op == 0x50)
				{
					_nmd_decode_operand_Gy(instruction, &instruction->operands[0]);
					_nmd_decode_operand_Udq(instruction, &instruction->operands[1]);
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
				}
				else if (_NMD_R(op) == 5 || (op >= 0x10 && op <= 0x16) || op == 0x28 || op == 0x2e || op == 0x2f || (op == 0x7e && instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT))
				{
					_nmd_decode_operand_Vdq(instruction, &instruction->operands[op == 0x11 || op == 0x13 ? 1 : 0]);
					_nmd_decode_operand_Wdq(instruction, &instruction->operands[op == 0x11 || op == 0x13 ? 0 : 1]);
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
				}
				else if (op == 0x7e)
				{
					_nmd_decode_operand_Ey(instruction, &instruction->operands[0]);
					instruction->operands[1].type = NMD_X86_OPERAND_TYPE_REGISTER;
					instruction->operands[1].fields.reg = (uint8_t)((instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_REG_XMM0 : NMD_X86_REG_MM0) + instruction->modrm.fields.reg);
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
				}
				else if (_NMD_R(op) == 6 || op == 0x70 || (op >= 0x74 && op <= 0x76) || (op >= 0x7c && op <= 0x7f))
				{
					if (!instruction->simd_prefix)
					{
						_nmd_decode_operand_Pq(instruction, &instruction->operands[op == 0x7f ? 1 : 0]);

						if (op == 0x6e)
							_nmd_decode_operand_Ey(instruction, &instruction->operands[1]);
						else
							_nmd_decode_operand_Qq(instruction, &instruction->operands[op == 0x7f ? 0 : 1]);
					}
					else
					{
						_nmd_decode_operand_Vdq(instruction, &instruction->operands[0]);

						if (op == 0x6e)
							_nmd_decode_operand_Ey(instruction, &instruction->operands[1]);
						else
							_nmd_decode_operand_Wdq(instruction, &instruction->operands[1]);
					}

					if (op == 0x70)
						instruction->operands[2].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;

					instruction->operands[0].action = (uint8_t)(((op >= 0x60 && op <= 0x6d) || (op >= 0x74 && op <= 0x76)) ? NMD_X86_OPERAND_ACTION_READWRITE : NMD_X86_OPERAND_ACTION_WRITE);
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
				}
				else if (op >= 0x71 && op <= 0x73)
				{
					if (instruction->simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
						_nmd_decode_operand_Udq(instruction, &instruction->operands[0]);
					else
						_nmd_decode_operand_Qq(instruction, &instruction->operands[0]);
					instruction->operands[1].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READWRITE;
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
				}
				else if (op == 0x78 || op == 0x79)
				{
					if (instruction->simd_prefix)
					{
						if (op == 0x78)
						{
							i = 0;
							if (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
								_nmd_decode_operand_Vdq(instruction, &instruction->operands[i++]);
							_nmd_decode_operand_Udq(instruction, &instruction->operands[i + 0]);
							instruction->operands[i + 1].type = instruction->operands[i + 2].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
							/* FIXME: We should not access the buffer from here
							instruction->operands[i + 1].fields.imm = b[1];
							instruction->operands[i + 2].fields.imm = b[2];
							*/
						}
						else
						{
							_nmd_decode_operand_Vdq(instruction, &instruction->operands[0]);
							_nmd_decode_operand_Wdq(instruction, &instruction->operands[1]);
						}
					}
					else
					{
						_nmd_decode_operand_Ey(instruction, &instruction->operands[op == 0x78 ? 0 : 1]);
						_nmd_decode_operand_Gy(instruction, &instruction->operands[op == 0x78 ? 1 : 0]);
					}
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
				}
				else if (_NMD_R(op) == 0xa && (op % 8) < 2)
				{
					instruction->operands[0].type = NMD_X86_OPERAND_TYPE_REGISTER;
					instruction->operands[0].fields.reg = (uint8_t)(op > 0xa8 ? NMD_X86_REG_GS : NMD_X86_REG_FS);
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READ;
				}
				else if ((_NMD_R(op) == 0xa && ((op % 8) >= 3 && (op % 8) <= 5)) || op == 0xb3 || op == 0xbb)
				{
					_nmd_decode_operand_Ev(instruction, &instruction->operands[0]);
					_nmd_decode_operand_Gv(instruction, &instruction->operands[1]);

					if (_NMD_R(op) == 0xa)
					{
						if ((op % 8) == 4)
							instruction->operands[2].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
						else if ((op % 8) == 5)
						{
							instruction->operands[2].type = NMD_X86_OPERAND_TYPE_REGISTER;
							instruction->operands[2].fields.reg = NMD_X86_REG_CL;
						}
					}

					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READWRITE;
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
				}
				else if (op == 0xaf || op == 0xb8)
				{
					_nmd_decode_operand_Gv(instruction, &instruction->operands[0]);
					_nmd_decode_operand_Ev(instruction, &instruction->operands[1]);
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READWRITE;
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
				}
				else if (op == 0xba)
				{
					_nmd_decode_operand_Ev(instruction, &instruction->operands[0]);
					instruction->operands[0].action = (uint8_t)(instruction->modrm.fields.reg <= 0b101 ? NMD_X86_OPERAND_ACTION_READ : NMD_X86_OPERAND_ACTION_READWRITE);
					instruction->operands[1].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
				}
				else if (_NMD_R(op) == 0xb && (op % 8) >= 6)
				{
					_nmd_decode_operand_Gv(instruction, &instruction->operands[0]);
					if ((op % 8) == 6)
						_nmd_decode_operand_Eb(instruction, &instruction->operands[1]);
					else
						_nmd_decode_operand_Ew(instruction, &instruction->operands[1]);
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
				}
				else if (_NMD_R(op) == 0x4 || (_NMD_R(op) == 0xb && ((op % 8) == 0x4 || (op % 8) == 0x5)))
				{
					_nmd_decode_operand_Gv(instruction, &instruction->operands[0]);
					_nmd_decode_operand_Ev(instruction, &instruction->operands[1]);
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
				}
				else if ((_NMD_R(op) == 0xb || _NMD_R(op) == 0xc) && _NMD_C(op) < 2)
				{
					if (_NMD_C(op) == 0)
					{
						_nmd_decode_operand_Eb(instruction, &instruction->operands[0]);
						_nmd_decode_operand_Gb(instruction, &instruction->operands[1]);
					}
					else
					{
						_nmd_decode_operand_Ev(instruction, &instruction->operands[0]);
						_nmd_decode_operand_Gv(instruction, &instruction->operands[1]);
					}

					if (_NMD_R(op) == 0xb)
					{
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READ | NMD_X86_OPERAND_ACTION_CONDWRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else
						instruction->operands[0].action = instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READWRITE;
				}
				else if (op == 0xb2)
				{
					_nmd_decode_operand_Gv(instruction, &instruction->operands[0]);
					_nmd_decode_modrm_upper32(instruction, &instruction->operands[1]);
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
				}
				else if (op == 0xc3)
				{
					_nmd_decode_modrm_upper32(instruction, &instruction->operands[0]);
					_nmd_decode_operand_Gy(instruction, &instruction->operands[1]);
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
				}
				else if (op == 0xc2 || op == 0xc6)
				{
					_nmd_decode_operand_Vdq(instruction, &instruction->operands[0]);
					_nmd_decode_operand_Wdq(instruction, &instruction->operands[1]);
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READWRITE;
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					instruction->operands[2].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
				}
				else if (op == 0xc4)
				{
					if (instruction->prefixes == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
						_nmd_decode_operand_Vdq(instruction, &instruction->operands[0]);
					else
						_nmd_decode_operand_Pq(instruction, &instruction->operands[0]);
					_nmd_decode_operand_Ey(instruction, &instruction->operands[1]);
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					instruction->operands[2].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
				}
				else if (op == 0xc5)
				{
					_nmd_decode_operand_Gd(instruction, &instruction->operands[0]);
					if (instruction->prefixes == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
						_nmd_decode_operand_Udq(instruction, &instruction->operands[1]);
					else
						_nmd_decode_operand_Nq(instruction, &instruction->operands[1]);
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					instruction->operands[2].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
				}
				else if (op == 0xc7)
				{
					if (instruction->modrm.fields.mod == 0b11)
						_nmd_decode_operand_Ev(instruction, &instruction->operands[0]);
					else
						_nmd_decode_modrm_upper32(instruction, &instruction->operands[0]);
					instruction->operands[0].action = (uint8_t)(instruction->modrm.fields.reg == 0b001 ? (NMD_X86_OPERAND_ACTION_READ | NMD_X86_OPERAND_ACTION_CONDWRITE) : (instruction->modrm.fields.mod == 0b11 || !instruction->simd_prefix ? NMD_X86_OPERAND_ACTION_WRITE : NMD_X86_OPERAND_ACTION_READ));
				}
				else if (op >= 0xc8 && op <= 0xcf)
				{
					instruction->operands[0].type = NMD_X86_OPERAND_TYPE_REGISTER;
					instruction->operands[0].fields.reg = (uint8_t)((instruction->prefixes & (NMD_X86_PREFIXES_REX_W | NMD_X86_PREFIXES_REX_B)) == (NMD_X86_PREFIXES_REX_W | NMD_X86_PREFIXES_REX_B) ? NMD_X86_REG_R8 : (instruction->prefixes & NMD_X86_PREFIXES_REX_W ? NMD_X86_REG_RAX : (instruction->prefixes & NMD_X86_PREFIXES_REX_B ? NMD_X86_REG_R8D : NMD_X86_REG_EAX)) + (op % 8));
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READWRITE;
				}
				else if (_NMD_R(op) >= 0xd)
				{
					if (op == 0xff)
					{
						_nmd_decode_operand_Gd(instruction, &instruction->operands[0]);
						_nmd_decode_memory_operand(instruction, &instruction->operands[1], NMD_X86_REG_EAX);
					}
					else if (op == 0xd6 && instruction->simd_prefix != NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
					{
						if (instruction->simd_prefix == NMD_X86_PREFIXES_REPEAT)
						{
							_nmd_decode_operand_Vdq(instruction, &instruction->operands[0]);
							_nmd_decode_operand_Qq(instruction, &instruction->operands[1]);
						}
						else
						{
							_nmd_decode_operand_Pq(instruction, &instruction->operands[0]);
							_nmd_decode_operand_Wdq(instruction, &instruction->operands[1]);
						}
					}
					else
					{
						const size_t first_operand_index = op == 0xe7 || op == 0xd6 ? 1 : 0;
						const size_t second_operand_index = op == 0xe7 || op == 0xd6 ? 0 : 1;

						if (!instruction->simd_prefix)
						{
							if (op == 0xd7)
								_nmd_decode_operand_Gd(instruction, &instruction->operands[0]);
							else
								_nmd_decode_operand_Pq(instruction, &instruction->operands[first_operand_index]);
							_nmd_decode_operand_Qq(instruction, &instruction->operands[second_operand_index]);
						}
						else
						{
							if (op == 0xd7)
								_nmd_decode_operand_Gd(instruction, &instruction->operands[0]);
							else
								_nmd_decode_operand_Vdq(instruction, &instruction->operands[first_operand_index]);
							_nmd_decode_operand_Wdq(instruction, &instruction->operands[second_operand_index]);
						}
					}
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
				}
			}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_OPERANDS */
		}
	}
	else /* 1 byte opcode */
	{
		instruction->opcode_size = 1;
		instruction->opcode = op;
		instruction->opcode_map = NMD_X86_OPCODE_MAP_DEFAULT;

		/* Check for ModR/M, SIB and displacement. */
		if (_NMD_R(op) == 8 || _nmd_find_byte(_nmd_op1_modrm, sizeof(_nmd_op1_modrm), op) || (_NMD_R(op) < 4 && (_NMD_C(op) < 4 || (_NMD_C(op) >= 8 && _NMD_C(op) < 0xC))) || (_NMD_R(op) == 0xD && _NMD_C(op) >= 8) /* FIXME: We should not access the buffer directly from here || (remaining_size > 1 && ((nmd_x86_modrm*)(b + 1))->fields.mod != 0b11 && (op == 0xc4 || op == 0xc5 || op == 0x62)) */)
		{
			if (!_nmd_decode_modrm(&b, &buffer_size, instruction))
				return false;
		}

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_EVEX
		/* Check if instruction is EVEX. */
		if (flags & NMD_X86_DECODER_FLAGS_EVEX && op == 0x62 && !instruction->has_modrm)
		{
			instruction->encoding = NMD_X86_ENCODING_EVEX;
			return false;
		}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_EVEX */
#if !defined(NMD_ASSEMBLY_DISABLE_DECODER_EVEX) && !defined(NMD_ASSEMBLY_DISABLE_DECODER_VEX)
		else
#endif
#ifndef NMD_ASSEMBLY_DISABLE_DECODER_VEX
			/* Check if instruction is VEX. */
			if (flags & NMD_X86_DECODER_FLAGS_VEX && (op == 0xc4 || op == 0xc5) && !instruction->has_modrm)
			{
				instruction->encoding = NMD_X86_ENCODING_VEX;

				instruction->vex.vex[0] = op;

				uint8_t byte1;
				_NMD_READ_BYTE(b, buffer_size, byte1);

				instruction->vex.R = byte1 & 0b10000000;
				if (instruction->vex.vex[0] == 0xc4)
				{
					instruction->vex.X = (byte1 & 0b01000000) == 0b01000000;
					instruction->vex.B = (byte1 & 0b00100000) == 0b00100000;
					instruction->vex.m_mmmm = (uint8_t)(byte1 & 0b00011111);

					uint8_t byte2;
					_NMD_READ_BYTE(b, buffer_size, byte2);
					
					instruction->vex.W = (byte2 & 0b10000000) == 0b10000000;
					instruction->vex.vvvv = (uint8_t)((byte2 & 0b01111000) >> 3);
					instruction->vex.L = (byte2 & 0b00000100) == 0b00000100;
					instruction->vex.pp = (uint8_t)(byte2 & 0b00000011);

					_NMD_READ_BYTE(b, buffer_size, op);
					instruction->opcode = op;

					if (op == 0x0c || op == 0x0d || op == 0x40 || op == 0x41 || op == 0x17 || op == 0x21 || op == 0x42)
                    {
						instruction->imm_mask = NMD_X86_IMM8;
                        _NMD_READ_BYTE(b, buffer_size, instruction->immediate);
                    }

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_VALIDITY_CHECK
					/* Check if the instruction is invalid. */
					if (op == 0x0c && instruction->vex.m_mmmm != 3)
						return false;
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_VALIDITY_CHECK */


#ifndef NMD_ASSEMBLY_DISABLE_DECODER_INSTRUCTION_ID
					/*if(op == 0x0c)
						instruction->id = NMD_X86_INSTR
						*/
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_INSTRUCTION_ID */
				}
				else /* 0xc5 */
				{
					instruction->vex.vvvv = (uint8_t)(byte1 & 0b01111000);
					instruction->vex.L = byte1 & 0b00000100;
					instruction->vex.pp = (uint8_t)(byte1 & 0b00000011);

					_NMD_READ_BYTE(b, buffer_size, op);
					instruction->opcode = op;
				}

				if (!_nmd_decode_modrm(&b, &buffer_size, instruction))
					return false;
			}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_VEX */
#if !(defined(NMD_ASSEMBLY_DISABLE_DECODER_EVEX) && defined(NMD_ASSEMBLY_DISABLE_DECODER_VEX))
			else
#endif
			{
				const nmd_x86_modrm modrm = instruction->modrm;
#ifndef NMD_ASSEMBLY_DISABLE_DECODER_VALIDITY_CHECK
				/* Check if the instruction is invalid. */
				if (flags & NMD_X86_DECODER_FLAGS_VALIDITY_CHECK)
				{
					if (op == 0xC6 || op == 0xC7)
					{
						if ((modrm.fields.reg != 0b000 && modrm.fields.reg != 0b111) || (modrm.fields.reg == 0b111 && (modrm.fields.mod != 0b11 || modrm.fields.rm != 0b000)))
							return false;
					}
					else if (op == 0x8f)
					{
						if (modrm.fields.reg != 0b000)
							return false;
					}
					else if (op == 0xfe)
					{
						if (modrm.fields.reg >= 0b010)
							return false;
					}
					else if (op == 0xff)
					{
						if (modrm.fields.reg == 0b111 || (modrm.fields.mod == 0b11 && (modrm.fields.reg == 0b011 || modrm.fields.reg == 0b101)))
							return false;
					}
					else if (op == 0x8c)
					{
						if (modrm.fields.reg >= 0b110)
							return false;
					}
					else if (op == 0x8e)
					{
						if (modrm.fields.reg == 0b001 || modrm.fields.reg >= 0b110)
							return false;
					}
					else if (op == 0x62)
					{
						if (mode == NMD_X86_MODE_64)
							return false;
					}
					else if (op == 0x8d)
					{
						if (modrm.fields.mod == 0b11)
							return false;
					}
					else if (op == 0xc4 || op == 0xc5)
					{
						if (mode == NMD_X86_MODE_64 && instruction->has_modrm && modrm.fields.mod != 0b11)
							return false;
					}
					else if (op >= 0xd8 && op <= 0xdf)
					{
						switch (op)
						{
						case 0xd9:
							if ((modrm.fields.reg == 0b001 && modrm.fields.mod != 0b11) || (modrm.modrm > 0xd0 && modrm.modrm < 0xd8) || modrm.modrm == 0xe2 || modrm.modrm == 0xe3 || modrm.modrm == 0xe6 || modrm.modrm == 0xe7 || modrm.modrm == 0xef)
								return false;
							break;
						case 0xda:
							if (modrm.modrm >= 0xe0 && modrm.modrm != 0xe9)
								return false;
							break;
						case 0xdb:
							if (((modrm.fields.reg == 0b100 || modrm.fields.reg == 0b110) && modrm.fields.mod != 0b11) || (modrm.modrm >= 0xe5 && modrm.modrm <= 0xe7) || modrm.modrm >= 0xf8)
								return false;
							break;
						case 0xdd:
							if ((modrm.fields.reg == 0b101 && modrm.fields.mod != 0b11) || _NMD_R(modrm.modrm) == 0xf)
								return false;
							break;
						case 0xde:
							if (modrm.modrm == 0xd8 || (modrm.modrm >= 0xda && modrm.modrm <= 0xdf))
								return false;
							break;
						case 0xdf:
							if ((modrm.modrm >= 0xe1 && modrm.modrm <= 0xe7) || modrm.modrm >= 0xf8)
								return false;
							break;
						}
					}
					else if (mode == NMD_X86_MODE_64)
					{
						if (op == 0x6 || op == 0x7 || op == 0xe || op == 0x16 || op == 0x17 || op == 0x1e || op == 0x1f || op == 0x27 || op == 0x2f || op == 0x37 || op == 0x3f || (op >= 0x60 && op <= 0x62) || op == 0x82 || op == 0xce || (op >= 0xd4 && op <= 0xd6))
							return false;
					}
				}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_VALIDITY_CHECK */

				/* Check for immediate */		
				if (_nmd_find_byte(_nmd_op1_imm32, sizeof(_nmd_op1_imm32), op) || (_NMD_R(op) < 4 && (_NMD_C(op) == 5 || _NMD_C(op) == 0xD)) || (_NMD_R(op) == 0xB && _NMD_C(op) >= 8) || (op == 0xF7 && modrm.fields.reg == 0b000)) /* imm32,16 */
				{
					if (_NMD_R(op) == 0xB && _NMD_C(op) >= 8)
						instruction->imm_mask = (uint8_t)(instruction->prefixes & NMD_X86_PREFIXES_REX_W ? NMD_X86_IMM64 : (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE || (mode == NMD_X86_MODE_16 && !(instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)) ? NMD_X86_IMM16 : NMD_X86_IMM32));
					else
					{
						if ((mode == NMD_X86_MODE_16 && instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE) || (mode != NMD_X86_MODE_16 && !(instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)))
							instruction->imm_mask = NMD_X86_IMM32;
						else
							instruction->imm_mask = NMD_X86_IMM16;
					}
				}
				else if (_NMD_R(op) == 7 || (_NMD_R(op) == 0xE && _NMD_C(op) < 8) || (_NMD_R(op) == 0xB && _NMD_C(op) < 8) || (_NMD_R(op) < 4 && (_NMD_C(op) == 4 || _NMD_C(op) == 0xC)) || (op == 0xF6 && modrm.fields.reg <= 0b001) || _nmd_find_byte(_nmd_op1_imm8, sizeof(_nmd_op1_imm8), op)) /* imm8 */
					instruction->imm_mask = NMD_X86_IMM8;
				else if (_NMD_R(op) == 0xA && _NMD_C(op) < 4)
					instruction->imm_mask = (uint8_t)(mode == NMD_X86_MODE_64 ? (instruction->prefixes & NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE ? NMD_X86_IMM32 : NMD_X86_IMM64) : (instruction->prefixes & NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE ? NMD_X86_IMM16 : NMD_X86_IMM32));
				else if (op == 0xEA || op == 0x9A) /* imm32,48 */
				{
					if (mode == NMD_X86_MODE_64)
						return false;
					instruction->imm_mask = (uint8_t)(instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_IMM32 : NMD_X86_IMM48);
				}
				else if (op == 0xC2 || op == 0xCA) /* imm16 */
					instruction->imm_mask = NMD_X86_IMM16;
				else if (op == 0xC8) /* imm16 + imm8 */
					instruction->imm_mask = NMD_X86_IMM16 | NMD_X86_IMM8;
				
				/* Make sure we can read 'instruction->imm_mask' bytes from the buffer */
				if (buffer_size < instruction->imm_mask)
					return false;

				/* Copy 'instruction->imm_mask' bytes from the buffer */
				for (i = 0; i < instruction->imm_mask; i++)
					((uint8_t*)(&instruction->immediate))[i] = b[i];
				
				/* Increment the buffer and decrement the buffer's size */
				b += instruction->imm_mask;
				buffer_size -= instruction->imm_mask;

				/* Sign extend immediate for specific instructions */
				if (op == 0xe9 || op == 0xeb || op == 0xe8 || _NMD_R(op) == 7)
				{
					if (instruction->immediate & ((uint64_t)(1) << (instruction->imm_mask * 8 - 1)))
						instruction->immediate |= 0xffffffffffffffff << (instruction->imm_mask * 8);
				}
				else if (op == 0x68 && mode == NMD_X86_MODE_64 && instruction->immediate & ((uint64_t)1<<31))
					instruction->immediate |= 0xffffffff00000000;

				/* These are optional features */
#ifndef NMD_ASSEMBLY_DISABLE_DECODER_INSTRUCTION_ID
				if (flags & NMD_X86_DECODER_FLAGS_INSTRUCTION_ID)
				{
					const bool opszprfx = instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE;
					if ((op >= 0x88 && op <= 0x8c) || (op >= 0xa0 && op <= 0xa3) || _NMD_R(op) == 0xb || op == 0x8e)
						instruction->id = NMD_X86_INSTRUCTION_MOV;
					else if (_NMD_R(op) == 5)
						instruction->id = (uint16_t)((_NMD_C(op) < 8) ? NMD_X86_INSTRUCTION_PUSH : NMD_X86_INSTRUCTION_POP);
					else if (_NMD_R(op) < 4 && (op % 8 < 6))
						instruction->id = (NMD_X86_INSTRUCTION_ADD + (_NMD_R(op) << 1) + (_NMD_C(op) >= 8 ? 1 : 0));
					else if (op >= 0x80 && op <= 0x84)
						instruction->id = NMD_X86_INSTRUCTION_ADD + modrm.fields.reg;
					else if (op == 0xe8)
						instruction->id = NMD_X86_INSTRUCTION_CALL;
					else if (op == 0xcc)
						instruction->id = NMD_X86_INSTRUCTION_INT3;
					else if (op == 0x8d)
						instruction->id = NMD_X86_INSTRUCTION_LEA;
					else if (_NMD_R(op) == 4)
						instruction->id = (uint16_t)((_NMD_C(op) < 8) ? NMD_X86_INSTRUCTION_INC : NMD_X86_INSTRUCTION_DEC);
					else if (_NMD_R(op) == 7)
						instruction->id = NMD_X86_INSTRUCTION_JO + _NMD_C(op);
					else if (op == 0xff)
						instruction->id = NMD_X86_INSTRUCTION_INC + modrm.fields.reg;
					else if (op == 0xeb || op == 0xe9)
						instruction->id = NMD_X86_INSTRUCTION_JMP;
					else if (op == 0x90)
					{
						if (instruction->prefixes & NMD_X86_PREFIXES_REPEAT)
							instruction->id = NMD_X86_INSTRUCTION_PAUSE;
						else if (instruction->prefixes & NMD_X86_PREFIXES_REX_B)
							instruction->id = NMD_X86_INSTRUCTION_XCHG;
						else
							instruction->id = NMD_X86_INSTRUCTION_NOP;
					}
					else if (op == 0xc3 || op == 0xc2)
						instruction->id = NMD_X86_INSTRUCTION_RET;
					else if ((op >= 0x91 && op <= 0x97) || op == 0x86 || op == 0x87)
						instruction->id = NMD_X86_INSTRUCTION_XCHG;
					else if (op == 0xc0 || op == 0xc1 || (op >= 0xd0 && op <= 0xd3))
						instruction->id = NMD_X86_INSTRUCTION_ROL + modrm.fields.reg;
					else if (_NMD_R(op) == 0x0f && (op % 8 < 6))
						instruction->id = NMD_X86_INSTRUCTION_INT1 + (op - 0xf1);
					else if (op >= 0xd4 && op <= 0xd7)
						instruction->id = NMD_X86_INSTRUCTION_AAM + (op - 0xd4);
					else if (op >= 0xe0 && op <= 0xe3)
						instruction->id = NMD_X86_INSTRUCTION_LOOPNE + (op - 0xe0);
					else /* case 0x: instruction->id = NMD_X86_INSTRUCTION_; break; */
					{
						switch (op)
						{
						case 0x8f: instruction->id = NMD_X86_INSTRUCTION_POP; break;
						case 0xfe: instruction->id = (uint16_t)(modrm.fields.reg == 0b000 ? NMD_X86_INSTRUCTION_INC : NMD_X86_INSTRUCTION_DEC); break;
						case 0x84: case 0x85: case 0xa8: case 0xa9: instruction->id = NMD_X86_INSTRUCTION_TEST; break;
						case 0xf6: case 0xf7: instruction->id = NMD_X86_INSTRUCTION_TEST + modrm.fields.reg; break;
						case 0x69: case 0x6b: instruction->id = NMD_X86_INSTRUCTION_IMUL; break;
						case 0x9a: instruction->id = NMD_X86_INSTRUCTION_CALL; break;
						case 0x62: instruction->id = NMD_X86_INSTRUCTION_BOUND; break;
						case 0x63: instruction->id = (uint16_t)(mode == NMD_X86_MODE_64 ? NMD_X86_INSTRUCTION_MOVSXD : NMD_X86_INSTRUCTION_ARPL); break;
						case 0x68: case 0x6a: case 0x06: case 0x16: case 0x1e: case 0x0e: instruction->id = NMD_X86_INSTRUCTION_PUSH; break;
						case 0x6c: instruction->id = NMD_X86_INSTRUCTION_INSB; break;
						case 0x6d: instruction->id = (uint16_t)(opszprfx ? NMD_X86_INSTRUCTION_INSW : NMD_X86_INSTRUCTION_INSD); break;
						case 0x6e: instruction->id = NMD_X86_INSTRUCTION_OUTSB; break;
						case 0x6f: instruction->id = (uint16_t)(opszprfx ? NMD_X86_INSTRUCTION_OUTSW : NMD_X86_INSTRUCTION_OUTSD); break;
						case 0xc4: instruction->id = NMD_X86_INSTRUCTION_LES; break;
						case 0xc5: instruction->id = NMD_X86_INSTRUCTION_LDS; break;
						case 0xc6: case 0xc7: instruction->id = (uint16_t)(modrm.fields.reg == 0b000 ? NMD_X86_INSTRUCTION_MOV : (instruction->opcode == 0xc6 ? NMD_X86_INSTRUCTION_XABORT : NMD_X86_INSTRUCTION_XBEGIN)); break;
						case 0xc8: instruction->id = NMD_X86_INSTRUCTION_ENTER; break;
						case 0xc9: instruction->id = NMD_X86_INSTRUCTION_LEAVE; break;
						case 0xca: case 0xcb: instruction->id = NMD_X86_INSTRUCTION_RETF; break;
						case 0xcd: instruction->id = NMD_X86_INSTRUCTION_INT; break;
						case 0xce: instruction->id = NMD_X86_INSTRUCTION_INTO; break;
						case 0xcf: 
							if (instruction->rex_w_prefix)
								instruction->id = NMD_X86_INSTRUCTION_IRETQ;
							else if (mode == NMD_X86_MODE_16)
								instruction->id = (uint16_t)(opszprfx ? NMD_X86_INSTRUCTION_IRETD : NMD_X86_INSTRUCTION_IRET);
							else
								instruction->id = (uint16_t)(opszprfx ? NMD_X86_INSTRUCTION_IRET : NMD_X86_INSTRUCTION_IRETD);
							break;
						case 0xe4: case 0xe5: case 0xec: case 0xed: instruction->id = NMD_X86_INSTRUCTION_IN; break;
						case 0xe6: case 0xe7: case 0xee: case 0xef: instruction->id = NMD_X86_INSTRUCTION_OUT; break;
						case 0xea: instruction->id = NMD_X86_INSTRUCTION_LJMP; break;

						case 0x9c:
							if (opszprfx)
								instruction->id = (uint16_t)(mode == NMD_X86_MODE_16 ? NMD_X86_INSTRUCTION_PUSHFD : NMD_X86_INSTRUCTION_PUSHF);
							else
								instruction->id = (uint16_t)(mode == NMD_X86_MODE_16 ? NMD_X86_INSTRUCTION_PUSHF : (mode == NMD_X86_MODE_32 ? NMD_X86_INSTRUCTION_PUSHFD : NMD_X86_INSTRUCTION_PUSHFQ));
							break;
						case 0x9d:
							if (opszprfx)
								instruction->id = (uint16_t)(mode == NMD_X86_MODE_16 ? NMD_X86_INSTRUCTION_POPFD : NMD_X86_INSTRUCTION_POPF);
							else
								instruction->id = (uint16_t)(mode == NMD_X86_MODE_16 ? NMD_X86_INSTRUCTION_POPF : (mode == NMD_X86_MODE_32 ? NMD_X86_INSTRUCTION_POPFD : NMD_X86_INSTRUCTION_POPFQ));
							break;
						case 0x60: instruction->id = _NMD_GET_BY_MODE_OPSZPRFX(mode, opszprfx, NMD_X86_INSTRUCTION_PUSHA, NMD_X86_INSTRUCTION_PUSHAD); break;
						case 0x61: instruction->id = _NMD_GET_BY_MODE_OPSZPRFX(mode, opszprfx, NMD_X86_INSTRUCTION_POPA, NMD_X86_INSTRUCTION_POPAD); break;
						case 0x07: case 0x17: case 0x1f: instruction->id = NMD_X86_INSTRUCTION_POP; break;
						case 0x27: instruction->id = NMD_X86_INSTRUCTION_DAA; break;
						case 0x37: instruction->id = NMD_X86_INSTRUCTION_AAA; break;
						case 0x2f: instruction->id = NMD_X86_INSTRUCTION_DAS; break;
						case 0x3f: instruction->id = NMD_X86_INSTRUCTION_AAS; break;
						case 0x9b: instruction->id = NMD_X86_INSTRUCTION_FWAIT; break;
						case 0x9e: instruction->id = NMD_X86_INSTRUCTION_SAHF; break;
						case 0x9f: instruction->id = NMD_X86_INSTRUCTION_LAHF; break;
						case 0xA4: instruction->id = NMD_X86_INSTRUCTION_MOVSB; break;
						case 0xA5: instruction->id = _NMD_GET_BY_MODE_OPSZPRFX_W64(mode, opszprfx, instruction->rex_w_prefix, NMD_X86_INSTRUCTION_MOVSW, NMD_X86_INSTRUCTION_MOVSD, NMD_X86_INSTRUCTION_MOVSQ); break; /*(uint16_t)(instruction->rex_w_prefix ? NMD_X86_INSTRUCTION_MOVSQ : (opszprfx ? NMD_X86_INSTRUCTION_MOVSW : NMD_X86_INSTRUCTION_MOVSD)); break;*/
						case 0xA6: instruction->id = NMD_X86_INSTRUCTION_CMPSB; break;
						case 0xA7: instruction->id = _NMD_GET_BY_MODE_OPSZPRFX_W64(mode, opszprfx, instruction->rex_w_prefix, NMD_X86_INSTRUCTION_CMPSW, NMD_X86_INSTRUCTION_CMPSD, NMD_X86_INSTRUCTION_CMPSQ); break;
						case 0xAA: instruction->id = NMD_X86_INSTRUCTION_STOSB; break;
						case 0xAB: instruction->id = _NMD_GET_BY_MODE_OPSZPRFX_W64(mode, opszprfx, instruction->rex_w_prefix, NMD_X86_INSTRUCTION_STOSW, NMD_X86_INSTRUCTION_STOSD, NMD_X86_INSTRUCTION_STOSQ); break;
						case 0xAC: instruction->id = NMD_X86_INSTRUCTION_LODSB; break;
						case 0xAD: instruction->id = _NMD_GET_BY_MODE_OPSZPRFX_W64(mode, opszprfx, instruction->rex_w_prefix, NMD_X86_INSTRUCTION_LODSW, NMD_X86_INSTRUCTION_LODSD, NMD_X86_INSTRUCTION_LODSQ); break;
						case 0xAE: instruction->id = NMD_X86_INSTRUCTION_SCASB; break;
						case 0xAF: instruction->id = _NMD_GET_BY_MODE_OPSZPRFX_W64(mode, opszprfx, instruction->rex_w_prefix, NMD_X86_INSTRUCTION_SCASW, NMD_X86_INSTRUCTION_SCASD, NMD_X86_INSTRUCTION_SCASQ); break;
						case 0x98:
							if(instruction->prefixes & NMD_X86_PREFIXES_REX_W)
								instruction->id = (uint16_t)NMD_X86_INSTRUCTION_CDQE;
							else if(instruction->mode == NMD_X86_MODE_16)
								instruction->id = (uint16_t)(opszprfx ? NMD_X86_INSTRUCTION_CWDE : NMD_X86_INSTRUCTION_CBW);
							else
								instruction->id = (uint16_t)(opszprfx ? NMD_X86_INSTRUCTION_CBW : NMD_X86_INSTRUCTION_CWDE);
							break;
						case 0x99:
							if (instruction->prefixes & NMD_X86_PREFIXES_REX_W)
								instruction->id = (uint16_t)NMD_X86_INSTRUCTION_CQO;
							else if (instruction->mode == NMD_X86_MODE_16)
								instruction->id = (uint16_t)(opszprfx ? NMD_X86_INSTRUCTION_CDQ : NMD_X86_INSTRUCTION_CWD);
							else
								instruction->id = (uint16_t)(opszprfx ? NMD_X86_INSTRUCTION_CWD : NMD_X86_INSTRUCTION_CDQ);
							break;

						/* Floating-point opcodes. */
#define _NMD_F_OP_GET_OFFSET() ((_NMD_R(modrm.modrm) - 0xc) << 1) + (_NMD_C(op) >= 8 ? 1 : 0)
						case 0xd8: instruction->id = (NMD_X86_INSTRUCTION_FADD + (modrm.fields.mod == 0b11 ? _NMD_F_OP_GET_OFFSET() : modrm.fields.reg)); break;
						case 0xd9:
							if (modrm.fields.mod == 0b11)
							{
								if (modrm.modrm <= 0xcf)
									instruction->id = (uint16_t)(modrm.modrm <= 0xc7 ? NMD_X86_INSTRUCTION_FLD : NMD_X86_INSTRUCTION_FXCH);
								else if (modrm.modrm >= 0xd8 && modrm.modrm <= 0xdf)
									instruction->id = NMD_X86_INSTRUCTION_FSTPNCE;
								else if (modrm.modrm == 0xd0)
									instruction->id = NMD_X86_INSTRUCTION_FNOP;
								else
									instruction->id = NMD_X86_INSTRUCTION_FCHS + (modrm.modrm - 0xe0);
							}
							else
								instruction->id = NMD_X86_INSTRUCTION_FLD + modrm.fields.reg;
							break;
						case 0xda:
							if (modrm.fields.mod == 0b11)
								instruction->id = ((modrm.modrm == 0xe9) ? NMD_X86_INSTRUCTION_FUCOMPP : NMD_X86_INSTRUCTION_FCMOVB + _NMD_F_OP_GET_OFFSET());
							else
								instruction->id = NMD_X86_INSTRUCTION_FIADD + modrm.fields.reg;
							break;
						case 0xdb:
							if (modrm.fields.mod == 0b11)
								instruction->id = (modrm.modrm == 0xe2 ? NMD_X86_INSTRUCTION_FNCLEX : (modrm.modrm == 0xe2 ? NMD_X86_INSTRUCTION_FNINIT : NMD_X86_INSTRUCTION_FCMOVNB + _NMD_F_OP_GET_OFFSET()));
							else
								instruction->id = (modrm.fields.reg == 0b101 ? NMD_X86_INSTRUCTION_FLD : (modrm.fields.reg == 0b111 ? NMD_X86_INSTRUCTION_FSTP : NMD_X86_INSTRUCTION_FILD + modrm.fields.reg));
							break;
						case 0xdc:
							if (modrm.fields.mod == 0b11)
								instruction->id = (NMD_X86_INSTRUCTION_FADD + ((_NMD_R(modrm.modrm) - 0xc) << 1) + ((_NMD_C(modrm.modrm) >= 8 && _NMD_R(modrm.modrm) <= 0xd) ? 1 : 0));
							else
								instruction->id = NMD_X86_INSTRUCTION_FADD + modrm.fields.reg;
							break;
						case 0xdd:
							if (modrm.fields.mod == 0b11)
							{
								switch ((modrm.modrm - 0xc0) >> 3)
								{
								case 0b000: instruction->id = NMD_X86_INSTRUCTION_FFREE; break;
								case 0b001: instruction->id = NMD_X86_INSTRUCTION_FXCH; break;
								case 0b010: instruction->id = NMD_X86_INSTRUCTION_FST; break;
								case 0b011: instruction->id = NMD_X86_INSTRUCTION_FSTP; break;
								case 0b100: instruction->id = NMD_X86_INSTRUCTION_FUCOM; break;
								case 0b101: instruction->id = NMD_X86_INSTRUCTION_FUCOMP; break;
								}
							}
							else
							{
								switch (modrm.fields.reg)
								{
								case 0b000: instruction->id = NMD_X86_INSTRUCTION_FLD; break;
								case 0b001: instruction->id = NMD_X86_INSTRUCTION_FISTTP; break;
								case 0b010: instruction->id = NMD_X86_INSTRUCTION_FST; break;
								case 0b011: instruction->id = NMD_X86_INSTRUCTION_FSTP; break;
								case 0b100: instruction->id = NMD_X86_INSTRUCTION_FRSTOR; break;
								case 0b110: instruction->id = NMD_X86_INSTRUCTION_FNSAVE; break;
								case 0b111: instruction->id = NMD_X86_INSTRUCTION_FNSTSW; break;
								}
							}
							break;
						case 0xde:
							if (modrm.fields.mod == 0b11)
								instruction->id = (modrm.modrm == 0xd9 ? NMD_X86_INSTRUCTION_FCOMPP : ((modrm.modrm >= 0xd0 && modrm.modrm <= 0xd7) ? NMD_X86_INSTRUCTION_FCOMP : NMD_X86_INSTRUCTION_FADDP + _NMD_F_OP_GET_OFFSET()));
							else
								instruction->id = NMD_X86_INSTRUCTION_FIADD + modrm.fields.reg;
							break;
						case 0xdf:
							if (modrm.fields.mod == 0b11)
							{
								if (modrm.fields.reg == 0b000)
									instruction->id = NMD_X86_INSTRUCTION_FFREEP;
								else if (modrm.fields.reg == 0b001)
									instruction->id = NMD_X86_INSTRUCTION_FXCH;
								else if (modrm.fields.reg <= 3)
									instruction->id = NMD_X86_INSTRUCTION_FSTP;
								else if (modrm.modrm == 0xe0)
									instruction->id = NMD_X86_INSTRUCTION_FNSTSW;
								else if (modrm.fields.reg == 0b110)
									instruction->id = NMD_X86_INSTRUCTION_FCOMIP;
								else
									instruction->id = NMD_X86_INSTRUCTION_FUCOMIP;
							}
							else
								instruction->id = (modrm.fields.reg == 0b101 ? NMD_X86_INSTRUCTION_FILD : (modrm.fields.reg == 0b111 ? NMD_X86_INSTRUCTION_FISTP : (NMD_X86_INSTRUCTION_FILD + modrm.fields.reg)));
							break;
						}
					}
				}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_INSTRUCTION_ID */

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_CPU_FLAGS
				if (flags & NMD_X86_DECODER_FLAGS_CPU_FLAGS)
				{
					if (op == 0xcc || op == 0xcd) /* int3,int n */
					{
						instruction->cleared_flags.eflags = NMD_X86_EFLAGS_TF | NMD_X86_EFLAGS_RF;
						instruction->tested_flags.eflags = NMD_X86_EFLAGS_IOPL | NMD_X86_EFLAGS_VM;
						instruction->modified_flags.eflags = NMD_X86_EFLAGS_IF | NMD_X86_EFLAGS_NT | NMD_X86_EFLAGS_VM | NMD_X86_EFLAGS_AC | NMD_X86_EFLAGS_VIF;
					}
					else if (op == 0xce) /* into */
					{
						instruction->cleared_flags.eflags = NMD_X86_EFLAGS_RF;
						instruction->tested_flags.eflags = NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_IOPL | NMD_X86_EFLAGS_VM;
						instruction->modified_flags.eflags = NMD_X86_EFLAGS_TF | NMD_X86_EFLAGS_IF | NMD_X86_EFLAGS_NT | NMD_X86_EFLAGS_VM | NMD_X86_EFLAGS_AC;
					}
					else if (_NMD_R(op) == 7) /* conditional jump */
						_nmd_decode_conditional_flag(instruction, _NMD_C(op));
					else if (_NMD_R(op) == 4 || ((op == 0xfe || op == 0xff) && modrm.fields.reg <= 0b001)) /* inc,dec */
						instruction->modified_flags.eflags = NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_PF;
					else if (op <= 0x05 || (op >= 0x10 && op <= 0x15) || ((_NMD_R(op) == 1 || _NMD_R(op) == 2 || _NMD_R(op) == 3) && (_NMD_C(op) >= 0x8 && _NMD_C(op) <= 0x0d)) || ((op >= 0x80 && op <= 0x83) && (modrm.fields.reg == 0b000 || modrm.fields.reg == 0b010 || modrm.fields.reg == 0b011 || modrm.fields.reg == 0b010 || modrm.fields.reg == 0b101 || modrm.fields.reg == 0b111)) || (op == 0xa6 || op == 0xa7) || (op == 0xae || op == 0xaf)) /* add,adc,sbb,sub,cmp, cmps,cmpsb,cmpsw,cmpsd,cmpsq, scas,scasb,scasw,scasd */
						instruction->modified_flags.eflags = NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_PF;
					else if (op == 0x9c) /* pushf,pushfd,pushfq */
						instruction->tested_flags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_TF | NMD_X86_EFLAGS_IF | NMD_X86_EFLAGS_DF | NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_IOPL | NMD_X86_EFLAGS_NT | NMD_X86_EFLAGS_RF | NMD_X86_EFLAGS_VM | NMD_X86_EFLAGS_AC | NMD_X86_EFLAGS_VIF | NMD_X86_EFLAGS_VIP | NMD_X86_EFLAGS_ID;
					else if (op == 0x9d) /* popf,popfd,popfq */
					{
						instruction->modified_flags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_TF | NMD_X86_EFLAGS_IF | NMD_X86_EFLAGS_DF | NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_IOPL | NMD_X86_EFLAGS_NT | NMD_X86_EFLAGS_AC | NMD_X86_EFLAGS_VIF | NMD_X86_EFLAGS_ID;
						instruction->tested_flags.eflags = NMD_X86_EFLAGS_IOPL | NMD_X86_EFLAGS_VM | NMD_X86_EFLAGS_VIP;
						instruction->cleared_flags.eflags = NMD_X86_EFLAGS_RF;
					}
					else if (op == 0xcf) /* iret,iretd,iretf */
					{
						instruction->modified_flags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_TF | NMD_X86_EFLAGS_IF | NMD_X86_EFLAGS_DF | NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_IOPL | NMD_X86_EFLAGS_NT | NMD_X86_EFLAGS_RF | NMD_X86_EFLAGS_VM | NMD_X86_EFLAGS_AC | NMD_X86_EFLAGS_VIF | NMD_X86_EFLAGS_VIP | NMD_X86_EFLAGS_ID;
						instruction->tested_flags.eflags = NMD_X86_EFLAGS_IOPL | NMD_X86_EFLAGS_NT | NMD_X86_EFLAGS_VM;
					}
					else if ((op >= 0x08 && op <= 0x0d) || ((_NMD_R(op) == 2 || _NMD_R(op) == 3) && _NMD_C(op) <= 5) || ((op >= 0x80 && op <= 0x83) && (modrm.fields.reg == 0b001 || modrm.fields.reg == 0b100 || modrm.fields.reg == 0b110)) || (op == 0x84 || op == 0x85 || op == 0xa8 || op == 0xa9) || ((op == 0xf6 || op == 0xf7) && modrm.fields.reg == 0b000)) /* or,and,xor, test */
					{
						instruction->cleared_flags.eflags = NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_CF;
						instruction->undefined_flags.eflags = NMD_X86_EFLAGS_AF;
						instruction->modified_flags.eflags = NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_PF;
					}
					else if (op == 0x69 || op == 0x6b || ((op == 0xf6 || op == 0xf7) && (modrm.fields.reg == 0b100 || modrm.fields.reg == 0b101))) /* mul,imul */
					{
						instruction->modified_flags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_OF;
						instruction->undefined_flags.eflags = NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_PF;
					}
					else if (op == 0xf6 || op == 0xf7) /* Group 3 */
					{
						if (modrm.fields.reg == 0b011) /* neg */
							instruction->modified_flags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_OF;
						else if (modrm.fields.reg >= 0b110) /* div,idiv */
							instruction->undefined_flags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_PF;
					}
					else if (op == 0xc0 || op == 0xc1 || (op >= 0xd0 && op <= 0xd3))
					{
						if (modrm.fields.reg <= 0b011) /* rol,ror,rcl,rcr */
						{
							instruction->modified_flags.eflags = NMD_X86_EFLAGS_CF;
							instruction->undefined_flags.eflags = NMD_X86_EFLAGS_OF;
						}
						else /* shl,shr,sar */
						{
							instruction->modified_flags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_OF;
							instruction->undefined_flags.eflags = NMD_X86_EFLAGS_AF;
						}
					}
					else if (op == 0x27 || op == 0x2f) /* daa,das */
					{
						instruction->tested_flags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_AF;
						instruction->modified_flags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_PF;
						instruction->undefined_flags.eflags = NMD_X86_EFLAGS_OF;
					}
					else if (op == 0x37 || op == 0x3f) /* aaa,aas */
					{
						instruction->tested_flags.eflags = NMD_X86_EFLAGS_AF;
						instruction->modified_flags.eflags = NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_CF;
						instruction->undefined_flags.eflags = NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_PF;
					}
					else if (op == 0x63 && mode != NMD_X86_MODE_64) /* arpl */
						instruction->modified_flags.eflags = NMD_X86_EFLAGS_ZF;
					else if (op == 0x9b) /* fwait,wait */
						instruction->undefined_flags.fpu_flags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C1 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
					else if (op == 0x9e) /* sahf */
						instruction->modified_flags.eflags = NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_CF;
					else if (op == 0x9f) /* lahf */
						instruction->tested_flags.eflags = NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_CF;
					else if (op == 0xd4 || op == 0xd5) /* aam,aad */
					{
						instruction->modified_flags.eflags = NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_PF;
						instruction->undefined_flags.eflags = NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_CF;
					}
					else if (op == 0xd6) /* salc */
						instruction->tested_flags.eflags = NMD_X86_EFLAGS_CF;
					else if (op >= 0xd8 && op <= 0xdf) /* escape opcodes */
					{
						if (op == 0xd8 || op == 0xdc)
						{
							if (modrm.fields.reg == 0b000 || modrm.fields.reg == 0b001 || modrm.fields.reg == 0b100 || modrm.fields.reg == 0b101 || modrm.fields.reg == 0b110 || modrm.fields.reg == 0b111) /* fadd,fmul,fsub,fsubr,fdiv,fdivr */
							{
								instruction->modified_flags.fpu_flags = NMD_X86_FPU_FLAGS_C1;
								instruction->undefined_flags.fpu_flags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
							}
							else if (modrm.fields.reg == 0b010 || modrm.fields.reg == 0b011) /* fcom,fcomp */
							{
								instruction->modified_flags.fpu_flags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
								instruction->cleared_flags.fpu_flags = NMD_X86_FPU_FLAGS_C1;
							}
						}
						else if (op == 0xd9)
						{
							if (modrm.fields.mod != 0b11)
							{
								if (modrm.fields.reg == 0b000 || modrm.fields.reg == 0b010 || modrm.fields.reg == 0b011) /* fld,fst,fstp */
								{
									instruction->modified_flags.fpu_flags = NMD_X86_FPU_FLAGS_C1;
									instruction->undefined_flags.fpu_flags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
								}
								else if (modrm.fields.reg == 0b100) /* fldenv */
									instruction->modified_flags.fpu_flags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C1 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
								else if (modrm.fields.reg == 0b101 || modrm.fields.reg == 0b110 || modrm.fields.reg == 0b111) /* fldcw,fstenv,fstcw */
									instruction->undefined_flags.fpu_flags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C1 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
							}
							else
							{
								if (modrm.modrm < 0xc8) /* fld */
								{
									instruction->modified_flags.fpu_flags = NMD_X86_FPU_FLAGS_C1;
									instruction->undefined_flags.fpu_flags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
								}
								else /*if (modrm.modrm <= 0xcf)*/ /* fxch */
								{
									instruction->cleared_flags.fpu_flags = NMD_X86_FPU_FLAGS_C1;
									instruction->undefined_flags.fpu_flags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
								}
							}
						}
						else if (op == 0xda || op == 0xde)
						{
							if (modrm.fields.mod != 0b11)
							{
								if (modrm.fields.reg == 0b000 || modrm.fields.reg == 0b001 || modrm.fields.reg == 0b100 || modrm.fields.reg == 0b101 || modrm.fields.reg == 0b110 || modrm.fields.reg == 0b111) /* fiadd,fimul,fisub,fisubr,fidiv,fidivr */
								{
									instruction->modified_flags.fpu_flags = NMD_X86_FPU_FLAGS_C1;
									instruction->undefined_flags.fpu_flags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
								}
								else /*if (modrm.fields.reg == 0b010 || modrm.fields.reg == 0b011)*/ /* ficom,ficomp */
								{
									instruction->cleared_flags.fpu_flags = NMD_X86_FPU_FLAGS_C1;
									instruction->modified_flags.fpu_flags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
								}
							}
							else
							{

								if ((op == 0xda && modrm.modrm == 0xe9) || (op == 0xde && modrm.modrm == 0xd9))
									instruction->modified_flags.fpu_flags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C1 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
								else
								{
									instruction->modified_flags.fpu_flags = NMD_X86_FPU_FLAGS_C1;
									instruction->undefined_flags.fpu_flags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
								}
							}
						}
						else if (op == 0xdb || op == 0xdd || op == 0xdf)
						{
							if (modrm.fields.mod != 0b11)
							{
								if (modrm.fields.reg == 0b000 || modrm.fields.reg == 0b010 || modrm.fields.reg == 0b011 || modrm.fields.reg == 0b101 || modrm.fields.reg == 0b111) /* fild,fist,fistp,fld,fstp */
								{
									instruction->modified_flags.fpu_flags = NMD_X86_FPU_FLAGS_C1;
									instruction->undefined_flags.fpu_flags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
								}
								else if (modrm.fields.reg == 0b001) /* fisttp */
								{
									instruction->cleared_flags.fpu_flags = NMD_X86_FPU_FLAGS_C1;
									instruction->undefined_flags.fpu_flags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
								}
							}
							else
							{
								if (modrm.fields.reg <= 0b011) /* fcmovnb,fcmovne,fcmovnbe,fcmovnu */
								{
									instruction->modified_flags.fpu_flags = NMD_X86_FPU_FLAGS_C1;
									instruction->undefined_flags.fpu_flags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
								}
								else if (modrm.modrm == 0xe0 || modrm.modrm == 0xe2) /* fstsw,fclex */
									instruction->undefined_flags.fpu_flags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C1 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
								else if (modrm.modrm == 0xe3) /* finit */
									instruction->cleared_flags.fpu_flags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C1 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
								else /* fucomi,fcomi */
								{
									instruction->cleared_flags.fpu_flags = NMD_X86_FPU_FLAGS_C1;
									instruction->undefined_flags.fpu_flags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
								}
							}
						}
					}
					else if (op == 0xf5) /* cmc */
					{
						instruction->modified_flags.eflags = NMD_X86_EFLAGS_CF;
						instruction->tested_flags.eflags = NMD_X86_EFLAGS_CF;
					}
					else if (op == 0xf8) /* clc */
						instruction->cleared_flags.eflags = NMD_X86_EFLAGS_CF;
					else if (op == 0xf9) /* stc */
						instruction->set_flags.eflags = NMD_X86_EFLAGS_CF;
					else if (op == 0xfa || op == 0xfb) /* cli,sti */
					{
						instruction->modified_flags.eflags = NMD_X86_EFLAGS_IF | NMD_X86_EFLAGS_VIF;
						instruction->tested_flags.eflags = NMD_X86_EFLAGS_IOPL;
					}
					else if (op == 0xfc) /* cld */
						instruction->cleared_flags.eflags = NMD_X86_EFLAGS_DF;
					else if (op == 0xfd) /* std */
						instruction->set_flags.eflags = NMD_X86_EFLAGS_DF;
				}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_CPU_FLAGS */

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_GROUP
				/* Parse the instruction's group. */
				if (flags & NMD_X86_DECODER_FLAGS_GROUP)
				{
					if (_NMD_R(op) == 7 || op == 0xe3)
						instruction->group = NMD_GROUP_JUMP | NMD_GROUP_CONDITIONAL_BRANCH | NMD_GROUP_RELATIVE_ADDRESSING;
					else if (op == 0xe9 || op == 0xea || op == 0xeb || (op == 0xff && (modrm.fields.reg == 0b100 || modrm.fields.reg == 0b101)))
						instruction->group = NMD_GROUP_JUMP | NMD_GROUP_UNCONDITIONAL_BRANCH | (op == 0xe9 || op == 0xeb ? NMD_GROUP_RELATIVE_ADDRESSING : 0);
					else if (op == 0x9a || op == 0xe8 || (op == 0xff && (modrm.fields.reg == 0b010 || modrm.fields.reg == 0b011)))
						instruction->group = NMD_GROUP_CALL | NMD_GROUP_UNCONDITIONAL_BRANCH | (op == 0xe8 ? NMD_GROUP_RELATIVE_ADDRESSING : 0);
					else if (op == 0xc2 || op == 0xc3 || op == 0xca || op == 0xcb)
						instruction->group = NMD_GROUP_RET;
					else if ((op >= 0xcc && op <= 0xce) || op == 0xf1)
						instruction->group = NMD_GROUP_INT;
					else if (op == 0xf4)
						instruction->group = NMD_GROUP_PRIVILEGE;
					else if (op == 0xc7 && modrm.modrm == 0xf8)
						instruction->group = NMD_GROUP_UNCONDITIONAL_BRANCH | NMD_GROUP_RELATIVE_ADDRESSING;
					else if (op >= 0xe0 && op <= 0xe2)
						instruction->group = NMD_GROUP_CONDITIONAL_BRANCH | NMD_GROUP_RELATIVE_ADDRESSING;
					else if (op == 0x8d && mode == NMD_X86_MODE_64)
						instruction->group = NMD_GROUP_RELATIVE_ADDRESSING;
					else if(op == 0xcf)
						instruction->group = NMD_GROUP_RET | NMD_GROUP_INT;
				}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_GROUP */

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_OPERANDS
				if (flags & NMD_X86_DECODER_FLAGS_OPERANDS)
				{
					if (op >= 0xd8 && op <= 0xdf)
					{
						if (modrm.fields.mod == 0b11)
						{
							if ((op == 0xd9 && (_NMD_R(modrm.modrm) == 0xc || (op >= 0xc8 && op <= 0xcf))) ||
								(op == 0xda && _NMD_R(modrm.modrm) <= 0xd) ||
								(op == 0xdb && (_NMD_R(modrm.modrm) <= 0xd || modrm.modrm >= 0xe8)) ||
								(op == 0xde && modrm.modrm != 0xd9) ||
								(op == 0xdf && modrm.modrm != 0xe0))
								instruction->num_operands = 2;
						}
						else
							instruction->num_operands = 1;
					}
					else if ((_NMD_R(op) < 4 && op % 8 <= 5) || (_NMD_R(op) >= 8 && _NMD_R(op) <= 0xa && op != 0x8f && op != 0x90 && !(op >= 0x98 && op <= 0x9f)) || op == 0x62 || op == 0x63 || (op >= 0x6c && op <= 0x6f) || op == 0xc0 || op == 0xc1 || (op >= 0xc4 && op <= 0xc8) || (op >= 0xd0 && op <= 0xd3) || (_NMD_R(op) == 0xe && op % 8 >= 4))
						instruction->num_operands = 2;
					else if (_NMD_R(op) == 4 || op == 0x8f || op == 0x9a || op == 0xd4 || op == 0xd5 || (_NMD_R(op) == 0xe && op % 8 <= 3 && op != 0xe9))
						instruction->num_operands = 1;
					else if (op == 0x69 || op == 0x6b)
						instruction->num_operands = 3;
					
					const bool opszprfx = instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE;
					if (_NMD_R(op) == 0xb) /* mov reg,imm */
					{
						instruction->num_operands = 2;
						_NMD_SET_REG_OPERAND(instruction->operands[0], false, NMD_X86_OPERAND_ACTION_WRITE, (op < 0xb8 ? (instruction->prefixes & NMD_X86_PREFIXES_REX_B ? NMD_X86_REG_R8B : NMD_X86_REG_AL) : (instruction->prefixes & NMD_X86_PREFIXES_REX_W ? (instruction->prefixes & NMD_X86_PREFIXES_REX_B ? NMD_X86_REG_R8 : NMD_X86_REG_RAX) : (instruction->prefixes & NMD_X86_PREFIXES_REX_B ? NMD_X86_REG_R8D : NMD_X86_REG_EAX))) + op % 8);
						_NMD_SET_IMM_OPERAND(instruction->operands[1], false, NMD_X86_OPERAND_ACTION_READ, instruction->immediate);
					}
					else if (_NMD_R(op) == 5) /* push reg,pop reg */
					{
						instruction->num_operands = 3;
						_NMD_SET_REG_OPERAND(instruction->operands[0], false, _NMD_C(op) < 8 ? NMD_X86_OPERAND_ACTION_READ : NMD_X86_OPERAND_ACTION_WRITE, (instruction->prefixes & NMD_X86_PREFIXES_REX_B ? (opszprfx ? NMD_X86_REG_R8W : NMD_X86_REG_R8) : (opszprfx ? (instruction->mode == NMD_X86_MODE_16 ? NMD_X86_REG_EAX : NMD_X86_REG_AX) : (NMD_X86_REG_AX + (instruction->mode >> 2) * 8))) + (op % 8));
						_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_READWRITE, _NMD_GET_GPR(NMD_X86_REG_SP));
						_NMD_SET_MEM_OPERAND(instruction->operands[2], true, _NMD_C(op) < 8 ? NMD_X86_OPERAND_ACTION_WRITE : NMD_X86_OPERAND_ACTION_READ, NMD_X86_REG_SS, _NMD_GET_GPR(NMD_X86_REG_SP), NMD_X86_REG_NONE, 0, 0);
					}
					else if (_NMD_R(op) == 7) /* jCC */
					{
						instruction->num_operands = 2;
						_NMD_SET_IMM_OPERAND(instruction->operands[0], false, NMD_X86_OPERAND_ACTION_READ, instruction->immediate);
						_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_READWRITE, _NMD_GET_IP());
					}
					else if (op == 0xe9) /* jmp rel16,rel32 */
					{
						instruction->num_operands = 2;
						_NMD_SET_IMM_OPERAND(instruction->operands[0], false, NMD_X86_OPERAND_ACTION_READ, instruction->immediate);
						_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_READWRITE, _NMD_GET_IP());
					}
					else if (op == 0x6a || op == 0x68) /* push imm8,push imm32/imm16 */
					{
						instruction->num_operands = 3;
						_NMD_SET_IMM_OPERAND(instruction->operands[0], false, NMD_X86_OPERAND_ACTION_READ, instruction->immediate);
						_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_READWRITE, _NMD_GET_GPR(NMD_X86_REG_SP));
						_NMD_SET_MEM_OPERAND(instruction->operands[2], true, NMD_X86_OPERAND_ACTION_WRITE, NMD_X86_REG_SS, _NMD_GET_GPR(NMD_X86_REG_SP), NMD_X86_REG_NONE, 0, 0);
					}
					else if (_NMD_R(op) == 4) /* inc,dec*/
					{
						instruction->num_operands = 1;
						_NMD_SET_REG_OPERAND(instruction->operands[0], false, NMD_X86_OPERAND_ACTION_READWRITE, _NMD_GET_BY_MODE_OPSZPRFX(mode, opszprfx, NMD_X86_REG_AX, NMD_X86_REG_EAX) + (op % 8));
					}
					else if (op == 0xcc || op == 0xf1 || op == 0xce) /* int3,int1,into */
					{
						instruction->num_operands = 1;
						_NMD_SET_REG_OPERAND(instruction->operands[0], true, NMD_X86_OPERAND_ACTION_WRITE, _NMD_GET_IP());
					}
					else if (op == 0xcd) /* int n */
					{
						instruction->num_operands = 2;
						_NMD_SET_IMM_OPERAND(instruction->operands[0], false, NMD_X86_OPERAND_ACTION_READ, instruction->immediate);
						_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_WRITE, _NMD_GET_IP());
					}
					else if (op == 0xe8) /* call rel32 */
					{
						instruction->num_operands = 4;
						_NMD_SET_IMM_OPERAND(instruction->operands[0], false, NMD_X86_OPERAND_ACTION_READ, instruction->immediate);
						_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_READWRITE, _NMD_GET_IP());
						_NMD_SET_REG_OPERAND(instruction->operands[2], true, NMD_X86_OPERAND_ACTION_READWRITE, _NMD_GET_GPR(NMD_X86_REG_SP));
						_NMD_SET_MEM_OPERAND(instruction->operands[3], true, NMD_X86_OPERAND_ACTION_WRITE, NMD_X86_REG_SS, _NMD_GET_GPR(NMD_X86_REG_SP), NMD_X86_REG_NONE, 0, 0);
					}
                    else if (_NMD_R(op) < 4 && _NMD_C(op) < 6) /* add,adc,and,xor,or,sbb,sub,cmp Eb,Gb / Ev,Gv / Gb,Eb / Gv,Ev / AL,lb / rAX,lz */
					{
                        /*
                        if (op % 8 == 0)
                        {
                            if (modrm.mod == 0b11)
                            {
                                
                            }
                            else
                            {
                                _nmd_decode_modrm_upper32(instruction, &instruction->operands[0]);
                            }
                        }
                        */
						if (op % 8 == 0 || op % 8 == 2)
						{
							_nmd_decode_operand_Eb(instruction, &instruction->operands[op % 8 == 0 ? 0 : 1]);
							_nmd_decode_operand_Gb(instruction, &instruction->operands[op % 8 == 0 ? 1 : 0]);
						}
						else if (op % 8 == 1 || op % 8 == 3)
						{
							_nmd_decode_operand_Ev(instruction, &instruction->operands[op % 8 == 1 ? 0 : 1]);
							_nmd_decode_operand_Gv(instruction, &instruction->operands[op % 8 == 1 ? 1 : 0]);
						}
						else if (op % 8 == 4 || op % 8 == 5)
						{
							instruction->operands[0].type = NMD_X86_OPERAND_TYPE_REGISTER;
							if (op % 8 == 4)
								instruction->operands[0].fields.reg = NMD_X86_REG_AL;
							else
								instruction->operands[0].fields.reg = (uint8_t)(instruction->rex_w_prefix ? NMD_X86_REG_RAX : (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_REG_AX : NMD_X86_REG_EAX));

							instruction->operands[1].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
						}

						instruction->operands[0].action = instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
						if (!(_NMD_R(op) == 3 && _NMD_C(op) >= 8))
							instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READWRITE;
					}
					else if (op == 0xc3 || op == 0xcb || op == 0xcf) /* ret,retf,iret,iretd,iretf */
					{
						instruction->num_operands = 3;
						_NMD_SET_REG_OPERAND(instruction->operands[0], true, NMD_X86_OPERAND_ACTION_WRITE, _NMD_GET_IP());
						_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_READWRITE, _NMD_GET_GPR(NMD_X86_REG_SP));
						_NMD_SET_MEM_OPERAND(instruction->operands[2], true, NMD_X86_OPERAND_ACTION_READ, NMD_X86_REG_SS, _NMD_GET_GPR(NMD_X86_REG_SP), NMD_X86_REG_NONE, 0, 0);
					}
					else if (op == 0xc2 || op == 0xca) /* ret imm16,retf imm16 */
					{
						instruction->num_operands = 4;
						_NMD_SET_IMM_OPERAND(instruction->operands[0], false, NMD_X86_OPERAND_ACTION_READ, instruction->immediate);
						_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_WRITE, _NMD_GET_IP());
						_NMD_SET_REG_OPERAND(instruction->operands[2], true, NMD_X86_OPERAND_ACTION_READWRITE, _NMD_GET_GPR(NMD_X86_REG_SP));
						_NMD_SET_MEM_OPERAND(instruction->operands[3], true, NMD_X86_OPERAND_ACTION_READ, NMD_X86_REG_SS, _NMD_GET_GPR(NMD_X86_REG_SP), NMD_X86_REG_NONE, 0, 0);
					}
					else if (op == 0xff && modrm.fields.reg == 6) /* push mem */
					{
						instruction->num_operands = 3;
						_NMD_SET_MEM_OPERAND(instruction->operands[0], false, NMD_X86_OPERAND_ACTION_READ, NMD_X86_REG_DS, (instruction->prefixes& NMD_X86_PREFIXES_REX_B ? (opszprfx ? NMD_X86_REG_R8W : NMD_X86_REG_R8) : (opszprfx ? (instruction->mode == NMD_X86_MODE_16 ? NMD_X86_REG_EAX : NMD_X86_REG_AX) : (NMD_X86_REG_AX + (instruction->mode >> 2) * 8))) + modrm.fields.rm, NMD_X86_REG_NONE, 0, 0);
						_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_READWRITE, _NMD_GET_GPR(NMD_X86_REG_SP));
						_NMD_SET_MEM_OPERAND(instruction->operands[2], true, NMD_X86_OPERAND_ACTION_WRITE, NMD_X86_REG_SS, _NMD_GET_GPR(NMD_X86_REG_SP), NMD_X86_REG_NONE, 0, 0);

					}
					else if (op == 0x9c) /* pushf,pushfd,pushfq */
					{
						instruction->num_operands = 2;
						_NMD_SET_REG_OPERAND(instruction->operands[0], true, NMD_X86_OPERAND_ACTION_READWRITE, _NMD_GET_GPR(NMD_X86_REG_SP));
						_NMD_SET_MEM_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_WRITE, NMD_X86_REG_SS, _NMD_GET_GPR(NMD_X86_REG_SP), NMD_X86_REG_NONE, 0, 0);
					}
					else if (op == 0x9d) /* popf,popfd,popfq */
					{
						instruction->num_operands = 2;
						_NMD_SET_REG_OPERAND(instruction->operands[0], true, NMD_X86_OPERAND_ACTION_READWRITE, _NMD_GET_GPR(NMD_X86_REG_SP));
						_NMD_SET_MEM_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_READ, NMD_X86_REG_SS, _NMD_GET_GPR(NMD_X86_REG_SP), NMD_X86_REG_NONE, 0, 0);
					}
					else if (op == 0xc9) /* leave */
					{
						instruction->num_operands = 3;
						_NMD_SET_REG_OPERAND(instruction->operands[0], true, NMD_X86_OPERAND_ACTION_READWRITE, _NMD_GET_GPR(NMD_X86_REG_SP));
						_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_READWRITE, _NMD_GET_GPR(NMD_X86_REG_BP));
						_NMD_SET_MEM_OPERAND(instruction->operands[2], true, NMD_X86_OPERAND_ACTION_READ, NMD_X86_REG_SS, _NMD_GET_GPR(NMD_X86_REG_SP), NMD_X86_REG_NONE, 0, 0);
					}
					else if (op == 0x06 || op == 0x16 || op == 0x0e || op == 0x1e) /* push es,push ss,push ds,push cs */
					{
						instruction->num_operands = 3;
						_NMD_SET_REG_OPERAND(instruction->operands[0], false, NMD_X86_OPERAND_ACTION_READ, op == 0x06 ? NMD_X86_REG_ES : (op == 0x16 ? NMD_X86_REG_SS : (op == 0x1e ? NMD_X86_REG_DS : NMD_X86_REG_CS)));
						_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_READWRITE, _NMD_GET_GPR(NMD_X86_REG_SP));
						_NMD_SET_MEM_OPERAND(instruction->operands[2], true, NMD_X86_OPERAND_ACTION_WRITE, NMD_X86_REG_SS, _NMD_GET_GPR(NMD_X86_REG_SP), NMD_X86_REG_NONE, 0, 0);
					}
					else if (op == 0x07 || op == 0x17 || op == 0x1f) /* pop es,pop ss,pop ds */
					{
						instruction->num_operands = 3;
						_NMD_SET_REG_OPERAND(instruction->operands[0], false, NMD_X86_OPERAND_ACTION_WRITE, op == 0x07 ? NMD_X86_REG_ES : (op == 0x17 ? NMD_X86_REG_SS : NMD_X86_REG_DS));
						_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_READWRITE, _NMD_GET_GPR(NMD_X86_REG_SP));
						_NMD_SET_MEM_OPERAND(instruction->operands[2], true, NMD_X86_OPERAND_ACTION_READ, NMD_X86_REG_SS, _NMD_GET_GPR(NMD_X86_REG_SP), NMD_X86_REG_NONE, 0, 0);
					}
					else if (op == 0x60) /* pusha,pushad */
					{
						instruction->num_operands = 10;
						const uint32_t base_reg = _NMD_GET_BY_MODE_OPSZPRFX(mode, opszprfx, NMD_X86_REG_AX, NMD_X86_REG_EAX);
						_NMD_SET_REG_OPERAND(instruction->operands[0], true, NMD_X86_OPERAND_ACTION_READ, base_reg + 0);
						_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_READ, base_reg + 1);
						_NMD_SET_REG_OPERAND(instruction->operands[2], true, NMD_X86_OPERAND_ACTION_READ, base_reg + 2);
						_NMD_SET_REG_OPERAND(instruction->operands[3], true, NMD_X86_OPERAND_ACTION_READ, base_reg + 3);
						_NMD_SET_REG_OPERAND(instruction->operands[4], true, NMD_X86_OPERAND_ACTION_READ, base_reg + 4);
						_NMD_SET_REG_OPERAND(instruction->operands[5], true, NMD_X86_OPERAND_ACTION_READ, base_reg + 5);
						_NMD_SET_REG_OPERAND(instruction->operands[6], true, NMD_X86_OPERAND_ACTION_READ, base_reg + 6);
						_NMD_SET_REG_OPERAND(instruction->operands[7], true, NMD_X86_OPERAND_ACTION_READ, base_reg + 7);
						_NMD_SET_REG_OPERAND(instruction->operands[8], true, NMD_X86_OPERAND_ACTION_READWRITE, _NMD_GET_BY_MODE_OPSZPRFX(mode, opszprfx, NMD_X86_REG_SP, NMD_X86_REG_ESP));
						_NMD_SET_MEM_OPERAND(instruction->operands[9], true, NMD_X86_OPERAND_ACTION_WRITE, NMD_X86_REG_SS, _NMD_GET_BY_MODE_OPSZPRFX(mode, opszprfx, NMD_X86_REG_SP, NMD_X86_REG_ESP), NMD_X86_REG_NONE, 0, 0);
					}
					else if (op == 0x61) /* popa,popad */
					{
						instruction->num_operands = 10;
						const uint32_t base_reg = _NMD_GET_BY_MODE_OPSZPRFX(mode, opszprfx, NMD_X86_REG_AX, NMD_X86_REG_EAX);
						_NMD_SET_REG_OPERAND(instruction->operands[0], true, NMD_X86_OPERAND_ACTION_WRITE, base_reg + 0);
						_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_WRITE, base_reg + 1);
						_NMD_SET_REG_OPERAND(instruction->operands[2], true, NMD_X86_OPERAND_ACTION_WRITE, base_reg + 2);
						_NMD_SET_REG_OPERAND(instruction->operands[3], true, NMD_X86_OPERAND_ACTION_WRITE, base_reg + 3);
						_NMD_SET_REG_OPERAND(instruction->operands[4], true, NMD_X86_OPERAND_ACTION_WRITE, base_reg + 4);
						_NMD_SET_REG_OPERAND(instruction->operands[5], true, NMD_X86_OPERAND_ACTION_WRITE, base_reg + 5);
						_NMD_SET_REG_OPERAND(instruction->operands[6], true, NMD_X86_OPERAND_ACTION_WRITE, base_reg + 6);
						_NMD_SET_REG_OPERAND(instruction->operands[7], true, NMD_X86_OPERAND_ACTION_WRITE, base_reg + 7);
						_NMD_SET_REG_OPERAND(instruction->operands[8], true, NMD_X86_OPERAND_ACTION_READWRITE, _NMD_GET_BY_MODE_OPSZPRFX(mode, opszprfx, NMD_X86_REG_SP, NMD_X86_REG_ESP));
						_NMD_SET_MEM_OPERAND(instruction->operands[9], true, NMD_X86_OPERAND_ACTION_READ, NMD_X86_REG_SS, _NMD_GET_BY_MODE_OPSZPRFX(mode, opszprfx, NMD_X86_REG_SP, NMD_X86_REG_ESP), NMD_X86_REG_NONE, 0, 0);
					}
					else if (op == 0x27 || op == 0x2f) /* daa,das */
					{
						instruction->num_operands = 1;
						_NMD_SET_REG_OPERAND(instruction->operands[0], true, NMD_X86_OPERAND_ACTION_READWRITE, NMD_X86_REG_AL);
					}
					else if (op == 0x37 || op == 0x3f) /* aaa,aas */
					{
						instruction->num_operands = 2;
						_NMD_SET_REG_OPERAND(instruction->operands[0], true, NMD_X86_OPERAND_ACTION_READWRITE, NMD_X86_REG_AL);
						_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_READWRITE, NMD_X86_REG_AH);
					}
					else if (op == 0xd7) /* xlat */
					{
						instruction->num_operands = 2;
						_NMD_SET_REG_OPERAND(instruction->operands[0], true, NMD_X86_OPERAND_ACTION_WRITE, NMD_X86_REG_AL);
						_NMD_SET_MEM_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_READ, NMD_X86_REG_DS, _NMD_GET_GPR(NMD_X86_REG_BX), NMD_X86_REG_AL, 1, 0);
					}
					else if (op == 0x9e || op == 0x9f) /* sahf,lahf */
					{
						instruction->num_operands = 1;
						_NMD_SET_REG_OPERAND(instruction->operands[0], true, op == 0x9e ? NMD_X86_OPERAND_ACTION_READ : NMD_X86_OPERAND_ACTION_WRITE, NMD_X86_REG_AH);
					}
					else if (op == 0x98) /* cbw,cwde,cdqe */
					{
						instruction->num_operands = 2;
						const NMD_X86_REG reg = instruction->mode == NMD_X86_MODE_64 && instruction->rex_w_prefix ? NMD_X86_REG_RAX : (((instruction->mode == NMD_X86_MODE_16 && instruction->simd_prefix & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE) || (instruction->mode != NMD_X86_MODE_16 && !(instruction->simd_prefix & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE))) ? NMD_X86_REG_EAX : NMD_X86_REG_AX);
						_NMD_SET_REG_OPERAND(instruction->operands[0], true, NMD_X86_OPERAND_ACTION_WRITE, reg);
						_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_READ, reg-8);
					}
					else if (op == 0x99) /* cwd,cdq,cqo */
					{
						instruction->num_operands = 2;
						const NMD_X86_REG reg = instruction->mode == NMD_X86_MODE_64 && instruction->rex_w_prefix ? NMD_X86_REG_RAX : (((instruction->mode == NMD_X86_MODE_16 && instruction->simd_prefix & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE) || (instruction->mode != NMD_X86_MODE_16 && !(instruction->simd_prefix & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE))) ? NMD_X86_REG_EAX : NMD_X86_REG_AX);
						_NMD_SET_REG_OPERAND(instruction->operands[0], true, NMD_X86_OPERAND_ACTION_WRITE, reg + 2);
						_NMD_SET_REG_OPERAND(instruction->operands[1], true, NMD_X86_OPERAND_ACTION_READ, reg);
					}
					else if (op == 0xd6) /* salc */
					{
						instruction->num_operands = 1;
						_NMD_SET_REG_OPERAND(instruction->operands[0], true, NMD_X86_OPERAND_ACTION_WRITE, NMD_X86_REG_AL);
					}
                    else if (op == 0xc7 && modrm.fields.reg == 0b000) /* mov Ev,lz */
                    {
                        instruction->num_operands = 2;
                        if (modrm.fields.mod == 0b11)
                        {
                            const NMD_X86_REG reg = (NMD_X86_REG)(_NMD_GET_BY_MODE_OPSZPRFX_W64(mode, opszprfx, instruction->rex_w_prefix, NMD_X86_REG_AX, NMD_X86_REG_EAX, NMD_X86_REG_RAX) + modrm.fields.rm);
                            _NMD_SET_REG_OPERAND(instruction->operands[0], false, NMD_X86_OPERAND_ACTION_WRITE, reg);
                        }
                        else
                        {
                            _nmd_decode_modrm_upper32(instruction, &instruction->operands[0]);
                            instruction->operands[0].is_implicit = false;
                            instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
                        }
                        _NMD_SET_IMM_OPERAND(instruction->operands[1], false, NMD_X86_OPERAND_ACTION_READ, instruction->immediate);						
                    }
					else if (op >= 0x84 && op <= 0x8b)
					{
						if (op % 2 == 0)
						{
							_nmd_decode_operand_Eb(instruction, &instruction->operands[op == 0x8a ? 1 : 0]);
							_nmd_decode_operand_Gb(instruction, &instruction->operands[op == 0x8a ? 0 : 1]);
						}
						else
						{
							_nmd_decode_operand_Ev(instruction, &instruction->operands[op == 0x8b ? 1 : 0]);
							_nmd_decode_operand_Gv(instruction, &instruction->operands[op == 0x8b ? 0 : 1]);
						}

						if (op >= 0x88)
						{
							instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
							instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
						}
						else if (op >= 0x86)
							instruction->operands[0].action = instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READWRITE;
					}
					else if (op >= 0x80 && op <= 0x83)
					{
						if (op % 2 == 0)
							_nmd_decode_operand_Eb(instruction, &instruction->operands[0]);
						else
							_nmd_decode_operand_Ev(instruction, &instruction->operands[0]);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READWRITE;
						instruction->operands[1].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
					}
					else if (_NMD_R(op) == 7 || op == 0x9a || op == 0xcd || op == 0xd4 || op == 0xd5)
						instruction->operands[0].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
					else if (op == 0x90 && instruction->prefixes & NMD_X86_PREFIXES_REX_B)
					{
						instruction->operands[0].type = instruction->operands[1].type = NMD_X86_OPERAND_TYPE_REGISTER;
						instruction->operands[0].fields.reg = (uint8_t)(instruction->prefixes & NMD_X86_PREFIXES_REX_W ? NMD_X86_REG_R8 : NMD_X86_REG_R8D);
						instruction->operands[1].fields.reg = (uint8_t)(instruction->prefixes & NMD_X86_PREFIXES_REX_W ? NMD_X86_REG_RAX : NMD_X86_REG_EAX);
					}
					else if (_NMD_R(op) == 5)
					{
						instruction->operands[0].type = NMD_X86_OPERAND_TYPE_REGISTER;
						instruction->operands[0].fields.reg = (uint8_t)((instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_REG_AX : (mode == NMD_X86_MODE_64 ? NMD_X86_REG_RAX : NMD_X86_REG_EAX)) + (op % 8));
						instruction->operands[0].action = (uint8_t)(_NMD_C(op) < 8 ? NMD_X86_OPERAND_ACTION_READ : NMD_X86_OPERAND_ACTION_WRITE);
					}
					else if (op == 0x62)
					{
						_nmd_decode_operand_Gv(instruction, &instruction->operands[0]);
						_nmd_decode_modrm_upper32(instruction, &instruction->operands[1]);
						instruction->operands[0].action = instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if (op == 0x63)
					{
						if (mode == NMD_X86_MODE_64)
						{
							_nmd_decode_operand_Gv(instruction, &instruction->operands[0]);
							_nmd_decode_operand_Ev(instruction, &instruction->operands[1]);
							instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
							instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
						}
						else
						{
							if (instruction->modrm.fields.mod == 0b11)
							{
								instruction->operands[0].type = NMD_X86_OPERAND_TYPE_REGISTER;
								instruction->operands[0].fields.reg = NMD_X86_REG_AX + instruction->modrm.fields.rm;
							}
							else
								_nmd_decode_modrm_upper32(instruction, &instruction->operands[0]);

							instruction->operands[1].type = NMD_X86_OPERAND_TYPE_REGISTER;
							instruction->operands[1].fields.reg = NMD_X86_REG_AX + instruction->modrm.fields.reg;
							instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READWRITE;
							instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
						}
					}
					else if (op == 0x69 || op == 0x6b)
					{
						_nmd_decode_operand_Gv(instruction, &instruction->operands[0]);
						_nmd_decode_operand_Ev(instruction, &instruction->operands[1]);
						instruction->operands[2].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
						instruction->operands[2].fields.imm = (int64_t)(instruction->immediate);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = instruction->operands[2].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if (op == 0x8c)
					{
						_nmd_decode_operand_Ev(instruction, &instruction->operands[0]);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
						instruction->operands[1].type = NMD_X86_OPERAND_TYPE_REGISTER;
						instruction->operands[1].fields.reg = NMD_X86_REG_ES + instruction->modrm.fields.reg;
					}
					else if (op == 0x8d)
					{
						_nmd_decode_operand_Gv(instruction, &instruction->operands[0]);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
						_nmd_decode_modrm_upper32(instruction, &instruction->operands[1]);
					}
					else if (op == 0x8e)
					{
						instruction->operands[0].type = NMD_X86_OPERAND_TYPE_REGISTER;
						instruction->operands[0].fields.reg = NMD_X86_REG_ES + instruction->modrm.fields.reg;
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
						_nmd_decode_operand_Ew(instruction, &instruction->operands[1]);
					}
					else if (op == 0x8f)
					{
						_nmd_decode_operand_Ev(instruction, &instruction->operands[0]);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
					}
					else if (op >= 0x91 && op <= 0x97)
					{
						_nmd_decode_operand_Gv(instruction, &instruction->operands[0]);
						instruction->operands[0].fields.reg = instruction->operands[0].fields.reg + _NMD_C(op);
						instruction->operands[1].type = NMD_X86_OPERAND_TYPE_REGISTER;
						instruction->operands[1].fields.reg = (uint8_t)(instruction->rex_w_prefix ? NMD_X86_REG_RAX : (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && mode != NMD_X86_MODE_16 ? NMD_X86_REG_AX : NMD_X86_REG_EAX));
						instruction->operands[0].action = instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READWRITE;
					}
					else if (op >= 0xa0 && op <= 0xa3)
					{
						instruction->operands[op < 0xa2 ? 0 : 1].type = NMD_X86_OPERAND_TYPE_REGISTER;
						instruction->operands[op < 0xa2 ? 0 : 1].fields.reg = (uint8_t)(op % 2 == 0 ? NMD_X86_REG_AL : (instruction->rex_w_prefix ? NMD_X86_REG_RAX : ((instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && mode != NMD_X86_MODE_16) || (mode == NMD_X86_MODE_16 && !(instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)) ? NMD_X86_REG_AX : NMD_X86_REG_EAX)));
						instruction->operands[op < 0xa2 ? 1 : 0].type = NMD_X86_OPERAND_TYPE_MEMORY;
						
						/* FIXME: We should not access the buffer from here
						instruction->operands[op < 0xa2 ? 1 : 0].fields.mem.disp = (mode == NMD_X86_MODE_64) ? *(uint64_t*)(b + 1) : *(uint32_t*)(b + 1);
						*/
						
						_nmd_decode_operand_segment_reg(instruction, &instruction->operands[op < 0xa2 ? 1 : 0]);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if (op == 0xa8 || op == 0xa9)
					{
						instruction->operands[0].type = NMD_X86_OPERAND_TYPE_REGISTER;
						instruction->operands[0].fields.reg = (uint8_t)(op == 0xa8 ? NMD_X86_REG_AL : (instruction->rex_w_prefix ? NMD_X86_REG_RAX : ((instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && mode != NMD_X86_MODE_16) || (mode == NMD_X86_MODE_16 && !(instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)) ? NMD_X86_REG_AX : NMD_X86_REG_EAX)));
						instruction->operands[1].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
					}
					else if (op == 0xc0 || op == 0xc1 || op == 0xc6)
					{
						if (!(op >= 0xc6 && instruction->modrm.fields.reg))
						{
							if (op % 2 == 0)
								_nmd_decode_operand_Eb(instruction, &instruction->operands[0]);
							else
								_nmd_decode_operand_Ev(instruction, &instruction->operands[0]);
						}
						instruction->operands[op >= 0xc6 && instruction->modrm.fields.reg ? 0 : 1].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
						instruction->operands[0].action = (uint8_t)(op <= 0xc1 ? NMD_X86_OPERAND_ACTION_READWRITE : NMD_X86_OPERAND_ACTION_WRITE);
					}					
					else if (op == 0xc4 || op == 0xc5)
					{
						instruction->operands[0].type = NMD_X86_OPERAND_TYPE_REGISTER;
						instruction->operands[0].fields.reg = (uint8_t)((instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_REG_AX : NMD_X86_REG_EAX) + instruction->modrm.fields.reg);
						_nmd_decode_modrm_upper32(instruction, &instruction->operands[1]);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if (op == 0xc8)
					{
						instruction->operands[0].type = instruction->operands[1].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
						/* FIXME: We should not access the buffer from here
						instruction->operands[0].fields.imm = *(uint16_t*)(b + 1);
						instruction->operands[1].fields.imm = b[3];
						*/
					}
					else if (op >= 0xd0 && op <= 0xd3)
					{
						if (op % 2 == 0)
							_nmd_decode_operand_Eb(instruction, &instruction->operands[0]);
						else
							_nmd_decode_operand_Ev(instruction, &instruction->operands[0]);

						if (op < 0xd2)
						{
							instruction->operands[1].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
							instruction->operands[1].fields.imm = 1;
						}
						else
						{
							instruction->operands[1].type = NMD_X86_OPERAND_TYPE_REGISTER;
							instruction->operands[1].fields.reg = NMD_X86_REG_CL;
						}
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READWRITE;
					}
					else if (op >= 0xd8 && op <= 0xdf)
					{
						if (instruction->modrm.fields.mod != 0b11 ||
							op == 0xd8 ||
							(op == 0xd9 && _NMD_C(instruction->modrm.modrm) == 0xc) ||
							(op == 0xda && _NMD_C(instruction->modrm.modrm) <= 0xd) ||
							(op == 0xdb && (_NMD_C(instruction->modrm.modrm) <= 0xd || instruction->modrm.modrm >= 0xe8)) ||
							op == 0xdc ||
							op == 0xdd ||
							(op == 0xde && instruction->modrm.modrm != 0xd9) ||
							(op == 0xdf && instruction->modrm.modrm != 0xe0))
						{
							instruction->operands[0].type = instruction->operands[1].type = NMD_X86_OPERAND_TYPE_REGISTER;
							instruction->operands[0].is_implicit = true;
							instruction->operands[0].fields.reg = NMD_X86_REG_ST0;
							instruction->operands[1].fields.reg = NMD_X86_REG_ST0 + instruction->modrm.fields.reg;
						}
					}
					else if (_NMD_R(op) == 0xe)
					{
						if (op % 8 < 4)
						{
							instruction->operands[0].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
							instruction->operands[0].fields.imm = (int64_t)(instruction->immediate);
						}
						else
						{
							if (op < 0xe8)
							{
								instruction->operands[0].type = (uint8_t)(_NMD_C(op) < 6 ? NMD_X86_OPERAND_TYPE_REGISTER : NMD_X86_OPERAND_TYPE_IMMEDIATE);
								instruction->operands[1].type = (uint8_t)(_NMD_C(op) < 6 ? NMD_X86_OPERAND_TYPE_IMMEDIATE : NMD_X86_OPERAND_TYPE_REGISTER);
								instruction->operands[0].fields.imm = instruction->operands[1].fields.imm = (int64_t)(instruction->immediate);
							}
							else
							{
								instruction->operands[0].type = instruction->operands[1].type = NMD_X86_OPERAND_TYPE_REGISTER;
								instruction->operands[0].fields.reg = instruction->operands[1].fields.reg = NMD_X86_REG_DX;
							}

							if (op % 2 == 0)
								instruction->operands[op % 8 == 4 ? 0 : 1].fields.reg = NMD_X86_REG_AL;
							else
								instruction->operands[op % 8 == 5 ? 0 : 1].fields.reg = (uint8_t)((instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_REG_AX : NMD_X86_REG_EAX) + instruction->modrm.fields.reg);

							instruction->operands[op % 8 <= 5 ? 0 : 1].action = NMD_X86_OPERAND_ACTION_WRITE;
							instruction->operands[op % 8 <= 5 ? 1 : 0].action = NMD_X86_OPERAND_ACTION_READ;
						}
					}
					else if (op == 0xf6 || op == 0xfe)
					{
						_nmd_decode_operand_Eb(instruction, &instruction->operands[0]);
						instruction->operands[0].action = (uint8_t)(op == 0xfe && instruction->modrm.fields.reg >= 0b010 ? NMD_X86_OPERAND_ACTION_READ : NMD_X86_OPERAND_ACTION_READWRITE);
					}
					else if (op == 0xf7 || op == 0xff)
					{
						_nmd_decode_operand_Ev(instruction, &instruction->operands[0]);
						instruction->operands[0].action = (uint8_t)(op == 0xff && instruction->modrm.fields.reg >= 0b010 ? NMD_X86_OPERAND_ACTION_READ : NMD_X86_OPERAND_ACTION_READWRITE);
					}
				}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_OPERANDS */
			}
	}

	if (instruction->prefixes & NMD_X86_PREFIXES_LOCK)
	{
		if (!(instruction->has_modrm && instruction->modrm.fields.mod != 0b11 &&
			((instruction->opcode_size == 1 && (op == 0x86 || op == 0x87 || (_NMD_R(op) < 4 && (op % 8) < 2 && op < 0x38) || ((op >= 0x80 && op <= 0x83) && instruction->modrm.fields.reg != 0b111) || (op >= 0xfe && instruction->modrm.fields.reg < 2) || ((op == 0xf6 || op == 0xf7) && (instruction->modrm.fields.reg == 0b010 || instruction->modrm.fields.reg == 0b011)))) ||
				(instruction->opcode_size == 2 && (_nmd_find_byte(_nmd_two_opcodes, sizeof(_nmd_two_opcodes), op) || op == 0xab || (op == 0xba && instruction->modrm.fields.reg != 0b100) || (op == 0xc7 && instruction->modrm.fields.reg == 0b001))))))
			return false;
	}

	instruction->length = (uint8_t)((ptrdiff_t)(b) - (ptrdiff_t)(buffer));
	for (i = 0; i < instruction->length; i++)
		instruction->buffer[i] = ((const uint8_t* const)(buffer))[i];

	instruction->valid = true;

	return true;
}