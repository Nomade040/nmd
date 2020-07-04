#include "nmd_common.h"

void parseOperandSegmentRegister(const NMD_X86Instruction* instruction, NMD_X86Operand* operand)
{
	if (instruction->segmentOverride)
		operand->fields.reg = (uint8_t)(NMD_X86_REG_ES + nmd_getBitNumber(instruction->segmentOverride));
	else
		operand->fields.reg = !(instruction->prefixes & NMD_X86_PREFIXES_REX_B) && (instruction->modrm.fields.rm == 0b100 || instruction->modrm.fields.rm == 0b101) ? NMD_X86_REG_SS : NMD_X86_REG_DS;
}

void parseModrmUpper32(const NMD_X86Instruction* instruction, NMD_X86Operand* operand)
{
	operand->type = NMD_X86_OPERAND_TYPE_MEMORY;

	if (instruction->flags.fields.hasSIB)
	{
		operand->size++;

		if (instruction->sib.fields.base == 0b101)
		{
			if (instruction->modrm.fields.mod != 0b00)
				operand->fields.mem.base = instruction->mode == NMD_X86_MODE_64 && !(instruction->prefixes & NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE) ? (instruction->prefixes & NMD_X86_PREFIXES_REX_B ? NMD_X86_REG_R13 : NMD_X86_REG_RBP) : NMD_X86_REG_EBP;
		}
		else
			operand->fields.mem.base = ((instruction->mode == NMD_X86_MODE_64 && !(instruction->prefixes & NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE) ? (instruction->prefixes & NMD_X86_PREFIXES_REX_B ? NMD_X86_REG_R8 : NMD_X86_REG_RAX) : NMD_X86_REG_EAX) + instruction->sib.fields.base);

		if (instruction->sib.fields.index != 0b100)
			operand->fields.mem.index = ((instruction->mode == NMD_X86_MODE_64 && !(instruction->prefixes & NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE) ? (instruction->prefixes & NMD_X86_PREFIXES_REX_X ? NMD_X86_REG_R8 : NMD_X86_REG_RAX) : NMD_X86_REG_EAX) + instruction->sib.fields.index);

		if (instruction->prefixes & NMD_X86_PREFIXES_REX_X && instruction->sib.fields.index == 0b100)
		{
			operand->fields.mem.index = NMD_X86_REG_R12;
			operand->fields.mem.scale = instruction->sib.fields.scale;
		}
	}
	else if (!(instruction->modrm.fields.mod == 0b00 && instruction->modrm.fields.rm == 0b101))
	{
		if ((instruction->prefixes & (NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE | NMD_X86_PREFIXES_REX_B)) == (NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE | NMD_X86_PREFIXES_REX_B) && instruction->mode == NMD_X86_MODE_64)
			operand->fields.mem.base = NMD_X86_REG_R8D + instruction->modrm.fields.rm;
		else
			operand->fields.mem.base = ((instruction->mode == NMD_X86_MODE_64 && !(instruction->prefixes & NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE) ? (instruction->prefixes & NMD_X86_PREFIXES_REX_B ? NMD_X86_REG_R8 : NMD_X86_REG_RAX) : NMD_X86_REG_EAX) + instruction->modrm.fields.rm);
	}

	parseOperandSegmentRegister(instruction, operand);

	operand->fields.mem.disp = instruction->displacement;
	operand->size += (uint8_t)(instruction->dispMask);
}

void parseMemoryOperand(const NMD_X86Instruction* instruction, NMD_X86Operand* operand, uint8_t mod11baseReg)
{
	/* At least one byte is used for ModR/M. */
	operand->size = 1;

	if (instruction->modrm.fields.mod == 0b11)
	{
		operand->type = NMD_X86_OPERAND_TYPE_REGISTER;
		operand->fields.reg = mod11baseReg + instruction->modrm.fields.rm;
	}
	else
		parseModrmUpper32(instruction, operand);
}

void parseOperandEb(const NMD_X86Instruction* instruction, NMD_X86Operand* operand)
{
	parseMemoryOperand(instruction, operand, NMD_X86_REG_AL);
}

void parseOperandEw(const NMD_X86Instruction* instruction, NMD_X86Operand* operand)
{
	parseMemoryOperand(instruction, operand, NMD_X86_REG_AX);
}

void parseOperandEv(const NMD_X86Instruction* instruction, NMD_X86Operand* operand)
{
	parseMemoryOperand(instruction, operand, (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_REG_AX : NMD_X86_REG_EAX));
}

void parseOperandEy(const NMD_X86Instruction* instruction, NMD_X86Operand* operand)
{
	parseMemoryOperand(instruction, operand, (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_REG_AX : NMD_X86_REG_EAX));
}

void parseOperandQq(const NMD_X86Instruction* instruction, NMD_X86Operand* operand)
{
	parseMemoryOperand(instruction, operand, NMD_X86_REG_MM0);
}

void parseOperandWdq(const NMD_X86Instruction* instruction, NMD_X86Operand* operand)
{
	parseMemoryOperand(instruction, operand, NMD_X86_REG_XMM0);
}

void parseOperandGb(const NMD_X86Instruction* instruction, NMD_X86Operand* operand)
{
	operand->type = NMD_X86_OPERAND_TYPE_REGISTER;
	operand->fields.reg = NMD_X86_REG_AL + instruction->modrm.fields.reg;
	operand->size = 1;
}

void parseOperandGd(const NMD_X86Instruction* instruction, NMD_X86Operand* operand)
{
	operand->type = NMD_X86_OPERAND_TYPE_REGISTER;
	operand->fields.reg = NMD_X86_REG_EAX + instruction->modrm.fields.reg;
	operand->size = 1;
}

void parseOperandGw(const NMD_X86Instruction* instruction, NMD_X86Operand* operand)
{
	operand->type = NMD_X86_OPERAND_TYPE_REGISTER;
	operand->fields.reg = NMD_X86_REG_AX + instruction->modrm.fields.reg;
	operand->size = 1;
}

void parseOperandGv(const NMD_X86Instruction* instruction, NMD_X86Operand* operand)
{
	operand->type = NMD_X86_OPERAND_TYPE_REGISTER;
	if (instruction->prefixes & NMD_X86_PREFIXES_REX_B)
		operand->fields.reg = ((!(instruction->prefixes & NMD_X86_PREFIXES_REX_W) ? NMD_X86_REG_R8D : NMD_X86_REG_R8) + instruction->modrm.fields.reg);
	else
		operand->fields.reg = ((instruction->flags.fields.operandSize64 ? NMD_X86_REG_RAX : (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && instruction->mode != NMD_X86_MODE_16 ? NMD_X86_REG_AX : NMD_X86_REG_EAX)) + instruction->modrm.fields.reg);
	operand->size = 1;
}

void parseOperandRv(const NMD_X86Instruction* instruction, NMD_X86Operand* operand)
{
	operand->type = NMD_X86_OPERAND_TYPE_REGISTER;
	if (instruction->prefixes & NMD_X86_PREFIXES_REX_R)
		operand->fields.reg = ((!(instruction->prefixes & NMD_X86_PREFIXES_REX_W) ? NMD_X86_REG_R8D : NMD_X86_REG_R8) + instruction->modrm.fields.rm);
	else
		operand->fields.reg = ((instruction->flags.fields.operandSize64 ? NMD_X86_REG_RAX : ((instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && instruction->mode != NMD_X86_MODE_16) || (instruction->mode == NMD_X86_MODE_16 && !(instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)) ? NMD_X86_REG_AX : NMD_X86_REG_EAX)) + instruction->modrm.fields.rm);
	operand->size = 1;
}

void parseOperandGy(const NMD_X86Instruction* instruction, NMD_X86Operand* operand)
{
	operand->type = NMD_X86_OPERAND_TYPE_REGISTER;
	operand->fields.reg = ((instruction->mode == NMD_X86_MODE_64 ? NMD_X86_REG_RAX : NMD_X86_REG_EAX) + instruction->modrm.fields.reg);
	operand->size = 1;
}

void parseOperandPq(const NMD_X86Instruction* instruction, NMD_X86Operand* operand)
{
	operand->type = NMD_X86_OPERAND_TYPE_REGISTER;
	operand->fields.reg = NMD_X86_REG_MM0 + instruction->modrm.fields.reg;
	operand->size = 1;
}

void parseOperandNq(const NMD_X86Instruction* instruction, NMD_X86Operand* operand)
{
	operand->type = NMD_X86_OPERAND_TYPE_REGISTER;
	operand->fields.reg = NMD_X86_REG_MM0 + instruction->modrm.fields.rm;
	operand->size = 1;
}

void parseOperandVdq(const NMD_X86Instruction* instruction, NMD_X86Operand* operand)
{
	operand->type = NMD_X86_OPERAND_TYPE_REGISTER;
	operand->fields.reg = NMD_X86_REG_XMM0 + instruction->modrm.fields.reg;
	operand->size = 1;
}

void parseOperandUdq(const NMD_X86Instruction* instruction, NMD_X86Operand* operand)
{
	operand->type = NMD_X86_OPERAND_TYPE_REGISTER;
	operand->fields.reg = NMD_X86_REG_XMM0 + instruction->modrm.fields.rm;
	operand->size = 1;
}

void decodeConditionalFlag(NMD_X86Instruction* instruction, const uint8_t condition)
{
	switch (condition)
	{
		case 0x0: instruction->testedFlags.fields.OF = 1; break;                                                                           /* Jump if overflow (OF=1) */
		case 0x1: instruction->testedFlags.fields.OF = 1; break;                                                                           /* Jump if not overflow (OF=0) */
		case 0x2: instruction->testedFlags.fields.CF = 1; break;                                                                           /* Jump if not above or equal (CF=1) */
		case 0x3: instruction->testedFlags.fields.CF = 1; break;                                                                           /* Jump if not below (CF=0) */
		case 0x4: instruction->testedFlags.fields.ZF = 1; break;                                                                           /* Jump if equal (ZF=1) */
		case 0x5: instruction->testedFlags.fields.ZF = 1; break;                                                                           /* Jump if not equal (ZF=0) */
		case 0x6: instruction->testedFlags.fields.CF = instruction->testedFlags.fields.ZF = 1; break;                                      /* Jump if not above (CF=1 or ZF=1) */
		case 0x7: instruction->testedFlags.fields.CF = instruction->testedFlags.fields.ZF = 1; break;                                      /* Jump if not below or equal (CF=0 and ZF=0) */
		case 0x8: instruction->testedFlags.fields.SF = 1; break;                                                                           /* Jump if sign (SF=1) */
		case 0x9: instruction->testedFlags.fields.SF = 1; break;                                                                           /* Jump if not sign (SF=0) */
		case 0xa: instruction->testedFlags.fields.PF = 1; break;                                                                           /* Jump if parity/parity even (PF=1) */
		case 0xb: instruction->testedFlags.fields.PF = 1; break;                                                                           /* Jump if parity odd (PF=0) */
		case 0xc: instruction->testedFlags.fields.SF = instruction->testedFlags.fields.OF = 1; break;                                      /* Jump if not greater or equal (SF != OF) */
		case 0xd: instruction->testedFlags.fields.SF = instruction->testedFlags.fields.OF = 1; break;                                      /* Jump if not less (SF=OF) */
		case 0xe: instruction->testedFlags.fields.ZF = instruction->testedFlags.fields.SF = instruction->testedFlags.fields.OF = 1; break; /* Jump if not greater (ZF=1 or SF != OF) */
		case 0xf: instruction->testedFlags.fields.ZF = instruction->testedFlags.fields.SF = instruction->testedFlags.fields.OF = 1; break; /* Jump if not less or equal (ZF=0 and SF=OF) */
	}
}

/* 'remaningSize' in the context of this function is the number of bytes the instruction takes not counting prefixes and opcode. */
bool parseModrm(const uint8_t** b, NMD_X86Instruction* const instruction, const size_t remainingSize)
{
	if (remainingSize == 0)
		return false;

	instruction->flags.fields.hasModrm = true;
	instruction->modrm.modrm = *++ * b;
	const bool addressPrefix = (bool)(instruction->prefixes & NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE);

	if (instruction->mode == NMD_X86_MODE_16)
	{
		if (instruction->modrm.fields.mod != 0b11)
		{
			if (instruction->modrm.fields.mod == 0b00)
			{
				if (instruction->modrm.fields.rm == 0b110)
					instruction->dispMask = NMD_X86_DISP16;
			}
			else
				instruction->dispMask = instruction->modrm.fields.mod == 0b01 ? NMD_X86_DISP8 : NMD_X86_DISP16;
		}
	}
	else
	{
		if (addressPrefix && instruction->mode == NMD_X86_MODE_32)
		{
			if ((instruction->modrm.fields.mod == 0b00 && instruction->modrm.fields.rm == 0b110) || instruction->modrm.fields.mod == 0b10)
				instruction->dispMask = NMD_X86_DISP16;
			else if (instruction->modrm.fields.mod == 0b01)
				instruction->dispMask = NMD_X86_DISP8;
		}
		else /*if (!addressPrefix || /*(addressPrefix && **b >= 0x40) || (addressPrefix && instruction->mode == NMD_X86_MODE_64)) */
		{
			/* Check for SIB byte */
			if (instruction->modrm.modrm < 0xC0 && instruction->modrm.fields.rm == 0b100 && (!addressPrefix || (addressPrefix && instruction->mode == NMD_X86_MODE_64)))
			{
				if (remainingSize < 2)
					return false;

				instruction->flags.fields.hasSIB = true;
				instruction->sib.sib = *++ * b;
			}

			if (instruction->modrm.fields.mod == 0b01) /* disp8 (ModR/M) */
				instruction->dispMask = NMD_X86_DISP8;
			else if ((instruction->modrm.fields.mod == 0b00 && instruction->modrm.fields.rm == 0b101) || instruction->modrm.fields.mod == 0b10) /* disp16,32 (ModR/M) */
				instruction->dispMask = (addressPrefix && !(instruction->mode == NMD_X86_MODE_64 && instruction->prefixes & NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE) ? NMD_X86_DISP16 : NMD_X86_DISP32);
			else if (instruction->flags.fields.hasSIB && instruction->sib.fields.base == 0b101) /* disp8,32 (SIB) */
				instruction->dispMask = (instruction->modrm.fields.mod == 0b01 ? NMD_X86_DISP8 : NMD_X86_DISP32);
		}
	}

	if (remainingSize - (instruction->flags.fields.hasSIB ? 2 : 1) < instruction->dispMask)
		return false;

	size_t i = 0;
	for (; i < (size_t)instruction->dispMask; i++, (*b)++)
		((uint8_t*)(&instruction->displacement))[i] = *(*b + 1);

	return true;
}

/*
Decodes an instruction. Returns true if the instruction is valid, false otherwise.
Parameters:
  buffer       [in]  A pointer to a buffer containing a encoded instruction.
  bufferSize   [in]  The buffer's size in bytes.
  instruction  [out] A pointer to a variable of type 'NMD_X86Instruction' that receives information about the instruction.
  mode         [in]  The architecture mode. 'NMD_X86_MODE_32', 'NMD_X86_MODE_64' or 'NMD_X86_MODE_16'.
  featureFlags [in]  A mask of 'NMD_X86_FEATURE_FLAGS_XXX' that specifies which features the decoder is allowed to use. If uncertain, use 'NMD_X86_FEATURE_FLAGS_MINIMAL'.
*/
bool nmd_x86_decode_buffer(const void* const buffer, const size_t bufferSize, NMD_X86Instruction* const instruction, const NMD_X86_MODE mode, const uint32_t featureFlags)
{
	if (bufferSize == 0)
		return false;

	const uint8_t op1modrm[] = { 0xFF, 0x62, 0x63, 0x69, 0x6B, 0xC0, 0xC1, 0xC4, 0xC5, 0xC6, 0xC7, 0xD0, 0xD1, 0xD2, 0xD3, 0xF6, 0xF7, 0xFE };
	const uint8_t op1imm8[] = { 0x6A, 0x6B, 0x80, 0x82, 0x83, 0xA8, 0xC0, 0xC1, 0xC6, 0xCD, 0xD4, 0xD5, 0xEB };
	const uint8_t op1imm32[] = { 0xE8, 0xE9, 0x68, 0x81, 0x69, 0xA9, 0xC7 };

	/* Clear 'instruction'. */
	size_t i = 0;
	for (; i < sizeof(NMD_X86Instruction); i++)
		((uint8_t*)(instruction))[i] = 0x00;

	instruction->mode = mode;

	const uint8_t* b = (const uint8_t*)(buffer);

	/* Parse legacy prefixes & REX prefixes. */
	i = 0;
	for (; i < NMD_X86_MAXIMUM_INSTRUCTION_LENGTH; i++, b++)
	{
		switch (*b)
		{
		case 0xF0: instruction->prefixes = (instruction->prefixes | (instruction->simdPrefix = NMD_X86_PREFIXES_LOCK)); continue;
		case 0xF2: instruction->prefixes = (instruction->prefixes | (instruction->simdPrefix = NMD_X86_PREFIXES_REPEAT_NOT_ZERO)), instruction->flags.fields.repeatPrefix = false; continue;
		case 0xF3: instruction->prefixes = (instruction->prefixes | (instruction->simdPrefix = NMD_X86_PREFIXES_REPEAT)), instruction->flags.fields.repeatPrefix = true; continue;
		case 0x2E: instruction->prefixes = (instruction->prefixes | (instruction->segmentOverride = NMD_X86_PREFIXES_CS_SEGMENT_OVERRIDE)); continue;
		case 0x36: instruction->prefixes = (instruction->prefixes | (instruction->segmentOverride = NMD_X86_PREFIXES_SS_SEGMENT_OVERRIDE)); continue;
		case 0x3E: instruction->prefixes = (instruction->prefixes | (instruction->segmentOverride = NMD_X86_PREFIXES_DS_SEGMENT_OVERRIDE)); continue;
		case 0x26: instruction->prefixes = (instruction->prefixes | (instruction->segmentOverride = NMD_X86_PREFIXES_ES_SEGMENT_OVERRIDE)); continue;
		case 0x64: instruction->prefixes = (instruction->prefixes | (instruction->segmentOverride = NMD_X86_PREFIXES_FS_SEGMENT_OVERRIDE)); continue;
		case 0x65: instruction->prefixes = (instruction->prefixes | (instruction->segmentOverride = NMD_X86_PREFIXES_GS_SEGMENT_OVERRIDE)); continue;
		case 0x66: instruction->prefixes = (instruction->prefixes | (instruction->simdPrefix = NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)), instruction->flags.fields.operandSize64 = false; continue;
		case 0x67: instruction->prefixes = (instruction->prefixes | NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE); continue;
		default:
			if ((instruction->mode == NMD_X86_MODE_64) && NMD_R(*b) == 4) /* 0x40 */
			{
				instruction->flags.fields.hasRex = true;
				instruction->rex = *b;
				instruction->prefixes = (instruction->prefixes & ~(NMD_X86_PREFIXES_REX_B | NMD_X86_PREFIXES_REX_X | NMD_X86_PREFIXES_REX_R | NMD_X86_PREFIXES_REX_W));

				if (*b & 0b0001) /* Bit position 0. */
					instruction->prefixes = instruction->prefixes | NMD_X86_PREFIXES_REX_B;
				if (*b & 0b0010) /* Bit position 1. */
					instruction->prefixes = instruction->prefixes | NMD_X86_PREFIXES_REX_X;
				if (*b & 0b0100) /* Bit position 2. */
					instruction->prefixes = instruction->prefixes | NMD_X86_PREFIXES_REX_R;
				if (*b & 0b1000) /* Bit position 3. */
				{
					instruction->prefixes = instruction->prefixes | NMD_X86_PREFIXES_REX_W;
					instruction->flags.fields.operandSize64 = true;
				}

				continue;
			}
		}

		break;
	}

	instruction->numPrefixes = (uint8_t)((ptrdiff_t)(b)-(ptrdiff_t)(buffer));

	const size_t remainingValidBytes = (NMD_X86_MAXIMUM_INSTRUCTION_LENGTH - instruction->numPrefixes);
	if (remainingValidBytes == 0)
		return false;

	const size_t remainingBufferSize = bufferSize - instruction->numPrefixes;
	if (remainingBufferSize == 0)
		return false;

	const size_t remainingSize = remainingValidBytes < remainingBufferSize ? remainingValidBytes : remainingBufferSize;

	/* Assume NMD_X86_INSTRUCTION_ENCODING_LEGACY. */
	instruction->encoding = NMD_X86_INSTRUCTION_ENCODING_LEGACY;

	/* Opcode byte. This variable is used because it's easier to write 'op' than 'instruction->opcode'. */
	uint8_t op;

	/* Parse opcode. */
	if (*b == 0x0F) /* 2 or 3 byte opcode. */
	{
		if (remainingSize == 1)
			return false;

		b++;
		instruction->opcodeOffset = instruction->numPrefixes;

		if (*b == 0x38 || *b == 0x3A) /* 3 byte opcode. */
		{
			if (remainingSize < 4)
				return false;

			instruction->opcodeSize = 3;
			instruction->opcode = *++b;

			op = instruction->opcode;

			const NMD_Modrm modrm = *(NMD_Modrm*)(b + 1);
			if (*(b - 1) == 0x38)
			{
				instruction->opcodeMap = NMD_X86_OPCODE_MAP_0F_38;

				if (!parseModrm(&b, instruction, remainingSize - 3))
					return false;

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_VALIDITY_CHECK
				if (featureFlags & NMD_X86_FEATURE_FLAGS_VALIDITY_CHECK)
				{
					/* Check if the instruction is invalid. */
					
					if (((op == 0xf0 || op == 0xf1) && ((instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE || !(instruction->prefixes & (NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE | NMD_X86_PREFIXES_REPEAT | NMD_X86_PREFIXES_REPEAT_NOT_ZERO))) && modrm.fields.mod == 0b11)) ||
						!((!(instruction->prefixes & (NMD_X86_PREFIXES_REPEAT | NMD_X86_PREFIXES_REPEAT_NOT_ZERO)) && (op <= 0xb || (op >= 0x1c && op <= 0x1e))) || (op >= 0xc8 && op <= 0xcd && !instruction->simdPrefix) ||
							(instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && (op == 0x10 || op == 0x14 || op == 0x15 || op == 0x17 || (op >= 0x20 && op <= 0x25) || (op == 0x2a && modrm.fields.mod != 0b11) || (op >= 0x28 && op <= 0x2b && op != 0x2a) || (NMD_R(op) == 3 && op != 0x36) || op == 0x40 || op == 0x41 || ((op >= 0x80 && op <= 0x82) && modrm.fields.mod != 0b11) || op == 0xcf || (op >= 0xdb && op <= 0xdf))) ||
							((op == 0xf0 || op == 0xf1) && !(instruction->prefixes & NMD_X86_PREFIXES_REPEAT)) || (op == 0xf6 && !(instruction->prefixes & NMD_X86_PREFIXES_REPEAT_NOT_ZERO) && !(!instruction->simdPrefix && modrm.fields.mod == 0b11)) || (op == 0xf5 && instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && modrm.fields.mod != 0b11) || (op == 0xf8 && modrm.fields.mod != 0b11 && instruction->simdPrefix != 0x00) || (op == 0xf9 && !instruction->simdPrefix && modrm.fields.mod != 0b11)))
						return false;
				}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_VALIDITY_CHECK */

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_INSTRUCTION_ID
				if (featureFlags & NMD_X86_FEATURE_FLAGS_INSTRUCTION_ID)
				{
					if (NMD_R(op) == 0x00)
						instruction->id = NMD_X86_INSTRUCTION_PSHUFB + op;
					else if (op >= 0x1c && op <= 0x1e)
						instruction->id = NMD_X86_INSTRUCTION_PABSB + (op - 0x1c);
					else if (NMD_R(op) == 2)
						instruction->id = NMD_X86_INSTRUCTION_PMOVSXBW + NMD_C(op);
					else if (NMD_R(op) == 3)
						instruction->id = NMD_X86_INSTRUCTION_PMOVZXBW + NMD_C(op);
					else if (NMD_R(op) == 8)
						instruction->id = NMD_X86_INSTRUCTION_INVEPT + NMD_C(op);
					else if (NMD_R(op) == 0xc)
						instruction->id = NMD_X86_INSTRUCTION_SHA1NEXTE + (NMD_C(op) - 8);
					else if (NMD_R(op) == 0xd)
						instruction->id = NMD_X86_INSTRUCTION_AESIMC + (NMD_C(op) - 0xb);
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
						case 0xf0: case 0xf1: instruction->id = (instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE || instruction->simdPrefix == 0x00) ? NMD_X86_INSTRUCTION_MOVBE : NMD_X86_INSTRUCTION_CRC32; break;
						case 0xf6: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_ADCX : NMD_X86_INSTRUCTION_ADOX; break;
						}
					}
				}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_INSTRUCTION_ID */
				
#ifndef NMD_ASSEMBLY_DISABLE_DECODER_CPU_FLAGS
				if (featureFlags & NMD_X86_FEATURE_FLAGS_CPU_FLAGS)
				{
					if (op == 0x80 || op == 0x81) /* invept,invvpid */
					{
						instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_ZF;
						instruction->clearedFlags.eflags = NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_OF;
					}
					else if (op == 0xf6)
					{
						if (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE) /* adcx */
							instruction->modifiedFlags.eflags = instruction->testedFlags.eflags = NMD_X86_EFLAGS_CF;
						if (instruction->prefixes & NMD_X86_PREFIXES_REPEAT) /* adox */
							instruction->modifiedFlags.eflags = instruction->testedFlags.eflags = NMD_X86_EFLAGS_OF;
					}
				}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_CPU_FLAGS */

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_OPERANDS
				if (featureFlags & NMD_X86_FEATURE_FLAGS_OPERANDS)
				{
					instruction->numOperands = 2;
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READ_WRITE;
					instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;

					if (NMD_R(op) == 0 || (op >= 0x1c && op <= 0x1e))
					{
						parseOperandPq(instruction, &instruction->operands[0]);
						parseOperandQq(instruction, &instruction->operands[1]);
					}
					else if (NMD_R(op) == 8)
					{
						parseOperandGy(instruction, &instruction->operands[0]);
						parseModrmUpper32(instruction, &instruction->operands[1]);
					}
					else if (NMD_R(op) >= 1 && NMD_R(op) <= 0xe)
					{
						parseOperandVdq(instruction, &instruction->operands[0]);
						parseOperandWdq(instruction, &instruction->operands[1]);
					}
					else if (op == 0xf6)
					{
						parseOperandGy(instruction, &instruction->operands[!instruction->simdPrefix ? 1 : 0]);
						parseOperandEy(instruction, &instruction->operands[!instruction->simdPrefix ? 0 : 1]);
					}
					else if (op == 0xf0 || op == 0xf1)
					{
						if (instruction->prefixes == NMD_X86_PREFIXES_REPEAT_NOT_ZERO || instruction->prefixes == (NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE | NMD_X86_PREFIXES_REPEAT_NOT_ZERO) == (NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE | NMD_X86_PREFIXES_REPEAT_NOT_ZERO))
						{
							parseOperandGd(instruction, &instruction->operands[0]);
							if (op == 0xf0)
								parseOperandEb(instruction, &instruction->operands[1]);
							else if (instruction->prefixes == NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
								parseOperandEy(instruction, &instruction->operands[1]);
							else
								parseOperandEw(instruction, &instruction->operands[1]);
						}
						else
						{
							if (instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
								parseOperandGw(instruction, &instruction->operands[op == 0xf0 ? 0 : 1]);
							else
								parseOperandGy(instruction, &instruction->operands[op == 0xf0 ? 0 : 1]);

							parseMemoryOperand(instruction, &instruction->operands[op == 0xf0 ? 1 : 0], instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_REG_AX : (instruction->flags.fields.operandSize64 ? NMD_X86_REG_RAX : NMD_X86_REG_EAX));
						}
					}
				}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_OPERANDS */
			}
			else /* 0x3a */
			{
				if (remainingSize < 5)
					return false;

				instruction->opcodeMap = NMD_X86_OPCODE_MAP_0F_3A;
				instruction->immMask = NMD_X86_IMM8;

				if (!parseModrm(&b, instruction, remainingSize - 3))
					return false;

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_VALIDITY_CHECK
				if (featureFlags & NMD_X86_FEATURE_FLAGS_VALIDITY_CHECK)
				{
					/* Check if the instruction is invalid. */
					if (!(((instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE || !instruction->simdPrefix) && op == 0xf) || (op == 0xcc && !instruction->simdPrefix) ||
						(instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && ((op >= 0x8 && op <= 0xe) || (op >= 0x14 && op <= 0x17) || (op >= 0x20 && op <= 0x22) || (op >= 0x40 && op <= 0x42) || op == 0x44 || (op >= 0x60 && op <= 0x63) || op == 0xdf || op == 0xce || op == 0xcf))))
						return false;
				}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_VALIDITY_CHECK */

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_INSTRUCTION_ID
				if (featureFlags & NMD_X86_FEATURE_FLAGS_INSTRUCTION_ID)
				{
					if (NMD_R(op) == 0)
						instruction->id = NMD_X86_INSTRUCTION_ROUNDPS + (op - 8);
					else if (NMD_R(op) == 4)
						instruction->id = NMD_X86_INSTRUCTION_DPPS + NMD_C(op);
					else if (NMD_R(op) == 6)
						instruction->id = NMD_X86_INSTRUCTION_PCMPESTRM + NMD_C(op);
					else
					{
						switch (op)
						{
						case 0x14: instruction->id = NMD_X86_INSTRUCTION_PEXTRB; break;
						case 0x15: instruction->id = NMD_X86_INSTRUCTION_PEXTRW; break;
						case 0x16: instruction->id = instruction->prefixes & NMD_X86_PREFIXES_REX_W ? NMD_X86_INSTRUCTION_PEXTRQ : NMD_X86_INSTRUCTION_PEXTRD; break;
						case 0x17: instruction->id = NMD_X86_INSTRUCTION_EXTRACTPS; break;
						case 0x20: instruction->id = NMD_X86_INSTRUCTION_PINSRB; break;
						case 0x21: instruction->id = NMD_X86_INSTRUCTION_INSERTPS; break;
						case 0x22: instruction->id = instruction->prefixes & NMD_X86_PREFIXES_REX_W ? NMD_X86_INSTRUCTION_PINSRQ : NMD_X86_INSTRUCTION_PINSRD; break;
						case 0xcc: instruction->id = NMD_X86_INSTRUCTION_SHA1RNDS4; break;
						case 0xdf: instruction->id = NMD_X86_INSTRUCTION_AESKEYGENASSIST; break;
						}
					}
				}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_INSTRUCTION_ID */
				
#ifndef NMD_ASSEMBLY_DISABLE_DECODER_CPU_FLAGS
				if (featureFlags & NMD_X86_FEATURE_FLAGS_CPU_FLAGS)
				{
				}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_CPU_FLAGS */

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_OPERANDS
				if (featureFlags & NMD_X86_FEATURE_FLAGS_OPERANDS)
				{
					instruction->numOperands = 3;
					instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READ_WRITE;
					instruction->operands[1].action = instruction->operands[2].action = NMD_X86_OPERAND_ACTION_READ;
					instruction->operands[2].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;

					if (op == 0x0f && !instruction->simdPrefix)
					{
						parseOperandPq(instruction, &instruction->operands[0]);
						parseOperandQq(instruction, &instruction->operands[1]);
					}
					else if (NMD_R(op) == 1)
					{
						parseMemoryOperand(instruction, &instruction->operands[0], NMD_X86_REG_EAX);
						parseOperandVdq(instruction, &instruction->operands[1]);
					}
					else if (NMD_R(op) == 2)
					{
						parseOperandVdq(instruction, &instruction->operands[0]);
						parseMemoryOperand(instruction, &instruction->operands[1], NMD_C(op) == 1 ? NMD_X86_REG_XMM0 : NMD_X86_REG_EAX);
					}
					else if (op == 0xcc || op == 0xdf || NMD_R(op) == 4 || NMD_R(op) == 6 || NMD_R(op) == 0)
					{
						parseOperandVdq(instruction, &instruction->operands[0]);
						parseOperandWdq(instruction, &instruction->operands[1]);
					}
				}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_OPERANDS */
			}
		}
		else /* 2 byte opcode. */
		{
			instruction->opcodeSize = 2;
			instruction->opcode = *b;
			instruction->opcodeMap = NMD_X86_OPCODE_MAP_0F;

			op = instruction->opcode;

			/* Check for ModR/M, SIB and displacement. */
			if (op >= 0x20 && op <= 0x23 && remainingSize == 2)
				instruction->flags.fields.hasModrm = true, instruction->modrm.modrm = *++b;
			else if (op < 4 || (NMD_R(op) != 3 && NMD_R(op) > 0 && NMD_R(op) < 7) || (op >= 0xD0 && op != 0xFF) || (NMD_R(op) == 7 && NMD_C(op) != 7) || NMD_R(op) == 9 || NMD_R(op) == 0xB || (NMD_R(op) == 0xC && NMD_C(op) < 8) || (NMD_R(op) == 0xA && (op % 8) >= 3) || op == 0x0ff || op == 0x0)
			{
				if (!parseModrm(&b, instruction, remainingSize - 2))
					return false;
			}

			const NMD_Modrm modrm = instruction->modrm;
#ifndef NMD_ASSEMBLY_DISABLE_DECODER_VALIDITY_CHECK
			if (featureFlags & NMD_X86_FEATURE_FLAGS_VALIDITY_CHECK)
			{
				/* Check if the instruction is invalid. */
				const uint8_t invalid2op[] = { 0x04, 0x0a, 0x0c, 0x0f, 0x7a, 0x7b, 0x36, 0x39 };
				if (nmd_findByte(invalid2op, sizeof(invalid2op), op))
					return false;
				else if (op == 0xc7)
				{
					if ((!instruction->simdPrefix && (modrm.fields.mod == 0b11 ? modrm.fields.reg <= 0b101 : modrm.fields.reg == 0b000 || modrm.fields.reg == 0b010)) || (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO && (modrm.fields.mod == 0b11 || modrm.fields.reg != 0b001)) || ((instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE || instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT) && (modrm.fields.mod == 0b11 ? modrm.fields.reg <= (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT ? 0b110 : 0b101) : (modrm.fields.reg != 0b001 && modrm.fields.reg != 0b110))))
						return false;
				}
				else if (op == 0x00)
				{
					if (modrm.fields.reg >= 0b110)
						return false;
				}
				else if (op == 0x01)
				{
					if ((modrm.fields.mod == 0b11 ? ((instruction->prefixes & (NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE | NMD_X86_PREFIXES_REPEAT_NOT_ZERO | NMD_X86_PREFIXES_REPEAT) && ((modrm.modrm >= 0xc0 && modrm.modrm <= 0xc5) || (modrm.modrm >= 0xc8 && modrm.modrm <= 0xcb) || (modrm.modrm >= 0xcf && modrm.modrm <= 0xd1) || (modrm.modrm >= 0xd4 && modrm.modrm <= 0xd7) || modrm.modrm == 0xee || modrm.modrm == 0xef || modrm.modrm == 0xfa || modrm.modrm == 0xfb)) || (modrm.fields.reg == 0b000 && modrm.fields.rm >= 0b110) || (modrm.fields.reg == 0b001 && modrm.fields.rm >= 0b100 && modrm.fields.rm <= 0b110) || (modrm.fields.reg == 0b010 && (modrm.fields.rm == 0b010 || modrm.fields.rm == 0b011)) || (modrm.fields.reg == 0b101 && modrm.fields.rm < 0b110 && (!(instruction->prefixes & NMD_X86_PREFIXES_REPEAT) || (instruction->prefixes & NMD_X86_PREFIXES_REPEAT && (modrm.fields.rm != 0b000 && modrm.fields.rm != 0b010)))) || (modrm.fields.reg == 0b111 && (modrm.fields.rm > 0b101 || (!(instruction->mode == NMD_X86_MODE_64) && modrm.fields.rm == 0b000)))) : (!(instruction->prefixes & NMD_X86_PREFIXES_REPEAT) && modrm.fields.reg == 0b101)))
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
				else if (NMD_R(op) == 5)
				{
					if ((op == 0x50 && modrm.fields.mod != 0b11) || (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && (op == 0x52 || op == 0x53)) || (instruction->prefixes & NMD_X86_PREFIXES_REPEAT && (op == 0x50 || (op >= 0x54 && op <= 0x57))) || (instruction->prefixes & NMD_X86_PREFIXES_REPEAT_NOT_ZERO && (op == 0x50 || (op >= 0x52 && op <= 0x57) || op == 0x5b)))
						return false;
				}
				else if (NMD_R(op) == 6)
				{
					if ((!(instruction->prefixes & (NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE | NMD_X86_PREFIXES_REPEAT | NMD_X86_PREFIXES_REPEAT_NOT_ZERO)) && (op == 0x6c || op == 0x6d)) || (instruction->prefixes & NMD_X86_PREFIXES_REPEAT && op != 0x6f) || instruction->prefixes & NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
						return false;
				}
				else if (op == 0x78 || op == 0x79)
				{
					if ((((instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && op == 0x78) && !(modrm.fields.mod == 0b11 && modrm.fields.reg == 0b000)) || ((instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE || instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO) && modrm.fields.mod != 0b11)) || (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT))
						return false;
				}
				else if (op == 0x7c || op == 0x7d)
				{
					if (instruction->prefixes & NMD_X86_PREFIXES_REPEAT || !(instruction->prefixes & (NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE | NMD_X86_PREFIXES_REPEAT | NMD_X86_PREFIXES_REPEAT_NOT_ZERO)))
						return false;
				}
				else if (op == 0x7e || op == 0x7f)
				{
					if (instruction->prefixes & NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
						return false;
				}
				else if (op >= 0x71 && op <= 0x73)
				{
					if(instruction->prefixes & (NMD_X86_PREFIXES_REPEAT | NMD_X86_PREFIXES_REPEAT_NOT_ZERO) || modrm.modrm <= 0xcf || (modrm.modrm >= 0xe8 && modrm.modrm <= 0xef))
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
					if (((!instruction->simdPrefix && modrm.fields.mod == 0b11 && modrm.fields.reg <= 0b100) || (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO && !(modrm.fields.mod == 0b11 && modrm.fields.reg == 0b110)) || (instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && (modrm.fields.reg < 0b110 || (modrm.fields.mod == 0b11 && modrm.fields.reg == 0b111))) || (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT && (modrm.fields.reg != 0b100 && modrm.fields.reg != 0b110) && !(modrm.fields.mod == 0b11 && modrm.fields.reg == 0b101))))
						return false;
				}
				else if (op == 0xb8)
				{
					if (!(instruction->prefixes & NMD_X86_PREFIXES_REPEAT))
						return false;
				}
				else if (op == 0xba)
				{
					if (modrm.fields.reg <= 0b011)
						return false;
				}
				else if (op == 0xd0)
				{
					if (!instruction->simdPrefix || instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT)
						return false;
				}
				else if (op == 0xe0)
				{
					if (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT || instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
						return false;
				}
				else if (op == 0xf0)
				{
					if (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? modrm.fields.mod == 0b11 : true)
						return false;
				}
				else if (instruction->prefixes & (NMD_X86_PREFIXES_REPEAT | NMD_X86_PREFIXES_REPEAT_NOT_ZERO))
				{
					if ((op >= 0x13 && op <= 0x17 && !(op == 0x16 && instruction->prefixes & NMD_X86_PREFIXES_REPEAT)) || op == 0x28 || op == 0x29 || op == 0x2e || op == 0x2f || (op <= 0x76 && op >= 0x74))
						return false;
				}
				else if (op == 0x71 || op == 0x72 || (op == 0x73 && !(instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)))
				{
					if ((modrm.modrm >= 0xd8 && modrm.modrm <= 0xdf) || modrm.modrm >= 0xf8)
						return false;
				}
				else if (op >= 0xc3 && op <= 0xc6)
				{
					if ((op == 0xc5 && modrm.fields.mod != 0b11) || (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT || instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO) || (op == 0xc3 && instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE))
						return false;
				}
				else if(NMD_R(op) >= 0xd && NMD_C(op) != 0 && op != 0xff && ((NMD_C(op) == 6 && NMD_R(op) != 0xf) ? (!instruction->simdPrefix || (NMD_R(op) == 0xD && (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT || instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO) ? modrm.fields.mod != 0b11 : false)) : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT || instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO || ((NMD_C(op) == 7 && NMD_R(op) != 0xe) ? modrm.fields.mod != 0b11 : false))))
					return false;
				else if (modrm.fields.mod == 0b11)
				{
					if (op == 0xb2 || op == 0xb4 || op == 0xb5 || op == 0xc3 || op == 0xe7 || op == 0x2b || (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && (op == 0x12 || op == 0x16)) || (!(instruction->prefixes & (NMD_X86_PREFIXES_REPEAT | NMD_X86_PREFIXES_REPEAT_NOT_ZERO)) && (op == 0x13 || op == 0x17)))
						return false;
				}
			}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_VALIDITY_CHECK */

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_INSTRUCTION_ID
			if (featureFlags & NMD_X86_FEATURE_FLAGS_INSTRUCTION_ID)
			{
				if (NMD_R(op) == 8)
					instruction->id = NMD_X86_INSTRUCTION_JO + NMD_C(op);
				else if (op >= 0xa2 && op <= 0xa5)
					instruction->id = NMD_X86_INSTRUCTION_CPUID + (op - 0xa2);
				else if (op == 0x05)
					instruction->id = NMD_X86_INSTRUCTION_SYSCALL;
				else if (NMD_R(op) == 4)
					instruction->id = NMD_X86_INSTRUCTION_CMOVO + NMD_C(op);
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
						case 0b111: instruction->id = modrm.fields.rm == 0b000 ? NMD_X86_INSTRUCTION_SWAPGS : NMD_X86_INSTRUCTION_RDTSCP; break;
						}
					}
					else
						instruction->id = NMD_X86_INSTRUCTION_SGDT + modrm.fields.reg;
				}
				else if (op <= 0x0b)
					instruction->id = NMD_X86_INSTRUCTION_LAR + (op - 2);
				else if (op == 0x19 || (op >= 0x1c && op <= 0x1f))
				{
					if(op == 0x1e && modrm.modrm == 0xfa)
						instruction->id = NMD_X86_INSTRUCTION_ENDBR64;
					else if (op == 0x1e && modrm.modrm == 0xfb)
						instruction->id = NMD_X86_INSTRUCTION_ENDBR32;
					else
						instruction->id = NMD_X86_INSTRUCTION_NOP;
				}
				else if (op >= 0x10 && op <= 0x17)
				{
					switch (instruction->simdPrefix)
					{
					case NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE: instruction->id = NMD_X86_INSTRUCTION_VMOVUPS + NMD_C(op); break;
					case NMD_X86_PREFIXES_REPEAT: instruction->id = NMD_X86_INSTRUCTION_VMOVUPS + NMD_C(op); break;
					case NMD_X86_PREFIXES_REPEAT_NOT_ZERO: instruction->id = NMD_X86_INSTRUCTION_VMOVUPS + NMD_C(op); break;
					default: instruction->id = NMD_X86_INSTRUCTION_VMOVUPS + NMD_C(op); break;
					}
				}
				else if (op >= 0x20 && op <= 0x23)
					instruction->id = NMD_X86_INSTRUCTION_MOV;
				else if (NMD_R(op) == 3)
					instruction->id = NMD_X86_INSTRUCTION_WRMSR + NMD_C(op);
				else if (NMD_R(op) == 5)
				{
					switch (instruction->simdPrefix)
					{
					case NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE: instruction->id = NMD_X86_INSTRUCTION_MOVMSKPD + NMD_C(op); break;
					case NMD_X86_PREFIXES_REPEAT: instruction->id = NMD_X86_INSTRUCTION_BNDMOV + NMD_C(op); break;
					case NMD_X86_PREFIXES_REPEAT_NOT_ZERO: instruction->id = NMD_X86_INSTRUCTION_BNDCL + NMD_C(op); break;
					default:   instruction->id = NMD_X86_INSTRUCTION_MOVMSKPS + NMD_C(op); break;
					}
				}
				else if (op >= 0x60 && op <= 0x6d)
					instruction->id = NMD_X86_INSTRUCTION_PUNPCKLBW + NMD_C(op);
				else if (op >= 0x74 && op <= 0x76)
					instruction->id = NMD_X86_INSTRUCTION_PCMPEQB + (op - 0x74);
				else if (op >= 0xb2 && op <= 0xb5)
					instruction->id = NMD_X86_INSTRUCTION_LSS + (op - 0xb2);
				else if (op >= 0xc3 && op <= 0xc5)
					instruction->id = NMD_X86_INSTRUCTION_MOVNTI + (op - 0xc3);
				else if (op == 0xc7)
				{
					if (modrm.fields.reg == 0b001)
						instruction->id = instruction->flags.fields.operandSize64 ? NMD_X86_INSTRUCTION_CMPXCHG16B : NMD_X86_INSTRUCTION_CMPXCHG8B;
					else if (modrm.fields.reg == 0b111)
						instruction->id = modrm.fields.mod == 0b11 ? (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_RDPID : NMD_X86_INSTRUCTION_RDSEED) : NMD_X86_INSTRUCTION_VMPTRST;
					else
						instruction->id = modrm.fields.mod == 0b11 ? NMD_X86_INSTRUCTION_RDRAND : (instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_VMCLEAR : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_VMXON : NMD_X86_INSTRUCTION_VMPTRLD));
				}
				else if (op >= 0xc8 && op <= 0xcf)
					instruction->id = NMD_X86_INSTRUCTION_BSWAP;
				else if (op == 0xa3)
					instruction->id = ((modrm.fields.mod == 0b11 ? NMD_X86_INSTRUCTION_RDFSBASE : NMD_X86_INSTRUCTION_FXSAVE) + modrm.fields.reg);
				else if (op >= 0xd1 && op <= 0xfe)
				{
					if (op == 0xd6)
						instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_MOVQ : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_MOVQ2DQ : NMD_X86_INSTRUCTION_MOVDQ2Q);
					else if (op == 0xe6)
						instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_CVTTPD2DQ : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_CVTDQ2PD : NMD_X86_INSTRUCTION_CVTPD2DQ);
					else if (op == 0xe7)
						instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_MOVNTDQ : NMD_X86_INSTRUCTION_MOVNTQ;
					else if (op == 0xf7)
						instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_MASKMOVDQU : NMD_X86_INSTRUCTION_MASKMOVQ;
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
					case 0x10: case 0x11: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_MOVUPD : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_MOVSS : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? NMD_X86_INSTRUCTION_MOVSD : NMD_X86_INSTRUCTION_MOVUPD)); break;
					case 0x12: case 0x13: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_MOVLPD : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_MOVSLDUP : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? NMD_X86_INSTRUCTION_MOVDDUP : NMD_X86_INSTRUCTION_MOVLPS)); break;
					case 0x14: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_UNPCKLPD : NMD_X86_INSTRUCTION_UNPCKLPS; break;
					case 0x15: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_UNPCKHPD : NMD_X86_INSTRUCTION_UNPCKHPS; break;
					case 0x16: case 0x17: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_MOVHPD : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_MOVSHDUP : NMD_X86_INSTRUCTION_MOVHPS); break;
					case 0x18: instruction->id = modrm.fields.reg >= 0b100 ? NMD_X86_INSTRUCTION_NOP : (modrm.fields.reg == 0b000 ? NMD_X86_INSTRUCTION_PREFETCHNTA : (modrm.fields.reg == 0b001 ? NMD_X86_INSTRUCTION_PREFETCHT0 : (modrm.fields.reg == 0b010 ? NMD_X86_INSTRUCTION_PREFETCHT1 : NMD_X86_INSTRUCTION_PREFETCHT2))); break;
					case 0x1a: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_BNDMOV : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_BNDCL : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? NMD_X86_INSTRUCTION_BNDCU : NMD_X86_INSTRUCTION_BNDLDX)); break;
					case 0x1b: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_BNDMOV : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_BNDMK : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? NMD_X86_INSTRUCTION_BNDCN : NMD_X86_INSTRUCTION_BNDSTX)); break;
					case 0x28: case 0x29: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_MOVAPD : NMD_X86_INSTRUCTION_MOVAPS; break;
					case 0x2a: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_CVTPI2PD : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_CVTSI2SS : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? NMD_X86_INSTRUCTION_CVTSI2SD : NMD_X86_INSTRUCTION_CVTPI2PS)); break;
					case 0x2b: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_MOVNTPD : NMD_X86_INSTRUCTION_MOVNTPS; break;
					case 0x2c: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_CVTTPD2PI : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_CVTTSS2SI : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? NMD_X86_INSTRUCTION_CVTTSS2SI : NMD_X86_INSTRUCTION_CVTTPS2PI)); break;
					case 0x2d: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_CVTPD2PI : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_CVTSS2SI : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? NMD_X86_INSTRUCTION_CVTSS2SI : NMD_X86_INSTRUCTION_CVTPS2PI)); break;
					case 0x2e: case 0x2f: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_UCOMISD : NMD_X86_INSTRUCTION_UCOMISS; break;
					case 0x6e: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && !instruction->flags.fields.operandSize64 && (instruction->prefixes & NMD_X86_PREFIXES_REX_W) ? NMD_X86_INSTRUCTION_MOVQ : NMD_X86_INSTRUCTION_MOVD; break;
					case 0x6f: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_MOVDQA : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_MOVDQU : NMD_X86_INSTRUCTION_MOVQ); break;
					case 0x70: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_PSHUFD : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_PSHUFHW : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? NMD_X86_INSTRUCTION_PSHUFLW : NMD_X86_INSTRUCTION_PSHUFW)); break;
					case 0x71: instruction->id = modrm.fields.reg == 0b101 ? NMD_X86_INSTRUCTION_PSRLQ : (modrm.fields.reg == 0b100 ? NMD_X86_INSTRUCTION_PSRAW : NMD_X86_INSTRUCTION_PSLLW); break;
					case 0x72: instruction->id = modrm.fields.reg == 0b101 ? NMD_X86_INSTRUCTION_PSRLD : (modrm.fields.reg == 0b100 ? NMD_X86_INSTRUCTION_PSRAD : NMD_X86_INSTRUCTION_PSLLD); break;
					case 0x73: instruction->id = modrm.fields.reg == 0b010 ? NMD_X86_INSTRUCTION_PSRLQ : (modrm.fields.reg == 0b011 ? NMD_X86_INSTRUCTION_PSRLDQ : (modrm.fields.reg == 0b110 ? NMD_X86_INSTRUCTION_PSLLQ : NMD_X86_INSTRUCTION_PSLLDQ)); break;
					case 0x77: instruction->id = NMD_X86_INSTRUCTION_EMMS; break;
					case 0x78: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_EXTRQ : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? NMD_X86_INSTRUCTION_INSERTQ : NMD_X86_INSTRUCTION_VMREAD); break;
					case 0x79: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_EXTRQ : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? NMD_X86_INSTRUCTION_INSERTQ : NMD_X86_INSTRUCTION_VMWRITE); break;
					case 0x7c: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_HADDPD : NMD_X86_INSTRUCTION_HADDPS; break;
					case 0x7d: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_HSUBPD : NMD_X86_INSTRUCTION_HSUBPS; break;
					case 0x7e: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT || (instruction->flags.fields.operandSize64 && instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE) ? NMD_X86_INSTRUCTION_MOVQ : NMD_X86_INSTRUCTION_MOVD; break;
					case 0x7f: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_MOVDQA : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_MOVDQU : NMD_X86_INSTRUCTION_MOVQ); break;
					case 0xa3: instruction->id = NMD_X86_INSTRUCTION_BT; break;
					case 0xa4: case 0xa5: instruction->id = NMD_X86_INSTRUCTION_SHLD; break;
					case 0xaa: instruction->id = NMD_X86_INSTRUCTION_RSM; break;
					case 0xab: instruction->id = NMD_X86_INSTRUCTION_BTS; break;
					case 0xac: case 0xad: instruction->id = NMD_X86_INSTRUCTION_SHRD; break;
					case 0xb6: case 0xb7: instruction->id = NMD_X86_INSTRUCTION_MOVZX; break;
					case 0xb8: instruction->id = NMD_X86_INSTRUCTION_POPCNT; break;
					case 0xb9: instruction->id = NMD_X86_INSTRUCTION_UD1; break;
					case 0xba: instruction->id = (modrm.fields.reg == 0b100 ? NMD_X86_INSTRUCTION_BT : (modrm.fields.reg == 0b101 ? NMD_X86_INSTRUCTION_BTS : (modrm.fields.reg == 0b110 ? NMD_X86_INSTRUCTION_BTR : NMD_X86_INSTRUCTION_BTC))); break;
					case 0xbb: instruction->id = NMD_X86_INSTRUCTION_BTC; break;
					case 0xbc: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_BSF : NMD_X86_INSTRUCTION_TZCNT; break;
					case 0xbd: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_BSR : NMD_X86_INSTRUCTION_LZCNT; break;
					case 0xbe: case 0xbf: instruction->id = NMD_X86_INSTRUCTION_MOVSX; break;
					case 0xc0: case 0xc1: instruction->id = NMD_X86_INSTRUCTION_XADD; break;
					case 0xc2: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_CMPPD : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT ? NMD_X86_INSTRUCTION_CMPSS : (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO ? NMD_X86_INSTRUCTION_CMPSD : NMD_X86_INSTRUCTION_CMPPS)); break;
					case 0xd0: instruction->id = instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_INSTRUCTION_ADDSUBPD : NMD_X86_INSTRUCTION_ADDSUBPS; break;
					case 0xff: instruction->id = NMD_X86_INSTRUCTION_UD0; break;
					}
				}
			}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_INSTRUCTION_ID */

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_CPU_FLAGS
			if (featureFlags & NMD_X86_FEATURE_FLAGS_CPU_FLAGS)
			{
				if (NMD_R(op) == 4 || NMD_R(op) == 8 || NMD_R(op) == 9) /* Conditional Move (CMOVcc),Conditional jump(Jcc),Byte set on condition(SETcc) */
					decodeConditionalFlag(instruction, NMD_C(op));
				else if (op == 0x05 || op == 0x07) /* syscall,sysret */
					instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_TF | NMD_X86_EFLAGS_IF | NMD_X86_EFLAGS_DF | NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_IOPL | NMD_X86_EFLAGS_AC | NMD_X86_EFLAGS_VIF | NMD_X86_EFLAGS_VIP | NMD_X86_EFLAGS_ID;
				else if (op == 0xaf) /* mul */
				{
					instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_OF;
					instruction->undefinedFlags.eflags = NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_SF;
				}
				else if (op == 0xb0 || op == 0xb1) /* cmpxchg */
					instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_OF;
				else if (op == 0xc0 || op == 0xc1) /* xadd */
					instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_OF;
				else if (op == 0x00 && (modrm.fields.reg == 0b100 || modrm.fields.reg == 0b101)) /* verr,verw*/
					instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_OF;
				else if (op == 0x01 && modrm.fields.mod == 0b11)
				{
					if (modrm.fields.reg == 0b000)
					{
						if (modrm.fields.rm == 0b001 || modrm.fields.rm == 0b010 || modrm.fields.rm == 0b011) /* vmcall,vmlaunch,vmresume */
						{
							instruction->testedFlags.eflags = NMD_X86_EFLAGS_IOPL | NMD_X86_EFLAGS_VM;
							instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_TF | NMD_X86_EFLAGS_IF | NMD_X86_EFLAGS_DF | NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_IOPL | NMD_X86_EFLAGS_NT | NMD_X86_EFLAGS_RF | NMD_X86_EFLAGS_VM | NMD_X86_EFLAGS_AC | NMD_X86_EFLAGS_VIF | NMD_X86_EFLAGS_VIP | NMD_X86_EFLAGS_ID;
						}
					}
				}
				else if (op == 0x34)
					instruction->clearedFlags.eflags = NMD_X86_EFLAGS_VM | NMD_X86_EFLAGS_IF;
				else if (op == 0x78 || op == 0x79) /* vmread,vmwrite */
				{
					instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_ZF;
					instruction->clearedFlags.eflags = NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_OF;
				}
				else if (op == 0x02 || op == 0x03) /* lar,lsl */
					instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_ZF;
				else if (op == 0xa3 || op == 0xab || op == 0xb3 || op == 0xba || op == 0xbb) /* bt,bts,btc */
				{
					instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_CF;
					instruction->undefinedFlags.eflags = NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_PF;
				}
				else if (op == 0xa4 || op == 0xa5 || op == 0xac || op == 0xad || op == 0xbc) /* shld,shrd */
				{
					instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_SF;
					instruction->undefinedFlags.eflags = NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_OF;
				}
				else if (op == 0xaa) /* rsm */
					instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_TF | NMD_X86_EFLAGS_IF | NMD_X86_EFLAGS_DF | NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_IOPL | NMD_X86_EFLAGS_NT | NMD_X86_EFLAGS_RF | NMD_X86_EFLAGS_VM | NMD_X86_EFLAGS_AC | NMD_X86_EFLAGS_VIF | NMD_X86_EFLAGS_VIP | NMD_X86_EFLAGS_ID;
				else if ((op == 0xbc || op == 0xbd) && instruction->prefixes & NMD_X86_PREFIXES_REPEAT) /* tzcnt */
				{
					instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_ZF;
					instruction->undefinedFlags.eflags = NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_OF;
				}
				else if (op == 0xbc || op == 0xbd) /* bsf */
				{
					instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_ZF;
					instruction->undefinedFlags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_OF;
				}
			}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_CPU_FLAGS */

			if (NMD_R(op) == 8) /* imm32 */
				instruction->immMask = (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_IMM16 : NMD_X86_IMM32);
			else if ((NMD_R(op) == 7 && NMD_C(op) < 4) || op == 0xA4 || op == 0xC2 || (op > 0xC3 && op <= 0xC6) || op == 0xBA || op == 0xAC) /* imm8 */
				instruction->immMask = NMD_X86_IMM8;
			else if (op == 0x78 && (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO || instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)) /* imm8 + imm8 = "imm16" */
				instruction->immMask = NMD_X86_IMM16;
			
#ifndef NMD_ASSEMBLY_DISABLE_DECODER_GROUP
			/* Parse the instruction's group. */
			if (featureFlags & NMD_X86_FEATURE_FLAGS_GROUP)
			{
				if (NMD_R(op) == 8)
					instruction->group = NMD_GROUP_JUMP | NMD_GROUP_CONDITIONAL_BRANCH | NMD_GROUP_RELATIVE_ADDRESSING;
				else if ((op == 0x01 && modrm.fields.rm == 0b111 && (modrm.fields.mod == 0b00 || modrm.modrm == 0xf8)) || op == 0x06 || op == 0x08 || op == 0x09)
					instruction->group = NMD_GROUP_PRIVILEGE;
				else if (op == 0x05)
					instruction->group = NMD_GROUP_INT;
			}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_GROUP */

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_OPERANDS
			if (featureFlags & NMD_X86_FEATURE_FLAGS_OPERANDS)
			{
				if (op == 0x2 || op == 0x3 || (op >= 0x10 && op <= 0x17) || NMD_R(op) == 2 || (NMD_R(op) >= 4 && NMD_R(op) <= 7) || op == 0xa3 || op == 0xab || op == 0xaf || (NMD_R(op) >= 0xc && op != 0xc7 && op != 0xff))
					instruction->numOperands = 2;
				else if (NMD_R(op) == 8 || NMD_R(op) == 9 || (NMD_R(op) == 0xa && op % 8 < 2) || op == 0xc7)
					instruction->numOperands = 1;
				else if (op == 0xa4 || op == 0xa5 || op == 0xc2 || (op >= 0xc4 && op <= 0xc6))
					instruction->numOperands = 3;

				if (instruction->numOperands > 0)
				{
					if (op == 0x00)
					{
						if (instruction->modrm.fields.reg >= 0b010)
							parseOperandEw(instruction, &instruction->operands[0]);
						else
							parseOperandEv(instruction, &instruction->operands[0]);

						instruction->operands[0].action = instruction->modrm.fields.reg >= 0b010 ? NMD_X86_OPERAND_ACTION_READ : NMD_X86_OPERAND_ACTION_WRITE;
					}
					else if (op == 0x01)
					{
						if (instruction->modrm.fields.mod != 0b11)
						{
							parseModrmUpper32(instruction, &instruction->operands[0]);
							instruction->operands[0].action = instruction->modrm.fields.reg >= 0b010 ? NMD_X86_OPERAND_ACTION_READ : NMD_X86_OPERAND_ACTION_WRITE;
						}
						else if (instruction->modrm.fields.reg == 0b100)
							parseOperandRv(instruction, &instruction->operands[0]);
						else if (instruction->modrm.fields.reg == 0b110)
						{
							parseOperandEw(instruction, &instruction->operands[0]);
							instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READ;
						}

						if (instruction->modrm.fields.reg == 0b100)
							instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
					}
					else if (op == 0x02 || op == 0x03)
					{
						parseOperandGv(instruction, &instruction->operands[0]);
						parseOperandEw(instruction, &instruction->operands[1]);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if (op == 0x0d)
					{
						parseOperandEv(instruction, &instruction->operands[0]);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if (NMD_R(op) == 0x8)
					{
						instruction->operands[0].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
					}
					else if (NMD_R(op) == 9)
					{
						parseOperandEb(instruction, &instruction->operands[0]);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
					}
					else if (op == 0x17)
					{
						parseModrmUpper32(instruction, &instruction->operands[0]);
						parseOperandVdq(instruction, &instruction->operands[1]);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if (op >= 0x20 && op <= 0x23)
					{
						instruction->operands[0].type = instruction->operands[1].type = NMD_X86_OPERAND_TYPE_REGISTER;
						instruction->operands[op < 0x22 ? 0 : 1].fields.reg = NMD_X86_REG_EAX + instruction->modrm.fields.rm;
						instruction->operands[op < 0x22 ? 1 : 0].fields.reg = ((op % 2 == 0 ? NMD_X86_REG_CR0 : NMD_X86_REG_DR0) + instruction->modrm.fields.reg);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if (op == 0x29 || op == 0x2b || (op == 0x7f && instruction->simdPrefix))
					{
						parseOperandWdq(instruction, &instruction->operands[0]);
						parseOperandVdq(instruction, &instruction->operands[1]);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if (op == 0x2a || op == 0x2c || op == 0x2d)
					{
						if (op == 0x2a)
							parseOperandVdq(instruction, &instruction->operands[0]);
						else if (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT || instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
							parseOperandGy(instruction, &instruction->operands[0]);
						else if (op == 0x2d && instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
							parseOperandQq(instruction, &instruction->operands[0]);
						else
							parseOperandPq(instruction, &instruction->operands[0]);

						if (op == 0x2a)
						{
							if (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT || instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
								parseOperandEy(instruction, &instruction->operands[1]);
							else
								parseOperandQq(instruction, &instruction->operands[1]);
						}
						else
							parseOperandWdq(instruction, &instruction->operands[1]);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if (op == 0x50)
					{
						parseOperandGy(instruction, &instruction->operands[0]);
						parseOperandUdq(instruction, &instruction->operands[1]);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if (NMD_R(op) == 5 || (op >= 0x10 && op <= 0x16) || op == 0x28 || op == 0x2e || op == 0x2f || (op == 0x7e && instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT))
					{
						parseOperandVdq(instruction, &instruction->operands[op == 0x11 || op == 0x13 ? 1 : 0]);
						parseOperandWdq(instruction, &instruction->operands[op == 0x11 || op == 0x13 ? 0 : 1]);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if (op == 0x7e)
					{
						parseOperandEy(instruction, &instruction->operands[0]);
						instruction->operands[1].type = NMD_X86_OPERAND_TYPE_REGISTER;
						instruction->operands[1].size = 1;
						instruction->operands[1].fields.reg = ((instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_REG_XMM0 : NMD_X86_REG_MM0) + instruction->modrm.fields.reg);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if (NMD_R(op) == 6 || op == 0x70 || (op >= 0x74 && op <= 0x76) || (op >= 0x7c && op <= 0x7f))
					{
						if (!instruction->simdPrefix)
						{
							parseOperandPq(instruction, &instruction->operands[op == 0x7f ? 1 : 0]);

							if (op == 0x6e)
								parseOperandEy(instruction, &instruction->operands[1]);
							else
								parseOperandQq(instruction, &instruction->operands[op == 0x7f ? 0 : 1]);
						}
						else
						{
							parseOperandVdq(instruction, &instruction->operands[0]);

							if (op == 0x6e)
								parseOperandEy(instruction, &instruction->operands[1]);
							else
								parseOperandWdq(instruction, &instruction->operands[1]);
						}

						if (op == 0x70)
							instruction->operands[2].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;

						instruction->operands[0].action = ((op >= 0x60 && op <= 0x6d) || (op >= 0x74 && op <= 0x76)) ? NMD_X86_OPERAND_ACTION_READ_WRITE : NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if (op >= 0x71 && op <= 0x73)
					{
						if (instruction->simdPrefix == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
							parseOperandUdq(instruction, &instruction->operands[0]);
						else
							parseOperandQq(instruction, &instruction->operands[0]);
						instruction->operands[1].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READ_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if (op == 0x78 || op == 0x79)
					{
						if (instruction->simdPrefix)
						{
							if (op == 0x78)
							{
								size_t i = 0;
								if (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT_NOT_ZERO)
									parseOperandVdq(instruction, &instruction->operands[i++]);
								parseOperandUdq(instruction, &instruction->operands[i + 0]);
								instruction->operands[i + 1].type = instruction->operands[i + 2].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
								instruction->operands[i + 1].size = instruction->operands[i + 2].size = 1;
								instruction->operands[i + 1].fields.imm = b[1];
								instruction->operands[i + 2].fields.imm = b[2];
							}
							else
							{
								parseOperandVdq(instruction, &instruction->operands[0]);
								parseOperandWdq(instruction, &instruction->operands[1]);
							}
						}
						else
						{
							parseOperandEy(instruction, &instruction->operands[op == 0x78 ? 0 : 1]);
							parseOperandGy(instruction, &instruction->operands[op == 0x78 ? 1 : 0]);
						}
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if (NMD_R(op) == 0xa && (op % 8) < 2)
					{
						instruction->operands[0].type = NMD_X86_OPERAND_TYPE_REGISTER;
						instruction->operands[0].fields.reg = op > 0xa8 ? NMD_X86_REG_GS : NMD_X86_REG_FS;
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if ((NMD_R(op) == 0xa && ((op % 8) >= 3 && (op % 8) <= 5)) || op == 0xb3 || op == 0xbb)
					{
						parseOperandEv(instruction, &instruction->operands[0]);
						parseOperandGv(instruction, &instruction->operands[1]);

						if (NMD_R(op) == 0xa)
						{
							if ((op % 8) == 4)
								instruction->operands[2].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
							else if ((op % 8) == 5)
							{
								instruction->operands[2].type = NMD_X86_OPERAND_TYPE_REGISTER;
								instruction->operands[2].fields.reg = NMD_X86_REG_CL;
							}
						}

						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READ_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if (op == 0xaf || op == 0xb8)
					{
						parseOperandGv(instruction, &instruction->operands[0]);
						parseOperandEv(instruction, &instruction->operands[1]);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READ_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if (op == 0xba)
					{
						parseOperandEv(instruction, &instruction->operands[0]);
						instruction->operands[0].action = instruction->modrm.fields.reg <= 0b101 ? NMD_X86_OPERAND_ACTION_READ : NMD_X86_OPERAND_ACTION_READ_WRITE;
						instruction->operands[1].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if (NMD_R(op) == 0xb && (op % 8) >= 6)
					{
						parseOperandGv(instruction, &instruction->operands[0]);
						if ((op % 8) == 6)
							parseOperandEb(instruction, &instruction->operands[1]);
						else
							parseOperandEw(instruction, &instruction->operands[1]);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if (NMD_R(op) == 0x4 || (NMD_R(op) == 0xb && ((op % 8) == 0x4 || (op % 8) == 0x5)))
					{
						parseOperandGv(instruction, &instruction->operands[0]);
						parseOperandEv(instruction, &instruction->operands[1]);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if ((NMD_R(op) == 0xb || NMD_R(op) == 0xc) && NMD_C(op) < 2)
					{
						if (NMD_C(op) == 0)
						{
							parseOperandEb(instruction, &instruction->operands[0]);
							parseOperandGb(instruction, &instruction->operands[1]);
						}
						else
						{
							parseOperandEv(instruction, &instruction->operands[0]);
							parseOperandGv(instruction, &instruction->operands[1]);
						}

						if (NMD_R(op) == 0xb)
						{
							instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READ | NMD_X86_OPERAND_ACTION_CONDITIONAL_WRITE;
							instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
						}
						else
							instruction->operands[0].action = instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ_WRITE;
					}
					else if (op == 0xb2)
					{
						parseOperandGv(instruction, &instruction->operands[0]);
						parseModrmUpper32(instruction, &instruction->operands[1]);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if (op == 0xc3)
					{
						parseModrmUpper32(instruction, &instruction->operands[0]);
						parseOperandGy(instruction, &instruction->operands[1]);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					}
					else if (op == 0xc2 || op == 0xc6)
					{
						parseOperandVdq(instruction, &instruction->operands[0]);
						parseOperandWdq(instruction, &instruction->operands[1]);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READ_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
						instruction->operands[2].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
					}
					else if (op == 0xc4)
					{
						if (instruction->prefixes == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
							parseOperandVdq(instruction, &instruction->operands[0]);
						else
							parseOperandPq(instruction, &instruction->operands[0]);
						parseOperandEy(instruction, &instruction->operands[1]);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
						instruction->operands[2].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
					}
					else if (op == 0xc5)
					{
						parseOperandGd(instruction, &instruction->operands[0]);
						if (instruction->prefixes == NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
							parseOperandUdq(instruction, &instruction->operands[1]);
						else
							parseOperandNq(instruction, &instruction->operands[1]);
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
						instruction->operands[2].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
					}
					else if (op == 0xc7)
					{
						if (instruction->modrm.fields.mod == 0b11)
							parseOperandEv(instruction, &instruction->operands[0]);
						else
							parseModrmUpper32(instruction, &instruction->operands[0]);
						instruction->operands[0].action = instruction->modrm.fields.reg == 0b001 ? (NMD_X86_OPERAND_ACTION_READ | NMD_X86_OPERAND_ACTION_CONDITIONAL_WRITE) : (instruction->modrm.fields.mod == 0b11 || !instruction->simdPrefix ? NMD_X86_OPERAND_ACTION_WRITE : NMD_X86_OPERAND_ACTION_READ);
					}
					else if (op >= 0xc8 && op <= 0xcf)
					{
						instruction->operands[0].type = NMD_X86_OPERAND_TYPE_REGISTER;
						instruction->operands[0].fields.reg = ((instruction->prefixes & (NMD_X86_PREFIXES_REX_W | NMD_X86_PREFIXES_REX_B)) == (NMD_X86_PREFIXES_REX_W | NMD_X86_PREFIXES_REX_B) ? NMD_X86_REG_R8 : (instruction->prefixes & NMD_X86_PREFIXES_REX_W ? NMD_X86_REG_RAX : (instruction->prefixes & NMD_X86_PREFIXES_REX_B ? NMD_X86_REG_R8D : NMD_X86_REG_EAX)) + (op % 8));
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READ_WRITE;
					}
					else if (NMD_R(op) >= 0xd)
					{
						if (op == 0xff)
						{
							parseOperandGd(instruction, &instruction->operands[0]);
							parseMemoryOperand(instruction, &instruction->operands[1], NMD_X86_REG_EAX);
						}
						else if (op == 0xd6 && instruction->simdPrefix != NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
						{
							if (instruction->simdPrefix == NMD_X86_PREFIXES_REPEAT)
							{
								parseOperandVdq(instruction, &instruction->operands[0]);
								parseOperandQq(instruction, &instruction->operands[1]);
							}
							else
							{
								parseOperandPq(instruction, &instruction->operands[0]);
								parseOperandWdq(instruction, &instruction->operands[1]);
							}
						}
						else
						{
							const size_t opIndex1 = op == 0xe7 || op == 0xd6 ? 1 : 0;
							const size_t opIndex2 = op == 0xe7 || op == 0xd6 ? 0 : 1;

							if (!instruction->simdPrefix)
							{
								if (op == 0xd7)
									parseOperandGd(instruction, &instruction->operands[0]);
								else
									parseOperandPq(instruction, &instruction->operands[opIndex1]);
								parseOperandQq(instruction, &instruction->operands[opIndex2]);
							}
							else
							{
								if (op == 0xd7)
									parseOperandGd(instruction, &instruction->operands[0]);
								else
									parseOperandVdq(instruction, &instruction->operands[opIndex1]);
								parseOperandWdq(instruction, &instruction->operands[opIndex2]);
							}
						}
						instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
					}
				}
			}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_OPERANDS */
		}
	}
	else /* 1 byte */
	{
		instruction->opcodeSize = 1;
		instruction->opcode = *b;
		instruction->opcodeMap = NMD_X86_OPCODE_MAP_DEFAULT;

		op = instruction->opcode;

		/* Check for ModR/M, SIB and displacement. */
		if (NMD_R(op) == 8 || nmd_findByte(op1modrm, sizeof(op1modrm), op) || (NMD_R(op) < 4 && (NMD_C(op) < 4 || (NMD_C(op) >= 8 && NMD_C(op) < 0xC))) || (NMD_R(op) == 0xD && NMD_C(op) >= 8))
		{
			if (!parseModrm(&b, instruction, remainingSize - 1))
				return false;
		}

		const NMD_Modrm modrm = instruction->modrm;
#ifndef NMD_ASSEMBLY_DISABLE_DECODER_VALIDITY_CHECK
			/* Check if the instruction is invalid. */
			if (featureFlags & NMD_X86_FEATURE_FLAGS_VALIDITY_CHECK)
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
					if(modrm.fields.reg == 0b111 || (modrm.fields.mod == 0b11 && (modrm.fields.reg == 0b011 || modrm.fields.reg == 0b101)))
						return false;
				}
				else if (op == 0x8c || op == 0x8e)
				{
					if (modrm.fields.reg >= 6)
						return false;
				}
				else if (op == 0x8e)
				{
					if (modrm.fields.reg == 0b001)
						return false;
				}
				else if (op == 0x8d || op == 0x62)
				{
					if (modrm.fields.mod == 0b11)
						return false;
				}
				else if (op == 0xc4 || op == 0xc5)
				{
					if (mode == NMD_X86_MODE_64 && modrm.fields.mod != 0b11)
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
						if ((modrm.fields.reg == 0b101 && modrm.fields.mod != 0b11) || NMD_R(modrm.modrm) == 0xf)
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
		
#ifndef NMD_ASSEMBLY_DISABLE_DECODER_INSTRUCTION_ID
		if (featureFlags & NMD_X86_FEATURE_FLAGS_INSTRUCTION_ID)
		{
			const bool operandSize = instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE;
			if ((op >= 0x88 && op <= 0x8c) || (op >= 0xa0 && op <= 0xa3) || NMD_R(op) == 0xb || op == 0x8e)
				instruction->id = NMD_X86_INSTRUCTION_MOV;
			else if (NMD_R(op) == 5)
				instruction->id = (NMD_C(op) < 8) ? NMD_X86_INSTRUCTION_PUSH : NMD_X86_INSTRUCTION_POP;
			else if (NMD_R(op) < 4 && (op % 8 < 6))
				instruction->id = (NMD_X86_INSTRUCTION_ADD + (NMD_R(op) << 1) + (NMD_C(op) >= 8 ? 1 : 0));
			else if (op >= 0x80 && op <= 0x84)
				instruction->id = NMD_X86_INSTRUCTION_ADD + modrm.fields.reg;
			else if (op == 0xe8)
				instruction->id = NMD_X86_INSTRUCTION_CALL;
			else if (op == 0xcc)
				instruction->id = NMD_X86_INSTRUCTION_INT3;
			else if (op == 0x8d)
				instruction->id = NMD_X86_INSTRUCTION_LEA;
			else if (NMD_R(op) == 4)
				instruction->id = (NMD_C(op) < 8) ? NMD_X86_INSTRUCTION_INC : NMD_X86_INSTRUCTION_DEC;
			else if (NMD_R(op) == 7)
				instruction->id = NMD_X86_INSTRUCTION_JO + NMD_C(op);
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
			else if (NMD_R(op) == 0x0f && (op % 8 < 6))
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
				case 0xfe: instruction->id = modrm.fields.reg == 0b000 ? NMD_X86_INSTRUCTION_INC : NMD_X86_INSTRUCTION_DEC; break;
				case 0x84: case 0x85: case 0xa8: case 0xa9: instruction->id = NMD_X86_INSTRUCTION_TEST; break;
				case 0xf6: case 0xf7: instruction->id = NMD_X86_INSTRUCTION_TEST + modrm.fields.reg; break;
				case 0x69: case 0x6b: instruction->id = NMD_X86_INSTRUCTION_IMUL; break;
				case 0x9a: instruction->id = NMD_X86_INSTRUCTION_CALL; break;
				case 0x62: instruction->id = NMD_X86_INSTRUCTION_BOUND; break;
				case 0x63: instruction->id = instruction->mode == NMD_X86_MODE_64 ? NMD_X86_INSTRUCTION_MOVSXD : NMD_X86_INSTRUCTION_ARPL; break;
				case 0x68: case 0x6a: case 0x06: case 0x16: case 0x1e: case 0x0e: instruction->id = NMD_X86_INSTRUCTION_PUSH; break;
				case 0x6c: instruction->id = NMD_X86_INSTRUCTION_INSB; break;
				case 0x6d: instruction->id = operandSize ? NMD_X86_INSTRUCTION_INSW : NMD_X86_INSTRUCTION_INSD; break;
				case 0x6e: instruction->id = NMD_X86_INSTRUCTION_OUTSB; break;
				case 0x6f: instruction->id = operandSize ? NMD_X86_INSTRUCTION_OUTSW : NMD_X86_INSTRUCTION_OUTSD; break;
				case 0xc2: case 0xc3: ; break;
				case 0xc4: instruction->id = NMD_X86_INSTRUCTION_LES; break;
				case 0xc5: instruction->id = NMD_X86_INSTRUCTION_LDS; break;
				case 0xc6: case 0xc7: instruction->id = (modrm.fields.reg == 0b000 ? NMD_X86_INSTRUCTION_MOV : (instruction->opcode == 0xc6 ? NMD_X86_INSTRUCTION_XABORT : NMD_X86_INSTRUCTION_XBEGIN)); break;
				case 0xc8: instruction->id = NMD_X86_INSTRUCTION_ENTER; break;
				case 0xc9: instruction->id = NMD_X86_INSTRUCTION_LEAVE; break;
				case 0xca: case 0xcb: instruction->id = NMD_X86_INSTRUCTION_RETF; break;
				case 0xcd: instruction->id = NMD_X86_INSTRUCTION_INT; break;
				case 0xce: instruction->id = NMD_X86_INSTRUCTION_INTO; break;
				case 0xcf: instruction->id = (instruction->prefixes & NMD_X86_PREFIXES_REX_W) ? NMD_X86_INSTRUCTION_IRETQ : (operandSize ? NMD_X86_INSTRUCTION_IRET : NMD_X86_INSTRUCTION_IRETD); break;
				case 0xe4: case 0xe5: case 0xec: case 0xed: instruction->id = NMD_X86_INSTRUCTION_IN; break;
				case 0xe6: case 0xe7: case 0xee: case 0xef: instruction->id = NMD_X86_INSTRUCTION_OUT; break;
				case 0xea: instruction->id = NMD_X86_INSTRUCTION_LJMP; break;
				case 0x9c: instruction->id = instruction->mode == NMD_X86_MODE_64 && !(instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE) ? NMD_X86_INSTRUCTION_PUSHFQ : (operandSize ? NMD_X86_INSTRUCTION_PUSHF : NMD_X86_INSTRUCTION_PUSHFD); break;
				case 0x9d: instruction->id = instruction->mode == NMD_X86_MODE_64 && !(instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE) ? NMD_X86_INSTRUCTION_POPFQ : (operandSize ? NMD_X86_INSTRUCTION_POPF : NMD_X86_INSTRUCTION_POPFD); break;
				case 0x60:
				case 0x61:
					instruction->id = operandSize ? (instruction->opcode == 0x60 ? NMD_X86_INSTRUCTION_PUSHA : NMD_X86_INSTRUCTION_POPA) : (instruction->opcode == 0x60 ? NMD_X86_INSTRUCTION_PUSHAD : NMD_X86_INSTRUCTION_POPAD);
					break;
				case 0x07: case 0x17: case 0x1f: instruction->id = NMD_X86_INSTRUCTION_POP; break;
				case 0x27: instruction->id = NMD_X86_INSTRUCTION_DAA; break;
				case 0x37: instruction->id = NMD_X86_INSTRUCTION_AAA; break;
				case 0x2f: instruction->id = NMD_X86_INSTRUCTION_DAS; break;
				case 0x3f: instruction->id = NMD_X86_INSTRUCTION_AAS; break;
				case 0x9b: instruction->id = NMD_X86_INSTRUCTION_WAIT; break;
				case 0x9e: instruction->id = NMD_X86_INSTRUCTION_SAHF; break;
				case 0x9f: instruction->id = NMD_X86_INSTRUCTION_LAHF; break;
				case 0xA4: instruction->id = NMD_X86_INSTRUCTION_MOVSB; break;
				case 0xA5: instruction->id = instruction->flags.fields.operandSize64 ? NMD_X86_INSTRUCTION_MOVSQ : (operandSize ? NMD_X86_INSTRUCTION_MOVSW : NMD_X86_INSTRUCTION_MOVSD); break;
				case 0xA6: instruction->id = NMD_X86_INSTRUCTION_CMPSB; break;
				case 0xA7: instruction->id = instruction->flags.fields.operandSize64 ? NMD_X86_INSTRUCTION_CMPSQ : (operandSize ? NMD_X86_INSTRUCTION_CMPSW : NMD_X86_INSTRUCTION_CMPSD); break;
				case 0xAA: instruction->id = NMD_X86_INSTRUCTION_STOSB; break;
				case 0xAB: instruction->id = instruction->flags.fields.operandSize64 ? NMD_X86_INSTRUCTION_STOSQ : (operandSize ? NMD_X86_INSTRUCTION_STOSW : NMD_X86_INSTRUCTION_STOSD); break;
				case 0xAC: instruction->id = NMD_X86_INSTRUCTION_LODSB; break;
				case 0xAD: instruction->id = instruction->flags.fields.operandSize64 ? NMD_X86_INSTRUCTION_LODSQ : (operandSize ? NMD_X86_INSTRUCTION_LODSW : NMD_X86_INSTRUCTION_LODSD); break;
				case 0xAE: instruction->id = NMD_X86_INSTRUCTION_SCASB; break;
				case 0xAF: instruction->id = instruction->flags.fields.operandSize64 ? NMD_X86_INSTRUCTION_SCASQ : (operandSize ? NMD_X86_INSTRUCTION_SCASW : NMD_X86_INSTRUCTION_SCASD); break;
				case 0x98: instruction->id = (instruction->prefixes & NMD_X86_PREFIXES_REX_W ? NMD_X86_INSTRUCTION_CDQE : (operandSize ? NMD_X86_INSTRUCTION_CBW : NMD_X86_INSTRUCTION_CWDE)); break;
				case 0x99: instruction->id = (instruction->prefixes & NMD_X86_PREFIXES_REX_W ? NMD_X86_INSTRUCTION_CQO : (operandSize ? NMD_X86_INSTRUCTION_CWD : NMD_X86_INSTRUCTION_CDQ)); break;
				case 0xd6: instruction->id = NMD_X86_INSTRUCTION_SALC; break;

					/* Floating-point opcodes. */
#define NMD_F_OP_GET_OFFSET() ((NMD_R(modrm.modrm) - 0xc) << 1) + (NMD_C(op) >= 8 ? 1 : 0)
				case 0xd8: instruction->id = (NMD_X86_INSTRUCTION_FADD + (modrm.fields.mod == 0b11 ? NMD_F_OP_GET_OFFSET() : modrm.fields.reg)); break;
				case 0xd9:
					if (modrm.fields.mod == 0b11)
					{
						if (modrm.modrm <= 0xcf)
							instruction->id = modrm.modrm <= 0xc7 ? NMD_X86_INSTRUCTION_FLD : NMD_X86_INSTRUCTION_FXCH;
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
						instruction->id = ((modrm.modrm == 0xe9) ? NMD_X86_INSTRUCTION_FUCOMPP : NMD_X86_INSTRUCTION_FCMOVB + NMD_F_OP_GET_OFFSET());
					else
						instruction->id = NMD_X86_INSTRUCTION_FIADD + modrm.fields.reg;
					break;
				case 0xdb:
					if (modrm.fields.mod == 0b11)
						instruction->id = (modrm.modrm == 0xe2 ? NMD_X86_INSTRUCTION_FNCLEX : (modrm.modrm == 0xe2 ? NMD_X86_INSTRUCTION_FNINIT : NMD_X86_INSTRUCTION_FCMOVNB + NMD_F_OP_GET_OFFSET()));
					else
						instruction->id = (modrm.fields.reg == 0b101 ? NMD_X86_INSTRUCTION_FLD : (modrm.fields.reg == 0b111 ? NMD_X86_INSTRUCTION_FSTP : NMD_X86_INSTRUCTION_FILD + modrm.fields.reg));
					break;
				case 0xdc:
					if (modrm.fields.mod == 0b11)
						instruction->id = (NMD_X86_INSTRUCTION_FADD + ((NMD_R(modrm.modrm) - 0xc) << 1) + ((NMD_C(modrm.modrm) >= 8 && NMD_R(modrm.modrm) <= 0xd) ? 1 : 0));
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
						instruction->id = (modrm.modrm == 0xd9 ? NMD_X86_INSTRUCTION_FCOMPP : ((modrm.modrm >= 0xd0 && modrm.modrm <= 0xd7) ? NMD_X86_INSTRUCTION_FCOMP : NMD_X86_INSTRUCTION_FADDP + NMD_F_OP_GET_OFFSET()));
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
		if (featureFlags & NMD_X86_FEATURE_FLAGS_CPU_FLAGS)
		{
			if (op == 0xcc || op == 0xcd || op == 0xce || op == 0xf1) /* int3,int,into,int1 */
			{
				instruction->clearedFlags.eflags = NMD_X86_EFLAGS_TF | NMD_X86_EFLAGS_RF;
				instruction->testedFlags.eflags = NMD_X86_EFLAGS_NT | NMD_X86_EFLAGS_VM;
				instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_IF | NMD_X86_EFLAGS_NT | NMD_X86_EFLAGS_VM | NMD_X86_EFLAGS_AC | NMD_X86_EFLAGS_VIF;
			}
			else if (NMD_R(op) == 7) /* conditional jump */
				decodeConditionalFlag(instruction, NMD_C(op));
			else if (NMD_R(op) == 4 || ((op == 0xfe || op == 0xff) && modrm.fields.reg <= 0b001)) /* inc,dec */
				instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_PF;
			else if (op <= 0x05 || (op >= 0x10 && op <= 0x15) || ((NMD_R(op) == 1 || NMD_R(op) == 2 || NMD_R(op) == 3) && (NMD_C(op) >= 0x8 && NMD_C(op) <= 0x0d)) || ((op >= 0x80 && op <= 0x83) && (modrm.fields.reg == 0b000 || modrm.fields.reg == 0b010 || modrm.fields.reg == 0b011 || modrm.fields.reg == 0b010 || modrm.fields.reg == 0b101 || modrm.fields.reg == 0b111)) || (op == 0xa6 || op == 0xa7) || (op == 0xae || op == 0xaf)) /* add,adc,sbb,sub,cmp, cmps,cmpsb,cmpsw,cmpsd,cmpsq, scas,scasb,scasw,scasd */
				instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_PF;
			else if ((op >= 0x08 && op <= 0x0d) || ((NMD_R(op) == 2 || NMD_R(op) == 3) && NMD_C(op) <= 5) || ((op >= 0x80 && op <= 0x83) && (modrm.fields.reg == 0b001 || modrm.fields.reg == 0b100 || modrm.fields.reg == 0b110)) || (op == 0x84 || op == 0x85 || op == 0xa8 || op == 0xa9) || ((op == 0xf6 || op == 0xf7) && modrm.fields.reg == 0b000)) /* or,and,xor, test */
			{
				instruction->clearedFlags.eflags = NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_CF;
				instruction->undefinedFlags.eflags = NMD_X86_EFLAGS_AF;
				instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_PF;
			}
			else if (op == 0x69 || op == 0x6b || ((op == 0xf6 || op == 0xf7) && (modrm.fields.reg == 0b100 || modrm.fields.reg == 0b101))) /* mul,imul */
			{
				instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_OF;
				instruction->undefinedFlags.eflags = NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_PF;
			}
			else if (op == 0xf6 || op == 0xf7) /* Group 3 */
			{
				if (modrm.fields.reg == 0b011) /* neg */
					instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_OF;
				else if (modrm.fields.reg >= 0b110) /* div,idiv */
					instruction->undefinedFlags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_PF;
			}
			else if (op == 0xc0 || op == 0xc1 || (op >= 0xd0 && op <= 0xd3))
			{
				if (modrm.fields.reg <= 0b011) /* rol,ror,rcl,rcr */
				{
					instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_CF;
					instruction->undefinedFlags.eflags = NMD_X86_EFLAGS_OF;
				}
				else /* shl,shr,sar */
				{
					instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_OF;
					instruction->undefinedFlags.eflags = NMD_X86_EFLAGS_AF;
				}
			}
			else if (op == 0x27 || op == 0x2f) /* daa,das */
			{
				instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_CF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_PF;
				instruction->undefinedFlags.eflags = NMD_X86_EFLAGS_OF;
			}
			else if (op == 0x37 || op == 0x3f) /* aaa,aas */
			{
				instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_CF;
				instruction->undefinedFlags.eflags = NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_PF;
			}
			else if (op == 0x63 && instruction->mode != NMD_X86_MODE_64) /* arpl */
				instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_ZF;
			else if (op == 0x9b) /* fwait,wait */
				instruction->undefinedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C1 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
			else if (op == 0x9e) /* sahf */
				instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_PF | NMD_X86_EFLAGS_CF;
			else if (op == 0xd4 || op == 0xd5) /* aam,aad */
			{
				instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_SF | NMD_X86_EFLAGS_ZF | NMD_X86_EFLAGS_PF;
				instruction->undefinedFlags.eflags = NMD_X86_EFLAGS_OF | NMD_X86_EFLAGS_AF | NMD_X86_EFLAGS_CF;
			}
			else if (op >= 0xd8 && op <= 0xdf) /* escape opcodes */
			{
				if (op == 0xd8 || op == 0xdc)
				{
					if (modrm.fields.reg == 0b000 || modrm.fields.reg == 0b001 || modrm.fields.reg == 0b100 || modrm.fields.reg == 0b101 || modrm.fields.reg == 0b110 || modrm.fields.reg == 0b111) /* fadd,fmul,fsub,fsubr,fdiv,fdivr */
					{
						instruction->modifiedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C1;
						instruction->undefinedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
					}
					else if (modrm.fields.reg == 0b010 || modrm.fields.reg == 0b011) /* fcom,fcomp */
					{
						instruction->modifiedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
						instruction->clearedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C1;
					}
				}
				else if (op == 0xd9)
				{
					if (modrm.fields.mod != 0b11)
					{
						if (modrm.fields.reg == 0b000 || modrm.fields.reg == 0b010 || modrm.fields.reg == 0b011) /* fld,fst,fstp */
						{
							instruction->modifiedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C1;
							instruction->undefinedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
						}
						else if(modrm.fields.reg == 0b100) /* fldenv */
							instruction->modifiedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C1 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
						else if(modrm.fields.reg == 0b101 || modrm.fields.reg == 0b110 || modrm.fields.reg == 0b111) /* fldcw,fstenv,fstcw */
							instruction->undefinedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C1 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
					}
					else
					{
						if(modrm.modrm < 0xc8) /* fld */
						{
							instruction->modifiedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C1;
							instruction->undefinedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
						}
						else /*if (modrm.modrm <= 0xcf)*/ /* fxch */
						{
							instruction->clearedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C1;
							instruction->undefinedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
						}
					}
				}
				else if (op == 0xda || op == 0xde)
				{
					if (modrm.fields.mod != 0b11)
					{
						if (modrm.fields.reg == 0b000 || modrm.fields.reg == 0b001 || modrm.fields.reg == 0b100 || modrm.fields.reg == 0b101 || modrm.fields.reg == 0b110 || modrm.fields.reg == 0b111) /* fiadd,fimul,fisub,fisubr,fidiv,fidivr */
						{
							instruction->modifiedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C1;
							instruction->undefinedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
						}
						else /*if (modrm.fields.reg == 0b010 || modrm.fields.reg == 0b011)*/ /* ficom,ficomp */
						{
							instruction->clearedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C1;
							instruction->modifiedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
						}
					}
					else
					{

						if((op == 0xda && modrm.modrm == 0xe9) || (op == 0xde && modrm.modrm == 0xd9))
							instruction->modifiedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C1 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
						else
						{
							instruction->modifiedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C1;
							instruction->undefinedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
						}
					}
				}
				else if (op == 0xdb || op == 0xdd || op == 0xdf)
				{
					if (modrm.fields.mod != 0b11)
					{
						if (modrm.fields.reg == 0b000 || modrm.fields.reg == 0b010 || modrm.fields.reg == 0b011 || modrm.fields.reg == 0b101 || modrm.fields.reg == 0b111) /* fild,fist,fistp,fld,fstp */
						{
							instruction->modifiedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C1;
							instruction->undefinedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
						}
						else if (modrm.fields.reg == 0b001) /* fisttp */
						{
							instruction->clearedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C1;
							instruction->undefinedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
						}
					}
					else
					{
						if (modrm.fields.reg <= 0b011) /* fcmovnb,fcmovne,fcmovnbe,fcmovnu */
						{
							instruction->modifiedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C1;
							instruction->undefinedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
						}
						else if(modrm.modrm == 0xe0 || modrm.modrm == 0xe2) /* fstsw,fclex */
							instruction->undefinedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C1 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
						else if(modrm.modrm == 0xe3) /* finit */
							instruction->clearedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C1 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
						else /* fucomi,fcomi */
						{
							instruction->clearedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C1;
							instruction->undefinedFlags.fpuFlags = NMD_X86_FPU_FLAGS_C0 | NMD_X86_FPU_FLAGS_C2 | NMD_X86_FPU_FLAGS_C3;
						}
					}
				}
			}
			else if (op == 0xf5) /* cmc */
				instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_CF;
			else if (op == 0xf8) /* clc */
				instruction->clearedFlags.eflags = NMD_X86_EFLAGS_CF;
			else if (op == 0xf9) /* stc */
				instruction->setFlags.eflags = NMD_X86_EFLAGS_CF;
			else if (op == 0xfa || op == 0xfb) /* cli,sti */
			{
				instruction->modifiedFlags.eflags = NMD_X86_EFLAGS_IF | NMD_X86_EFLAGS_VIF;
				instruction->testedFlags.eflags = NMD_X86_EFLAGS_IOPL;
			}
			else if (op == 0xfc) /* cld */
				instruction->clearedFlags.eflags = NMD_X86_EFLAGS_DF;
			else if (op == 0xfd) /* std */
				instruction->setFlags.eflags = NMD_X86_EFLAGS_DF;
		}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_CPU_FLAGS */

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_EVEX
		/* Check if instruction is EVEX. */
		if (featureFlags & NMD_X86_FEATURE_FLAGS_EVEX && op == 0x62 && modrm.fields.mod == 0b11)
		{
			instruction->opcodeOffset = instruction->numPrefixes + 4;

			instruction->encoding = NMD_X86_INSTRUCTION_ENCODING_VEX;
		}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_EVEX */

#if !defined(NMD_ASSEMBLY_DISABLE_DECODER_EVEX) && !defined(NMD_ASSEMBLY_DISABLE_DECODER_VEX)
		else
#endif
#ifndef NMD_ASSEMBLY_DISABLE_DECODER_VEX
			/* Check if instruction is VEX. */
			if (featureFlags & NMD_X86_FEATURE_FLAGS_VEX && (op == 0xc4 || op == 0xc5) && modrm.fields.mod == 0b11)
			{
				instruction->encoding = NMD_X86_INSTRUCTION_ENCODING_VEX;

				instruction->vex.vex[0] = op;
				const uint8_t byte1 = *++b;

				instruction->vex.R = byte1 & 0b10000000;
				if (instruction->vex.vex[1] == 0xc4)
				{
					instruction->opcodeOffset = instruction->numPrefixes + 3;

					instruction->vex.X = (byte1 & 0b01000000) == 0b01000000;
					instruction->vex.B = (byte1 & 0b00100000) == 0b00100000;
					instruction->vex.m_mmmm = (uint8_t)(byte1 & 0b00011111);

					const uint8_t byte2 = *++b;
					instruction->vex.W = (byte2 & 0b10000000) == 0b10000000;
					instruction->vex.vvvv = (uint8_t)((byte2 & 0b01111000) >> 3);
					instruction->vex.L = (byte2 & 0b00000100) == 0b00000100;
					instruction->vex.pp = (uint8_t)(byte2 & 0b00000011);

					instruction->opcode = *++b;
					op = instruction->opcode;

					if (op == 0x0c || op == 0x0d || op == 0x40 || op == 0x41 || op == 0x17 || op == 0x21 || op == 0x42)
						instruction->immMask = NMD_X86_IMM8;

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
					instruction->opcodeOffset = instruction->numPrefixes + 3;

					instruction->vex.vvvv = (uint8_t)(byte1 & 0b01111000);
					instruction->vex.L = byte1 & 0b00000100;
					instruction->vex.pp = (uint8_t)(byte1 & 0b00000011);

					b++;
					instruction->opcode = *b;
					op = instruction->opcode;
				}

				if (!parseModrm(&b, instruction, remainingSize - (instruction->vex.vex[0] == 0xc4 ? 3 : 2)))
					return false;
			}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_VEX */
#if !(defined(NMD_ASSEMBLY_DISABLE_DECODER_EVEX) && defined(NMD_ASSEMBLY_DISABLE_DECODER_VEX))
			else
#endif
			{
				instruction->opcodeOffset = instruction->numPrefixes;

				/* Check for immediate */		
				if (nmd_findByte(op1imm32, sizeof(op1imm32), op) || (NMD_R(op) < 4 && (NMD_C(op) == 5 || NMD_C(op) == 0xD)) || (NMD_R(op) == 0xB && NMD_C(op) >= 8) || (op == 0xF7 && !(*(b + 1) & 48))) /* imm32,16 */
				{
					if (NMD_R(op) == 0xB && NMD_C(op) >= 8)
						instruction->immMask = instruction->prefixes & NMD_X86_PREFIXES_REX_W ? NMD_X86_IMM64 : (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE || (instruction->mode == NMD_X86_MODE_16 && !(instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)) ? NMD_X86_IMM16 : NMD_X86_IMM32);
					else
						instruction->immMask = (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && instruction->mode == NMD_X86_MODE_32) || (instruction->mode == NMD_X86_MODE_16 && !(instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)) ? NMD_X86_IMM16 : NMD_X86_IMM32;
				}
				else if (NMD_R(op) == 7 || (NMD_R(op) == 0xE && NMD_C(op) < 8) || (NMD_R(op) == 0xB && NMD_C(op) < 8) || (NMD_R(op) < 4 && (NMD_C(op) == 4 || NMD_C(op) == 0xC)) || (op == 0xF6 && modrm.fields.reg <= 0b001) || nmd_findByte(op1imm8, sizeof(op1imm8), op)) /* imm8 */
					instruction->immMask = NMD_X86_IMM8;
				else if (NMD_R(op) == 0xA && NMD_C(op) < 4)
					instruction->immMask = (instruction->mode == NMD_X86_MODE_64) ? (instruction->prefixes & NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE ? NMD_X86_IMM32 : NMD_X86_IMM64) : (instruction->prefixes & NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE ? NMD_X86_IMM16 : NMD_X86_IMM32);
				else if (op == 0xEA || op == 0x9A) /* imm32,48 */
				{
					if (instruction->mode == NMD_X86_MODE_64)
						return false;
					instruction->immMask = (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_IMM32 : NMD_X86_IMM48);
				}
				else if (op == 0xC2 || op == 0xCA) /* imm16 */
					instruction->immMask = NMD_X86_IMM16;
				else if (op == 0xC8) /* imm16 + imm8 */
					instruction->immMask = NMD_X86_IMM16 | NMD_X86_IMM8;
							
#ifndef NMD_ASSEMBLY_DISABLE_DECODER_GROUP
				/* Parse the instruction's group. */
				if (featureFlags & NMD_X86_FEATURE_FLAGS_GROUP)
				{
					if (NMD_R(op) == 7 || op == 0xe3)
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
					else if (op == 0x8d && instruction->mode == NMD_X86_MODE_64)
						instruction->group = NMD_GROUP_RELATIVE_ADDRESSING;
					else if(op == 0xcf)
						instruction->group = NMD_GROUP_RET | NMD_GROUP_INT;
				}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_GROUP */

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_OPERANDS
				if (featureFlags & NMD_X86_FEATURE_FLAGS_OPERANDS)
				{
					if (op >= 0xd8 && op <= 0xdf)
					{
						if (modrm.fields.mod == 0b11)
						{
							if ((op == 0xd9 && (NMD_R(modrm.modrm) == 0xc || (op >= 0xc8 && op <= 0xcf))) ||
								(op == 0xda && NMD_R(modrm.modrm) <= 0xd) ||
								(op == 0xdb && (NMD_R(modrm.modrm) <= 0xd || modrm.modrm >= 0xe8)) ||
								(op == 0xde && modrm.modrm != 0xd9) ||
								(op == 0xdf && modrm.modrm != 0xe0))
								instruction->numOperands = 2;
						}
						else
							instruction->numOperands = 1;
					}
					else if ((NMD_R(op) < 4 && op % 8 < 6) || (NMD_R(op) >= 8 && NMD_R(op) <= 0xb && op != 0x8f && op != 0x90 && !(op >= 0x98 && op <= 0x9f)) || op == 0x62 || op == 0x63 || (op >= 0x6c && op <= 0x6f) || op == 0xc0 || op == 0xc1 || (op >= 0xc4 && op <= 0xc8) || (op >= 0xd0 && op <= 0xd3) || (NMD_R(op) == 0xe && op % 8 >= 4))
						instruction->numOperands = 2;
					else if (NMD_R(op) == 4 || NMD_R(op) == 5 || NMD_R(op) == 7 || (op == 0x68 || op == 0x6a) || op == 0x8f || op == 0x9a || op == 0xc2 || op == 0xca || op == 0xcd || op == 0xd4 || op == 0xd5 || (NMD_R(op) == 0xe && op % 8 <= 3) || (NMD_R(op) == 0xf && op % 8 >= 6))
						instruction->numOperands = 1;
					else if (op == 0x69 || op == 0x6b)
						instruction->numOperands = 3;

					if (instruction->numOperands > 0)
					{
						if (op >= 0x84 && op <= 0x8b)
						{
							if (op % 2 == 0)
							{
								parseOperandEb(instruction, &instruction->operands[op == 0x8a ? 1 : 0]);
								parseOperandGb(instruction, &instruction->operands[op == 0x8a ? 0 : 1]);
							}
							else
							{
								parseOperandEv(instruction, &instruction->operands[op == 0x8b ? 1 : 0]);
								parseOperandGv(instruction, &instruction->operands[op == 0x8b ? 0 : 1]);
							}

							if (op >= 0x88)
							{
								instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
								instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
							}
							else if (op >= 0x86)
								instruction->operands[0].action = instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ_WRITE;
						}
						else if (op >= 0x80 && op <= 0x83)
						{
							if (op % 2 == 0)
								parseOperandEb(instruction, &instruction->operands[0]);
							else
								parseOperandEv(instruction, &instruction->operands[0]);
							instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READ_WRITE;
							instruction->operands[1].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
						}
						else if (op == 0x68 || NMD_R(op) == 7 || op == 0x6a || op == 0x9a || op == 0xc2 || op == 0xca || op == 0xcd || op == 0xd4 || op == 0xd5)
							instruction->operands[0].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
						else if (op == 0x90 && instruction->prefixes & NMD_X86_PREFIXES_REX_B)
						{
							instruction->operands[0].type = instruction->operands[1].type = NMD_X86_OPERAND_TYPE_REGISTER;
							instruction->operands[0].fields.reg = instruction->prefixes & NMD_X86_PREFIXES_REX_W ? NMD_X86_REG_R8 : NMD_X86_REG_R8D;
							instruction->operands[1].fields.reg = instruction->prefixes & NMD_X86_PREFIXES_REX_W ? NMD_X86_REG_RAX : NMD_X86_REG_EAX;
						}
						else if (NMD_R(op) < 4)
						{
							const size_t opMod8 = (size_t)(op % 8);
							if (opMod8 == 0 || opMod8 == 2)
							{
								parseOperandEb(instruction, &instruction->operands[opMod8 == 0 ? 0 : 1]);
								parseOperandGb(instruction, &instruction->operands[opMod8 == 0 ? 1 : 0]);
							}
							else if (opMod8 == 1 || opMod8 == 3)
							{
								parseOperandEv(instruction, &instruction->operands[opMod8 == 1 ? 0 : 1]);
								parseOperandGv(instruction, &instruction->operands[opMod8 == 1 ? 1 : 0]);
							}
							else if (opMod8 == 4 || opMod8 == 5)
							{
								instruction->operands[0].type = NMD_X86_OPERAND_TYPE_REGISTER;
								if (opMod8 == 4)
									instruction->operands[0].fields.reg = NMD_X86_REG_AL;
								else
									instruction->operands[0].fields.reg = instruction->flags.fields.operandSize64 ? NMD_X86_REG_RAX : (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_REG_AX : NMD_X86_REG_EAX);

								instruction->operands[1].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
							}

							instruction->operands[0].action = instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
							if (!(NMD_R(op) == 3 && NMD_C(op) >= 8))
								instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READ_WRITE;
						}
						else if (NMD_R(op) == 4)
						{
							instruction->operands[0].type = NMD_X86_OPERAND_TYPE_REGISTER;
							instruction->operands[0].fields.reg = ((instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_REG_AX : NMD_X86_REG_EAX) + (op % 8));
							instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READ_WRITE;
						}
						else if (NMD_R(op) == 5)
						{
							instruction->operands[0].type = NMD_X86_OPERAND_TYPE_REGISTER;
							instruction->operands[0].fields.reg = ((instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_REG_AX : (instruction->mode == NMD_X86_MODE_64 ? NMD_X86_REG_RAX : NMD_X86_REG_EAX)) + (op % 8));
							instruction->operands[0].action = NMD_C(op) < 8 ? NMD_X86_OPERAND_ACTION_READ : NMD_X86_OPERAND_ACTION_WRITE;
						}
						else if (op == 0x62)
						{
							parseOperandGv(instruction, &instruction->operands[0]);
							parseModrmUpper32(instruction, &instruction->operands[1]);
							instruction->operands[0].action = instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
						}
						else if (op == 0x63)
						{
							if (instruction->mode == NMD_X86_MODE_64)
							{
								parseOperandGv(instruction, &instruction->operands[0]);
								parseOperandEv(instruction, &instruction->operands[1]);
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
									parseModrmUpper32(instruction, &instruction->operands[0]);

								instruction->operands[1].type = NMD_X86_OPERAND_TYPE_REGISTER;
								instruction->operands[1].fields.reg = NMD_X86_REG_AX + instruction->modrm.fields.reg;
								instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READ_WRITE;
								instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
							}
						}
						else if (op == 0x69 || op == 0x6b)
						{
							parseOperandGv(instruction, &instruction->operands[0]);
							parseOperandEv(instruction, &instruction->operands[1]);
							instruction->operands[2].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
							instruction->operands[2].fields.imm = (int64_t)(instruction->immediate);
							instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
							instruction->operands[1].action = instruction->operands[2].action = NMD_X86_OPERAND_ACTION_READ;
						}
						else if (op == 0x8c)
						{
							parseOperandEv(instruction, &instruction->operands[0]);
							instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
							instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
							instruction->operands[1].type = NMD_X86_OPERAND_TYPE_REGISTER;
							instruction->operands[1].fields.reg = NMD_X86_REG_ES + instruction->modrm.fields.reg;
						}
						else if (op == 0x8d)
						{
							parseOperandGv(instruction, &instruction->operands[0]);
							instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
							instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
							parseModrmUpper32(instruction, &instruction->operands[1]);
						}
						else if (op == 0x8e)
						{
							instruction->operands[0].type = NMD_X86_OPERAND_TYPE_REGISTER;
							instruction->operands[0].fields.reg = NMD_X86_REG_ES + instruction->modrm.fields.reg;
							instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
							instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
							parseOperandEw(instruction, &instruction->operands[1]);
						}
						else if (op == 0x8f)
						{
							parseOperandEv(instruction, &instruction->operands[0]);
							instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						}
						else if (op >= 0x91 && op <= 0x97)
						{
							parseOperandGv(instruction, &instruction->operands[0]);
							instruction->operands[0].fields.reg = instruction->operands[0].fields.reg + NMD_C(op);
							instruction->operands[1].type = NMD_X86_OPERAND_TYPE_REGISTER;
							instruction->operands[1].fields.reg = instruction->flags.fields.operandSize64 ? NMD_X86_REG_RAX : (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && instruction->mode != NMD_X86_MODE_16 ? NMD_X86_REG_AX : NMD_X86_REG_EAX);
							instruction->operands[0].action = instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ_WRITE;
						}
						else if (op >= 0xa0 && op <= 0xa3)
						{
							instruction->operands[op < 0xa2 ? 0 : 1].type = NMD_X86_OPERAND_TYPE_REGISTER;
							instruction->operands[op < 0xa2 ? 0 : 1].fields.reg = op % 2 == 0 ? NMD_X86_REG_AL : (instruction->flags.fields.operandSize64 ? NMD_X86_REG_RAX : ((instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && instruction->mode != NMD_X86_MODE_16) || (instruction->mode == NMD_X86_MODE_16 && !(instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)) ? NMD_X86_REG_AX : NMD_X86_REG_EAX));
							instruction->operands[op < 0xa2 ? 1 : 0].type = NMD_X86_OPERAND_TYPE_MEMORY;
							instruction->operands[op < 0xa2 ? 1 : 0].fields.mem.disp = (instruction->mode == NMD_X86_MODE_64) ? *(uint64_t*)(b + 1) : *(uint32_t*)(b + 1);
							parseOperandSegmentRegister(instruction, &instruction->operands[op < 0xa2 ? 1 : 0]);
							instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
							instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
						}
						else if (op == 0xa8 || op == 0xa9)
						{
							instruction->operands[0].type = NMD_X86_OPERAND_TYPE_REGISTER;
							instruction->operands[0].fields.reg = op == 0xa8 ? NMD_X86_REG_AL : (instruction->flags.fields.operandSize64 ? NMD_X86_REG_RAX : ((instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE && instruction->mode != NMD_X86_MODE_16) || (instruction->mode == NMD_X86_MODE_16 && !(instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)) ? NMD_X86_REG_AX : NMD_X86_REG_EAX));
							instruction->operands[1].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
						}
						else if (NMD_R(op) == 0xb)
						{
							instruction->operands[0].type = NMD_X86_OPERAND_TYPE_REGISTER;
							instruction->operands[0].fields.reg = ((op < 0xb8 ? (instruction->prefixes & NMD_X86_PREFIXES_REX_B ? NMD_X86_REG_R8B : NMD_X86_REG_AL) : (instruction->prefixes & NMD_X86_PREFIXES_REX_W ? (instruction->prefixes & NMD_X86_PREFIXES_REX_B ? NMD_X86_REG_R8 : NMD_X86_REG_RAX) : (instruction->prefixes & NMD_X86_PREFIXES_REX_B ? NMD_X86_REG_R8D : NMD_X86_REG_EAX))) + op % 8);
							instruction->operands[1].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
							instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
						}
						else if (op == 0xc0 || op == 0xc1 || op == 0xc6 || op == 0xc7)
						{
							if (!(op >= 0xc6 && instruction->modrm.fields.reg))
							{
								if (op % 2 == 0)
									parseOperandEb(instruction, &instruction->operands[0]);
								else
									parseOperandEv(instruction, &instruction->operands[0]);
							}
							instruction->operands[op >= 0xc6 && instruction->modrm.fields.reg ? 0 : 1].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
							instruction->operands[0].action = op <= 0xc1 ? NMD_X86_OPERAND_ACTION_READ_WRITE : NMD_X86_OPERAND_ACTION_WRITE;
						}
						else if (op == 0xc4 || op == 0xc5)
						{
							instruction->operands[0].type = NMD_X86_OPERAND_TYPE_REGISTER;
							instruction->operands[0].fields.reg = ((instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_REG_AX : NMD_X86_REG_EAX) + instruction->modrm.fields.reg);
							parseModrmUpper32(instruction, &instruction->operands[1]);
							instruction->operands[0].action = NMD_X86_OPERAND_ACTION_WRITE;
							instruction->operands[1].action = NMD_X86_OPERAND_ACTION_READ;
						}
						else if (op == 0xc8)
						{
							instruction->operands[0].type = instruction->operands[1].type = NMD_X86_OPERAND_TYPE_IMMEDIATE;
							instruction->operands[0].size = 2;
							instruction->operands[0].fields.imm = *(uint16_t*)(b + 1);
							instruction->operands[1].size = 1;
							instruction->operands[1].fields.imm = b[3];
						}
						else if (op >= 0xd0 && op <= 0xd3)
						{
							if (op % 2 == 0)
								parseOperandEb(instruction, &instruction->operands[0]);
							else
								parseOperandEv(instruction, &instruction->operands[0]);

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
							instruction->operands[0].action = NMD_X86_OPERAND_ACTION_READ_WRITE;
						}
						else if (op >= 0xd8 && op <= 0xdf)
						{
							if (instruction->modrm.fields.mod != 0b11 ||
								op == 0xd8 ||
								(op == 0xd9 && NMD_C(instruction->modrm.modrm) == 0xc) ||
								(op == 0xda && NMD_C(instruction->modrm.modrm) <= 0xd) ||
								(op == 0xdb && (NMD_C(instruction->modrm.modrm) <= 0xd || instruction->modrm.modrm >= 0xe8)) ||
								op == 0xdc ||
								op == 0xdd ||
								(op == 0xde && instruction->modrm.modrm != 0xd9) ||
								(op == 0xdf && instruction->modrm.modrm != 0xe0))
							{
								instruction->operands[0].type = instruction->operands[1].type = NMD_X86_OPERAND_TYPE_REGISTER;
								instruction->operands[0].isImplicit = true;
								instruction->operands[0].fields.reg = NMD_X86_REG_ST0;
								instruction->operands[1].fields.reg = NMD_X86_REG_ST0 + instruction->modrm.fields.reg;
							}
						}
						else if (NMD_R(op) == 0xe)
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
									instruction->operands[0].type = NMD_C(op) < 6 ? NMD_X86_OPERAND_TYPE_REGISTER : NMD_X86_OPERAND_TYPE_IMMEDIATE;
									instruction->operands[1].type = NMD_C(op) < 6 ? NMD_X86_OPERAND_TYPE_IMMEDIATE : NMD_X86_OPERAND_TYPE_REGISTER;
									instruction->operands[0].fields.imm = instruction->operands[1].fields.imm = (int64_t)(instruction->immediate);
								}
								else
								{
									instruction->operands[0].type = instruction->operands[1].type = NMD_X86_OPERAND_TYPE_REGISTER;
									instruction->operands[0].fields.reg = instruction->operands[0].fields.reg = NMD_X86_REG_DX;
								}

								if (op % 2 == 0)
									instruction->operands[op % 8 == 4 ? 0 : 1].fields.reg = NMD_X86_REG_AL;
								else
									instruction->operands[op % 8 == 5 ? 0 : 1].fields.reg = ((instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE ? NMD_X86_REG_AX : NMD_X86_REG_EAX) + instruction->modrm.fields.reg);

								instruction->operands[op % 8 <= 5 ? 0 : 1].action = NMD_X86_OPERAND_ACTION_WRITE;
								instruction->operands[op % 8 <= 5 ? 1 : 0].action = NMD_X86_OPERAND_ACTION_READ;
							}
						}
						else if (op == 0xf6 || op == 0xfe)
						{
							parseOperandEb(instruction, &instruction->operands[0]);
							instruction->operands[0].action = op == 0xfe && instruction->modrm.fields.reg >= 0b010 ? NMD_X86_OPERAND_ACTION_READ : NMD_X86_OPERAND_ACTION_READ_WRITE;
						}
						else if (op == 0xf7 || op == 0xff)
						{
							parseOperandEv(instruction, &instruction->operands[0]);
							instruction->operands[0].action = op == 0xff && instruction->modrm.fields.reg >= 0b010 ? NMD_X86_OPERAND_ACTION_READ : NMD_X86_OPERAND_ACTION_READ_WRITE;
						}
					}
				}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_OPERANDS */
			}
	}

	if (instruction->prefixes & NMD_X86_PREFIXES_LOCK)
	{
		const uint8_t twoOpcodes[] = { 0xb0, 0xb1, 0xb3, 0xbb, 0xc0, 0xc1 };
		if (!(instruction->flags.fields.hasModrm && instruction->modrm.fields.mod != 0b11 &&
			((instruction->opcodeSize == 1 && (op == 0x86 || op == 0x87 || (NMD_R(op) < 4 && (op % 8) < 2 && op < 0x38) || ((op >= 0x80 && op <= 0x83) && instruction->modrm.fields.reg != 0b111) || (op >= 0xfe && instruction->modrm.fields.reg < 2) || ((op == 0xf6 || op == 0xf7) && (instruction->modrm.fields.reg == 0b010 || instruction->modrm.fields.reg == 0b011)))) ||
				(instruction->opcodeSize == 2 && (nmd_findByte(twoOpcodes, sizeof(twoOpcodes), op) || op == 0xab || (op == 0xba && instruction->modrm.fields.reg != 0b100) || (op == 0xc7 && instruction->modrm.fields.reg == 0b001))))))
			return false;
	}

	instruction->length = (uint8_t)((ptrdiff_t)(++b + (size_t)instruction->immMask) - (ptrdiff_t)(buffer));
	for (i = 0; i < instruction->length; i++)
		instruction->fullInstruction[i] = ((const uint8_t*)(buffer))[i];

	for (i = 0; i < (size_t)instruction->immMask; i++)
		((uint8_t*)(&instruction->immediate))[i] = b[i];

#ifndef NMD_ASSEMBLY_DISABLE_DECODER_OPERANDS
	for (i = 0; i < instruction->numOperands; i++)
	{
		if (instruction->operands[i].type == NMD_X86_OPERAND_TYPE_IMMEDIATE)
		{
			if (instruction->operands[i].action == NMD_X86_OPERAND_ACTION_NONE)
				instruction->operands[i].action = NMD_X86_OPERAND_ACTION_READ;

			if (instruction->operands[i].size == 0)
			{
				instruction->operands[i].size = (uint8_t)instruction->immMask;
				instruction->operands[i].fields.imm = instruction->immediate;
			}
		}
	}
#endif /* NMD_ASSEMBLY_DISABLE_DECODER_OPERANDS */

	instruction->flags.fields.valid = true;

	return true;
}