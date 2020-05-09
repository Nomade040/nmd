// This is a C library containing a x86 length disassembler. It's really just the part from the nmd_x86_decode_buffer() function from "nmd_assembly.h" without some features.
//
// Define the 'NMD_LDISASM_IMPLEMENTATION' macro in one source file to instantiate the implementation. Example:
// #define NMD_LDISASM_IMPLEMENTATION
// #include "nmd_ldisasm.h"
//
// Features:
// - No runtime initialization.
// - Thread-safe by design.
// - No dynamic memory allocation.
// - No global variables.
// - Optimized for speed and low memory usage.
// - One single function.
// - C99 compatible.
// - The only dependencies are <stdbool.h>, <stdint.h> and <stddef.h>. Check out the 'NMD_LDISASM_NO_INCLUDES' macro.
// - All of the code is in this single header file.
//
// Using absolutely no dependecies(other headers...):
//  Define the 'NMD_LDISASM_NO_INCLUDES' macro to tell the library not to include any headers. By doing so it will define the required types.
//  Be careful: Using this macro when the types(i.e. uint8_t, size_t, etc...) are already defined may cause compiler errors.
//
/* Example:
#define NMD_LDISASM_IMPLEMENTATION
#include "nmd_ldisasm.h"

#include <stdio.h>

int main()
{
	const uint8_t code[] = { 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0x81, 0xEC, 0x70, 0x01, 0x00, 0x00, 0xA1, 0x60, 0x33, 0x82, 0x77, 0x33, 0xC4 };

	size_t i = 0;
	do
	{
		const size_t length = nmd_x86_ldisasm(code + i, NMD_LDISASM_X86_MODE_32);
		if (!length)
			break;

		printf("%d\n", length);

		i += length;

	} while (i < sizeof(code));

	return 0;
}
*/

#ifndef NMD_LDISASM_H
#define NMD_LDISASM_H

#ifdef NMD_LDISASM_NO_INCLUDES

#ifndef __cplusplus

#define bool  _Bool
#define false 0
#define true  1

#endif // __cplusplus

typedef unsigned char    uint8_t;

#ifdef _WIN64
typedef unsigned __int64 size_t;
typedef __int64          ptrdiff_t;
#else
typedef unsigned int     size_t;
typedef int              ptrdiff_t;
#endif // _WIN64

#else
//Dependencies
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#endif // NMD_LDISASM_NO_INCLUDES

//X86 mode.
typedef enum NMD_LDISASM_X86_MODE
{
	NMD_LDISASM_X86_MODE_16 = 1,
	NMD_LDISASM_X86_MODE_32 = 2,
	NMD_LDISASM_X86_MODE_64 = 3,
} NMD_LDISASM_X86_MODE;

//Returns zero if the instruction is invalid, otherwise the instruction's length is returned.
//If you are using this function to parse a sequence of contiguous instructions and the return
//value is zero, you should skip one byte.
//Parameters:
//	buffer [in] A pointer to a sequence of bytes.
//	mode   [in] A member of the 'NMD_LDISASM_X86_MODE' enum.
size_t nmd_x86_ldisasm(const void* buffer, NMD_LDISASM_X86_MODE mode);

#ifdef NMD_LDISASM_IMPLEMENTATION

#define NMD_R(b) ((b) >> 4) // Four high-order bits of an opcode to index a row of the opcode table
#define NMD_C(b) ((b) & 0xF) // Four low-order bits to index a column of the table

bool nmd_ldisasm_findByte(const uint8_t* arr, const size_t N, const uint8_t x) { for (size_t i = 0; i < N; i++) { if (arr[i] == x) { return true; } }; return 0; }

typedef union NMD_ldisasm_Modrm
{
	struct
	{
		uint8_t rm : 3;
		uint8_t reg : 3;
		uint8_t mod : 2;
	};
	uint8_t modrm;
} NMD_ldisasm_Modrm;

void nmd_ldisasm_parseModrm(uint8_t** b, const bool addressPrefix, NMD_LDISASM_X86_MODE mode)
{
	const NMD_ldisasm_Modrm modrm = *(NMD_ldisasm_Modrm*)(++*b);

	if (mode == NMD_LDISASM_X86_MODE_16)
	{
		if (modrm.mod != 0b11)
		{
			if (modrm.mod == 0b00)
			{
				if (modrm.rm == 0b110)
					*b += 2;
			}
			else
				*b += modrm.mod == 0b01 ? 1 : 2;
		}
	}
	else
	{
		if (addressPrefix && mode == NMD_LDISASM_X86_MODE_32)
		{
			if ((modrm.mod == 0b00 && modrm.rm == 0b110) || modrm.mod == 0b10)
				*b += 2;
			else if (modrm.mod == 0b01)
				(*b)++;
		}
		else
		{
			//Check for SIB byte
			bool hasSIB = false;
			uint8_t sib;
			if (modrm.modrm < 0xC0 && modrm.rm == 0b100 && (!addressPrefix || (addressPrefix && mode == NMD_LDISASM_X86_MODE_64)))
				hasSIB = true, sib = *++*b;

			if (modrm.mod == 0b01) // disp8 (ModR/M)
				(*b)++;
			else if ((modrm.mod == 0b00 && modrm.rm == 0b101) || modrm.mod == 0b10) //disp16,32 (ModR/M)
				*b += (addressPrefix && !(mode == NMD_LDISASM_X86_MODE_64 && addressPrefix) ? 2 : 4);
			else if (hasSIB && (sib & 0b111) == 0b101) //disp8,32 (SIB)
				*b += (modrm.mod == 0b01 ? 1 : 4);
		}
	}
	
}

size_t nmd_x86_ldisasm(const void* buffer, NMD_LDISASM_X86_MODE mode)
{
	const uint8_t prefixes[] = { 0xF0, 0xF2, 0xF3, 0x2E, 0x36, 0x3E, 0x26, 0x64, 0x65, 0x66, 0x67 };
	const uint8_t op1modrm[] = { 0x62, 0x63, 0x69, 0x6B, 0xC0, 0xC1, 0xC4, 0xC5, 0xC6, 0xC7, 0xD0, 0xD1, 0xD2, 0xD3, 0xF6, 0xF7, 0xFE, 0xFF };
	const uint8_t op1imm8[] = { 0x6A, 0x6B, 0x80, 0x82, 0x83, 0xA8, 0xC0, 0xC1, 0xC6, 0xCD, 0xD4, 0xD5, 0xEB };
	const uint8_t op1imm32[] = { 0x68, 0x69, 0x81, 0xA9, 0xC7, 0xE8, 0xE9 };
	const uint8_t op2modrm[] = { 0x0D, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xff };

	size_t offset = 0;
	bool operandPrefix = false, addressPrefix = false, repeatPrefix = false, repeatNotZeroPrefix = false, rexW = false, lockPrefix = false;
	uint8_t simdPrefix = 0x00;
	uint8_t* b = (uint8_t*)(buffer);
	uint8_t op;
	bool hasModrm = false;
	NMD_ldisasm_Modrm modrm;
	size_t opcodeSize;

	//Parse legacy prefixes & REX prefixes
	for (int i = 0; i < 14 && nmd_ldisasm_findByte(prefixes, sizeof(prefixes), *b) || (mode == NMD_LDISASM_X86_MODE_64 ? (NMD_R(*b) == 4) : false); i++, b++)
	{
		if (*b == 0x66)
			operandPrefix = true, simdPrefix = *b;
		else if (*b == 0x67)
			addressPrefix = true;
		else if (*b == 0xf0)
			lockPrefix = true;
		else if (*b == 0xf2)
			repeatNotZeroPrefix = true, simdPrefix = *b;
		else if (*b == 0xf3)
			repeatPrefix = true, simdPrefix = *b;
		else if (NMD_R(*b) == 4 && NMD_C(*b) >= 8)
			rexW = true;
	}

	//Parse opcode(s)
	if (*b == 0x0F) // 2,3 bytes
	{
		b++;

		if (*b == 0x38 || *b == 0x3A) // 3 bytes
		{
			op = *++b;
			modrm = *(NMD_ldisasm_Modrm*)(b + 1);
			opcodeSize = 3;

			if (*(b - 1) == 0x38)
			{
				// Check if the instruction is invalid.
				if (((*b == 0xf0 || *b == 0xf1) && ((operandPrefix || !simdPrefix) && modrm.mod == 0b11)) ||
					!((!(repeatPrefix || repeatNotZeroPrefix) && (*b <= 0xb || (*b >= 0x1c && *b <= 0x1e))) || (*b >= 0xc8 && *b <= 0xcd && !simdPrefix) ||
						(simdPrefix == 0x66 && (*b == 0x10 || *b == 0x14 || *b == 0x15 || *b == 0x17 || (*b >= 0x20 && *b <= 0x25) || (*b == 0x2a && modrm.mod != 0b11) || (*b >= 0x28 && *b <= 0x2b && *b != 0x2a) || (NMD_R(*b) == 3 && *b != 0x36) || *b == 0x40 || *b == 0x41 || ((*b >= 0x80 && *b <= 0x82) && modrm.mod != 0b11) || *b == 0xcf || (*b >= 0xdb && *b <= 0xdf))) ||
						((*b == 0xf0 || *b == 0xf1) && !(repeatPrefix)) || (*b == 0xf6 && !(repeatNotZeroPrefix) && !(!simdPrefix && modrm.mod == 0b11)) || (*b == 0xf5 && simdPrefix == 0x66 && modrm.mod != 0b11) || (*b == 0xf8 && modrm.mod != 0b11 && simdPrefix != 0x00) || (*b == 0xf9 && !simdPrefix && modrm.mod != 0b11)))
					return 0;
			}
			else
			{
				offset++;

				// Check if the instruction is invalid.
				if (!(((operandPrefix || !simdPrefix) && *b == 0xf) || (*b == 0xcc && !simdPrefix) ||
					(simdPrefix == 0x66 && ((*b >= 0x8 && *b <= 0xe) || (*b >= 0x14 && *b <= 0x17) || (*b >= 0x20 && *b <= 0x22) || (*b >= 0x40 && *b <= 0x42) || *b == 0x44 || (*b >= 0x60 && *b <= 0x63) || *b == 0xdf || *b == 0xce || *b == 0xcf))))
					return 0;

			}
			hasModrm = true;
			nmd_ldisasm_parseModrm(&b, addressPrefix, mode);
		}
		else // 2 bytes
		{
			opcodeSize = 2;
			modrm = *(NMD_ldisasm_Modrm*)(b + 1);
			op = *b;
			// Check if the instruction is invalid.
			const uint8_t invalid2op[] = { 0x04, 0x0a, 0x0c, 0x0f, 0x7a, 0x7b };
			if (nmd_ldisasm_findByte(invalid2op, sizeof(invalid2op), *b) ||
				(*b == 0xc7 && ((!simdPrefix && (modrm.mod == 0b11 ? modrm.reg <= 0b101 : modrm.reg == 0b000 || modrm.reg == 0b010)) || (simdPrefix == 0xf2 && (modrm.mod == 0b11 || modrm.reg != 0b001)) || ((simdPrefix == 0x66 || simdPrefix == 0xf3) && (modrm.mod == 0b11 ? modrm.reg <= (simdPrefix == 0xf3 ? 0b110 : 0b101) : (modrm.reg != 0b001 && modrm.reg != 0b110))))) ||
				(*b == 0x00 && modrm.reg >= 0b110) ||
				(*b == 0x01 && (modrm.mod == 0b11 ? ((simdPrefix && ((modrm.modrm >= 0xc0 && modrm.modrm <= 0xc5) || (modrm.modrm >= 0xc8 && modrm.modrm <= 0xcb) || (modrm.modrm >= 0xcf && modrm.modrm <= 0xd1) || (modrm.modrm >= 0xd4 && modrm.modrm <= 0xd7) || modrm.modrm == 0xee || modrm.modrm == 0xef || modrm.modrm == 0xfa || modrm.modrm == 0xfb)) || (modrm.reg == 0b000 && modrm.rm >= 0b110) || (modrm.reg == 0b001 && modrm.rm >= 0b100 && modrm.rm <= 0b110) || (modrm.reg == 0b010 && (modrm.rm == 0b010 || modrm.rm == 0b011)) || (modrm.reg == 0b101 && modrm.rm < 0b110 && (!(repeatPrefix) || (repeatPrefix && (modrm.rm != 0b000 && modrm.rm != 0b010)))) || (modrm.reg == 0b111 && (modrm.rm > 0b101 || (!(mode == NMD_LDISASM_X86_MODE_64) && modrm.rm == 0b000)))) : (!(repeatPrefix) && modrm.reg == 0b101))) ||
				((repeatPrefix || repeatNotZeroPrefix) && ((*b >= 0x13 && *b <= 0x17 && !(*b == 0x16 && repeatPrefix)) || *b == 0x28 || *b == 0x29 || *b == 0x2e || *b == 0x2f || (*b <= 0x76 && *b >= 0x74))) ||
				(modrm.mod == 0b11 && (*b == 0xb2 || *b == 0xb4 || *b == 0xb5 || *b == 0xc3 || *b == 0xe7 || *b == 0x2b || (operandPrefix && (*b == 0x12 || *b == 0x16)) || (!(repeatPrefix || repeatNotZeroPrefix) && (*b == 0x13 || *b == 0x17)))) ||
				((*b == 0x1A || *b == 0x1B) && modrm.mod == 0b11) ||
				((*b == 0x20 || *b == 0x22) && (modrm.reg == 0b001 || modrm.reg >= 0b101)) ||
				(*b >= 0x24 && *b <= 0x27) || *b == 0x36 || *b == 0x39 || (*b >= 0x3b && *b <= 0x3f) ||
				(NMD_R(*b) == 5 && ((*b == 0x50 && modrm.mod != 0b11) || (operandPrefix && (*b == 0x52 || *b == 0x53)) || (repeatPrefix && (*b == 0x50 || (*b >= 0x54 && *b <= 0x57))) || (repeatNotZeroPrefix && (*b == 0x50 || (*b >= 0x52 && *b <= 0x57) || *b == 0x5b)))) ||
				(NMD_R(*b) == 6 && ((!simdPrefix && (*b == 0x6c || *b == 0x6d)) || (repeatPrefix && *b != 0x6f) || repeatNotZeroPrefix)) ||
				((*b == 0x78 || *b == 0x79) && ((((simdPrefix == 0x66 && *b == 0x78) && !(modrm.mod == 0b11 && modrm.reg == 0b000)) || ((simdPrefix == 0x66 || simdPrefix == 0xf2) && modrm.mod != 0b11)) || (simdPrefix == 0xf3))) ||
				((*b == 0x7c || *b == 0x7d) && (repeatPrefix || !simdPrefix)) ||
				((*b == 0x7e || *b == 0x7f) && repeatNotZeroPrefix) ||
				(((*b >= 0x71 && *b <= 0x73) && ((repeatPrefix || repeatNotZeroPrefix) || modrm.modrm <= 0xcf || (modrm.modrm >= 0xe8 && modrm.modrm <= 0xef)))) ||
				((*b == 0x71 || *b == 0x72 || (*b == 0x73 && !operandPrefix)) && ((modrm.modrm >= 0xd8 && modrm.modrm <= 0xdf) || modrm.modrm >= 0xf8)) ||
				(*b == 0x73 && (modrm.modrm >= 0xe0 && modrm.modrm <= 0xe8)) ||
				(*b == 0xa6 && modrm.modrm != 0xc0 && modrm.modrm != 0xc8 && modrm.modrm != 0xd0) ||
				(*b == 0xa7 && !(modrm.mod == 0b11 && modrm.reg <= 0b101 && modrm.rm == 0b000)) ||
				(*b == 0xae && ((!simdPrefix && modrm.mod == 0b11 && modrm.reg <= 0b100) || (simdPrefix == 0xf2 && !(modrm.mod == 0b11 && modrm.reg == 0b110)) || (simdPrefix == 0x66 && (modrm.reg < 0b110 || (modrm.mod == 0b11 && modrm.reg == 0b111))) || (simdPrefix == 0xf3 && (modrm.reg != 0b100 && modrm.reg != 0b110) && !(modrm.mod == 0b11 && modrm.reg == 0b101)))) ||
				(*b == 0xb8 && !(repeatPrefix)) ||
				(*b == 0xba && modrm.reg <= 0b011) ||
				((*b >= 0xc3 && *b <= 0xc6) && ((*b == 0xc5 && modrm.mod != 0b11) || (simdPrefix == 0xf3 || simdPrefix == 0xf2) || (*b == 0xc3 && simdPrefix == 0x66))) ||
				(*b == 0xd0 && (!simdPrefix || simdPrefix == 0xf3)) ||
				(*b == 0xe0 && (simdPrefix == 0xf3 || simdPrefix == 0xf2)) ||
				(*b == 0xf0 && (simdPrefix == 0xf2 ? modrm.mod == 0b11 : true)) ||
				(NMD_R(*b) >= 0xd && NMD_C(*b) != 0 && *b != 0xff && ((NMD_C(*b) == 6 && NMD_R(*b) != 0xf) ? (!simdPrefix || (NMD_R(*b) == 0xD && (simdPrefix == 0xf3 || simdPrefix == 0xf2) ? modrm.mod != 0b11 : false)) : (simdPrefix == 0xf3 || simdPrefix == 0xf2 || ((NMD_C(*b) == 7 && NMD_R(*b) != 0xe) ? modrm.mod != 0b11 : false)))))
				return 0;

			if (NMD_R(*b) == 8) // imm32
				offset += operandPrefix ? 2 : 4;
			else if (*b == 0x78 && (simdPrefix == 0xf2 || simdPrefix == 0x66)) // imm8 + imm8 = "imm16"
				offset += 2;
			else if ((NMD_R(*b) == 7 && NMD_C(*b) < 4) || *b == 0xA4 || *b == 0xC2 || (*b > 0xC3 && *b <= 0xC6) || *b == 0xBA || *b == 0xAC) //imm8
				offset++;

			// Check for ModR/M, SIB and displacement.
			if (*b >= 0x20 && *b <= 0x23)
				b++;
			else if (nmd_ldisasm_findByte(op2modrm, sizeof(op2modrm), *b) || *b < 4 || (NMD_R(*b) != 3 && NMD_R(*b) > 0 && NMD_R(*b) < 7) || (*b >= 0xD0 && *b != 0xFF) || (NMD_R(*b) == 7 && NMD_C(*b) != 7) || NMD_R(*b) == 9 || NMD_R(*b) == 0xB || (NMD_R(*b) == 0xC && NMD_C(*b) < 8))
				nmd_ldisasm_parseModrm(&b, addressPrefix, mode), hasModrm = true;
		}
	}
	else // 1 byte
	{
		opcodeSize = 1;
		modrm = *(NMD_ldisasm_Modrm*)(b + 1);
		op = *b;
		// Check if the instruction is invalid.
		if (((*b == 0xC6 || *b == 0xC7) && ((modrm.reg != 0b000 && modrm.reg != 0b111) || (modrm.reg == 0b111 && (modrm.mod != 0b11 || modrm.rm != 0b000)))) ||
			(*b == 0x8f && modrm.reg != 0b000) ||
			(*b == 0xfe && modrm.reg >= 0b010) ||
			(*b == 0xff && (modrm.reg == 0b111 || (modrm.mod == 0b11 && (modrm.reg == 0b011 || modrm.reg == 0b101)))) ||
			((*b == 0x8c || *b == 0x8e) && modrm.reg >= 6) ||
			(*b == 0x8e && modrm.reg == 0b001) ||
			(modrm.mod == 0b11 && (*b == 0x8d || *b == 0x62)) ||
			((*b == 0xc4 || *b == 0xc5) && mode == NMD_LDISASM_X86_MODE_64 && modrm.mod != 0b11) ||
			(mode == NMD_LDISASM_X86_MODE_64 && (*b == 0x6 || *b == 0x7 || *b == 0xe || *b == 0x16 || *b == 0x17 || *b == 0x1e || *b == 0x1f || *b == 0x27 || *b == 0x2f || *b == 0x37 || *b == 0x3f || (*b >= 0x60 && *b <= 0x62) || *b == 0x82 || *b == 0xce || (*b >= 0xd4 && *b <= 0xd6))))
			return 0;
		else if (*b >= 0xd8 && *b <= 0xdf)
		{
			switch (*b)
			{
			case 0xd9:
				if ((modrm.reg == 0b001 && modrm.mod != 0b11) || (modrm.modrm > 0xd0 && modrm.modrm < 0xd8) || modrm.modrm == 0xe2 || modrm.modrm == 0xe3 || modrm.modrm == 0xe6 || modrm.modrm == 0xe7 || modrm.modrm == 0xef)
					return 0;
				break;
			case 0xda:
				if (modrm.modrm >= 0xe0 && modrm.modrm != 0xe9)
					return 0;
				break;
			case 0xdb:
				if (((modrm.reg == 0b100 || modrm.reg == 0b110) && modrm.mod != 0b11) || (modrm.modrm >= 0xe5 && modrm.modrm <= 0xe7) || modrm.modrm >= 0xf8)
					return 0;
				break;
			case 0xdd:
				if ((modrm.reg == 0b101 && modrm.mod != 0b11) || NMD_R(modrm.modrm) == 0xf)
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

		// Check for immediate field
		if ((NMD_R(*b) == 0xE && NMD_C(*b) < 8) || (NMD_R(*b) == 0xB && NMD_C(*b) < 8) || NMD_R(*b) == 7 || (NMD_R(*b) < 4 && (NMD_C(*b) == 4 || NMD_C(*b) == 0xC)) || (*b == 0xF6 && !(*(b + 1) & 48)) || nmd_ldisasm_findByte(op1imm8, sizeof(op1imm8), *b)) //imm8
			offset++;
		else if (*b == 0xC2 || *b == 0xCA) //imm16
			offset += 2;
		else if (*b == 0xC8) //imm16 + imm8
			offset += 3;
		else if ((NMD_R(*b) < 4 && (NMD_C(*b) == 5 || NMD_C(*b) == 0xD)) || (NMD_R(*b) == 0xB && NMD_C(*b) >= 8) || (*b == 0xF7 && !(*(b + 1) & 48)) || nmd_ldisasm_findByte(op1imm32, sizeof(op1imm32), *b)) //imm32,16
		{
			if (NMD_R(*b) == 0xB && NMD_C(*b) >= 8)
				offset += rexW ? 8 : (operandPrefix || (mode == NMD_LDISASM_X86_MODE_16 && !operandPrefix) ? 2 : 4);
			else
				offset += (operandPrefix && mode == NMD_LDISASM_X86_MODE_32) || (mode == NMD_LDISASM_X86_MODE_16 && !operandPrefix) ? 2 : 4;
		}
		else if (*b == 0xEA || *b == 0x9A) //imm32,48
		{
			if (mode == NMD_LDISASM_X86_MODE_64)
				return 0;
			offset += operandPrefix ? 4 : 6;
		}
		else if (NMD_R(*b) == 0xA && NMD_C(*b) < 4)
			offset += mode == NMD_LDISASM_X86_MODE_64 ? (addressPrefix ? 4 : 8) : (addressPrefix ? 2 : 4);

		// Check for ModR/M, SIB and displacement.
		if (nmd_ldisasm_findByte(op1modrm, sizeof(op1modrm), *b) || (NMD_R(*b) < 4 && (NMD_C(*b) < 4 || (NMD_C(*b) >= 8 && NMD_C(*b) < 0xC))) || NMD_R(*b) == 8 || (NMD_R(*b) == 0xD && NMD_C(*b) >= 8))
			nmd_ldisasm_parseModrm(&b, addressPrefix, mode), hasModrm = true;
	}

	if (lockPrefix)
	{
		const uint8_t twoOpcodes[] = { 0xb0, 0xb1, 0xb3, 0xbb, 0xc0, 0xc1 };
		if (!(hasModrm && modrm.mod != 0b11 &&
			((opcodeSize == 1 && (op == 0x86 || op == 0x87 || (NMD_R(op) < 4 && (op % 8) < 2 && op < 0x38) || ((op >= 0x80 && op <= 0x83) && modrm.reg != 0b111) || (op >= 0xfe && modrm.reg < 2) || ((op == 0xf6 || op == 0xf7) && (modrm.reg == 0b010 || modrm.reg == 0b011)))) ||
				(opcodeSize == 2 && (nmd_ldisasm_findByte(twoOpcodes, sizeof(twoOpcodes), op) || op == 0xab || (op == 0xba && modrm.reg != 0b100) || (op == 0xc7 && modrm.reg == 0b001))))))
			return 0;
	}

	return (size_t)((ptrdiff_t)(++b + offset) - (ptrdiff_t)(buffer));
}

#endif // NMD_LDISASM_IMPLEMENTATION

#endif // NMD_LDISASM_H