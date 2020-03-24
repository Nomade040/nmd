// Important structure:
//  'Instruction': represents one x86 instruction.
//
// Interface:
//  assemble() -> take as input a string and fill a 'Instruction'.
//  disassemble() -> take as input a sequence of bytes and fill a 'Instruction'.
//  construct_string() -> take as input a 'Instruction' and construct its string representation.
//
// define the 'NMD_ASSEMBLY_IMPLEMENTATION' macro in one and only one source file.
// Example:
// #include <...>
// #include <...>
// #define NMD_ASSEMBLY_IMPLEMENTATION
// #include "nmd_assembly.h"

#ifndef NMD_ASSEMBLY_H
#define NMD_ASSEMBLY_H

#include <stdbool.h>
#include <stdint.h>

#define MAXIMUM_INSTRUCTION_STRING_LENGTH 128

enum FORMAT_FLAGS
{
	FORMAT_FLAGS_HEX                   = 0x01, // If set, numbers are displayed in hex base, otherwise they are displayed in decimal base.
	FORMAT_FLAGS_POINTER_SIZE          = 0x02, // Pointer sizes(e.g. 'dword ptr', 'byte ptr') are displayed.
	FORMAT_FLAGS_ONLY_SEGMENT_OVERRIDE = 0x04, // If set, only segment overrides using prefixes(e.g. '2EH', '64H') are displayed, otherwise a segment is always present before a memory operand.
	FORMAT_FLAGS_ARGUMENT_SPACES       = 0x08, // A space is placed between arguments.
	FORMAT_FLAGS_UPPERCASE             = 0x10, // The string is uppercase.
	FORMAT_FLAGS_0X_PREFIX             = 0x20, // Hexadecimal numbers have the '0x'('0X' if uppercase) prefix.
	FORMAT_FLAGS_H_SUFFIX              = 0x40, // Hexadecimal numbers have the 'h'('H' if uppercase') suffix.
	FORMAT_FLAGS_ALL                   = 0x7F, // Specifies all format flags.
	FORMAT_FLAGS_DEFAULT               = (FORMAT_FLAGS_HEX | FORMAT_FLAGS_ONLY_SEGMENT_OVERRIDE | FORMAT_FLAGS_ARGUMENT_SPACES)
};

enum INSTRUCTION_PREFIXES
{
	ES_SEGMENT_OVERRIDE_PREFIX   = 0x0001,
	CS_SEGMENT_OVERRIDE_PREFIX   = 0x0002,
	SS_SEGMENT_OVERRIDE_PREFIX   = 0x0004,
	DS_SEGMENT_OVERRIDE_PREFIX   = 0x0008,
	FS_SEGMENT_OVERRIDE_PREFIX   = 0x0010,
	GS_SEGMENT_OVERRIDE_PREFIX   = 0x0020,
	LOCK_PREFIX                  = 0x0040,
	REPEAT_NOT_ZERO_PREFIX       = 0x0080,
	REPEAT_PREFIX                = 0x0100,
	OPERAND_SIZE_OVERRIDE_PREFIX = 0x0200,
	ADDRESS_SIZE_OVERRIDE_PREFIX = 0x0400,
	REX_W_PREFIX                 = 0x0800,
	REX_R_PREFIX                 = 0x1000,
	REX_X_PREFIX                 = 0x2000,
	REX_B_PREFIX                 = 0x4000
};

enum IMM_MASK
{
	IMM8    = 0x01,
	IMM16   = 0x02,
	IMM32   = 0x04,
	IMM48   = 0x06,
	IMM64   = 0x08,
	ANY_IMM = (IMM8 | IMM16 | IMM32 | IMM64)
};

enum DISP_MASK
{
	DISP8    = 0x01,
	DISP16   = 0x02,
	DISP32   = 0x04,
	ANY_DISP = (DISP8 | DISP16 | DISP32)
};

typedef union Modrm
{
	struct
	{
		uint8_t rm  : 3;
		uint8_t reg : 3;
		uint8_t mod : 2;
	};
	uint8_t modrm;
} Modrm;

typedef union SIB
{
	struct
	{
		uint8_t base  : 3;
		uint8_t index : 3;
		uint8_t scale : 2;
	};
	uint8_t sib;
} SIB;

//You may use this enum to construct a mask and check against InstructionFlags::flags.
enum INSTRUCTION_FLAGS
{
	INSTRUCTION_FLAGS_VALID           = 0x01,
	INSTRUCTION_FLAGS_X86_64          = 0x02,
	INSTRUCTION_FLAGS_HAS_MODRM       = 0x04,
	INSTRUCTION_FLAGS_HAS_SIB         = 0x08,
	INSTRUCTION_FLAGS_OPERAND_SIZE_64 = 0x10,
};

typedef union InstructionFlags
{
	struct
	{
		bool valid         : 1; // If true, the instruction is valid.
		bool x86_64        : 1; // If true, the instruction was parsed as a x86_64 instruction.
		bool hasModrm      : 1;
		bool hasSIB        : 1;
		bool is3op38h      : 1; // If true, the instruction has a three byte opcode and the 3-byte escape is 38h.
		bool operandSize64 : 1; // If true, a REX.W is closer to the opcode than a operand size override prefix.
		bool repeatPrefix  : 1; // If true, a 'repeat' prefix is closer to the opcode than a 'repeat not zero' prefix.
	};
	uint8_t flags;
} InstructionFlags;

typedef struct Instruction
{
	InstructionFlags flags;
	uint8_t length;
	uint64_t address;
	uint8_t numPrefixes;
	uint16_t prefixes;
	uint8_t segmentOverride;
	uint8_t opcodeSize;
	uint8_t opcode;
	uint8_t fullInstruction[15];
	Modrm modrm;
	SIB sib;
	uint8_t immMask, dispMask;
	uint64_t immediate;
	uint32_t displacement;
} Instruction;

//Assembles an instruction from a string. Returns true if the operation was successful, false otherwise.
//Parameters:
//  string      [in]  A pointer to a string.
//  instruction [out] A pointer to a variable of type 'Instruction'.
//	x86_64      [in]  If true, disassembles x86-64 instructions, otherwise
//                    disassembles x86-32 instructions.
bool assemble(const char* string, Instruction* instruction, bool x86_64);

//Disassembles an instruction from a sequence of bytes. Returns true if the instruction is valid, false otherwise.
//Parameters:
//	buffer      [in]  A pointer to a sequence of bytes.
//	instruction [out] A pointer to a variable of type 'Instruction'.
//  address     [in]  The instruction's address on an executable. If this
//                    parameter is NULL, displacements are prefixed with '$+'.
//	x86_64      [in]  If true, disassembles x86-64 instructions, otherwise
//                    disassembles x86-32 instructions.
bool disassemble(const void* buffer, Instruction* instruction, uintptr_t address, bool x86_64);

//Constructs a string from a pointer to an 'Instruction' struct.
//Parameters:
//	instruction [in]  A pointer to a variable of type 'Instruction'.
//	buffer      [out] A pointer to buffer that receives the string.
//  formatFlags [in]  A mask of 'FORMAT_FLAGS_XXX' specifying how the
//                    string should be constructed.
void construct_string(const Instruction* instruction, char* buffer, uint32_t formatFlags);

#ifdef NMD_ASSEMBLY_IMPLEMENTATION

#define NMD_R(b) (b >> 4) // Four high-order bits of an opcode to index a row of the opcode table
#define NMD_C(b) (b & 0xF) // Four low-order bits to index a column of the table

bool nmd_asm_findByte(const uint8_t* arr, const size_t N, const uint8_t x) { for (size_t i = 0; i < N; i++) { if (arr[i] == x) { return true; } }; return false; }

void nmd_asm_parseModRM(const uint8_t** b, Instruction* const instruction)
{
	instruction->flags.hasModrm = true;
	uint8_t modrm = instruction->modrm.modrm = *++ * b;
	bool addressPrefix = (bool)(instruction->prefixes & ADDRESS_SIZE_OVERRIDE_PREFIX);

	if (addressPrefix && !instruction->flags.x86_64)
	{
		if ((instruction->modrm.mod == 0b00 && instruction->modrm.rm == 0b110) || instruction->modrm.mod == 0b10)
			instruction->dispMask = DISP16;
		else if (instruction->modrm.mod == 0b01)
			instruction->dispMask = DISP8;
	}
	else if (!addressPrefix || (addressPrefix && **b >= 0x40))
	{
		//Check for SIB byte
		if (**b < 0xC0 && (**b & 0b111) == 0b100 && !addressPrefix)
			instruction->flags.hasSIB = true, instruction->sib.sib = *(*b + 1), (*b)++;

		if (modrm >= 0x40 && modrm <= 0x7F) // disp8 (ModR/M)
			instruction->dispMask = DISP8;
		else if ((modrm <= 0x3F && (modrm & 0b111) == 0b101) || (modrm >= 0x80 && modrm <= 0xBF)) //disp16,32 (ModR/M)
			instruction->dispMask = (addressPrefix ? DISP16 : DISP32);
		else if (instruction->flags.hasSIB && (**b & 0b111) == 0b101) //disp8,32 (SIB)
			instruction->dispMask = (modrm & 0b01000000 ? DISP8 : DISP32);
	}
	else if (addressPrefix && modrm == 0x26)
		*b += 2;

	for (int i = 0; i < instruction->dispMask; i++, (*b)++)
		((uint8_t*)(&instruction->displacement))[i] = *(*b + 1);
};

const char* nmd_asm_strstr(const char* buffer, const char* string)
{
	const char* begin = buffer;
	const char* b = buffer;
	const char* s = string;
	for (; *b; b++, s++)
	{
		if (*s == '\0')
			return begin;

		if (*b != *s)
			s = string, begin = b;
	}

	return b;
}

bool assemble(const char* const string, Instruction* const instruction, const bool x86_64)
{
	if (*string == '\0')
		return false;

	//Clear instruction
	for (int i = 0; i < sizeof(Instruction); i++)
		((uint8_t*)(instruction))[i] = 0x00;

	instruction->flags.x86_64 = x86_64;

	char buffer[MAXIMUM_INSTRUCTION_STRING_LENGTH];
	char* b = buffer;

	//Copy 'string' to 'buffer' converting it to lowercase.
	size_t length = 0;
	for (; string[length]; length++)
	{
		const char c = string[length];
		buffer[length] = (c >= 'A' && c <= 'Z') ? c + 0x20 : c;
	}

	buffer[length] = '\0';

	//Remove any number of contiguous of white-spaces at the end of the string.
	for (size_t i = length - 1; i > 0; i--)
	{
		if (buffer[i] != ' ')
		{
			buffer[i + 1] = '\0';
			length = i + 1;
			break;
		}
	}

	//Remove all white-spaces where the previous character is not a lowercase letter. 
	for (size_t i = 0; i < length; i++)
	{
		if (buffer[i] == ' ' && (i == 0 || !(buffer[i - 1] >= 'a' && buffer[i - 1] <= 'z')))
		{
			for (size_t j = i; j < length; j++)
				buffer[j] = buffer[j + 1];

			length--;
		}
	}

	if (nmd_asm_strstr(buffer, "lock ") == buffer)
		instruction->prefixes |= LOCK_PREFIX, b += 5;
	else if (nmd_asm_strstr(buffer, "rep ") == buffer)
		instruction->prefixes |= REPEAT_PREFIX, b += 4;
	else if(nmd_asm_strstr(buffer, "repe ") == buffer || nmd_asm_strstr(buffer, "repz ") == buffer)
		instruction->prefixes |= REPEAT_PREFIX, b += 5;
	else if (nmd_asm_strstr(buffer, "repne ") == buffer || nmd_asm_strstr(buffer, "repnz ") == buffer)
		instruction->prefixes |= REPEAT_NOT_ZERO_PREFIX, b += 6;

	//find opcode...

	return true;
}

bool disassemble(const void* const buffer, Instruction* const instruction, const uintptr_t address, const bool x86_64)
{
	const uint8_t prefixes[] = { 0xF0, 0xF2, 0xF3, 0x2E, 0x36, 0x3E, 0x26, 0x64, 0x65, 0x66, 0x67 };
	const uint8_t op1modrm[] = { 0x62, 0x63, 0x69, 0x6B, 0xC0, 0xC1, 0xC4, 0xC5, 0xC6, 0xC7, 0xD0, 0xD1, 0xD2, 0xD3, 0xF6, 0xF7, 0xFE, 0xFF };
	const uint8_t op1imm8[] = { 0x6A, 0x6B, 0x80, 0x82, 0x83, 0xA8, 0xC0, 0xC1, 0xC6, 0xCD, 0xD4, 0xD5, 0xEB };
	const uint8_t op1imm32[] = { 0x68, 0x69, 0x81, 0xA9, 0xC7, 0xE8, 0xE9 };
	const uint8_t op2modrm[] = { 0x0D, 0xA3, 0xA4, 0xA5, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF };

	//Clear instruction
	for (int i = 0; i < sizeof(Instruction); i++)
		((uint8_t*)(instruction))[i] = 0x00;

	instruction->address = address;
	instruction->flags.x86_64 = x86_64;

	size_t offset = 0;
	const uint8_t* b = (const uint8_t*)(buffer);

	//Parse legacy prefixes & REX prefixes
	for (int i = 0; i < 14; i++, b++)
	{
		switch (*b)
		{
		case 0xF0: instruction->prefixes |= LOCK_PREFIX; continue;
		case 0xF2: instruction->prefixes |= REPEAT_NOT_ZERO_PREFIX, instruction->flags.repeatPrefix = false; continue;
		case 0xF3: instruction->prefixes |= REPEAT_PREFIX, instruction->flags.repeatPrefix = true; continue;
		case 0x2E: instruction->prefixes |= (instruction->segmentOverride = CS_SEGMENT_OVERRIDE_PREFIX); continue;
		case 0x36: instruction->prefixes |= (instruction->segmentOverride = SS_SEGMENT_OVERRIDE_PREFIX); continue;
		case 0x3E: instruction->prefixes |= (instruction->segmentOverride = DS_SEGMENT_OVERRIDE_PREFIX); continue;
		case 0x26: instruction->prefixes |= (instruction->segmentOverride = ES_SEGMENT_OVERRIDE_PREFIX); continue;
		case 0x64: instruction->prefixes |= (instruction->segmentOverride = FS_SEGMENT_OVERRIDE_PREFIX); continue;
		case 0x65: instruction->prefixes |= (instruction->segmentOverride = GS_SEGMENT_OVERRIDE_PREFIX); continue;
		case 0x66: instruction->prefixes |= OPERAND_SIZE_OVERRIDE_PREFIX, instruction->flags.operandSize64 = false; continue;
		case 0x67: instruction->prefixes |= ADDRESS_SIZE_OVERRIDE_PREFIX; continue;
		default:
			if (x86_64 && NMD_R(*b) == 4) // 0x40
			{
				if (*b & 0b0001) // bit position 0
					instruction->prefixes |= REX_B_PREFIX;
				else if (*b & 0b0010) // bit position 1
					instruction->prefixes |= REX_X_PREFIX;
				else if (*b & 0b0100) // bit position 2
					instruction->prefixes |= REX_R_PREFIX;
				else if (*b & 0b1000) // bit position 3
					instruction->prefixes |= REX_W_PREFIX, instruction->flags.operandSize64 = true;

				continue;
			}
		}

		break;
	}

	instruction->numPrefixes = (uint8_t)((ptrdiff_t)(b)-(ptrdiff_t)(buffer));

	//Parse opcode(s)
	if (*b == 0x0F) // 2,3 bytes
	{
		if (*b == 0x38 || *b == 0x3A) // 3 bytes
		{
			instruction->opcodeSize = 3;
			instruction->opcode = *b;

			if (*b == 0x38)
			{

			}
			else
			{
				instruction->immMask = IMM8, offset++;

				if (((instruction->prefixes & (OPERAND_SIZE_OVERRIDE_PREFIX | REPEAT_PREFIX | REPEAT_NOT_ZERO_PREFIX)) && (*b == 0x0f || *b == 0xcc)) ||
					(instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX) && *b != 0x44 && *b != 0xdf && !(*b >= 0x08 && *b <= 0x0f) && !(*b >= 0x14 && *b <= 0x17) && !(*b >= 0x20 && *b <= 0x22) && !(*b >= 0x40 && *b <= 0x42) && !(*b >= 0x60 && *b <= 0x63))
					return false;
			}

			nmd_asm_parseModRM(&b, instruction);
		}
		else // 2 bytes
		{
			instruction->opcodeSize = 2;
			instruction->opcode = *++b;

			const Modrm modrm = *(Modrm*)(b + 1);
			const uint8_t invalid2op[] = { 0x04, 0x0a, 0x0c, 0x0f };
			if (nmd_asm_findByte(invalid2op, sizeof(invalid2op), *b) ||
				(*b == 0x78 || *b == 0x79) && !(instruction->prefixes & (OPERAND_SIZE_OVERRIDE_PREFIX | REPEAT_PREFIX | REPEAT_NOT_ZERO_PREFIX)) ||
				(*b == 0xc7 && ((modrm.reg < 0b110 && !(modrm.reg == 0b001 && modrm.mod != 0b11)) || ((modrm.modrm & 0b11110000) && (instruction->prefixes & REPEAT_PREFIX)) || (modrm.reg == 0b111 && modrm.mod != 0b11 && (instruction->prefixes & (OPERAND_SIZE_OVERRIDE_PREFIX | REPEAT_PREFIX | REPEAT_NOT_ZERO_PREFIX))))) ||
				(*b == 0x00 && modrm.reg >= 0b110) ||
				(*b == 0x01 && (modrm.mod == 0b11 ? ((modrm.reg == 0b000 && modrm.rm >= 0b110) || (modrm.reg == 0b001 && modrm.rm >= 0b100 && modrm.rm <= 0b110) || (modrm.reg == 0b010 && (modrm.rm == 0b010 || modrm.rm == 0b011)) || (modrm.reg == 0b101 && modrm.rm < 0b110) || (modrm.reg == 0b111 && (modrm.rm > 0b101 || (!instruction->flags.x86_64 && modrm.rm == 0b000)))) : modrm.reg == 0b101)))
				return false;

			if (NMD_R(*b) == 8) //disp32
				instruction->immMask = IMM32, offset += 4;
			else if ((NMD_R(*b) == 7 && NMD_C(*b) < 4) || *b == 0xA4 || *b == 0xC2 || (*b > 0xC3 && *b <= 0xC6) || *b == 0xBA || *b == 0xAC) //imm8
				instruction->immMask = IMM8, offset++;

			//Check for ModR/M, SIB and displacement
			if (nmd_asm_findByte(op2modrm, sizeof(op2modrm), *b) || (NMD_R(*b) != 3 && NMD_R(*b) > 0 && NMD_R(*b) < 7) || *b >= 0xD0 || (NMD_R(*b) == 7 && NMD_C(*b) != 7) || NMD_R(*b) == 9 || NMD_R(*b) == 0xB || (NMD_R(*b) == 0xC && NMD_C(*b) < 8) || (NMD_R(*b) == 0 && NMD_C(*b) < 4))
				nmd_asm_parseModRM(&b, instruction);
		}
	}
	else // 1 byte
	{
		//Check for potential invalid instructions
		const Modrm modrm = *(Modrm*)(b + 1);
		if (*b == 0xC6 || *b == 0xC7)
		{
			if ((modrm.reg != 0b000 && modrm.reg != 0b111) || (modrm.reg == 0b111 && (modrm.mod != 0b11 || modrm.rm != 0b000)))
				return false;
		}
		else if (*b == 0x8f && modrm.reg)
			return false;
		else if (*b == 0xfe && modrm.reg >= 0b010)
			return false;
		else if (*b == 0xff && (modrm.reg == 0b111 || (modrm.mod == 0b11 && (modrm.reg == 0b011 || modrm.reg == 0b101))))
			return false;
		else if (*b >= 0xd8 && *b <= 0xdf)
		{
			switch (*b)
			{
			case 0xd9:
				if ((modrm.reg == 0b001 && modrm.mod != 0b11) || (modrm.modrm > 0xd0 && modrm.modrm < 0xd8) || modrm.modrm == 0xe2 || modrm.modrm == 0xe3 || modrm.modrm == 0xe6 || modrm.modrm == 0xe7 || modrm.modrm == 0xef)
					return false;
				break;
			case 0xda:
				if (modrm.modrm >= 0xe0 && modrm.modrm != 0xe9)
					return false;
				break;
			case 0xdb:
				if (((modrm.reg == 0b100 || modrm.reg == 0b110) && modrm.mod != 0b11) || (modrm.modrm >= 0xe5 && modrm.modrm <= 0xe7) || modrm.modrm >= 0xf8)
					return false;
				break;
			case 0xdd:
				if ((modrm.reg == 0b101 && modrm.mod != 0b11) || NMD_R(modrm.modrm) == 0xf)
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
		else if ((*b == 0x8c || *b == 0x8e) && modrm.reg >= 6)
			return false;
		else if (*b == 0x8e && modrm.reg == 0b001)
			return false;
		else if (modrm.mod == 0b11 && (*b == 0x62 || *b == 0x8d || *b == 0xc4 || *b == 0xc5))
			return false;

		instruction->opcodeSize = 1;
		instruction->opcode = *b;

		//Check for immediate field
		if ((NMD_R(*b) == 0xE && NMD_C(*b) < 8) || (NMD_R(*b) == 0xB && NMD_C(*b) < 8) || NMD_R(*b) == 7 || (NMD_R(*b) < 4 && (NMD_C(*b) == 4 || NMD_C(*b) == 0xC)) || (*b == 0xF6 && !(*(b + 1) & 48)) || nmd_asm_findByte(op1imm8, sizeof(op1imm8), *b)) //imm8
			instruction->immMask = IMM8, offset++;
		else if (*b == 0xC2 || *b == 0xCA) //imm16
			instruction->immMask = IMM16, offset += 2;
		else if (*b == 0xC8) //imm16 + imm8
			instruction->immMask = IMM16 | IMM8, offset += 3;
		else if ((NMD_R(*b) < 4 && (NMD_C(*b) == 5 || NMD_C(*b) == 0xD)) || (NMD_R(*b) == 0xB && NMD_C(*b) >= 8) || (*b == 0xF7 && !(*(b + 1) & 48)) || nmd_asm_findByte(op1imm32, sizeof(op1imm32), *b)) //imm32,16
		{
			instruction->immMask = ((NMD_R(*b) == 0xB && NMD_C(*b) >= 8) && (instruction->prefixes & REX_W_PREFIX)) ? IMM64 : (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? IMM16 : IMM32);
			offset += instruction->immMask;
		}
		else if (*b == 0xEA || *b == 0x9A) //imm32,48
		{
			if (x86_64)
				return false;
			instruction->immMask = (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? IMM32 : IMM48), offset += instruction->immMask;
		}
		else if (NMD_R(*b) == 0xA && NMD_C(*b) < 4)
			instruction->immMask = x86_64 ? (instruction->prefixes & ADDRESS_SIZE_OVERRIDE_PREFIX ? IMM32 : IMM64) : (instruction->prefixes & ADDRESS_SIZE_OVERRIDE_PREFIX ? IMM16 : IMM32), offset += instruction->immMask;

		//Check for ModR/M, SIB and displacement
		if (nmd_asm_findByte(op1modrm, sizeof(op1modrm), *b) || (NMD_R(*b) < 4 && (NMD_C(*b) < 4 || (NMD_C(*b) >= 8 && NMD_C(*b) < 0xC))) || NMD_R(*b) == 8 || (NMD_R(*b) == 0xD && NMD_C(*b) >= 8))
			nmd_asm_parseModRM(&b, instruction);
	}

	if (instruction->prefixes & LOCK_PREFIX)
	{
		const uint8_t op = instruction->opcode;
		const uint8_t twoOpcodes[] = { 0xb0, 0xb1, 0xb3, 0xbb, 0xc0, 0xc1 };
		if (!(instruction->flags.hasModrm && instruction->modrm.mod != 0b11 &&
			((instruction->opcodeSize == 1 && (op == 0x86 || op == 0x87 || (NMD_R(op) < 4 && (NMD_C(op) % 8) < 2 && op < 0x38) || ((op >= 0x80 && op <= 0x83) && instruction->modrm.reg != 0b111) || (op >= 0xfe && instruction->modrm.reg < 2) || ((op == 0xf6 || op == 0xf7) && (instruction->modrm.reg == 0b010 || instruction->modrm.reg == 0b011)))) ||
			(instruction->opcodeSize == 2 && (nmd_asm_findByte(twoOpcodes, sizeof(twoOpcodes), op) || (op == 0xba && instruction->modrm.reg != 0b100) || (op == 0xc7 && instruction->modrm.reg == 0b001))))))
			return false;
	}

	instruction->length = (uint8_t)((ptrdiff_t)(++b + offset) - (ptrdiff_t)(buffer));
	for (int i = 0; i < instruction->length; i++)
		instruction->fullInstruction[i] = ((const uint8_t*)(buffer))[i];

	for (int i = 0; i < instruction->immMask; i++)
		((uint8_t*)(&instruction->immediate))[i] = b[i];

	instruction->flags.valid = true;

	return true;
}

static const char* const reg8[] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
static const char* const reg16[] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" };
static const char* const reg32[] = { "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi" };
static const char* const reg64[] = { "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi" };
static const char* const regrx[] = { "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15" };
static const char* const segmentReg[] = { "es", "cs", "ss", "ds", "fs", "gs" };

static const char* const conditionSuffixes[] = { "o", "no", "b", "ae", "e", "ne", "be", "a", "s", "ns", "p", "np", "l", "ge", "le", "g" };

static const char* const op1OpcodeMapMnemonics[] = { "add", "adc", "and", "xor", "or", "sbb", "sub", "cmp" };
static const char* const opcodeExtensionsGrp1[] = { "add", "or", "adc", "sbb", "and", "sub", "xor", "cmp" };
static const char* const opcodeExtensionsGrp2[] = { "rol", "ror", "rcl", "rcr", "shl", "shr", "shl", "sar" };
static const char* const opcodeExtensionsGrp3[] = { "test", "test", "not", "neg", "mul", "imul", "div", "idiv" };
static const char* const opcodeExtensionsGrp5[] = { "inc", "dec", "call", "call far", "jmp", "jmp far", "push" };
static const char* const opcodeExtensionsGrp6[] = { "sldt", "str", "lldt", "ltr", "verr", "verw" };
static const char* const opcodeExtensionsGrp7[] = { "sgdt", "sidt", "lgdt", "lidt", "smsw", NULL, "lmsw", "invlpg" };
static const char* const opcodeExtensionsGrp7reg0[] = { "enclv", "vmcall", "vmlaunch", "vmresume", "vmxoff", "pconfig" };
static const char* const opcodeExtensionsGrp7reg1[] = { "monitor", "mwait", "clac", "stac", NULL, NULL, NULL, "encls" };
static const char* const opcodeExtensionsGrp7reg2[] = { "xgetbv", "xsetbv", NULL, NULL, "vmfunc", "xend", "xtest", "enclu" };
static const char* const opcodeExtensionsGrp7reg3[] = { "vmrun eax", "vmmcall", "vmload eax", "vmsave", "stgi", "clgi", "skinit eax", "invlpga eax,ecx" };
static const char* const opcodeExtensionsGrp7reg7[] = { "swapgs", "rdtscp", "monitorx", "mwaitx", "clzero eax", "rdpru" };

static const char* const escapeOpcodesD8[] = { "add", "mul", "com", "comp", "sub", "subr", "div", "divr" };
static const char* const escapeOpcodesD9[] = { "ld", NULL, "st", "stp", "ldenv", "ldcw", "nstenv", "nstcw" };
static const char* const escapeOpcodesDA_DE[] = { "iadd", "imul", "icom", "icomp", "isub", "isubr", "idiv", "idivr" };
static const char* const escapeOpcodesDB[] = { "ild", "isttp", "ist", "istp", NULL, "ld", NULL, "stp" };
static const char* const escapeOpcodesDC[] = { "add", "mul", "com", "comp", "sub", "subr", "div", "divr" };
static const char* const escapeOpcodesDD[] = { "ld", "isttp", "st", "stp", "rstor", NULL, "nsave", "nstsw" };
static const char* const escapeOpcodesDF[] = { "ild", "isttp", "ist", "istp", "bld", "ild", "bstp", "istp" };
static const char* const* escapeOpcodes[] = { escapeOpcodesD8, escapeOpcodesD9, escapeOpcodesDA_DE, escapeOpcodesDB, escapeOpcodesDC, escapeOpcodesDD, escapeOpcodesDA_DE, escapeOpcodesDF };

typedef struct StringInfo
{
	char* buffer;
	const Instruction* instruction;
	uint32_t formatFlags;
} StringInfo;

void appendString(StringInfo* const si, const char* source)
{
	while (*source)
		*si->buffer++ = *source++;
};

 size_t getNumDigits(uint64_t n, bool hex)
{
	size_t numDigits = 0;
	while ((n /= (hex ? 16 : 10)) > 0)
		numDigits++;

	return numDigits;
}

void appendNumber(StringInfo* const si, uint64_t n)
{
	size_t numDigits = getNumDigits(n, si->formatFlags & FORMAT_FLAGS_HEX);
	size_t bufferOffset = numDigits + 1;

	if ((si->formatFlags & (FORMAT_FLAGS_HEX | FORMAT_FLAGS_0X_PREFIX)) == (FORMAT_FLAGS_HEX | FORMAT_FLAGS_0X_PREFIX))
		*si->buffer++ = '0', * si->buffer++ = 'x';

	if (si->formatFlags & FORMAT_FLAGS_HEX)
	{
		do {
			size_t num = n % 16;
			*(si->buffer + numDigits--) = (char)(num > 9 ? 0x37 + num : 0x30 + num);
		} while ((n /= 16) > 0);
	}
	else
	{
		do {
			*(si->buffer + numDigits--) = (char)(0x30 + n % 10);
		} while ((n /= 10) > 0);
	}

	si->buffer += bufferOffset;

	if (si->formatFlags & FORMAT_FLAGS_H_SUFFIX)
		*si->buffer++ = 'h';
}

void appendSignedNumber(StringInfo* const si, int64_t n, bool showPositiveSign)
{
	if (n >= 0)
	{
		if (showPositiveSign)
			*si->buffer++ = '+';

		appendNumber(si, n);
	}
	else
	{
		*si->buffer++ = '-';
		appendNumber(si, ~n + 1);
	}
}

void appendModRmMemoryPrefix(StringInfo* const si, const char* addrSpecifierReg)
{
	if (si->formatFlags & FORMAT_FLAGS_POINTER_SIZE)
	{
		appendString(si, addrSpecifierReg);
		appendString(si, " ptr ");
	}

	if (!(si->formatFlags & FORMAT_FLAGS_ONLY_SEGMENT_OVERRIDE && !si->instruction->segmentOverride))
	{
		size_t i = 0;
		if (si->instruction->segmentOverride)
		{
			while (!(si->instruction->segmentOverride & (1 << i)))
				i++;
		}

		appendString(si, si->instruction->segmentOverride ? segmentReg[i] : ((si->instruction->modrm.mod == 0b01 || si->instruction->modrm.mod == 0b10) && si->instruction->modrm.rm == 0b101) ? "ss" : "ds");
		*si->buffer++ = ':';
	}

	*si->buffer++ = '[';
}

void appendModRm16Upper(StringInfo* const si, const char* addrSpecifierReg)
{
	appendModRmMemoryPrefix(si, addrSpecifierReg);

	const char* addresses[] = { "bx+si", "bx+di", "bp+si", "bp+di", "si", "di", "bp", "bx" };

	if (!(si->instruction->modrm.mod == 0b00 && si->instruction->modrm.rm == 0b110))
		appendString(si, addresses[si->instruction->modrm.rm]);

	if (si->instruction->displacement)
	{
		bool isSigned = (((si->instruction->dispMask == DISP8) ? (int8_t)(si->instruction->displacement) : ((si->instruction->dispMask == DISP32) ? (int32_t)(si->instruction->displacement) : (int16_t)(si->instruction->displacement))) > 0);
		if (si->instruction->modrm.mod == 0b00 && si->instruction->modrm.rm == 0b110)
			appendNumber(si, (uint32_t)(si->instruction->displacement));
		else
		{
			*si->buffer++ = isSigned ? '+' : '-';
			appendNumber(si, (size_t)(isSigned ? 1 : -1) * ((si->instruction->dispMask == DISP8) ? (int8_t)(si->instruction->displacement) : ((si->instruction->dispMask == DISP32) ? (int32_t)(si->instruction->displacement) : (int16_t)(si->instruction->displacement))));
		}
	}

	*si->buffer++ = ']';
}

void appendModRm32Upper(StringInfo* const si, const char* addrSpecifierReg)
{
	appendModRmMemoryPrefix(si, addrSpecifierReg);

	if (si->instruction->flags.hasSIB)
	{
		if (si->instruction->sib.base == 0b101 && si->instruction->modrm.mod == 0b00)
		{
			appendString(si, reg32[si->instruction->sib.index]);

			if (si->instruction->sib.scale != 0b00)
				*si->buffer++ = '*', * si->buffer++ = (char)(0x30 + (2 << (si->instruction->sib.scale - 1)));
		}
		else
		{
			if (si->instruction->sib.base == 0b101)
			{
				appendString(si, "ebp");
				if (si->instruction->sib.index != 0b100)
					*si->buffer++ = '+';
			}
			else
			{
				appendString(si, (si->instruction->flags.x86_64 ? reg64 : reg32)[si->instruction->sib.base]);
				if (si->instruction->sib.index != 0b100)
					*si->buffer++ = '+';
			}

			if (si->instruction->sib.index != 0b100)
			{
				appendString(si, reg32[si->instruction->sib.index]);

				if (si->instruction->sib.scale != 0b00)
					*si->buffer++ = '*', * si->buffer++ = (char)(0x30 + (2 << (si->instruction->sib.scale - 1)));
			}
		}
	}
	else if (!(si->instruction->modrm.mod == 0b00 && si->instruction->modrm.rm == 0b101))
		appendString(si, (si->instruction->flags.x86_64 && !(si->instruction->prefixes & ADDRESS_SIZE_OVERRIDE_PREFIX) ? reg64 : reg32)[si->instruction->modrm.rm]);

	if ((si->instruction->displacement || *(si->buffer - 1) == '[') && (si->instruction->modrm.mod > 0b00 || (si->instruction->modrm.mod == 0b00 && (si->instruction->modrm.rm == 0b101 || (si->instruction->modrm.rm == 0b100 && si->instruction->sib.base == 0b101)))))
	{
		bool isSigned = (((si->instruction->dispMask == DISP8) ? (int8_t)(si->instruction->displacement) : ((si->instruction->dispMask == DISP32) ? (int32_t)(si->instruction->displacement) : (int16_t)(si->instruction->displacement))) > 0);
		if (si->instruction->modrm.mod == 0b00 && si->instruction->modrm.rm == 0b101)
			appendNumber(si, (si->instruction->address && (si->instruction->opcode != 0xff && si->instruction->length == 1)) ? (ptrdiff_t)(si->instruction->address + si->instruction->length) + (int32_t)(si->instruction->displacement) : (uint32_t)(si->instruction->displacement));
		else
		{
			*si->buffer++ = isSigned ? '+' : '-';
			appendNumber(si, (size_t)(isSigned ? 1 : -1) * ((si->instruction->dispMask == DISP8) ? (int8_t)(si->instruction->displacement) : ((si->instruction->dispMask == DISP32) ? (int32_t)(si->instruction->displacement) : (int16_t)(si->instruction->displacement))));
		}
	}

	*si->buffer++ = ']';
}

void appendModRmUpper(StringInfo* const si, const char* addrSpecifierReg)
{
	if (si->instruction->prefixes & ADDRESS_SIZE_OVERRIDE_PREFIX && !si->instruction->flags.x86_64)
		appendModRm16Upper(si, addrSpecifierReg);
	else
		appendModRm32Upper(si, addrSpecifierReg);
}

void appendNq(StringInfo* const si)
{
	*si->buffer++ = 'm', * si->buffer++ = 'm';
	*si->buffer++ = (char)(0x30 + si->instruction->modrm.rm);
}

void appendPq(StringInfo* const si)
{
	*si->buffer++ = 'm', * si->buffer++ = 'm';
	*si->buffer++ = (char)(0x30 + si->instruction->modrm.reg);
}

void appendVdq(StringInfo* const si)
{
	*si->buffer++ = 'x';
	appendPq(si);
}

void appendVqq(StringInfo* const si)
{
	*si->buffer++ = 'y';
	appendPq(si);
}

void appendVx(StringInfo* const si)
{
	if (si->instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX)
		appendVdq(si);
	else
		appendVqq(si);
}

void appendUdq(StringInfo* const si)
{
	*si->buffer++ = 'x';
	appendNq(si);
}

void appendUqq(StringInfo* const si)
{
	*si->buffer++ = 'x';
	appendNq(si);
}

void appendUx(StringInfo* const si)
{
	if (si->instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX)
		appendUdq(si);
	else
		appendUqq(si);
}

void appendQq(StringInfo* const si)
{
	if (si->instruction->modrm.mod == 0b11)
		appendPq(si);
	else
		appendModRmUpper(si, "qword");
}

void appendEv(StringInfo* const si)
{
	if (si->instruction->modrm.mod == 0b11)
		appendString(si, (si->instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? reg16 : reg32)[si->instruction->modrm.rm]);
	else
		appendModRmUpper(si, si->instruction->flags.operandSize64 ? "qword" : (si->instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? "word" : "dword"));
}

void appendEb(StringInfo* const si)
{
	if (si->instruction->modrm.mod == 0b11)
		appendString(si, reg8[si->instruction->modrm.rm]);
	else
		appendModRmUpper(si, "byte");
}

void appendEw(StringInfo* const si)
{
	if (si->instruction->modrm.mod == 0b11)
		appendString(si, reg16[si->instruction->modrm.rm]);
	else
		appendModRmUpper(si, "word");
}

void appendEd(StringInfo* const si)
{
	if (si->instruction->modrm.mod == 0b11)
		appendString(si, reg32[si->instruction->modrm.rm]);
	else
		appendModRmUpper(si, "dword");
}

void appendEq(StringInfo* const si)
{
	if (si->instruction->modrm.mod == 0b11)
		appendString(si, reg64[si->instruction->modrm.rm]);
	else
		appendModRmUpper(si, "qword");
}

 void appendGv(StringInfo* const si)
{
	appendString(si, (si->instruction->flags.operandSize64 ? reg64 : (si->instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? reg16 : reg32))[si->instruction->modrm.reg]);
}

 void appendGb(StringInfo* const si)
{
	appendString(si, reg8[si->instruction->modrm.reg]);
}

 void appendGw(StringInfo* const si)
{
	appendString(si, reg16[si->instruction->modrm.reg]);
}

void appendW(StringInfo* const si)
{
	if (si->instruction->modrm.mod == 0b11)
		appendString(si, "xmm"), * si->buffer++ = (char)(0x30 + si->instruction->modrm.rm);
	else
		appendModRmUpper(si, "xmmword");
}

void construct_string(const Instruction* const instruction, char* const buffer, uint32_t formatFlags)
{
	if (!instruction->flags.valid)
		return;

	StringInfo si;
	si.buffer = buffer;
	si.instruction = instruction;
	si.formatFlags = formatFlags;

	const uint8_t op = instruction->opcode;

	if (instruction->prefixes & LOCK_PREFIX)
		appendString(&si, "lock ");

	//If the instruction has a 'repeat' or 'repeat not zero' prefix and has the lock prefix or the opcode is 0x86/0x87:
	if (instruction->prefixes & (REPEAT_PREFIX | REPEAT_NOT_ZERO_PREFIX) && 
		((instruction->prefixes & LOCK_PREFIX || ((op == 0x86 || op == 0x87) && instruction->modrm.mod != 0b11))))
		appendString(&si, instruction->flags.repeatPrefix ? "xrelease" : "xacquire ");

	if (instruction->opcodeSize == 1)
	{
		if (NMD_R(op) < 4 && (NMD_C(op) < 6 || (NMD_C(op) >= 8 && NMD_C(op) < 0xE)))
		{
			appendString(&si, op1OpcodeMapMnemonics[NMD_R((NMD_C(op) > 6 ? op + 0x40 : op))]);
			*si.buffer++ = ' ';

			switch (NMD_C(op) % 8)
			{
			case 0:
				appendEb(&si);
				*si.buffer++ = ',';
				appendGb(&si);
				break;
			case 1:
				appendEv(&si);
				*si.buffer++ = ',';
				appendGv(&si);
				break;
			case 2:
				appendGb(&si);
				*si.buffer++ = ',';
				appendEb(&si);
				break;
			case 3:
				appendGv(&si);
				*si.buffer++ = ',';
				appendEv(&si);
				break;
			case 4:
				appendString(&si, "al,");
				appendNumber(&si, instruction->immediate);
				break;
			case 5:
				appendString(&si, instruction->flags.operandSize64 ? "rax" : (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? "ax" : "eax"));
				*si.buffer++ = ',';
				appendNumber(&si, instruction->immediate);
				break;
			}
		}
		else if (NMD_R(op) == 4 || NMD_R(op) == 5) //Parse all one byte opcodes in the interval [0x40, 0x60[
		{
			appendString(&si, NMD_C(op) < 8 ? (NMD_R(op) == 4 ? "inc " : "push ") : (NMD_R(op) == 4 ? "dec " : "pop "));
			appendString(&si, (instruction->prefixes & REX_B_PREFIX ? regrx : (instruction->flags.x86_64 ? reg64 : (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? reg16 : reg32)))[NMD_C(op) % 8]);
		}
		else if (op == 0x62)
		{
			appendString(&si, "bound ");
			appendGv(&si);
			*si.buffer++ = ',';
			appendModRmUpper(&si, instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? "dword" : "qword");
		}
		else if (op == 0x63)
		{
			if (instruction->flags.x86_64)
			{
				appendString(&si, "movsdx ");
				appendGv(&si);
				*si.buffer++ = ',';
				appendEv(&si);
			}
			else
			{
				appendString(&si, "arpl ");
				appendEw(&si);
				*si.buffer++ = ',';
				appendGw(&si);
			}
		}
		else if (op == 0x68 || op == 0x6A)
		{
			appendString(&si, "push ");
			appendNumber(&si, (op == 0x6a && instruction->immediate > 0x7f) ? (0xffffff00 | instruction->immediate) : instruction->immediate);
		}
		else if (op == 0x69 || op == 0x6B)
		{
			appendString(&si, "imul ");
			appendGv(&si);
			*si.buffer++ = ',';
			appendEv(&si);
			*si.buffer++ = ',';
			appendNumber(&si, instruction->immediate);
		}
		else if (NMD_R(op) == 7)
		{
			*si.buffer++ = 'j';
			appendString(&si, conditionSuffixes[NMD_C(op)]);
			*si.buffer++ = ' ';
			if (instruction->address)
				appendNumber(&si, (ptrdiff_t)(instruction->address + instruction->length) + (int8_t)(instruction->immediate));
			else
			{
				*si.buffer++ = '$';
				appendSignedNumber(&si, (int64_t)((int8_t)(instruction->immediate) + (int8_t)(instruction->length)), true);
			}
		}
		else if (op >= 0x80 && op < 0x84) // [80,83]
		{
			appendString(&si, opcodeExtensionsGrp1[instruction->modrm.reg]);
			*si.buffer++ = ' ';
			if (op == 0x80 || op == 0x82)
				appendEb(&si);
			else
				appendEv(&si);
			*si.buffer++ = ',';
			appendNumber(&si, instruction->immediate);
		}
		else if (op >= 0x84 && op <= 0x87)
		{
			appendString(&si, op > 0x85 ? "xchg " : "test ");
			if (op % 2 == 0)
			{
				appendEb(&si);
				*si.buffer++ = ',';
				appendGb(&si);
			}
			else
			{
				appendEv(&si);
				*si.buffer++ = ',';
				appendGv(&si);
			}
		}
		else if (op >= 0x88 && op <= 0x8e && op != 0x8d)
		{
			appendString(&si, "mov ");
			if (op == 0x88)
			{
				appendEb(&si);
				*si.buffer++ = ',';
				appendGb(&si);
			}
			else if (op == 0x89)
			{
				appendEv(&si);
				*si.buffer++ = ',';
				appendGv(&si);
			}
			else if (op == 0x8a)
			{
				appendGb(&si);
				*si.buffer++ = ',';
				appendEb(&si);
			}
			else if (op == 0x8b)
			{
				appendGv(&si);
				*si.buffer++ = ',';
				appendEv(&si);
			}
			else if (op == 0x8c)
			{
				if (si.instruction->modrm.mod == 0b11)
					appendString(&si, (si.instruction->flags.operandSize64 ? reg64 : (si.instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? reg16 : reg32))[si.instruction->modrm.rm]);
				else
					appendModRmUpper(&si, "word");

				*si.buffer++ = ',';
				appendString(&si, segmentReg[instruction->modrm.reg]);
			}
			else if (op == 0x8e)
			{
				appendString(&si, segmentReg[instruction->modrm.reg]);
				*si.buffer++ = ',';
				appendEw(&si);
			}
		}
		else if (op == 0x8d)
		{
			appendString(&si, "lea ");
			appendGv(&si);
			*si.buffer++ = ',';
			appendModRmUpper(&si, "dword");
		}
		else if (op == 0x8f)
		{
			appendString(&si, "pop ");
			appendEv(&si);
		}
		else if (op == 0x90 && instruction->prefixes & REPEAT_PREFIX)
		{
			appendString(&si, "pause");
		}
		else if (op > 0x90 && op < 0x98)
		{
			appendString(&si, "xchg ");
			appendString(&si, (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? reg16 : reg32)[NMD_C(op)]);
			appendString(&si, (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? ",ax" : ",eax"));
		}
		else if (op >= 0xA0 && op < 0xA4)
		{
			appendString(&si, "mov ");

			if (op == 0xa0)
			{
				appendString(&si, "al,");
				appendModRmMemoryPrefix(&si, "byte");
				appendNumber(&si, (uint64_t)(instruction->immediate));
				*si.buffer++ = ']';
			}
			else if (op == 0xa1)
			{
				appendString(&si, instruction->flags.operandSize64 ? "rax,q" : (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? "ax," : "eax,d"));
				appendModRmMemoryPrefix(&si, "word");
				appendNumber(&si, (uint64_t)(instruction->immediate));
				*si.buffer++ = ']';
			}
			else if (op == 0xa2)
			{
				appendModRmMemoryPrefix(&si, "byte");
				appendNumber(&si, (uint64_t)(instruction->immediate));
				appendString(&si, "],al");
			}
			else if (op == 0xa3)
			{
				if (instruction->flags.operandSize64)
					*si.buffer++ = 'q';
				else if (!(instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX))
					*si.buffer++ = 'd';
				appendModRmMemoryPrefix(&si, "word");
				appendNumber(&si, (uint64_t)(instruction->immediate));
				appendString(&si, "],");
				appendString(&si, instruction->flags.operandSize64 ? "rax" : (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? "ax" : "eax"));
			}
		}
		else if (op == 0x9A)
		{
			appendString(&si, "call far ");
			appendNumber(&si, (uint64_t)(*(uint16_t*)((char*)(&instruction->immediate) + (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? 2 : 4))));
			*si.buffer++ = ':';
			appendNumber(&si, (uint64_t)(instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? *((uint16_t*)(&instruction->immediate)) : *((uint32_t*)(&instruction->immediate))));
		}
		else if (op == 0xa8)
		{
			appendString(&si, "test al,");
			appendNumber(&si, (uint64_t)(instruction->immediate));
		}
		else if (op == 0xa9)
		{
			appendString(&si, instruction->flags.operandSize64 ? "test rax" : (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? "test ax" : "test eax"));
			*si.buffer++ = ',';
			appendNumber(&si, (uint64_t)(instruction->immediate));
		}
		else if ((op >= 0x6c && op <= 0x6f) || (op >= 0xa4 && op <= 0xa7) || (op >= 0xaa && op <= 0xaf))
		{
			if (instruction->prefixes & REPEAT_PREFIX)
				appendString(&si, "rep ");
			else if (instruction->prefixes & REPEAT_NOT_ZERO_PREFIX)
				appendString(&si, "repne ");

			const char* str = 0;
			switch (op)
			{
			case 0x6c: case 0x6d: str = "ins"; break;
			case 0x6e: case 0x6f: str = "outs"; break;
			case 0xa4: case 0xa5: str = "movs"; break;
			case 0xa6: case 0xa7: str = "cmps"; break;
			case 0xaa: case 0xab: str = "stos"; break;
			case 0xac: case 0xad: str = "lods"; break;
			case 0xae: case 0xaf: str = "scas"; break;
			}
			appendString(&si, str);

			*si.buffer++ = ((op % 2 == 0) ? 'b' : (instruction->flags.operandSize64 ? 'q' : ((instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX) ? 'w' : 'd')));
		}
		else if (NMD_R(op) == 0xb)
		{
			appendString(&si, "mov ");
			appendString(&si, (NMD_C(op) < 8 ? reg8 : (instruction->flags.operandSize64 ? reg64 : (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? reg16 : reg32)))[NMD_C(op) % 8]);
			*si.buffer++ = ',';
			appendNumber(&si, (uint64_t)(instruction->immediate));
		}
		else if (op == 0xC0 || op == 0xC1 || (NMD_R(op) == 0xd && NMD_C(op) < 4))
		{
			appendString(&si, opcodeExtensionsGrp2[instruction->modrm.reg]);
			*si.buffer++ = ' ';
			if (op % 2 == 0)
				appendEb(&si);
			else
				appendEv(&si);
			*si.buffer++ = ',';
			if (NMD_R(op) == 0xc)
				appendNumber(&si, (uint64_t)(instruction->immediate));
			else if (NMD_C(op) < 2)
				appendNumber(&si, 1);
			else
				appendString(&si, "cl");
		}
		else if (op == 0xc2)
		{
			appendString(&si, "ret ");
			appendNumber(&si, (uint64_t)(instruction->immediate));
		}
		else if (op == 0xc4 || op == 0xc5)
		{
			appendString(&si, op == 0xc4 ? "les" : "lds");
			*si.buffer++ = ' ';
			appendGv(&si);
			*si.buffer++ = ',';
			if (si.instruction->modrm.mod == 0b11)
				appendString(&si, (si.instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? reg16 : reg32)[si.instruction->modrm.rm]);
			else
				appendModRmUpper(&si, si.instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? "dword" : "fword");
		}
		else if (op == 0xc6 || op == 0xc7)
		{
			appendString(&si, instruction->modrm.reg == 0b000 ? "mov " : (op == 0xc6 ? "xabort " : "xbegin "));
			if (op == 0xc6)
				appendEb(&si);
			else
				appendEv(&si);
			*si.buffer++ = ',';
			appendNumber(&si, (uint64_t)(instruction->immediate));
		}
		else if (op == 0xc8)
		{
			appendString(&si, "enter ");
			appendNumber(&si, (uint64_t)(*(uint16_t*)(&instruction->immediate)));
			*si.buffer++ = ',';
			appendNumber(&si, (uint64_t)(*((uint8_t*)(&instruction->immediate) + 2)));
		}
		else if (op == 0xca)
		{
			appendString(&si, "ret far ");
			appendNumber(&si, (uint64_t)(instruction->immediate));
		}
		else if (op == 0xcd)
		{
			appendString(&si, "int ");
			appendNumber(&si, (uint64_t)(instruction->immediate));
		}
		else if (op == 0xd4)
		{
			appendString(&si, "aam ");
			appendNumber(&si, (uint64_t)(instruction->immediate));
		}
		else if (op == 0xd5)
		{
			appendString(&si, "aad ");
			appendNumber(&si, (uint64_t)(instruction->immediate));
		}
		else if (op >= 0xd8 && op <= 0xdf)
		{
			*si.buffer++ = 'f';

			if (instruction->modrm.modrm < 0xc0)
			{
				appendString(&si, escapeOpcodes[NMD_C(op) - 8][instruction->modrm.reg]);
				*si.buffer++ = ' ';
				switch (op)
				{
				case 0xd8: case 0xda: appendModRmUpper(&si, "dword"); break;
				case 0xd9: appendModRmUpper(&si, instruction->modrm.reg & 0b100 ? (instruction->modrm.reg & 0b001 ? "word" : (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? "m14" : "m28")) : "dword"); break;
				case 0xdb: appendModRmUpper(&si, instruction->modrm.reg & 0b100 ? "tbyte" : "dword"); break;
				case 0xdc: appendModRmUpper(&si, "qword"); break;
				case 0xdd: appendModRmUpper(&si, instruction->modrm.reg & 0b100 ? ((instruction->modrm.reg & 0b111) == 0b111 ? "word" : "byte") : "qword"); break;
				case 0xde: appendModRmUpper(&si, "word"); break;
				case 0xdf: appendModRmUpper(&si, instruction->modrm.reg & 0b100 ? (instruction->modrm.reg & 0b001 ? "qword" : "tbyte") : "word"); break;
				}
			}
			else
			{
				switch (op)
				{
				case 0xd8:
					appendString(&si, escapeOpcodesD8[(NMD_R(instruction->modrm.modrm) - 0xc) * 2 + (NMD_C(instruction->modrm.modrm) > 7 ? 1 : 0)]);
					appendString(&si, " st(0),st(");
					*si.buffer++ = (char)(0x30 + instruction->modrm.modrm % 8), * si.buffer++ = ')';
					break;
				case 0xd9:
					if (NMD_R(instruction->modrm.modrm) == 0xc)
					{
						appendString(&si, NMD_C(instruction->modrm.modrm) < 8 ? "ld" : "xch");
						appendString(&si, " st(0),st(");
						*si.buffer++ = (char)(0x30 + instruction->modrm.modrm % 8), * si.buffer++ = ')';
					}
					else if (instruction->modrm.modrm >= 0xd8 && instruction->modrm.modrm <= 0xdf)
					{
						appendString(&si, "stpnce st(");
						*si.buffer++ = (char)(0x30 + instruction->modrm.modrm % 8);
						appendString(&si, "),st(0)");
					}
					else
					{
						const char* str = 0;
						switch (instruction->modrm.modrm)
						{
						case 0xd0: str = "nop"; break;
						case 0xe0: str = "chs"; break;
						case 0xe1: str = "abs"; break;
						case 0xe4: str = "tst"; break;
						case 0xe5: str = "xam"; break;
						case 0xe8: str = "ld1"; break;
						case 0xe9: str = "ldl2t"; break;
						case 0xea: str = "ldl2e"; break;
						case 0xeb: str = "ldpi"; break;
						case 0xec: str = "ldlg2"; break;
						case 0xed: str = "ldln2"; break;
						case 0xee: str = "ldz"; break;
						case 0xf0: str = "2xm1"; break;
						case 0xf1: str = "yl2x"; break;
						case 0xf2: str = "ptan"; break;
						case 0xf3: str = "patan"; break;
						case 0xf4: str = "xtract"; break;
						case 0xf5: str = "prem1"; break;
						case 0xf6: str = "decstp"; break;
						case 0xf7: str = "incstp"; break;
						case 0xf8: str = "prem"; break;
						case 0xf9: str = "yl2xp1"; break;
						case 0xfa: str = "sqrt"; break;
						case 0xfb: str = "sincos"; break;
						case 0xfc: str = "rndint"; break;
						case 0xfd: str = "scale"; break;
						case 0xfe: str = "sin"; break;
						case 0xff: str = "cos"; break;
						}
						appendString(&si, str);
					}
					break;
				case 0xda:
					if (instruction->modrm.modrm == 0xe9)
						appendString(&si, "ucompp");
					else
					{
						const char* mnemonics[4] = { "cmovb", "cmovbe", "cmove", "cmovu" };
						appendString(&si, mnemonics[(NMD_R(instruction->modrm.modrm) - 0xc) + (NMD_C(instruction->modrm.modrm) > 7 ? 2 : 0)]);
						appendString(&si, " st(0),st(");
						*si.buffer++ = (char)(0x30 + NMD_C(instruction->modrm.modrm) % 8);
						*si.buffer++ = ')';
					}
					break;
				case 0xdb:
					if (NMD_R(instruction->modrm.modrm) == 0xe && NMD_C(instruction->modrm.modrm) < 8)
					{
						const char* mnemonics[] = { "eni8087_nop", "disi8087_nop","nclex","ninit","setpm287_nop" };
						appendString(&si, mnemonics[NMD_C(instruction->modrm.modrm)]);
					}
					else
					{
						if (instruction->modrm.modrm >= 0xe0)
							appendString(&si, instruction->modrm.modrm < 0xf0 ? "ucomi" : "comi");
						else
						{
							appendString(&si, "cmovn");
							if (instruction->modrm.modrm < 0xc8)
								*si.buffer++ = 'b';
							else if (instruction->modrm.modrm < 0xd0)
								*si.buffer++ = 'e';
							else if (instruction->modrm.modrm >= 0xd8)
								*si.buffer++ = 'u';
							else
								appendString(&si, "be");
						}
						appendString(&si, " st(0),st(");
						*si.buffer++ = (char)(0x30 + NMD_C(instruction->modrm.modrm) % 8);
						*si.buffer++ = ')';
					}
					break;
				case 0xdc:
					if (NMD_R(instruction->modrm.modrm) == 0xc)
						appendString(&si, NMD_C(instruction->modrm.modrm) > 7 ? "mul" : "add");
					else
					{
						appendString(&si, NMD_R(instruction->modrm.modrm) == 0xd ? "com" : (NMD_R(instruction->modrm.modrm) == 0xe ? "subr" : "div"));
						if (NMD_R(instruction->modrm.modrm) == 0xd && NMD_C(instruction->modrm.modrm) >= 8)
						{
							if (NMD_R(instruction->modrm.modrm) >= 8)
								*si.buffer++ = 'p';
						}
						else
						{
							if (NMD_R(instruction->modrm.modrm) < 8)
								*si.buffer++ = 'r';
						}
					}

					if (NMD_R(instruction->modrm.modrm) == 0xd)
					{
						appendString(&si, " st(0),st(");
						*si.buffer++ = (char)(0x30 + NMD_C(instruction->modrm.modrm) % 8);
						*si.buffer++ = ')';
					}
					else
					{
						appendString(&si, " st(");
						*si.buffer++ = (char)(0x30 + NMD_C(instruction->modrm.modrm) % 8);
						appendString(&si, "),st(0)");
					}
					break;
				case 0xdd:
					if (NMD_R(instruction->modrm.modrm) == 0xc)
						appendString(&si, NMD_C(instruction->modrm.modrm) < 8 ? "free" : "xch");
					else
					{
						appendString(&si, instruction->modrm.modrm < 0xe0 ? "st" : "ucom");
						if (NMD_C(instruction->modrm.modrm) >= 8)
							*si.buffer++ = 'p';
					}

					appendString(&si, " st(");
					*si.buffer++ = (char)(0x30 + NMD_C(instruction->modrm.modrm) % 8);
					*si.buffer++ = ')';

					break;
				case 0xde:
					if (instruction->modrm.modrm == 0xd9)
						appendString(&si, "compp");
					else
					{
						if (instruction->modrm.modrm >= 0xd0 && instruction->modrm.modrm <= 0xd7)
						{
							appendString(&si, "comp st(0),st(");
							*si.buffer++ = (char)(0x30 + NMD_C(instruction->modrm.modrm) % 8);
							*si.buffer++ = ')';
						}
						else
						{
							if (NMD_R(instruction->modrm.modrm) == 0xc)
								appendString(&si, NMD_C(instruction->modrm.modrm) < 8 ? "add" : "mul");
							else
							{
								appendString(&si, instruction->modrm.modrm < 0xf0 ? "sub" : "div");
								if (NMD_R(instruction->modrm.modrm) < 8 || (NMD_R(instruction->modrm.modrm) >= 0xe && NMD_C(instruction->modrm.modrm) < 8))
									*si.buffer++ = 'r';
							}
							appendString(&si, "p st(");
							*si.buffer++ = (char)(0x30 + NMD_C(instruction->modrm.modrm) % 8);
							appendString(&si, "),st(0)");
						}
					}
					break;
				case 0xdf:
					if (instruction->modrm.modrm == 0xe0)
						appendString(&si, "nstsw ax");
					else
					{
						if (instruction->modrm.modrm >= 0xe8)
						{
							if (instruction->modrm.modrm < 0xf0)
								*si.buffer++ = 'u';
							appendString(&si, "comip");
							appendString(&si, " st(0),st(");
							*si.buffer++ = (char)(0x30 + NMD_C(instruction->modrm.modrm) % 8);
							*si.buffer++ = ')';
						}
						else
						{
							appendString(&si, instruction->modrm.modrm < 0xc8 ? "freep" : (instruction->modrm.modrm >= 0xd0 ? "stp" : "xch"));
							appendString(&si, " st(");
							*si.buffer++ = (char)(0x30 + NMD_C(instruction->modrm.modrm) % 8);
							*si.buffer++ = ')';
						}
					}

					break;
				}
			}
		}
		else if (op >= 0xe0 && op <= 0xe3)
		{
			const char* mnemonics[] = { "loopne", "loope", "loop", "jecxz" };
			appendString(&si, mnemonics[NMD_C(op)]);
			*si.buffer++ = ' ';

			if (instruction->address)
				appendNumber(&si, (ptrdiff_t)(instruction->address + instruction->length) + (int32_t)(instruction->immediate));
			else
			{
				*si.buffer++ = '$';
				appendSignedNumber(&si, (int64_t)((int32_t)(instruction->immediate) + (int32_t)(instruction->length)), true);
			}
		}
		else if (op == 0xe4 || op == 0xe5)
		{
			appendString(&si, "in ");
			appendString(&si, op == 0xe4 ? "al" : (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? "ax" : "eax"));
			*si.buffer++ = ',';
			appendNumber(&si, (uint64_t)(instruction->immediate));
		}
		else if (op == 0xe6 || op == 0xe7)
		{
			appendString(&si, "out ");
			appendNumber(&si, (uint64_t)(instruction->immediate));
			*si.buffer++ = ',';
			appendString(&si, op == 0xe6 ? "al" : (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? "ax" : "eax"));
		}
		else if (op == 0xe8 || op == 0xe9 || op == 0xeb)
		{
			appendString(&si, op == 0xe8 ? "call " : "jmp ");

			if (instruction->address)
				appendNumber(&si, (ptrdiff_t)(instruction->address + instruction->length) + (int32_t)(instruction->immediate));
			else
			{
				*si.buffer++ = '$';
				appendSignedNumber(&si, (int64_t)((int32_t)(instruction->immediate) + (int32_t)(instruction->length)), true);
			}
		}
		else if (op == 0xea)
		{
			appendString(&si, "jmp far ");
			appendNumber(&si, (uint64_t)(*(uint16_t*)(((uint8_t*)(&instruction->immediate) + 4))));
			*si.buffer++ = ':';
			appendNumber(&si, (uint64_t)(*(uint32_t*)(&instruction->immediate)));
		}
		else if (op == 0xec || op == 0xed)
		{
			appendString(&si, "in ");
			appendString(&si, op == 0xec ? "al" : (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? "ax" : "eax"));
			appendString(&si, ",dx");
		}
		else if (op == 0xee || op == 0xef)
		{
			appendString(&si, "out dx,");
			appendString(&si, op == 0xee ? "al" : (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? "ax" : "eax"));
		}
		else if (op == 0xf6 || op == 0xf7)
		{
			appendString(&si, opcodeExtensionsGrp3[instruction->modrm.reg]);
			*si.buffer++ = ' ';
			if (op == 0xf6)
				appendEb(&si);
			else
				appendEv(&si);

			if (instruction->modrm.reg <= 0b001)
			{
				*si.buffer++ = ',';
				appendNumber(&si, (uint64_t)(instruction->immediate));
			}
		}
		else if (op == 0xfe)
		{
			appendString(&si, instruction->modrm.reg == 0b000 ? "inc " : "dec ");
			appendEb(&si);
		}
		else if (op == 0xff)
		{
			appendString(&si, opcodeExtensionsGrp5[instruction->modrm.reg]);
			*si.buffer++ = ' ';
			if (si.instruction->modrm.mod == 0b11)
				appendString(&si, (si.instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? reg16 : reg32)[si.instruction->modrm.rm]);
			else
				appendModRmUpper(&si, (instruction->modrm.reg == 0b011 || instruction->modrm.reg == 0b101) ? "fword" : (si.instruction->flags.operandSize64 ? "qword" : (si.instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? "word" : "dword")));

		}
		else //Try to parse all opcodes not parsed by the checks above
		{
			const char* str = 0;
			switch (instruction->opcode)
			{
			case 0xcc: str = "int3"; break;
			case 0xc3: str = "ret"; break;
			case 0x90:
				if (instruction->prefixes & REPEAT_PREFIX)
					str = "pause";
				else if (instruction->prefixes & (REX_B_PREFIX | REX_W_PREFIX))
					str = "xchg r8, rax";
				else
					str = "nop";
				break;
			case 0x9c: str = instruction->flags.operandSize64 ? "pushfq" : (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? "pushf" : "pushfd"); break;
			case 0x9d: str = instruction->flags.operandSize64 ? "popfq" : (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? "popf" : "popfd"); break;
			case 0x60:
			case 0x61:
				str = (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? (instruction->opcode == 0x60 ? "pusha" : "popa") : (instruction->opcode == 0x60 ? "pushad" : "popad"));
				break;
			case 0xcb: str = "ret far"; break;
			case 0xc9: str = "leave"; break;
			case 0xf1: str = "int1"; break;
			case 0x06: str = "push es"; break;
			case 0x16: str = "push ss"; break;
			case 0x1e: str = "push ds"; break;
			case 0x0e: str = "push cs"; break;
			case 0x07: str = "pop es"; break;
			case 0x17: str = "pop ss"; break;
			case 0x1f: str = "pop ds"; break;
			case 0x27: str = "daa"; break;
			case 0x37: str = "aaa"; break;
			case 0x2f: str = "das"; break;
			case 0x3f: str = "aas"; break;
			case 0xd7: str = "xlat"; break;
			case 0x9b: str = "fwait"; break;
			case 0xf4: str = "hlt"; break;
			case 0xf5: str = "cmc"; break;
			case 0x9e: str = "sahf"; break;
			case 0x9f: str = "lahf"; break;
			case 0xce: str = "into"; break;
			case 0xcf: str = (instruction->prefixes & REX_W_PREFIX) ? "iretq" : "iretd"; break;
			case 0x98: str = (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? "cbw" : "cwde"); break;
			case 0x99: str = (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? "cwd" : "cdq"); break;
			case 0xd6: str = "salc"; break;
			case 0xf8: str = "clc"; break;
			case 0xf9: str = "stc"; break;
			case 0xfa: str = "cli"; break;
			case 0xfb: str = "sti"; break;
			case 0xfc: str = "cld"; break;
			case 0xfd: str = "std"; break;
			default: return;
			}
			appendString(&si, str);
		}
	}
	else if (instruction->opcodeSize == 2)
	{
		if (op == 0x00)
		{
			appendString(&si, opcodeExtensionsGrp6[instruction->modrm.reg]);
			*si.buffer++ = ' ';
			if (NMD_R(instruction->modrm.modrm) == 0xc)
				appendEv(&si);
			else
				appendEw(&si);
		}
		else if (op == 0x01)
		{
			if (instruction->modrm.mod == 0b11)
			{
				if (instruction->modrm.reg == 0b000)
					appendString(&si, opcodeExtensionsGrp7reg0[instruction->modrm.rm]);
				else if (instruction->modrm.reg == 0b001)
					appendString(&si, opcodeExtensionsGrp7reg1[instruction->modrm.rm]);
				else if (instruction->modrm.reg == 0b010)
					appendString(&si, opcodeExtensionsGrp7reg2[instruction->modrm.rm]);
				else if (instruction->modrm.reg == 0b011)
					appendString(&si, opcodeExtensionsGrp7reg3[instruction->modrm.rm]);
				else if (instruction->modrm.reg == 0b100)
					appendString(&si, "smsw "), appendString(&si, (instruction->flags.operandSize64 ? reg64 : (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? reg16 : reg32))[instruction->modrm.rm]);
				else if (instruction->modrm.reg == 0b101)
					appendString(&si, instruction->modrm.rm == 0b111 ? "wrpkru" : "rdpkru");
				else if (instruction->modrm.reg == 0b110)
					appendString(&si, "lmsw "), appendString(&si, reg16[instruction->modrm.rm]);
				else if (instruction->modrm.reg == 0b111)
					appendString(&si, opcodeExtensionsGrp7reg7[instruction->modrm.rm]);
			}
			else
			{
				appendString(&si, opcodeExtensionsGrp7[instruction->modrm.reg]);
				*si.buffer++ = ' ';
				if (si.instruction->modrm.reg == 0b110)
					appendEw(&si);
				else
					appendModRmUpper(&si, si.instruction->modrm.reg == 0b111 ? "byte" : si.instruction->modrm.reg == 0b100 ? "word" : "fword");
			}
		}
		else if (op == 0x02 || op == 0x03)
		{
			appendString(&si, op == 0x02 ? "lar" : "lsl");
			*si.buffer++ = ' ';
			appendGv(&si);
			*si.buffer++ = ',';
			if (si.instruction->modrm.mod == 0b11)
				appendString(&si, (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? reg16 : reg32)[si.instruction->modrm.rm]);
			else
				appendModRmUpper(&si, "word");
		}
		else if (op == 0x0d)
		{
			appendString(&si, "prefetch ");
			if (si.instruction->modrm.mod == 0b11)
				appendString(&si, (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? reg16 : reg32)[si.instruction->modrm.rm]);
			else
				appendModRmUpper(&si, "byte");
		}
		else if (op >= 0x10 && op <= 0x17)
		{
			const char* noPrefixMnemonics[] = { "movups", "movups", "movlps", "movlps", "unpcklps", "unpckhps", "vmovhps", "vmovhps" };
			const char* prefix66Mnemonics[] = { "movupd", "movupd", "movlpd", "movlpd", "unpcklpd", "unpckhpd", "vmovhpd", "vmovhpd" };
			const char* prefixF3Mnemonics[] = { "movss", "movss", "movsldup", NULL, NULL, NULL, "movshdup" };
			const char* prefixF2Mnemonics[] = { "movsd", "movsd", "movddup" };
			if (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX)
			{
				appendString(&si, prefix66Mnemonics[NMD_C(op)]);
				*si.buffer++ = ' ';

				switch (NMD_C(op))
				{
				case 0:
					appendVx(&si);
					*si.buffer++ = ',';
					appendW(&si);
					break;
				case 1:
					appendW(&si);
					*si.buffer++ = ',';
					appendVx(&si);
					break;
				case 2:
					appendW(&si);
					*si.buffer++ = ',';
				default:
					break;
				}
			}
			else if (instruction->prefixes & REPEAT_PREFIX)
			{
				appendString(&si, prefixF3Mnemonics[NMD_C(op)]);
			}
			else if (instruction->prefixes & REPEAT_NOT_ZERO_PREFIX)
			{
				appendString(&si, prefixF2Mnemonics[NMD_C(op)]);
			}
			else
			{
				appendString(&si, noPrefixMnemonics[NMD_C(op)]);
				*si.buffer++ = ' ';

				switch (NMD_C(op))
				{
				case 0:
					appendVx(&si);
					*si.buffer++ = ',';
					appendW(&si);
					break;
				case 1:
					appendW(&si);
					*si.buffer++ = ',';
					appendVx(&si);
					break;
				case 2:
					appendW(&si);
					*si.buffer++ = ',';
					appendModRmUpper(&si, "qword");
				default:
					break;
				}

			}
		}
		else if (op >= 0x20 && op < 0x24)
		{
			appendString(&si, "mov ");
			if (op < 0x22)
			{
				appendString(&si, reg32[instruction->modrm.rm]);
				appendString(&si, op == 0x20 ? ",cr" : ",dr");
				*si.buffer++ = (char)(0x30 + instruction->modrm.reg);
			}
			else
			{
				appendString(&si, op == 0x22 ? "cr" : "dr");
				*si.buffer++ = (char)(0x30 + instruction->modrm.reg);
				*si.buffer++ = ',';
				appendString(&si, reg32[instruction->modrm.rm]);
			}
		}
		else if (op == 0x2f)
		{
			appendString(&si, "commis");
			*si.buffer++ = instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? 'd' : 's';
			*si.buffer++ = ' ';
			appendVdq(&si);
			*si.buffer++ = ',';
			appendW(&si);
		}
		else if (NMD_R(op) == 4)
		{
			appendString(&si, "cmov");
			appendString(&si, conditionSuffixes[NMD_C(op)]);
			*si.buffer++ = ' ';
			appendGv(&si);
			*si.buffer++ = ',';
			appendEb(&si);
		}
		else if (op == 0x78)
		{
			appendString(&si, "vmread ");
			appendEb(&si);
			*si.buffer++ = ',';
			appendGv(&si);
		}
		else if (op == 0x79)
		{
			appendString(&si, "vmwrite ");
			appendGv(&si);
			*si.buffer++ = ',';
			appendEb(&si);
		}
		else if (NMD_R(op) == 8)
		{
			*si.buffer++ = 'j';
			appendString(&si, conditionSuffixes[NMD_C(op)]);
			*si.buffer++ = ' ';

			if (instruction->address)
				appendNumber(&si, (ptrdiff_t)(instruction->address + instruction->length) + (int32_t)(instruction->immediate));
			else
			{
				*si.buffer++ = '$';
				appendSignedNumber(&si, (int64_t)((int32_t)(instruction->immediate) + (int32_t)(instruction->length)), true);
			}
		}
		else if (NMD_R(op) == 9)
		{
			appendString(&si, "set");
			appendString(&si, conditionSuffixes[NMD_C(op)]);
			*si.buffer++ = ' ';
			appendEb(&si);
		}
		else if ((NMD_R(op) == 0xA || NMD_R(op) == 0xB) && (NMD_C(op) % 8) >= 3 && (NMD_C(op) % 8) <= 5)
		{
			const char* mnemonics[] = { "bt", "shld", "shld", "bts", "shrd", "shrd" };
			appendString(&si, mnemonics[(NMD_C(op) % 8) - 3 + (NMD_C(op) > 8 ? 3 : 0)]);
			*si.buffer++ = ' ';
			appendEb(&si);
			*si.buffer++ = ',';
			appendVx(&si);
		}
		else if (op == 0xC7)
		{
			if (instruction->modrm.reg == 0b110)
			{
				if (instruction->modrm.mod == 0b11)
				{
					appendString(&si, "rdrand");
					*si.buffer++ = ' ';
					appendGv(&si);
				}
				else
				{
					appendString(&si, (instruction->prefixes & REPEAT_PREFIX) ? "vmxon" : ((instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX) ? "vmclear" : "vmptrld"));
					*si.buffer++ = ' ';
					appendEq(&si);
				}
			}
			else if (instruction->modrm.reg == 0b001)
			{
				appendString(&si, "cmpxchg8b ");
				appendEq(&si);
			}
			else
			{
				if (instruction->modrm.mod == 0b11)
				{

					appendString(&si, instruction->prefixes & REPEAT_PREFIX ? "rdpid " : "rdseed ");
					appendString(&si, (instruction->prefixes & REPEAT_PREFIX ? (instruction->flags.x86_64 ? reg64 : reg32) : (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX ? reg16 : reg32))[instruction->modrm.reg]);

				}
				else
				{
					appendString(&si, "vmptrst ");
					appendEq(&si);
				}
			}
		}
		else if (NMD_R(op) == 0xb && (op % 8) >= 6)
		{
			appendString(&si, op > 0xb8 ? "movsx " : "movzx ");
			appendGv(&si);
			*si.buffer++ = ',';
			if ((op % 8) == 6)
				appendEb(&si);
			else
				appendEw(&si);
		}
		else if (op == 0xb9 || op == 0xff)
		{
			appendString(&si, op == 0xb9 ? "ud1 " : "ud0 ");
			appendString(&si, reg32[instruction->modrm.reg]);
			*si.buffer++ = ',';
			appendEd(&si);
		}
		else if (NMD_R(op) == 0xF)
		{
			const char* mnemonics[] = { NULL, "sllw", "slld", "sllq", "muludq", "maddwd", "sadbw", NULL, "subb", "subw", "subd", "subq", "addb", "addw", "addd" };

			if (op == 0xF0)
			{
				appendVx(&si);
				*si.buffer++ = ',';
				appendModRmUpper(&si, "xmmword");
			}
			else if (op == 0xF7)
			{
				if (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX)
				{
					appendString(&si, "maskmovdqu ");
					appendVdq(&si);
					*si.buffer++ = ',';
					appendUdq(&si);
				}
				else
				{
					appendString(&si, "maskmovq ");
					appendPq(&si);
					*si.buffer++ = ',';
					appendNq(&si);
				}
			}
			else if (instruction->prefixes & OPERAND_SIZE_OVERRIDE_PREFIX)
			{
				appendString(&si, mnemonics[NMD_C(op)]);
				*si.buffer++ = ' ';
				appendVx(&si);
				*si.buffer++ = ',';
			}
			else
			{
				appendString(&si, mnemonics[NMD_C(op)]);
				*si.buffer++ = ' ';
				appendPq(&si);
				*si.buffer++ = ',';
				appendQq(&si);
			}
		}
		else
		{
			const char* str = NULL;
			switch (op)
			{
			case 0x05: str = "syscall"; break;
			case 0x06: str = "clts"; break;
			case 0x07: str = "sysret"; break;
			case 0x08: str = "invd"; break;
			case 0x09: str = "wbinvd"; break;
			case 0x0b: str = "ud2"; break;
			case 0x0e: str = "femms"; break;
			case 0x30: str = "wrmsr"; break;
			case 0x31: str = "rdtsc"; break;
			case 0x32: str = "rdmsr"; break;
			case 0x33: str = "rdpmc"; break;
			case 0x34: str = "sysenter"; break;
			case 0x35: str = "sysexit"; break;
			case 0x37: str = "getsec"; break;
			case 0x77: str = "emms"; break;
			case 0xa0: str = "push fs"; break;
			case 0xa1: str = "pop fs"; break;
			case 0xa2: str = "cpuid"; break;
			case 0xa8: str = "push gs"; break;
			case 0xa9: str = "pop gs"; break;
			case 0xaa: str = "rsm"; break;
			}
			appendString(&si, str);
		}
	}
	else if (instruction->flags.is3op38h)
	{

	}
	else
	{
		if (op >= 0x60)
		{
			const char* mnemonics[] = { "pcmpestrm", "pcmpestri", "pcmpistrm", "pcmpistri" };
			appendString(&si, (op == 0xcc ? "sha1rnds4" : (op == 0xdf ? "aeskeygenassist" : mnemonics[NMD_C(op)])));
			*si.buffer++ = ' ';
			appendVx(&si);
			*si.buffer++ = ',';
			appendW(&si);
			*si.buffer++ = ',';
			appendNumber(&si, (uint64_t)(instruction->immediate));
		}
	}

	size_t stringLength = si.buffer - buffer;
	if (formatFlags & FORMAT_FLAGS_UPPERCASE)
	{
		for (size_t i = 0; i < stringLength; i++)
		{
			const char c = buffer[i];
			buffer[i] = (c > 0x60 && c <= 0x7A) ? c - 0x20 : c;
		}
	}

	if (formatFlags & FORMAT_FLAGS_ARGUMENT_SPACES)
	{
		for (size_t i = 0; i < stringLength; i++)
		{
			if (buffer[i] == ',')
			{
				for (size_t j = stringLength; j > i; j--)
					buffer[j] = buffer[j - 1];

				buffer[i + 1] = ' ';
				si.buffer++, stringLength++;
			}
		}
	}

	*si.buffer = '\0';
}

#endif // NMD_ASSEMBLY_IMPLEMENTATION

#endif //NMD_ASSEMBLY_H