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

void copyByMode(void* dst, void* src, NMD_X86_MODE mode)
{
	if (mode == NMD_X86_MODE_32)
		*(int32_t*)(dst) = *(int32_t*)(src);
	else if (mode == NMD_X86_MODE_64)
		*(int64_t*)(dst) = *(int64_t*)(src);
	else /* (mode == NMD_X86_MODE_16) */
		*(int16_t*)(dst) = *(int16_t*)(src);
}

#define NMD_GET_GREG(index) (&cpu->rax + (index)) /* general register */
#define NMD_GET_RREG(index) (&cpu->r8 + (index)) /* r8,r9...r15 */
#define NMD_GET_PHYSICAL_ADDRESS(address) (uint8_t*)((uint64_t)(cpu->physicalMemory)+((address)-cpu->virtualAddress))
#define NMD_IN_BOUNDARIES(address) (address >= cpu->physicalMemory && address < endPhysicalMemory)
/* #define NMD_TEST(value, bit) ((value&(1<<bit))==(1<<bit)) */

/*
Emulates x86 code according to the cpu's state. You MUST initialize the following variables before calling this
function: 'cpu->mode', 'cpu->physicalMemory', 'cpu->physicalMemorySize', 'cpu->virtualAddress' and 'cpu->rip'.
You may optionally initialize 'cpu->rsp' if a stack is desirable. Below is a short description of each variable:
 - 'cpu->mode': The emulator's operating architecture mode. 'NMD_X86_MODE_32', 'NMD_X86_MODE_64' or 'NMD_X86_MODE_16'.
 - 'cpu->physicalMemory': A pointer to a buffer used as the emulator's memory.
 - 'cpu->physicalMemorySize': The size of the buffer pointer by 'physicalMemory' in bytes.
 - 'cpu->virtualAddress': The starting address of the emulator's virtual address space.
 - 'cpu->rip': The virtual address where emulation starts.
 - 'cpu->rsp': The virtual address of the bottom of the stack.
Parameters:
 - cpu      [in] A pointer to a variable of type 'NMD_X86Cpu' that holds the state of the cpu.
 - maxCount [in] The maximum number of instructions that can be executed, or zero for unlimited instructions.
*/
bool nmd_x86_emulate(NMD_X86Cpu* cpu, size_t maxCount)
{
	const uint64_t endVirtualAddress = cpu->virtualAddress + cpu->physicalMemorySize;
	const void* endPhysicalMemory = (uint8_t*)cpu->physicalMemory + cpu->physicalMemorySize;
	size_t count = 0;

	cpu->running = true;

	while (cpu->running)
	{
		NMD_X86Instruction instruction;
		const void* buffer = NMD_GET_PHYSICAL_ADDRESS(cpu->rip);
		const bool validBuffer = NMD_IN_BOUNDARIES(buffer);
		if (!validBuffer || !nmd_x86_decode_buffer(buffer, endVirtualAddress - cpu->rip, &instruction, (NMD_X86_MODE)cpu->mode, NMD_X86_DECODER_FLAGS_MINIMAL))
		{
			if (cpu->callback)
				cpu->callback(cpu, &instruction, validBuffer ? NMD_X86_EMULATOR_EXCEPTION_BAD_INSTRUCTION : NMD_X86_EMULATOR_EXCEPTION_BAD_MEMORY);
			return false;
		}

		if (instruction.opcodeMap == NMD_X86_OPCODE_MAP_DEFAULT)
		{
			if (instruction.opcode >= 0x88 && instruction.opcode <= 0x8b) /* mov [88,8b] */
			{
				NMD_X86Register* r0 = NMD_GET_GREG(instruction.modrm.fields.reg);
				void* addr;
				if (instruction.modrm.fields.mod == 0b11)
					addr = NMD_GET_GREG(instruction.modrm.fields.rm);
				else
				{
					int64_t x = NMD_GET_GREG(instruction.modrm.fields.rm)->l64 + instruction.displacement;
					addr = NMD_GET_PHYSICAL_ADDRESS(x);
					if (!NMD_IN_BOUNDARIES(addr))
					{
						if (cpu->callback)
							cpu->callback(cpu, &instruction, NMD_X86_EMULATOR_EXCEPTION_BAD_MEMORY);
						return false;
					}
				}

				if (instruction.opcode == 0x88)
					*(int8_t*)(addr) = r0->l8;
				else if (instruction.opcode == 0x89)
				{
					copyByMode(addr, r0, (NMD_X86_MODE)cpu->mode);
				}
				else if (instruction.opcode == 0x8a)
					r0->l8 = *(int8_t*)(addr);
				else /* if (instruction.opcode == 0x8b) */
				{
					copyByMode(r0, addr, (NMD_X86_MODE)cpu->mode);
				}
			}
			else if (NMD_R(instruction.opcode) == 5) /* push,pop [50,5f] */
			{
				NMD_X86Register* r0 = NMD_GET_GREG(instruction.opcode % 8);
				void* dst, * src;

				if (instruction.opcode < 0x58) /* push */
				{
					cpu->rsp.l64 -= (int8_t)cpu->mode;
					dst = NMD_GET_PHYSICAL_ADDRESS(cpu->rsp.l64);
					src = r0;
				}
				else /* pop */
				{
					src = NMD_GET_PHYSICAL_ADDRESS(cpu->rsp.l64);
					cpu->rsp.l64 += (int8_t)cpu->mode;
					dst = r0;
				}

				copyByMode(dst, src, (NMD_X86_MODE)cpu->mode);
			}
			else if (instruction.opcode == 0xe8) /* call */
			{
				/* push the instruction pointer onto the stack. */
				cpu->rsp.l64 -= (int8_t)cpu->mode;
				copyByMode(NMD_GET_PHYSICAL_ADDRESS(cpu->rsp.l64), &cpu->rip, (NMD_X86_MODE)cpu->mode);

				/* jump */
				cpu->rip += (int32_t)instruction.immediate;
			}
			else if (instruction.opcode == 0xc3) /* ret */
			{
				/* pop rip */
				copyByMode(&cpu->rip, NMD_GET_PHYSICAL_ADDRESS(cpu->rsp.l64), (NMD_X86_MODE)cpu->mode);
				cpu->rsp.l64 += (int8_t)cpu->mode;
			}
			else if (instruction.opcode == 0xc2) /* ret imm8 */
			{
				/* pop rip */
				copyByMode(&cpu->rip, NMD_GET_PHYSICAL_ADDRESS(cpu->rsp.l64), (NMD_X86_MODE)cpu->mode);
				cpu->rsp.l64 += (int8_t)(cpu->mode + instruction.immediate);
			}
			else if (instruction.opcode == 0x8d) /* lea */
			{
				NMD_X86Register* r0 = NMD_GET_GREG(instruction.modrm.fields.reg);
				r0->l64 = 0;
				/* compute... */
			}
			else if (instruction.opcode == 0xe9) /* jmp r32 */
				cpu->rip += (int32_t)instruction.immediate;
			else if (instruction.opcode == 0xeb) /* jmp r8 */
				cpu->rip += (int8_t)instruction.immediate;
			else if (instruction.opcode == 0x03) /* add reg, mem/reg */
				NMD_GET_GREG(instruction.modrm.fields.reg)->l32 += NMD_GET_GREG(instruction.modrm.fields.rm)->l32;
			else if (instruction.opcode == 0x04) /* add al, imm8 */
				cpu->rax.l8 += (int8_t)instruction.immediate;
			else if (instruction.opcode == 0x05) /* add ax/eax/rax, imm16/imm32/imm32*/
				cpu->rax.l32 += (int32_t)instruction.immediate;
			else if (NMD_R(instruction.opcode) == 4) /* inc/dec [40,4f] */
			{
				NMD_X86Register* r0 = NMD_GET_GREG(instruction.opcode % 8);
				instruction.opcode < 0x48 ? r0->l64++ : r0->l64--;
			}
			else if (NMD_R(instruction.opcode) == 7 && check_jump_condition(cpu, NMD_C(instruction.opcode))) /* conditional jump r8 */
				cpu->rip += (int8_t)(instruction.immediate);
			else if (instruction.opcode >= 0x91 && instruction.opcode <= 0x97) /* xchg rax, ... */
			{
				const NMD_X86Register tmp = cpu->rax;
				NMD_X86Register* r0 = NMD_GET_GREG(instruction.opcode - 0x91);
				cpu->rax = *r0;
				*r0 = tmp;
			}
			else if (instruction.opcode == 0x60) /* pusha,pushad */
			{
				void* stack = NMD_GET_PHYSICAL_ADDRESS(cpu->rsp.l32);
				cpu->rsp.l32 -= cpu->mode * 8;
				if (instruction.mode == NMD_X86_MODE_32) /* pushad */
				{
					((uint32_t*)(stack))[0] = cpu->rax.l32;
					((uint32_t*)(stack))[1] = cpu->rcx.l32;
					((uint32_t*)(stack))[2] = cpu->rdx.l32;
					((uint32_t*)(stack))[3] = cpu->rbx.l32;
					((uint32_t*)(stack))[4] = cpu->rsp.l32;
					((uint32_t*)(stack))[5] = cpu->rbp.l32;
					((uint32_t*)(stack))[6] = cpu->rsi.l32;
					((uint32_t*)(stack))[7] = cpu->rdi.l32;
				}
				else /* if (instruction.mode == NMD_X86_MODE_16) pusha */
				{
					((uint16_t*)(stack))[0] = cpu->rax.l16;
					((uint16_t*)(stack))[1] = cpu->rcx.l16;
					((uint16_t*)(stack))[2] = cpu->rdx.l16;
					((uint16_t*)(stack))[3] = cpu->rbx.l16;
					((uint16_t*)(stack))[4] = cpu->rsp.l16;
					((uint16_t*)(stack))[5] = cpu->rbp.l16;
					((uint16_t*)(stack))[6] = cpu->rsi.l16;
					((uint16_t*)(stack))[7] = cpu->rdi.l16;
				}
			}
			else if (instruction.opcode == 0x61) /* popa,popad */
			{
				void* stack = NMD_GET_PHYSICAL_ADDRESS(cpu->rsp.l32);
				if (instruction.mode == NMD_X86_MODE_32) /* popad */
				{
					cpu->rax.l32 = ((uint32_t*)(stack))[0];
					cpu->rcx.l32 = ((uint32_t*)(stack))[1];
					cpu->rdx.l32 = ((uint32_t*)(stack))[2];
					cpu->rbx.l32 = ((uint32_t*)(stack))[3];
					cpu->rsp.l32 = ((uint32_t*)(stack))[4];
					cpu->rbp.l32 = ((uint32_t*)(stack))[5];
					cpu->rsi.l32 = ((uint32_t*)(stack))[6];
					cpu->rdi.l32 = ((uint32_t*)(stack))[7];
				}
				else /* if (instruction.mode == NMD_X86_MODE_16) popa */
				{
					cpu->rax.l16 = ((uint16_t*)(stack))[0];
					cpu->rcx.l16 = ((uint16_t*)(stack))[1];
					cpu->rdx.l16 = ((uint16_t*)(stack))[2];
					cpu->rbx.l16 = ((uint16_t*)(stack))[3];
					cpu->rsp.l16 = ((uint16_t*)(stack))[4];
					cpu->rbp.l16 = ((uint16_t*)(stack))[5];
					cpu->rsi.l16 = ((uint16_t*)(stack))[6];
					cpu->rdi.l16 = ((uint16_t*)(stack))[7];
				}
				cpu->rsp.l32 += cpu->mode * 8;
			}
			else if (NMD_R(instruction.opcode) == 0xb) /* mov reg, imm */
			{
				const uint8_t width = (instruction.prefixes & NMD_X86_PREFIXES_REX_W && instruction.opcode >= 0xb8) ? 8 : instruction.mode;
				NMD_X86Register* r0 = instruction.prefixes & NMD_X86_PREFIXES_REX_B ? NMD_GET_RREG(NMD_C(instruction.opcode)) : NMD_GET_GREG(NMD_C(instruction.opcode));
				copyByMode(r0, &instruction.immediate, (NMD_X86_MODE)width);
			}
			else if (instruction.opcode == 0x90)
			{
				if (instruction.simdPrefix == NMD_X86_PREFIXES_REPEAT) /* pause */
				{
					/* spin-wait loop ahead? */
				}
			}
			else if (instruction.opcode == 0x9e) /* sahf */
				cpu->flags.l8 = cpu->rax.l8;
			else if (instruction.opcode == 0x9f) /* lahf */
				cpu->rax.l8 = cpu->flags.l8;
			else if (instruction.opcode == 0xcc) /* int3 */
			{
				if (cpu->callback)
					cpu->callback(cpu, &instruction, NMD_X86_EMULATOR_EXCEPTION_BREAKPOINT);
			}
			else if (instruction.opcode == 0xf1) /* int1 */
			{
				if (cpu->callback)
					cpu->callback(cpu, &instruction, NMD_X86_EMULATOR_EXCEPTION_DEBUG);
			}
			else if (instruction.opcode == 0xce) /* into */
			{
				if (cpu->callback)
					cpu->callback(cpu, &instruction, NMD_X86_EMULATOR_EXCEPTION_OVERFLOW);
			}
			else if (instruction.opcode == 0xcd) /* int n */
			{
				if (cpu->callback)
					cpu->callback(cpu, &instruction, NMD_X86_EMULATOR_EXCEPTION_GENERAL_PROTECTION);
			}
			else if (instruction.opcode == 0xf4) /* hlt */
				cpu->running = false;
			else if (instruction.opcode == 0xf5) /* cmc */
				cpu->flags.fields.CF = ~cpu->flags.fields.CF;
			else if(instruction.opcode == 0xf8) /* clc */
				cpu->flags.fields.CF = 0;
			else if (instruction.opcode == 0xf9) /* stc */
				cpu->flags.fields.CF = 1;
			else if (instruction.opcode == 0xfa) /* cli */
				cpu->flags.fields.IF = 0;
			else if (instruction.opcode == 0xfb) /* sti */
				cpu->flags.fields.IF = 1;
			else if (instruction.opcode == 0xfc) /* cld */
				cpu->flags.fields.DF = 0;
			else if (instruction.opcode == 0xfd) /* std */
				cpu->flags.fields.DF = 1;
		}
		else if (instruction.opcodeMap == NMD_X86_OPCODE_MAP_0F)
		{
			if (NMD_R(instruction.opcode) == 8 && check_jump_condition(cpu, NMD_C(instruction.opcode))) /* conditional jump r32 */
				cpu->rip += (int32_t)(instruction.immediate);
		}
		else if (instruction.opcodeMap == NMD_X86_OPCODE_MAP_0F38)
		{

		}
		else /* if (instruction.opcodeMap == NMD_X86_OPCODE_MAP_0F_38) */
		{

		}

		/*
		if (r0)
		{
			cpu->flags.fields.ZF = (r0->l64 == 0);
			cpu->flags.fields.PF = isParityEven8(r0->l8);
			
		}
		*//* OF,SF,CF*/


		if (cpu->flags.fields.TF && cpu->callback)
			cpu->callback(cpu, &instruction, NMD_X86_EMULATOR_EXCEPTION_STEP);

		cpu->rip += instruction.length;

		if (maxCount > 0 && ++count >= maxCount)
			return true;
	}

	cpu->running = false;

	return true;
}