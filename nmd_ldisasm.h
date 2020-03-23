// define the 'NMD_LDISASM_IMPLEMENTATION' macro in one and only one source file.
// Example:
// #include <...>
// #include <...>
// #define NMD_LDISASM_IMPLEMENTATION
// #include "nmd_ldisasm.hpp"

#ifndef NMD_LDISASM_H
#define NMD_LDISASM_H

#include <stdbool.h>
#include <stdint.h>

//Returns zero if the instruction is invalid, otherwise the instruction's length is returned.
//Parameters:
//	buffer      [in] A pointer to a sequence of bytes.
//	x86_64_mode [in] If true, disassembles x86-64 instructions, otherwise
//                   disassembles x86-32 instructions.
size_t ldisasm(const void* buffer, bool x86_64_mode);

#ifdef NMD_LDISASM_IMPLEMENTATION

#define NMD_R(b) (b >> 4) // Four high-order bits of an opcode to index a row of the opcode table
#define NMD_C(b) (b & 0xF) // Four low-order bits to index a column of the table

bool nmd_ldisasm_findByte(const uint8_t* arr, const size_t N, const uint8_t x) { for (size_t i = 0; i < N; i++) { if (arr[i] == x) { return true; } }; return false; }

void nmd_ldisasm_parseModRM(const uint8_t** b, const bool addressPrefix, const bool x86_64)
{
	uint8_t modrm = *++*b;
	const uint8_t mod = (modrm & 0b11000000) >> 6;
	const uint8_t reg = (modrm & 0b111000) >> 3;
	const uint8_t rm = (modrm & 0b111);

	if (addressPrefix && !x86_64)
	{
		if ((mod == 0b00 && rm == 0b110) || mod == 0b10)
			*b += 2;
		else if (mod == 0b01)
			(*b)++;
	}
	else if (!addressPrefix || (addressPrefix && **b >= 0x40))
	{
		//Check for SIB byte
		bool hasSIB = false;
		if (**b < 0xC0 && (**b & 0b111) == 0b100 && !addressPrefix)
			hasSIB = true, (*b)++;
	
		if (modrm >= 0x40 && modrm <= 0x7F) // disp8 (ModR/M)
			(*b)++;
		else if ((modrm <= 0x3F && rm == 0b101) || (modrm >= 0x80 && modrm <= 0xBF)) //disp16,32 (ModR/M)
			*b += (addressPrefix ? 2 : 4);
		else if (hasSIB && (**b & 0b111) == 0b101) //disp8,32 (SIB)
			*b += (modrm & 0b01000000 ? 1 : 4);
	}
	else if (addressPrefix && modrm == 0x26)
		*b += 2;
};

size_t ldisasm(const void* const buffer, const bool x86_64)
{
	const uint8_t prefixes[] = { 0xF0, 0xF2, 0xF3, 0x2E, 0x36, 0x3E, 0x26, 0x64, 0x65, 0x66, 0x67 };
	const uint8_t op1modrm[] = { 0x62, 0x63, 0x69, 0x6B, 0xC0, 0xC1, 0xC4, 0xC5, 0xC6, 0xC7, 0xD0, 0xD1, 0xD2, 0xD3, 0xF6, 0xF7, 0xFE, 0xFF };
	const uint8_t op1imm8[] = { 0x6A, 0x6B, 0x80, 0x82, 0x83, 0xA8, 0xC0, 0xC1, 0xC6, 0xCD, 0xD4, 0xD5, 0xEB };
	const uint8_t op1imm32[] = { 0x68, 0x69, 0x81, 0xA9, 0xC7, 0xE8, 0xE9 };
	const uint8_t op2modrm[] = { 0x0D, 0xA3, 0xA4, 0xA5, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF };

	size_t offset = 0;
	bool operandPrefix = false, addressPrefix = false, repeatPrefix = false, repeatNotZeroPrefix = false, rexW = false;
	uint8_t* b = (uint8_t*)(buffer);
	
	//Parse legacy prefixes & REX prefixes
	for (int i = 0; i < 14 && nmd_ldisasm_findByte(prefixes, sizeof(prefixes), *b) || (x86_64 ? (NMD_R(*b) == 4) : false); i++, b++)
	{
		if (*b == 0x66)
			operandPrefix = true;
		else if (*b == 0x67)
			addressPrefix = true;
		else if (*b == 0xf2)
			repeatNotZeroPrefix = true;
		else if (*b == 0xf3)
			repeatPrefix = true;
		else if (NMD_R(*b) == 4 && NMD_C(*b) >= 8)
			rexW = true;
	}

	//Parse opcode(s)
	if (*b == 0x0F) // 2,3 bytes
	{
		if (*b == 0x38 || *b == 0x3A) // 3 bytes
		{
			if (*b == 0x38)
			{

			}
			else
			{
				if (((operandPrefix || repeatPrefix || repeatNotZeroPrefix) && (*b == 0x0f || *b == 0xcc)) ||
					operandPrefix && *b != 0x44 && *b != 0xdf && !(*b >= 0x08 && *b <= 0x0f) && !(*b >= 0x14 && *b <= 0x17) && !(*b >= 0x20 && *b <= 0x22) && !(*b >= 0x40 && *b <= 0x42) && !(*b >= 0x60 && *b <= 0x63))
					return false;
			}

			nmd_ldisasm_parseModRM(&b, addressPrefix, x86_64);
		}
		else // 2 bytes
		{
			const uint8_t modrm = *(b + 1);
			const uint8_t mod = (modrm & 0b11000000) >> 6;
			const uint8_t reg = (modrm & 0b111000) >> 3;
			const uint8_t rm = (modrm & 0b111);
			const uint8_t invalid2op[] = { 0x04, 0x0a, 0x0c, 0x0f };
			if (nmd_ldisasm_findByte(invalid2op, sizeof(invalid2op), *b) ||
				(*b == 0x78 || *b == 0x79) && !(operandPrefix && repeatPrefix && repeatNotZeroPrefix) ||
				(*b == 0xc7 && ((reg < 0b110 && !(reg == 0b001 && mod != 0b11)) || ((modrm & 0b11110000) && repeatPrefix) || (reg == 0b111 && mod != 0b11 && (operandPrefix || repeatPrefix || repeatNotZeroPrefix)))) ||
				(*b == 0x00 && reg >= 0b110) ||
				(*b == 0x01 && (mod == 0b11 ? ((reg == 0b000 && rm >= 0b110) || (reg == 0b001 && rm >= 0b100 && rm <= 0b110) || (reg == 0b010 && (rm == 0b010 || rm == 0b011)) || (reg == 0b101 && rm < 0b110) || (reg == 0b111 && (rm > 0b101 || (!x86_64 && rm == 0b000)))) : reg == 0b101)))
				return false;

			if (NMD_R(*b) == 8) //disp32
				offset += 4;
			else if ((NMD_R(*b) == 7 && NMD_C(*b) < 4) || *b == 0xA4 || *b == 0xC2 || (*b > 0xC3 && *b <= 0xC6) || *b == 0xBA || *b == 0xAC) //imm8
				offset++;

			//Check for ModR/M, SIB and displacement
			if (nmd_ldisasm_findByte(op2modrm, sizeof(op2modrm), *b) || (NMD_R(*b) != 3 && NMD_R(*b) > 0 && NMD_R(*b) < 7) || *b >= 0xD0 || (NMD_R(*b) == 7 && NMD_C(*b) != 7) || NMD_R(*b) == 9 || NMD_R(*b) == 0xB || (NMD_R(*b) == 0xC && NMD_C(*b) < 8) || (NMD_R(*b) == 0 && NMD_C(*b) < 4))
				nmd_ldisasm_parseModRM(&b, addressPrefix, x86_64);
		}
	}
	else // 1 byte
	{
		
		//Check for potential invalid instructions
		const uint8_t modrm = *(b + 1);
		const uint8_t mod = (modrm & 0b11000000) >> 6;
		const uint8_t reg = (modrm & 0b111000) >> 3;
		const uint8_t rm = (modrm & 0b111);
		if (*b == 0xC6 || *b == 0xC7)
		{
			if ((reg != 0b000 && reg != 0b111) || (reg == 0b111 && (mod != 0b11 || rm != 0b000)))
				return false;
		}
		else if (*b == 0x8f && reg)
			return false;
		else if (*b == 0xfe && reg >= 0b010)
			return false;
		else if (*b == 0xff && (reg == 0b111 || (mod == 0b11 && (reg == 0b011 || reg == 0b101))))
			return false;
		else if (*b >= 0xd8 && *b <= 0xdf)
		{
			switch (*b)
			{
			case 0xd9:
				if ((reg == 0b001 && mod != 0b11) || (modrm > 0xd0 && modrm < 0xd8) || modrm == 0xe2 || modrm == 0xe3 || modrm == 0xe6 || modrm == 0xe7 || modrm == 0xef)
					return false;
				break;
			case 0xda:
				if (modrm >= 0xe0 && modrm != 0xe9)
					return false;
				break;
			case 0xdb:
				if (((reg == 0b100 || reg == 0b110) && mod != 0b11) || (modrm >= 0xe5 && modrm <= 0xe7) || modrm >= 0xf8)
					return false;
				break;
			case 0xdd:
				if ((reg == 0b101 && mod != 0b11) || NMD_R(modrm) == 0xf)
					return false;
				break;
			case 0xde:
				if (modrm == 0xd8 || (modrm >= 0xda && modrm <= 0xdf))
					return false;
				break;
			case 0xdf:
				if ((modrm >= 0xe1 && modrm <= 0xe7) || modrm >= 0xf8)
					return false;
				break;
			}
		}
		else if ((*b == 0x8c || *b == 0x8e) && reg >= 6)
			return false;
		else if (*b == 0x8e && reg == 0b001)
			return false;
		else if (mod == 0b11 && (*b == 0x62 || *b == 0x8d || *b == 0xc4 || *b == 0xc5))
			return false;


		//Check for immediate field
		if ((NMD_R(*b) == 0xE && NMD_C(*b) < 8) || (NMD_R(*b) == 0xB && NMD_C(*b) < 8) || NMD_R(*b) == 7 || (NMD_R(*b) < 4 && (NMD_C(*b) == 4 || NMD_C(*b) == 0xC)) || (*b == 0xF6 && !(*(b + 1) & 48)) || nmd_ldisasm_findByte(op1imm8, sizeof(op1imm8), *b)) //imm8
			offset++;
		else if (*b == 0xC2 || *b == 0xCA) //imm16
			offset += 2;
		else if (*b == 0xC8) //imm16 + imm8
			offset += 3;
		else if ((NMD_R(*b) < 4 && (NMD_C(*b) == 5 || NMD_C(*b) == 0xD)) || (NMD_R(*b) == 0xB && NMD_C(*b) >= 8) || (*b == 0xF7 && !(*(b + 1) & 48)) || nmd_ldisasm_findByte(op1imm32, sizeof(op1imm32), *b)) //imm32,16
			offset += ((NMD_R(*b) == 0xB && NMD_C(*b) >= 8) && rexW) ? 8 : (operandPrefix ? 2 : 4);
		else if (*b == 0xEA || *b == 0x9A) //imm32,48
		{
			if (x86_64)
				return false;
			offset += (operandPrefix ? 4 : 6);
		}
		else if (NMD_R(*b) == 0xA && NMD_C(*b) < 4)
			offset += x86_64 ? (addressPrefix ? 4 : 8) : (addressPrefix ? 2 : 4);

		//Check for ModR/M, SIB and displacement
		if (nmd_ldisasm_findByte(op1modrm, sizeof(op1modrm), *b) || (NMD_R(*b) < 4 && (NMD_C(*b) < 4 || (NMD_C(*b) >= 8 && NMD_C(*b) < 0xC))) || NMD_R(*b) == 8 || (NMD_R(*b) == 0xD && NMD_C(*b) >= 8))
			nmd_ldisasm_parseModRM(&b, addressPrefix, x86_64);
	}

	return (size_t)((ptrdiff_t)(++b + offset) - (ptrdiff_t)(buffer));
}

#endif // NMD_LDISASM_IMPLEMENTATION

#endif // NMD_LDISASM_H