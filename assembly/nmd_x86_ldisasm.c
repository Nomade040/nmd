#include "nmd_common.h"

NMD_ASSEMBLY_API bool _nmd_ldisasm_decode_modrm(const uint8_t** p_buffer, size_t* p_buffer_size, bool address_prefix, NMD_X86_MODE mode, nmd_x86_modrm* p_modrm)
{
	_NMD_READ_BYTE(*p_buffer, *p_buffer_size, (*p_modrm).modrm);
    
	bool has_sib = false;
	size_t disp_size = 0;

	if (mode == NMD_X86_MODE_16)
	{
		if (p_modrm->fields.mod != 0b11)
		{
			if (p_modrm->fields.mod == 0b00)
			{
				if (p_modrm->fields.rm == 0b110)
					disp_size = 2;
			}
			else
				disp_size = p_modrm->fields.mod == 0b01 ? 1 : 2;
		}
	}
	else
	{
		if (address_prefix && mode == NMD_X86_MODE_32)
		{
			if ((p_modrm->fields.mod == 0b00 && p_modrm->fields.rm == 0b110) || p_modrm->fields.mod == 0b10)
				disp_size = 2;
			else if (p_modrm->fields.mod == 0b01)
				disp_size = 1;
		}
		else
		{
			/* Check for SIB byte */
			uint8_t sib = 0;
			if (p_modrm->modrm < 0xC0 && p_modrm->fields.rm == 0b100 && (!address_prefix || (address_prefix && mode == NMD_X86_MODE_64)))
			{
				has_sib = true;
                _NMD_READ_BYTE(*p_buffer, *p_buffer_size, sib);
			}

			if (p_modrm->fields.mod == 0b01) /* disp8 (ModR/M) */
				disp_size = 1;
			else if ((p_modrm->fields.mod == 0b00 && p_modrm->fields.rm == 0b101) || p_modrm->fields.mod == 0b10) /* disp16,32 (ModR/M) */
				disp_size = (address_prefix && !(mode == NMD_X86_MODE_64 && address_prefix) ? 2 : 4);
			else if (has_sib && (sib & 0b111) == 0b101) /* disp8,32 (SIB) */
				disp_size = (p_modrm->fields.mod == 0b01 ? 1 : 4);
		}
	}
    
    /* Make sure we can read 'instruction->disp_mask' bytes from the buffer */
    if (*p_buffer_size < disp_size)
		return false;
    
    /* Increment the buffer and decrement the buffer's size */
	*p_buffer += disp_size;
	*p_buffer_size -= disp_size;

	return true;
}

/*
Returns the length of the instruction if it is valid, zero otherwise.
Parameters:
 - buffer      [in] A pointer to a buffer containing an encoded instruction.
 - buffer_size [in] The size of the buffer in bytes.
 - mode        [in] The architecture mode. 'NMD_X86_MODE_32', 'NMD_X86_MODE_64' or 'NMD_X86_MODE_16'.
*/
NMD_ASSEMBLY_API size_t nmd_x86_ldisasm(const void* const buffer, size_t buffer_size, const NMD_X86_MODE mode)
{
	bool operand_prefix = false;
	bool address_prefix = false;
	bool repeat_prefix = false;
	bool repeat_not_zero_prefix = false;
	bool rexW = false;
	bool lock_prefix = false;
	uint16_t simd_prefix = NMD_X86_PREFIXES_NONE;
	uint8_t opcode_size = 0;
	bool has_modrm = false;
	nmd_x86_modrm modrm;
    modrm.modrm = 0;
    
    /* Security considerations for memory safety:
	The contents of 'buffer' should be considered untrusted and decoded carefully.
	
	'buffer' should always point to the start of the buffer. We use the 'b'
	buffer iterator to read data from the buffer, however before accessing it
	make sure to check 'buffer_size' to see if we can safely access it. Then,
	after reading data from the buffer we increment 'b' and decrement 'buffer_size'.
	Helper macros: _NMD_READ_BYTE()
	*/
    
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
			if (mode == NMD_X86_MODE_64 && _NMD_R(*b) == 4) /* REX prefixes [0x40,0x4f] */
			{
				if(_NMD_C(*b) & 0b1000)
					rexW = true;
				continue;
			}
		}

		break;
	}

	/* Calculate the number of prefixes based on how much the iterator moved */
	const size_t num_prefixes = (uint8_t)((ptrdiff_t)(b)-(ptrdiff_t)(buffer));
    
    /* Opcode byte. This variable is used because 'op' is simpler than 'instruction->opcode' */
	uint8_t op;
	_NMD_READ_BYTE(b, buffer_size, op);
    
	if (op == 0x0F) /* 2 or 3 byte opcode */
	{
		_NMD_READ_BYTE(b, buffer_size, op);
        
		if (op == 0x38 || op == 0x3A) /* 3 byte opcode */
		{
			const bool is_opcode_map38 = op == 0x38;
			opcode_size = 3;
            
            _NMD_READ_BYTE(b, buffer_size, op);
            
			if (!_nmd_ldisasm_decode_modrm(&b, &buffer_size, address_prefix, mode, &modrm))
				return 0;
			has_modrm = true;

			if (is_opcode_map38)
			{
#ifndef NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK
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
				else if (op == 0x10 || op == 0x14 || op == 0x15 || op == 0x17 || (op >= 0x20 && op <= 0x25) || op == 0x28 || op == 0x29 || op == 0x2b || _NMD_R(op) == 3 || op == 0x40 || op == 0x41 || op == 0xcf || (op >= 0xdb && op <= 0xdf))
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
                /* "Read" the immediate byte */
                uint8_t imm;
                _NMD_READ_BYTE(b, buffer_size, imm);
                
#ifndef NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK
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
		else if (op == 0x0f) /* 3DNow! opcode map*/
		{
#ifndef NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_3DNOW
			if (!_nmd_ldisasm_decode_modrm(&b, &buffer_size, address_prefix, mode, &modrm))
				return false;
			
            uint8_t imm;
			_NMD_READ_BYTE(b, buffer_size, imm);
            
#ifndef NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK
			if (!_nmd_find_byte(_nmd_valid_3DNow_opcodes, sizeof(_nmd_valid_3DNow_opcodes), imm))
				return false;
#endif /*NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK */
#else /* NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_3DNOW */
			return false;
#endif /* NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_3DNOW */
		}
		else /* 2 byte opcode. */
		{
			opcode_size = 2;

			/* Check for ModR/M, SIB and displacement */
			if (op >= 0x20 && op <= 0x23)
            {				
                has_modrm = true;
                _NMD_READ_BYTE(b, buffer_size, modrm.modrm);
            }
			else if (op < 4 || (_NMD_R(op) != 3 && _NMD_R(op) > 0 && _NMD_R(op) < 7) || (op >= 0xD0 && op != 0xFF) || (_NMD_R(op) == 7 && _NMD_C(op) != 7) || _NMD_R(op) == 9 || _NMD_R(op) == 0xB || (_NMD_R(op) == 0xC && _NMD_C(op) < 8) || (_NMD_R(op) == 0xA && (op % 8) >= 3) || op == 0x0ff || op == 0x00 || op == 0x0d)
			{
				if (!_nmd_ldisasm_decode_modrm(&b, &buffer_size, address_prefix, mode, &modrm))
					return 0;
				has_modrm = true;
			}

#ifndef NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK
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
			else if (_NMD_R(op) == 5)
			{
				if ((op == 0x50 && modrm.fields.mod != 0b11) || (simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && (op == 0x52 || op == 0x53)) || (simd_prefix == NMD_X86_PREFIXES_REPEAT && (op == 0x50 || (op >= 0x54 && op <= 0x57))) || (repeat_not_zero_prefix && (op == 0x50 || (op >= 0x52 && op <= 0x57) || op == 0x5b)))
					return 0;
			}
			else if (_NMD_R(op) == 6)
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
			else if (_NMD_R(op) >= 0xd && _NMD_C(op) != 0 && op != 0xff && ((_NMD_C(op) == 6 && _NMD_R(op) != 0xf) ? (!simd_prefix || (_NMD_R(op) == 0xD && (simd_prefix == NMD_X86_PREFIXES_REPEAT || simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO) ? modrm.fields.mod != 0b11 : false)) : (simd_prefix == NMD_X86_PREFIXES_REPEAT || simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO || ((_NMD_C(op) == 7 && _NMD_R(op) != 0xe) ? modrm.fields.mod != 0b11 : false))))
				return 0;
			else if (has_modrm && modrm.fields.mod == 0b11)
			{
				if (op == 0xb2 || op == 0xb4 || op == 0xb5 || op == 0xc3 || op == 0xe7 || op == 0x2b || (simd_prefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && (op == 0x12 || op == 0x16)) || (!(simd_prefix == NMD_X86_PREFIXES_REPEAT || simd_prefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO) && (op == 0x13 || op == 0x17)))
					return 0;
			}
#endif /* NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK */

            uint8_t imm_mask = 0;
			if (_NMD_R(op) == 8) /* imm32 */
				imm_mask = _NMD_GET_BY_MODE_OPSZPRFX_F64(mode, operand_prefix, 2, 4, 4);
			else if ((_NMD_R(op) == 7 && _NMD_C(op) < 4) || op == 0xA4 || op == 0xC2 || (op > 0xC3 && op <= 0xC6) || op == 0xBA || op == 0xAC) /* imm8 */
				imm_mask = 1;
			else if (op == 0x78 && (repeat_not_zero_prefix || operand_prefix)) /* imm8 + imm8 = "imm16" */
				imm_mask = 2;
            
            /* Make sure we can "read" 'imm_mask' bytes from the buffer */
            if (buffer_size < imm_mask)
                return false;
            
            /* Increment the buffer and decrement the buffer's size */
            b += imm_mask;
            buffer_size -= imm_mask;
		}
	}
	else /* 1 byte opcode */
	{
		opcode_size = 1;

		/* Check for ModR/M, SIB and displacement */
		if (_NMD_R(op) == 8 || _nmd_find_byte(_nmd_op1_modrm, sizeof(_nmd_op1_modrm), op) || (_NMD_R(op) < 4 && (_NMD_C(op) < 4 || (_NMD_C(op) >= 8 && _NMD_C(op) < 0xC))) || (_NMD_R(op) == 0xD && _NMD_C(op) >= 8)/* || ((op == 0xc4 || op == 0xc5) && remaining_size > 1 && ((nmd_x86_modrm*)(b + 1))->fields.mod != 0b11)*/)
		{
			if (!_nmd_ldisasm_decode_modrm(&b, &buffer_size, address_prefix, mode, &modrm))
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
				if ((modrm.fields.reg == 0b101 && modrm.fields.mod != 0b11) || _NMD_R(modrm.modrm) == 0xf)
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
		/* Check if instruction is VEX */
		if ((op == 0xc4 || op == 0xc5) && !has_modrm)
		{
			const uint8_t byte0 = op;
            
            uint8_t byte1;
			_NMD_READ_BYTE(b, buffer_size, byte1);

			if (byte0 == 0xc4)
			{
				uint8_t byte2;
				_NMD_READ_BYTE(b, buffer_size, byte2);

                _NMD_READ_BYTE(b, buffer_size, op);

				if (op == 0x0c || op == 0x0d || op == 0x40 || op == 0x41 || op == 0x17 || op == 0x21 || op == 0x42)
				{
                    uint8_t imm;
                    _NMD_READ_BYTE(b, buffer_size, imm);
                }
			}
			else /* 0xc5 */
			{
				_NMD_READ_BYTE(b, buffer_size, op);
			}

			if (!_nmd_ldisasm_decode_modrm(&b, &buffer_size, address_prefix, mode, &modrm))
				return false;
			has_modrm = true;
		}
		else
#endif /* NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VEX */

		{
			/* Check for immediate */
            uint8_t imm_mask = 0;
			if (_nmd_find_byte(_nmd_op1_imm32, sizeof(_nmd_op1_imm32), op) || (_NMD_R(op) < 4 && (_NMD_C(op) == 5 || _NMD_C(op) == 0xD)) || (_NMD_R(op) == 0xB && _NMD_C(op) >= 8) || (op == 0xF7 && modrm.fields.reg == 0b000)) /* imm32,16 */
			{
				if (_NMD_R(op) == 0xB && _NMD_C(op) >= 8)
					imm_mask = rexW ? 8 : (operand_prefix || (mode == NMD_X86_MODE_16 && !operand_prefix) ? 2 : 4);
				else
				{
					if ((mode == NMD_X86_MODE_16 && operand_prefix) || (mode != NMD_X86_MODE_16 && !operand_prefix))
						imm_mask = NMD_X86_IMM32;
					else
						imm_mask = NMD_X86_IMM16;
				}
			}
			else if (_NMD_R(op) == 7 || (_NMD_R(op) == 0xE && _NMD_C(op) < 8) || (_NMD_R(op) == 0xB && _NMD_C(op) < 8) || (_NMD_R(op) < 4 && (_NMD_C(op) == 4 || _NMD_C(op) == 0xC)) || (op == 0xF6 && modrm.fields.reg <= 0b001) || _nmd_find_byte(_nmd_op1_imm8, sizeof(_nmd_op1_imm8), op)) /* imm8 */
				imm_mask = 1;
			else if (_NMD_R(op) == 0xA && _NMD_C(op) < 4)
				imm_mask = (mode == NMD_X86_MODE_64) ? (address_prefix ? 4 : 8) : (address_prefix ? 2 : 4);
			else if (op == 0xEA || op == 0x9A) /* imm32,48 */
			{
				if (mode == NMD_X86_MODE_64)
					return 0;
				imm_mask = (operand_prefix ? 4 : 6);
			}
			else if (op == 0xC2 || op == 0xCA) /* imm16 */
				imm_mask = 2;
			else if (op == 0xC8) /* imm16 + imm8 */
				imm_mask = 3;
            
            /* Make sure we can "read" 'imm_mask' bytes from the buffer */
            if (buffer_size < imm_mask)
                return false;
            
            /* Increment the buffer and decrement the buffer's size */
            b += imm_mask;
            buffer_size -= imm_mask;
		}
	}

	if (lock_prefix)
	{
		if (!(has_modrm && modrm.fields.mod != 0b11 &&
			((opcode_size == 1 && (op == 0x86 || op == 0x87 || (_NMD_R(op) < 4 && (op % 8) < 2 && op < 0x38) || ((op >= 0x80 && op <= 0x83) && modrm.fields.reg != 0b111) || (op >= 0xfe && modrm.fields.reg < 2) || ((op == 0xf6 || op == 0xf7) && (modrm.fields.reg == 0b010 || modrm.fields.reg == 0b011)))) ||
				(opcode_size == 2 && (_nmd_find_byte(_nmd_two_opcodes, sizeof(_nmd_two_opcodes), op) || op == 0xab || (op == 0xba && modrm.fields.reg != 0b100) || (op == 0xc7 && modrm.fields.reg == 0b001))))))
			return 0;
	}

	return (size_t)((ptrdiff_t)(b) - (ptrdiff_t)(buffer));
}