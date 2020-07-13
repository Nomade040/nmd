#include "nmd_common.h"

bool check_jump_condition(NMD_X86Cpu* const cpu, uint8_t opcodeCondition)
{
	switch (opcodeCondition)
	{
	case 0x0: return cpu->flags.fields.OF == 1;                                                           /* Jump if overflow (OF=1) */
	case 0x1: return cpu->flags.fields.OF == 0;                                                           /* Jump if not overflow (OF=0) */
	case 0x2: return cpu->flags.fields.CF == 1;                                                           /* Jump if not above or equal (CF=1) */
	case 0x3: return cpu->flags.fields.CF == 0;                                                           /* Jump if not below (CF=0) */
	case 0x4: return cpu->flags.fields.ZF == 1;                                                           /* Jump if equal (ZF=1) */
	case 0x5: return cpu->flags.fields.ZF == 0;                                                           /* Jump if not equal (ZF=0) */
	case 0x6: return cpu->flags.fields.CF == 1 || cpu->flags.fields.ZF == 1;                              /* Jump if not above (CF=1 or ZF=1) */
	case 0x7: return cpu->flags.fields.CF == 0 && cpu->flags.fields.ZF == 0;                              /* Jump if not below or equal (CF=0 and ZF=0) */
	case 0x8: return cpu->flags.fields.SF == 1;                                                           /* Jump if sign (SF=1) */
	case 0x9: return cpu->flags.fields.SF == 0;                                                           /* Jump if not sign (SF=0) */
	case 0xa: return cpu->flags.fields.PF == 1;                                                           /* Jump if parity/parity even (PF=1) */
	case 0xb: return cpu->flags.fields.PF == 0;                                                           /* Jump if parity odd (PF=0) */
	case 0xc: return cpu->flags.fields.SF != cpu->flags.fields.OF;                                        /* Jump if not greater or equal (SF != OF) */
	case 0xd: return cpu->flags.fields.SF == cpu->flags.fields.OF;                                        /* Jump if not less (SF=OF) */
	case 0xe: return cpu->flags.fields.ZF == 1 || cpu->flags.fields.SF != cpu->flags.fields.OF;           /* Jump if not greater (ZF=1 or SF != OF) */
	case 0xf: return cpu->flags.fields.ZF == 0 && cpu->flags.fields.SF == cpu->flags.fields.OF;           /* Jump if not less or equal (ZF=0 and SF=OF) */
	default: return false;
	}
}

/* 
Checks if the number of set bits in an 8-bit number is even.
Credits: https://stackoverflow.com/questions/21617970/how-to-check-if-value-has-even-parity-of-bits-or-odd
*/
bool isParityEven8(uint8_t x)
{
	x ^= x >> 4;
	x ^= x >> 2;
	x ^= x >> 1;
	return !(x & 1);
}

void* getRealAddress(const NMD_X86Cpu* const cpu, uint64_t address)
{
	return (uint8_t*)cpu->memoryBlock + (address - cpu->address);
}

/*
Emulates x86 code. If you wish to use the stack, you have to change the esp register and set it to the stack's address.
Before calling this function you must fill the following variables of 'NMD_X86Cpu'(cpu):
 - mode: The architecture mode. 'NMD_X86_MODE_32', 'NMD_X86_MODE_64' or 'NMD_X86_MODE_16'.
 - memoryBlock: A pointer to a block of memory. You may use a buffer on the stack, on the heap or wherever you want.
 - memoryBlockSize: The size of the memory block in bytes.
 - address: The base address of the memory block. This address can be any value.
Parameters:
  cpu        [in] A pointer to a variable of type 'NMD_X86CpuState' that holds the cpu's state.
  entryPoint [in] The address where emulation starts.
  maxCount   [in] The maximum number of instruction to be executed, or 0(zero) for unlimited instructions.
*/
bool nmd_x86_emulate(NMD_X86Cpu* const cpu, const uint64_t entryPoint, const size_t maxCount)
{
	if (cpu->mode == NMD_X86_MODE_NONE)
		return false;

	const uintptr_t endAddress = cpu->address + cpu->memoryBlockSize;
	if (!(entryPoint >= cpu->address && entryPoint <= endAddress))
		return false;

	cpu->rip = entryPoint;
	size_t count = 0;

	cpu->running = true;

	while (cpu->rip < endAddress && cpu->running)
	{
		NMD_X86Register* r0 = 0; /* first operand(register) */
		NMD_X86Register* r1 = 0; /* second operand(register) */

		NMD_X86Instruction instruction;
		const uintptr_t relativeAddress = (cpu->rip - cpu->address);
		if (!nmd_x86_decode_buffer((uint8_t*)cpu->memoryBlock + relativeAddress, endAddress - relativeAddress, &instruction, cpu->mode, NMD_X86_DECODER_FLAGS_MINIMAL))
			break;

		cpu->rip += instruction.length;

		if (instruction.opcodeMap == NMD_X86_OPCODE_MAP_DEFAULT)
		{
			if (instruction.opcode == 0xe9) /* jmp r32 */
				cpu->rip += (int32_t)instruction.immediate;
			else if (instruction.opcode == 0xeb) /* jmp r8 */
				cpu->rip += (int8_t)instruction.immediate;
			else if (instruction.opcode == 0x03)
			{
				r0 = &cpu->rax + instruction.modrm.fields.reg;
				r1 = &cpu->rax + instruction.modrm.fields.rm;

				r0->l32 += r1->l32;
			}
			else if (instruction.opcode == 0x04) /* add al, imm8 */
				cpu->rax.l8 += (int8_t)instruction.immediate;
			else if (instruction.opcode == 0x05) /* add ax/eax/rax, imm16/imm32/imm32*/
				cpu->rax.l32 += (int32_t)instruction.immediate;
			else if (NMD_R(instruction.opcode) == 4) /* inc/dec [40,4f] */
			{
				r0 = &cpu->rax + (instruction.opcode % 8);
				if (instruction.opcode >= 0x48)
					r0->l64--;
				else
					r0->l64++;
			}
			else if (NMD_R(instruction.opcode) == 5) /* push,pop [50,5f] */
			{
				r0 = &cpu->rax + (instruction.opcode % 8);
				cpu->rsp.l64 -= (int)cpu->mode;
				void* addr = getRealAddress(cpu, cpu->rsp.l64);
				if (instruction.opcode >= 0x58)
					r0->l64--;
				else
					r0->l64++;
			}
			else if (NMD_R(instruction.opcode) == 7 && check_jump_condition(cpu, NMD_C(instruction.opcode))) /* conditional jump r8 */
				cpu->rip += (int8_t)(instruction.immediate);
			else if (instruction.opcode >= 0x91 && instruction.opcode <= 0x97) /* xchg rax, ... */
			{
				NMD_X86Register tmp = cpu->rax;
				r0 = &cpu->rax + (instruction.opcode - 0x91);
				cpu->rax = *r0;
				*r0 = tmp;
				r0 = 0; /* Prevent flag modification */
			}
			else if (instruction.opcode == 0xf4) /* HLT */
				break;
		}
		else if (instruction.opcodeMap == NMD_X86_OPCODE_MAP_0F)
		{
			if (NMD_R(instruction.opcode) == 8 && check_jump_condition(cpu, NMD_C(instruction.opcode))) /* conditional jump r32 */
				cpu->rip += (int32_t)(instruction.immediate);
		}
		else if (instruction.opcodeMap == NMD_X86_OPCODE_MAP_0F_38)
		{

		}
		else /* if (instruction.opcodeMap == NMD_X86_OPCODE_MAP_0F_38) */
		{

		}

		if (r0)
		{
			cpu->flags.fields.ZF = (r0->l64 == 0);
			cpu->flags.fields.PF = isParityEven8(r0->l8);
			/* OF,SF,CF*/
		}

		if (maxCount > 0 && ++count >= maxCount)
			return true;
	}

	cpu->running = false;

	return true;
}