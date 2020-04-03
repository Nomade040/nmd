// This is a C library containing a length disassembler. It's really just the disassemble() function from "nmd_assembly.h" without some features.
//
// define the 'NMD_LDISASM_IMPLEMENTATION' macro in one and only one source file.
// Example:
// #include <...>
// #include <...>
// #define NMD_LDISASM_IMPLEMENTATION
// #include "nmd_ldisasm.h"

#ifndef NMD_LDISASM_H
#define NMD_LDISASM_H

//Dependencies
#include <stdbool.h>
#include <stdint.h>

//X86 mode.
typedef enum X86_MODE
{
	X86_MODE_16 = 1,
	X86_MODE_32 = 2,
	X86_MODE_64 = 3,
} X86_MODE;

//Returns zero if the instruction is invalid, otherwise the instruction's length is returned.
//If you are using this function to parse a sequence of contiguous instructions and the return
//value is zero, you should skip one byte.
//Parameters:
//	buffer [in] A pointer to a sequence of bytes.
//	mode   [in]  A member of the 'X86_MODE' enum.
size_t ldisasm(const void* buffer, bool x86_64);

#ifdef NMD_LDISASM_IMPLEMENTATION

#define NMD_R(b) (b >> 4) // Four high-order bits of an opcode to index a row of the opcode table
#define NMD_C(b) (b & 0xF) // Four low-order bits to index a column of the table

bool nmd_ldisasm_findByte(const uint8_t* arr, const size_t N, const uint8_t x) { for (size_t i = 0; i < N; i++) { if (arr[i] == x) { return true; } }; return 0; }

void nmd_ldisasm_parseModRM(const uint8_t** b, const bool addressPrefix, X86_MODE mode)
{
	const uint8_t modrm = *++*b;

	if (mode == X86_MODE_16)
	{
		if ((modrm & 0b11000000) != 0b11000000)
		{
			if ((modrm & 0b11000000) == 0b00000000)
			{
				if ((modrm & 0b111) == 0b110)
					*b += 2;
			}
			else
				b += (modrm & 0b11000000) == 0b01000000 ? 1 : 2;
		}
	}
	else
	{
		if (addressPrefix && !(mode == X86_MODE_64))
		{
			if (((modrm & 0b11000000) == 0b00000000 && (modrm & 0b111) == 0b110) || (modrm & 0b11000000) == 0b10000000)
				b += 2;
			else if ((modrm & 0b11000000) == 0b01000000)
				b++;
		}
		else if (!addressPrefix || (addressPrefix && **b >= 0x40))
		{
			//Check for SIB byte
			bool hasSIB = false;
			if (**b < 0xC0 && (**b & 0b111) == 0b100 && !addressPrefix)
				hasSIB = true, (*b)++;

			if (modrm >= 0x40 && modrm <= 0x7F) // disp8 (ModR/M)
				b++;
			else if ((modrm <= 0x3F && (modrm & 0b111) == 0b101) || (modrm >= 0x80 && modrm <= 0xBF)) //disp16,32 (ModR/M)
				b += (addressPrefix ? 2 : 4);
			else if (hasSIB && (**b & 0b111) == 0b101) //disp8,32 (SIB)
				b += (modrm & 0b01000000 ? 1 : 4);
		}
		else if (addressPrefix && modrm == 0x26)
			*b += 2;
	}
};

size_t ldisasm(const void* const buffer, X86_MODE mode)
{

	const uint8_t prefixes[] = { 0xF0, 0xF2, 0xF3, 0x2E, 0x36, 0x3E, 0x26, 0x64, 0x65, 0x66, 0x67 };
	const uint8_t op1modrm[] = { 0x62, 0x63, 0x69, 0x6B, 0xC0, 0xC1, 0xC4, 0xC5, 0xC6, 0xC7, 0xD0, 0xD1, 0xD2, 0xD3, 0xF6, 0xF7, 0xFE, 0xFF };
	const uint8_t op1imm8[] = { 0x6A, 0x6B, 0x80, 0x82, 0x83, 0xA8, 0xC0, 0xC1, 0xC6, 0xCD, 0xD4, 0xD5, 0xEB };
	const uint8_t op1imm32[] = { 0x68, 0x69, 0x81, 0xA9, 0xC7, 0xE8, 0xE9 };
	const uint8_t op2modrm[] = { 0x0D, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF };

	size_t offset = 0;
	bool operandPrefix = false, addressPrefix = false, repeatPrefix = false, repeatNotZeroPrefix = false, rexW = false, lockPrefix = false;
	uint8_t* b = (uint8_t*)(buffer);

	//Parse legacy prefixes & REX prefixes
	for (int i = 0; i < 14 && nmd_ldisasm_findByte(prefixes, sizeof(prefixes), *b) || (mode == X86_MODE_64 ? (NMD_R(*b) == 4) : false); i++, b++)
	{
		if (*b == 0x66)
			operandPrefix = true;
		else if (*b == 0x67)
			addressPrefix = true;
		else if (*b == 0xf0)
			lockPrefix = true;
		else if (*b == 0xf2)
			repeatNotZeroPrefix = true;
		else if (*b == 0xf3)
			repeatPrefix = true;
		else if (NMD_R(*b) == 4 && NMD_C(*b) >= 8)
			rexW = true;
	}

	uint8_t op;
	bool hasModrm = false;
	uint8_t modrm;
	uint8_t mod;
	uint8_t reg;
	uint8_t rm;
	uint8_t opcodeSize;
	//Parse opcode(s)
	if (*b == 0x0F) // 2,3 bytes
	{
		b++;

		if (*b == 0x38 || *b == 0x3A) // 3 bytes
		{
			opcodeSize = 3;
			modrm = *(b + 1);
			mod = (modrm & 0b11000000) >> 6;
			reg = (modrm & 0b111000) >> 3;
			rm = (modrm & 0b111);

			if (*(b - 1) == 0x38)
			{
				op = *b;
				if (((*b == 0xf0 || *b == 0xf1) && ((operandPrefix || !(operandPrefix || repeatPrefix || repeatNotZeroPrefix)) && mod == 0b11)) ||
					!(*b <= 0xb || (*b >= 0x1c && *b <= 0x1e) || (*b >= 0xc8 && *b <= 0xcd) ||
						(operandPrefix && (*b == 0x10 || *b == 0x14 || *b == 0x15 || *b == 0x17 || (*b >= 0x20 && *b <= 0x25) || (*b == 0x2a && mod != 0b11) || (*b >= 0x28 && *b <= 0x2b && *b != 0x2a) || (NMD_R(*b) == 3 && *b != 0x36) || *b == 0x40 || *b == 0x41 || ((*b >= 0x80 && *b <= 0x82) && mod != 0b11) || (*b >= 0xdb && *b <= 0xdf))) ||
						((*b == 0xf0 || *b == 0xf1) && !(repeatPrefix)) ||
						(*b == 0xf6 && (operandPrefix || repeatPrefix))))
					return 0;
			}
			else
			{
				offset++;
				op = *b;
				if (!(((operandPrefix || !(operandPrefix || repeatPrefix || repeatNotZeroPrefix)) && *b == 0xf) || *b == 0xcc ||
					(operandPrefix && ((*b >= 0x8 && *b <= 0xe) || (*b >= 0x14 && *b <= 0x17) || (*b >= 0x20 && *b <= 0x22) || (*b >= 0x40 && *b <= 0x42) || *b == 0x44 || (*b >= 0x60 && *b <= 0x63) || *b == 0xdf))))
					return 0;
			}

			nmd_ldisasm_parseModRM(&b, addressPrefix, mode);
			hasModrm = true;
		}
		else // 2 bytes
		{
			opcodeSize = 2;
			modrm = *(b + 1);
			mod = (modrm & 0b11000000) >> 6;
			reg = (modrm & 0b111000) >> 3;
			rm = (modrm & 0b111);
			const uint8_t invalid2op[] = { 0x04, 0x0a, 0x0c, 0x0f, 0x7a, 0x7b };
			op = *b;
			if (nmd_ldisasm_findByte(invalid2op, sizeof(invalid2op), *b) ||
				(*b == 0xc7 && (reg == 0b000 || reg == 0b010 || (mod == 0b11 && (repeatPrefix ? reg != 0b111 : (!(repeatNotZeroPrefix) ? reg <= 0b101 : true))) || ((operandPrefix || repeatPrefix || repeatNotZeroPrefix) && reg >= 0b010 && reg <= 0b101))) ||
				(*b == 0x00 && reg >= 0b110) ||
				(*b == 0x01 && (mod == 0b11 ? (((operandPrefix || repeatNotZeroPrefix || repeatPrefix) && ((modrm >= 0xc0 && modrm <= 0xc5) || (modrm >= 0xc8 && modrm <= 0xcb) || (modrm >= 0xcf && modrm <= 0xd1) || (modrm >= 0xd4 && modrm <= 0xd7) || modrm == 0xee || modrm == 0xef || modrm == 0xfa || modrm == 0xfb)) || (reg == 0b000 && rm >= 0b110) || (reg == 0b001 && rm >= 0b100 && rm <= 0b110) || (reg == 0b010 && (rm == 0b010 || rm == 0b011)) || (reg == 0b101 && rm < 0b110 && (!(repeatPrefix) || (repeatPrefix && (rm != 0b000 && rm != 0b010)))) || (reg == 0b111 && (rm > 0b101 || (!(mode == X86_MODE_64) && rm == 0b000)))) : (!(repeatPrefix) && reg == 0b101))) ||
				((repeatPrefix || repeatNotZeroPrefix) && ((*b >= 0x13 && *b <= 0x17 && !(*b == 0x16 && repeatPrefix)) || *b == 0x28 || *b == 0x29 || *b == 0x2e || *b == 0x2f || (*b <= 0x76 && *b >= 0x74))) ||
				(mod == 0b11 && (*b == 0xb2 || *b == 0xb4 || *b == 0xb5 || *b == 0xc3 || *b == 0xe7 || *b == 0x2b || (operandPrefix && (*b == 0x12 || *b == 0x16)) || (!((repeatPrefix || repeatNotZeroPrefix)) && (*b == 0x13 || *b == 0x17)))) ||
				((*b == 0x1A || *b == 0x1B) && (reg >= 0b100 && mod != 0b11)) ||
				(*b >= 0x24 && *b <= 0x27) || *b == 0x36 || *b == 0x39 || (*b >= 0x3b && *b <= 0x3f) ||
				(NMD_R(*b) == 5 && ((*b == 0x50 && mod != 0b11) || (operandPrefix && (*b == 0x52 || *b == 0x53)) || (repeatPrefix && (*b == 0x50 || (*b >= 0x54 && *b <= 0x57))) || (repeatNotZeroPrefix && (*b == 0x50 || (*b >= 0x52 && *b <= 0x57) || *b == 0x5b)))) ||
				(NMD_R(*b) == 6 && ((!((operandPrefix || repeatPrefix || repeatNotZeroPrefix)) && (*b == 0x6c || *b == 0x6d)) || (repeatPrefix && *b != 0x6f) || repeatNotZeroPrefix)) ||
				((*b == 0x78 || *b == 0x79) && (operandPrefix || repeatPrefix || repeatNotZeroPrefix)) ||
				((*b == 0x7c || *b == 0x7d) && (repeatPrefix || !((operandPrefix || repeatPrefix || repeatNotZeroPrefix)))) ||
				((*b == 0x7e || *b == 0x7f) && repeatNotZeroPrefix) ||
				(((*b >= 0x71 && *b <= 0x73) && ((repeatPrefix || repeatNotZeroPrefix) || modrm <= 0xcf || (modrm >= 0xe8 && modrm <= 0xef)))) ||
				((*b == 0x71 || *b == 0x72 || (*b == 0x73 && !(operandPrefix))) && ((modrm >= 0xd8 && modrm <= 0xdf) || modrm >= 0xf8)) ||
				(*b == 0x73 && (modrm >= 0xe0 && modrm <= 0xe8)) ||
				(*b == 0xa6 && modrm != 0xc0 && modrm != 0xc8 && modrm != 0xd0) ||
				(*b == 0xa7 && !(mod == 0b11 && reg <= 0b101 && rm == 0b000)) ||
				(*b == 0xae && (repeatNotZeroPrefix || (!((operandPrefix || repeatPrefix || repeatNotZeroPrefix)) && modrm >= 0xc0 && modrm <= 0xe7) || (operandPrefix && (mod == 0b11 ? modrm != 0xf8 : reg <= 0b101)) || (repeatPrefix && (mod == 0b11 ? reg != 0b101 : (reg != 0b100 && reg != 0b110))))) ||
				(*b == 0xb8 && !(repeatPrefix)) ||
				(*b == 0xba && reg <= 0b011) ||
				(*b == 0xd0 && !(operandPrefix || repeatNotZeroPrefix)) ||
				((*b == 0xd6 || *b == 0xe6) && !((operandPrefix || repeatPrefix || repeatNotZeroPrefix))) ||
				(*b == 0xf0 && (!(repeatNotZeroPrefix) || mod == 0b11)) ||
				(((*b == 0xd6 && (repeatPrefix || repeatNotZeroPrefix)) || *b == 0xd7 || *b == 0xf7 || *b == 0xc5) && mod != 0b11))
				return 0;

			if (NMD_R(*b) == 8) //disp32
				offset = (operandPrefix ? 2 : 4);
			else if ((NMD_R(*b) == 7 && NMD_C(*b) < 4) || *b == 0xA4 || *b == 0xC2 || (*b > 0xC3 && *b <= 0xC6) || *b == 0xBA || *b == 0xAC) //imm8
				offset++;

			//Check for ModR/M, SIB and displacement
			if (*b >= 0x20 && *b <= 0x23)
				b++;
			else if (nmd_ldisasm_findByte(op2modrm, sizeof(op2modrm), *b) || (NMD_R(*b) != 3 && NMD_R(*b) > 0 && NMD_R(*b) < 7) || *b >= 0xD0 || (NMD_R(*b) == 7 && NMD_C(*b) != 7) || NMD_R(*b) == 9 || NMD_R(*b) == 0xB || (NMD_R(*b) == 0xC && NMD_C(*b) < 8))
				nmd_ldisasm_parseModRM(&b, addressPrefix, mode), hasModrm = true;
		}
	}
	else // 1 byte
	{
		opcodeSize = 1;
		//Check for potential invalid instructions
		modrm = *(b + 1);
		mod = (modrm & 0b11000000) >> 6;
		reg = (modrm & 0b111000) >> 3;
		rm = (modrm & 0b111);
		if (((*b == 0xC6 || *b == 0xC7) && ((reg != 0b000 && reg != 0b111) || (reg == 0b111 && (mod != 0b11 || rm != 0b000)))) ||
			(*b == 0x8f && reg != 0b000) ||
			(*b == 0xfe && reg >= 0b010) ||
			(*b == 0xff && (reg == 0b111 || (mod == 0b11 && (reg == 0b011 || reg == 0b101)))) ||
			((*b == 0x8c || *b == 0x8e) && reg >= 6) ||
			(*b == 0x8e && reg == 0b001) ||
			(mod == 0b11 && *b == 0x8d) ||
			((*b == 0xc4 || *b == 0xc5) && mode == X86_MODE_64 && mod != 0b11))
			return 0;
		else if (*b >= 0xd8 && *b <= 0xdf)
		{
			switch (*b)
			{
			case 0xd9:
				if ((reg == 0b001 && mod != 0b11) || (modrm > 0xd0 && modrm < 0xd8) || modrm == 0xe2 || modrm == 0xe3 || modrm == 0xe6 || modrm == 0xe7 || modrm == 0xef)
					return 0;
				break;
			case 0xda:
				if (modrm >= 0xe0 && modrm != 0xe9)
					return 0;
				break;
			case 0xdb:
				if (((reg == 0b100 || reg == 0b110) && mod != 0b11) || (modrm >= 0xe5 && modrm <= 0xe7) || modrm >= 0xf8)
					return 0;
				break;
			case 0xdd:
				if ((reg == 0b101 && mod != 0b11) || NMD_R(modrm) == 0xf)
					return 0;
				break;
			case 0xde:
				if (modrm == 0xd8 || (modrm >= 0xda && modrm <= 0xdf))
					return 0;
				break;
			case 0xdf:
				if ((modrm >= 0xe1 && modrm <= 0xe7) || modrm >= 0xf8)
					return 0;
				break;
			}
		}
		else if (mode == X86_MODE_64 && (*b == 0x6 || *b == 0x7 || *b == 0xe || *b == 0x16 || *b == 0x17 || *b == 0x1e || *b == 0x1f || *b == 0x27 || *b == 0x2f || *b == 0x37 || *b == 0x3f || (*b >= 0x60 && *b <= 0x62) || *b == 0x82 || *b == 0xce || (*b >= 0xd4 && *b <= 0xd6)))
			return 0;

			op = *b;
			//Check for immediate field
			if ((NMD_R(*b) == 0xE && NMD_C(*b) < 8) || (NMD_R(*b) == 0xB && NMD_C(*b) < 8) || NMD_R(*b) == 7 || (NMD_R(*b) < 4 && (NMD_C(*b) == 4 || NMD_C(*b) == 0xC)) || (*b == 0xF6 && !(*(b + 1) & 48)) || nmd_ldisasm_findByte(op1imm8, sizeof(op1imm8), *b)) //imm8
				offset++;
			else if (*b == 0xC2 || *b == 0xCA) //imm16
				offset += 2;
			else if (*b == 0xC8) //imm16 + imm8
				offset += 3;
			else if ((NMD_R(*b) < 4 && (NMD_C(*b) == 5 || NMD_C(*b) == 0xD)) || (NMD_R(*b) == 0xB && NMD_C(*b) >= 8) || (*b == 0xF7 && !(*(b + 1) & 48)) || nmd_ldisasm_findByte(op1imm32, sizeof(op1imm32), *b)) //imm32,16
			{
				offset = ((NMD_R(*b) == 0xB && NMD_C(*b) >= 8) && (rexW)) ? 8 : ((operandPrefix && mode != X86_MODE_32) || (mode == X86_MODE_16 && !(operandPrefix)) ? 2 : 4);
			}
			else if (*b == 0xEA || *b == 0x9A) //imm32,48
			{
				if (mode == X86_MODE_64)
					return 0;
				offset += (operandPrefix ? 4 : 6);
			}
			else if (NMD_R(*b) == 0xA && NMD_C(*b) < 4)
				offset = (mode == X86_MODE_64) ? (addressPrefix ? 4 : 8) : (addressPrefix ? 2 : 4);

			//Check for ModR/M, SIB and displacement
			if (nmd_ldisasm_findByte(op1modrm, sizeof(op1modrm), *b) || (NMD_R(*b) < 4 && (NMD_C(*b) < 4 || (NMD_C(*b) >= 8 && NMD_C(*b) < 0xC))) || NMD_R(*b) == 8 || (NMD_R(*b) == 0xD && NMD_C(*b) >= 8))
				nmd_ldisasm_parseModRM(&b, addressPrefix, mode), hasModrm = true;
	}

	if (lockPrefix)
	{
		const uint8_t twoOpcodes[] = { 0xb0, 0xb1, 0xb3, 0xbb, 0xc0, 0xc1 };
		if (!(hasModrm && mod != 0b11 &&
			((opcodeSize == 1 && (op == 0x86 || op == 0x87 || (NMD_R(op) < 4 && (NMD_C(op) % 8) < 2 && op < 0x38) || ((op >= 0x80 && op <= 0x83) && reg != 0b111) || (op >= 0xfe && reg < 2) || ((op == 0xf6 || op == 0xf7) && (reg == 0b010 || reg == 0b011)))) ||
				(opcodeSize == 2 && (nmd_ldisasm_findByte(twoOpcodes, sizeof(twoOpcodes), op) || (op == 0xba && reg != 0b100) || (op == 0xc7 && reg == 0b001))))))
			return 0;
	}

	return (size_t)((ptrdiff_t)(++b + offset) - (ptrdiff_t)(buffer));
}

#endif // NMD_LDISASM_IMPLEMENTATION

#endif // NMD_LDISASM_H