#include "nmd_common.h"

#define NMD_EMULATOR_RESOLVE_VA(va) ((void*)((uint64_t)cpu.physical_memory + (va - cpu.virtual_address)))

NMD_ASSEMBLY_API bool _nmd_check_jump_condition(nmd_x86_cpu* const cpu, uint8_t opcode_condition)
{
	switch (opcode_condition)
	{
	case 0x0: return cpu->flags.fields.OF == 1;                                                 /* Jump if overflow (OF=1) */
	case 0x1: return cpu->flags.fields.OF == 0;                                                 /* Jump if not overflow (OF=0) */
	case 0x2: return cpu->flags.fields.CF == 1;                                                 /* Jump if not above or equal (CF=1) */
	case 0x3: return cpu->flags.fields.CF == 0;                                                 /* Jump if not below (CF=0) */
	case 0x4: return cpu->flags.fields.ZF == 1;                                                 /* Jump if equal (ZF=1) */
	case 0x5: return cpu->flags.fields.ZF == 0;                                                 /* Jump if not equal (ZF=0) */
	case 0x6: return cpu->flags.fields.CF == 1 || cpu->flags.fields.ZF == 1;                    /* Jump if not above (CF=1 or ZF=1) */
	case 0x7: return cpu->flags.fields.CF == 0 && cpu->flags.fields.ZF == 0;                    /* Jump if not below or equal (CF=0 and ZF=0) */
	case 0x8: return cpu->flags.fields.SF == 1;                                                 /* Jump if sign (SF=1) */
	case 0x9: return cpu->flags.fields.SF == 0;                                                 /* Jump if not sign (SF=0) */
	case 0xa: return cpu->flags.fields.PF == 1;                                                 /* Jump if parity/parity even (PF=1) */
	case 0xb: return cpu->flags.fields.PF == 0;                                                 /* Jump if parity odd (PF=0) */
	case 0xc: return cpu->flags.fields.SF != cpu->flags.fields.OF;                              /* Jump if not greater or equal (SF != OF) */
	case 0xd: return cpu->flags.fields.SF == cpu->flags.fields.OF;                              /* Jump if not less (SF=OF) */
	case 0xe: return cpu->flags.fields.ZF == 1 || cpu->flags.fields.SF != cpu->flags.fields.OF; /* Jump if not greater (ZF=1 or SF != OF) */
	case 0xf: return cpu->flags.fields.ZF == 0 && cpu->flags.fields.SF == cpu->flags.fields.OF; /* Jump if not less or equal (ZF=0 and SF=OF) */
	default: return false;
	}
}

/* 
Checks if the number of set bits in an 8-bit number is even.
Credits: https://stackoverflow.com/questions/21617970/how-to-check-if-value-has-even-parity-of-bits-or-odd
*/
NMD_ASSEMBLY_API bool _nmd_is_parity_even8(uint8_t x)
{
	x ^= x >> 4;
	x ^= x >> 2;
	x ^= x >> 1;
	return !(x & 1);
}

NMD_ASSEMBLY_API void _nmd_copy_by_mode(void* dst, void* src, NMD_X86_MODE mode)
{
	if (mode == NMD_X86_MODE_32)
		*(int32_t*)(dst) = *(int32_t*)(src);
	else if (mode == NMD_X86_MODE_64)
		*(int64_t*)(dst) = *(int64_t*)(src);
	else /* (mode == NMD_X86_MODE_16) */
		*(int16_t*)(dst) = *(int16_t*)(src);
}

NMD_ASSEMBLY_API void _nmd_copy_by_operand_size(void* dst, void* src, nmd_x86_instruction* instruction)
{
	if(instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
		*(int16_t*)(dst) = *(int16_t*)(src);
	else
		*(int32_t*)(dst) = *(int32_t*)(src);
}

NMD_ASSEMBLY_API void _nmd_add_by_operand_size(void* dst, void* src, nmd_x86_instruction* instruction)
{
	if (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
		*(int16_t*)(dst) += *(int16_t*)(src);
	else
		*(int32_t*)(dst) += *(int32_t*)(src);
}

NMD_ASSEMBLY_API void _nmd_or_by_operand_size(void* dst, void* src, nmd_x86_instruction* instruction)
{
	if (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
		*(int16_t*)(dst) |= *(int16_t*)(src);
	else
		*(int32_t*)(dst) |= *(int32_t*)(src);
} 

NMD_ASSEMBLY_API void _nmd_adc_by_operand_size(void* dst, void* src, nmd_x86_cpu* cpu, nmd_x86_instruction* instruction)
{
	if (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
		*(int16_t*)(dst) += *(int16_t*)(src) + cpu->flags.fields.CF;
	else
		*(int32_t*)(dst) += *(int32_t*)(src) + cpu->flags.fields.CF;
}

NMD_ASSEMBLY_API void _nmd_sbb_by_operand_size(void* dst, void* src, nmd_x86_cpu* cpu, nmd_x86_instruction* instruction)
{
	if (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
		*(int16_t*)(dst) -= *(int16_t*)(src) + cpu->flags.fields.CF;
	else
		*(int32_t*)(dst) -= *(int32_t*)(src) + cpu->flags.fields.CF;
}

NMD_ASSEMBLY_API void _nmd_and_by_operand_size(void* dst, void* src, nmd_x86_instruction* instruction)
{
	if (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
		*(int16_t*)(dst) &= *(int16_t*)(src);
	else
		*(int32_t*)(dst) &= *(int32_t*)(src);
}

NMD_ASSEMBLY_API void _nmd_sub_by_operand_size(void* dst, void* src, nmd_x86_instruction* instruction)
{
	if (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
		*(int16_t*)(dst) -= *(int16_t*)(src);
	else
		*(int32_t*)(dst) -= *(int32_t*)(src);
}

NMD_ASSEMBLY_API void _nmd_xor_by_operand_size(void* dst, void* src, nmd_x86_instruction* instruction)
{
	if (instruction->prefixes & NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE)
		*(int16_t*)(dst) &= *(int16_t*)(src);
	else
		*(int32_t*)(dst) &= *(int32_t*)(src);
}

#define _NMD_GET_GREG(index) (&cpu->rax + (index)) /* general register */
#define _NMD_GET_RREG(index) (&cpu->r8 + (index)) /* r8,r9...r15 */
#define _NMD_GET_PHYSICAL_ADDRESS(address) (uint8_t*)((uint64_t)(cpu->physical_memory)+((address)-cpu->virtual_address))
#define _NMD_IN_BOUNDARIES(address) (address >= cpu->physical_memory && address < end_physical_memory)
/* #define NMD_TEST(value, bit) ((value&(1<<bit))==(1<<bit)) */

NMD_ASSEMBLY_API void* _nmd_resolve_memory_operand(nmd_x86_cpu* cpu, nmd_x86_instruction* instruction)
{
	if (instruction->modrm.fields.mod == 0b11)
		return &_NMD_GET_GREG(instruction->modrm.fields.rm)->l64;
	else
	{
		int64_t va_expr; /* virtual address expression */

		if (instruction->has_sib)
			va_expr = _NMD_GET_GREG(instruction->sib.fields.base)->l64 + _NMD_GET_GREG(instruction->sib.fields.index)->l64;
		else
			va_expr = _NMD_GET_GREG(instruction->modrm.fields.rm)->l64;

		va_expr += ((instruction->disp_mask == NMD_X86_DISP8) ? (int8_t)instruction->displacement : (int32_t)instruction->displacement);

		return _NMD_GET_PHYSICAL_ADDRESS(va_expr);
	}
}

NMD_ASSEMBLY_API int64_t _nmd_resolve_memory_operand_va(nmd_x86_cpu* cpu, nmd_x86_instruction* instruction)
{
	int64_t va_expr; /* virtual address expression */

	if (instruction->has_sib)
		va_expr = _NMD_GET_GREG(instruction->sib.fields.base)->l64 + _NMD_GET_GREG(instruction->sib.fields.index)->l64;
	else
		va_expr = _NMD_GET_GREG(instruction->modrm.fields.rm)->l64;

	return va_expr + ((instruction->disp_mask == NMD_X86_DISP8) ? (int8_t)instruction->displacement : (int32_t)instruction->displacement);
}

/*
Emulates x86 code according to the state of the cpu. You MUST initialize the following variables before calling this
function: 'cpu->mode', 'cpu->physical_memory', 'cpu->physical_memory_size', 'cpu->virtual_address' and 'cpu->rip'.
You may optionally initialize 'cpu->rsp' if a stack is desirable. Below is a short description of each variable:
 - 'cpu->mode': The emulator's operating architecture mode. 'NMD_X86_MODE_32', 'NMD_X86_MODE_64' or 'NMD_X86_MODE_16'.
 - 'cpu->physical_memory': A pointer to a buffer used as the emulator's memory.
 - 'cpu->physical_memory_size': The size of the 'physical_memory' buffer in bytes.
 - 'cpu->virtual_address': The starting address of the emulator's virtual address space.
 - 'cpu->rip': The virtual address where emulation starts.
 - 'cpu->rsp': The virtual address of the bottom of the stack.
Parameters:
 - cpu       [in] A pointer to a variable of type 'nmd_x86_cpu' that holds the state of the cpu.
 - max_count [in] The maximum number of instructions that can be executed, or zero for unlimited instructions.
*/
NMD_ASSEMBLY_API bool nmd_x86_emulate(nmd_x86_cpu* cpu, size_t max_count)
{
	const uint64_t end_virtual_address = cpu->virtual_address + cpu->physical_memory_size;
	const void* end_physical_memory = (uint8_t*)cpu->physical_memory + cpu->physical_memory_size;

	cpu->count = 0;
	cpu->running = true;

	while (cpu->running)
	{
		nmd_x86_instruction instruction;
		const void* buffer = _NMD_GET_PHYSICAL_ADDRESS(cpu->rip);
		const bool valid_buffer = _NMD_IN_BOUNDARIES(buffer);
		if (!valid_buffer || !nmd_x86_decode(buffer, (size_t)(end_virtual_address - cpu->rip), &instruction, (NMD_X86_MODE)cpu->mode, NMD_X86_DECODER_FLAGS_MINIMAL))
		{
			if (cpu->callback)
				cpu->callback(cpu, &instruction, valid_buffer ? NMD_X86_EMULATOR_EXCEPTION_BAD_INSTRUCTION : NMD_X86_EMULATOR_EXCEPTION_BAD_MEMORY);
			cpu->running = false;
			return false;
		}

		if (instruction.opcode_map == NMD_X86_OPCODE_MAP_DEFAULT)
		{
			if (instruction.opcode >= 0x88 && instruction.opcode <= 0x8b) /* mov [88,8b] */
			{
				nmd_x86_register* r0 = _NMD_GET_GREG(instruction.modrm.fields.reg);
				void* addr;
				if (instruction.modrm.fields.mod == 0b11)
					addr = _NMD_GET_GREG(instruction.modrm.fields.rm);
				else
				{
					int64_t x = _NMD_GET_GREG(instruction.modrm.fields.rm)->l64 + instruction.displacement;
					addr = _NMD_GET_PHYSICAL_ADDRESS(x);
					if (!_NMD_IN_BOUNDARIES(addr))
					{
						if (cpu->callback)
							cpu->callback(cpu, &instruction, NMD_X86_EMULATOR_EXCEPTION_BAD_MEMORY);
						cpu->running = false;
						return false;
					}
				}

				if (instruction.opcode == 0x88)
					*(int8_t*)(addr) = r0->l8;
				else if (instruction.opcode == 0x89)
				{
					_nmd_copy_by_mode(addr, r0, (NMD_X86_MODE)cpu->mode);
				}
				else if (instruction.opcode == 0x8a)
					r0->l8 = *(int8_t*)(addr);
				else /* if (instruction.opcode == 0x8b) */
				{
					_nmd_copy_by_mode(r0, addr, (NMD_X86_MODE)cpu->mode);
				}
			}
			else if (NMD_R(instruction.opcode) == 5) /* push,pop [50,5f] */
			{
				nmd_x86_register* r0 = _NMD_GET_GREG(instruction.opcode % 8);
				void* dst, * src;

				if (instruction.opcode < 0x58) /* push */
				{
					cpu->rsp.l64 -= (int8_t)cpu->mode;
					dst = _NMD_GET_PHYSICAL_ADDRESS(cpu->rsp.l64);
					src = r0;
				}
				else /* pop */
				{
					src = _NMD_GET_PHYSICAL_ADDRESS(cpu->rsp.l64);
					cpu->rsp.l64 += (int8_t)cpu->mode;
					dst = r0;
				}

				_nmd_copy_by_mode(dst, src, (NMD_X86_MODE)cpu->mode);
			}
			else if (instruction.opcode == 0xe8) /* call */
			{
				/* push the instruction pointer onto the stack. */
				cpu->rsp.l64 -= (int8_t)cpu->mode;
				_nmd_copy_by_mode(_NMD_GET_PHYSICAL_ADDRESS(cpu->rsp.l64), &cpu->rip, (NMD_X86_MODE)cpu->mode);

				/* jump */
				cpu->rip += (int32_t)instruction.immediate;
			}
			else if (instruction.opcode == 0xc3) /* ret */
			{
				/* pop rip */
				_nmd_copy_by_mode(&cpu->rip, _NMD_GET_PHYSICAL_ADDRESS(cpu->rsp.l64), (NMD_X86_MODE)cpu->mode);
				cpu->rsp.l64 += (int8_t)cpu->mode;
			}
			else if (instruction.opcode == 0xc2) /* ret imm8 */
			{
				/* pop rip */
				_nmd_copy_by_mode(&cpu->rip, _NMD_GET_PHYSICAL_ADDRESS(cpu->rsp.l64), (NMD_X86_MODE)cpu->mode);
				cpu->rsp.l64 += (int8_t)(cpu->mode + instruction.immediate);
			}
			else if (instruction.opcode == 0x8d) /* lea */
				_NMD_GET_GREG(instruction.modrm.fields.reg)->l64 = _nmd_resolve_memory_operand_va(cpu, &instruction);
			else if (instruction.opcode == 0xe9) /* jmp r32 */
				cpu->rip += (int32_t)instruction.immediate;
			else if (instruction.opcode == 0xeb) /* jmp r8 */
				cpu->rip += (int8_t)instruction.immediate;

			else if (instruction.opcode == 0x00) /* add Eb, Gb */
				*(int8_t*)_nmd_resolve_memory_operand(cpu, &instruction) += _NMD_GET_GREG(instruction.modrm.fields.reg)->l8;
			else if (instruction.opcode == 0x01) /* add Ev, Gv */
				_nmd_add_by_operand_size(_nmd_resolve_memory_operand(cpu, &instruction), _NMD_GET_GREG(instruction.modrm.fields.reg), &instruction);
			else if (instruction.opcode == 0x02) /* add Gb, Eb */
				_NMD_GET_GREG(instruction.modrm.fields.reg)->l8 += *(int8_t*)_nmd_resolve_memory_operand(cpu, &instruction);
			else if (instruction.opcode == 0x03) /* add Gv, Ev */
				_nmd_add_by_operand_size(_NMD_GET_GREG(instruction.modrm.fields.reg), _nmd_resolve_memory_operand(cpu, &instruction), &instruction);
			else if (instruction.opcode == 0x04) /* add al, bl */
				cpu->rax.l8 += (int8_t)instruction.immediate;
			else if (instruction.opcode == 0x05) /* add rAX, lz */
				_nmd_add_by_operand_size(&cpu->rax, &instruction.immediate, &instruction);

			else if (instruction.opcode == 0x08) /* or Eb, Gb */
				*(int8_t*)_nmd_resolve_memory_operand(cpu, &instruction) |= _NMD_GET_GREG(instruction.modrm.fields.reg)->l8;
			else if (instruction.opcode == 0x09) /* or Ev, Gv */
				_nmd_or_by_operand_size(_nmd_resolve_memory_operand(cpu, &instruction), _NMD_GET_GREG(instruction.modrm.fields.reg), &instruction);
			else if (instruction.opcode == 0x0a) /* or Gb, Eb */
				_NMD_GET_GREG(instruction.modrm.fields.reg)->l8 |= *(int8_t*)_nmd_resolve_memory_operand(cpu, &instruction);
			else if (instruction.opcode == 0x0b) /* or Gv, Ev */
				_nmd_or_by_operand_size(_NMD_GET_GREG(instruction.modrm.fields.reg), _nmd_resolve_memory_operand(cpu, &instruction), &instruction);
			else if (instruction.opcode == 0x0c) /* or al, bl */
				cpu->rax.l8 |= (int8_t)instruction.immediate;
			else if (instruction.opcode == 0x0d) /* or rAX, lz */
				_nmd_or_by_operand_size(&cpu->rax, &instruction.immediate, &instruction);

			else if (instruction.opcode == 0x10) /* adc Eb, Gb */
				*(int8_t*)_nmd_resolve_memory_operand(cpu, &instruction) += _NMD_GET_GREG(instruction.modrm.fields.reg)->l8 + cpu->flags.fields.CF;
			else if (instruction.opcode == 0x11) /* adc Ev, Gv */
				_nmd_adc_by_operand_size(_nmd_resolve_memory_operand(cpu, &instruction), _NMD_GET_GREG(instruction.modrm.fields.reg), cpu, &instruction);
			else if (instruction.opcode == 0x12) /* adc Gb, Eb */
				_NMD_GET_GREG(instruction.modrm.fields.reg)->l8 += *(int8_t*)_nmd_resolve_memory_operand(cpu, &instruction) + cpu->flags.fields.CF;
			else if (instruction.opcode == 0x13) /* adc Gv, Ev */
				_nmd_adc_by_operand_size(_NMD_GET_GREG(instruction.modrm.fields.reg), _nmd_resolve_memory_operand(cpu, &instruction), cpu, &instruction);
			else if (instruction.opcode == 0x14) /* adc al, bl */
				cpu->rax.l8 += (int8_t)instruction.immediate + cpu->flags.fields.CF;
			else if (instruction.opcode == 0x15) /* adc rAX, lz */
				_nmd_adc_by_operand_size(&cpu->rax, &instruction.immediate, cpu, &instruction);

			else if (instruction.opcode == 0x18) /* sbb Eb, Gb */
				*(int8_t*)_nmd_resolve_memory_operand(cpu, &instruction) -= _NMD_GET_GREG(instruction.modrm.fields.reg)->l8 + cpu->flags.fields.CF;
			else if (instruction.opcode == 0x19) /* sbb Ev, Gv */
				_nmd_sbb_by_operand_size(_nmd_resolve_memory_operand(cpu, &instruction), _NMD_GET_GREG(instruction.modrm.fields.reg), cpu, &instruction);
			else if (instruction.opcode == 0x1a) /* sbb Gb, Eb */
				_NMD_GET_GREG(instruction.modrm.fields.reg)->l8 -= *(int8_t*)_nmd_resolve_memory_operand(cpu, &instruction) + cpu->flags.fields.CF;
			else if (instruction.opcode == 0x1b) /* sbb Gv, Ev */
				_nmd_sbb_by_operand_size(_NMD_GET_GREG(instruction.modrm.fields.reg), _nmd_resolve_memory_operand(cpu, &instruction), cpu, &instruction);
			else if (instruction.opcode == 0x1c) /* sbb al, bl */
				cpu->rax.l8 -= (int8_t)instruction.immediate + cpu->flags.fields.CF;
			else if (instruction.opcode == 0x1d) /* sbb rAX, lz */
				_nmd_sbb_by_operand_size(&cpu->rax, &instruction.immediate, cpu, &instruction);

			else if (instruction.opcode == 0x20) /* and Eb, Gb */
				*(int8_t*)_nmd_resolve_memory_operand(cpu, &instruction) &= _NMD_GET_GREG(instruction.modrm.fields.reg)->l8;
			else if (instruction.opcode == 0x21) /* and Ev, Gv */
				_nmd_and_by_operand_size(_nmd_resolve_memory_operand(cpu, &instruction), _NMD_GET_GREG(instruction.modrm.fields.reg), &instruction);
			else if (instruction.opcode == 0x22) /* and Gb, Eb */
				_NMD_GET_GREG(instruction.modrm.fields.reg)->l8 &= *(int8_t*)_nmd_resolve_memory_operand(cpu, &instruction);
			else if (instruction.opcode == 0x23) /* and Gv, Ev */
				_nmd_and_by_operand_size(_NMD_GET_GREG(instruction.modrm.fields.reg), _nmd_resolve_memory_operand(cpu, &instruction), &instruction);
			else if (instruction.opcode == 0x24) /* and al, bl */
				cpu->rax.l8 &= (int8_t)instruction.immediate;
			else if (instruction.opcode == 0x25) /* and rAX, lz */
				_nmd_and_by_operand_size(&cpu->rax, &instruction.immediate, &instruction);

			else if (instruction.opcode == 0x28) /* sub Eb, Gb */
				*(int8_t*)_nmd_resolve_memory_operand(cpu, &instruction) -= _NMD_GET_GREG(instruction.modrm.fields.reg)->l8;
			else if (instruction.opcode == 0x29) /* sub Ev, Gv */
				_nmd_sub_by_operand_size(_nmd_resolve_memory_operand(cpu, &instruction), _NMD_GET_GREG(instruction.modrm.fields.reg), &instruction);
			else if (instruction.opcode == 0x2a) /* sub Gb, Eb */
				_NMD_GET_GREG(instruction.modrm.fields.reg)->l8 -= *(int8_t*)_nmd_resolve_memory_operand(cpu, &instruction);
			else if (instruction.opcode == 0x2b) /* sub Gv, Ev */
				_nmd_sub_by_operand_size(_NMD_GET_GREG(instruction.modrm.fields.reg), _nmd_resolve_memory_operand(cpu, &instruction), &instruction);
			else if (instruction.opcode == 0x2c) /* sub al, bl */
				cpu->rax.l8 -= (int8_t)instruction.immediate;
			else if (instruction.opcode == 0x2d) /* sub rAX, lz */
				_nmd_sub_by_operand_size(&cpu->rax, &instruction.immediate, &instruction);

			else if (instruction.opcode == 0x08) /* xor Eb, Gb */
				*(int8_t*)_nmd_resolve_memory_operand(cpu, &instruction) ^= _NMD_GET_GREG(instruction.modrm.fields.reg)->l8;
			else if (instruction.opcode == 0x09) /* xor Ev, Gv */
				_nmd_xor_by_operand_size(_nmd_resolve_memory_operand(cpu, &instruction), _NMD_GET_GREG(instruction.modrm.fields.reg), &instruction);
			else if (instruction.opcode == 0x0a) /* xor Gb, Eb */
				_NMD_GET_GREG(instruction.modrm.fields.reg)->l8 ^= *(int8_t*)_nmd_resolve_memory_operand(cpu, &instruction);
			else if (instruction.opcode == 0x0b) /* xor Gv, Ev */
				_nmd_xor_by_operand_size(_NMD_GET_GREG(instruction.modrm.fields.reg), _nmd_resolve_memory_operand(cpu, &instruction), &instruction);
			else if (instruction.opcode == 0x0c) /* xor al, bl */
				cpu->rax.l8 ^= (int8_t)instruction.immediate;
			else if (instruction.opcode == 0x0d) /* xor rAX, lz */
				_nmd_xor_by_operand_size(&cpu->rax, &instruction.immediate, &instruction);

			else if (NMD_R(instruction.opcode) == 4) /* inc/dec [40,4f] */
			{
				nmd_x86_register* r0 = _NMD_GET_GREG(instruction.opcode % 8);
				instruction.opcode < 0x48 ? r0->l64++ : r0->l64--;
			}
			else if (NMD_R(instruction.opcode) == 7 && _nmd_check_jump_condition(cpu, NMD_C(instruction.opcode))) /* conditional jump r8 */
				cpu->rip += (int8_t)(instruction.immediate);
			else if (instruction.opcode >= 0x91 && instruction.opcode <= 0x97) /* xchg rax, ... */
			{
				const nmd_x86_register tmp = cpu->rax;
				nmd_x86_register* r0 = _NMD_GET_GREG(instruction.opcode - 0x91);
				cpu->rax = *r0;
				*r0 = tmp;
			}
			else if (instruction.opcode == 0x60) /* pusha,pushad */
			{
				void* stack = _NMD_GET_PHYSICAL_ADDRESS(cpu->rsp.l32);
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
				void* stack = _NMD_GET_PHYSICAL_ADDRESS(cpu->rsp.l32);
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
				nmd_x86_register* r0 = instruction.prefixes & NMD_X86_PREFIXES_REX_B ? _NMD_GET_RREG(NMD_C(instruction.opcode)) : _NMD_GET_GREG(NMD_C(instruction.opcode));
				_nmd_copy_by_mode(r0, &instruction.immediate, (NMD_X86_MODE)width);
			}
			else if (instruction.opcode == 0x90)
			{
				if (instruction.simd_prefix == NMD_X86_PREFIXES_REPEAT) /* pause */
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
			{
				cpu->running = false;
				return true;
			}
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

			/* Push/Pop segment registers */
			else if (instruction.opcode == 0x06) /* push es*/
			{
				cpu->rsp.l64 -= cpu->mode;
				*(uint16_t*)cpu->rsp.l64 = cpu->es;
			}
			else if (instruction.opcode == 0x07) /* pop es */
			{
				cpu->es = *(uint16_t*)cpu->rsp.l64;
				cpu->rsp.l64 += cpu->mode;
			}
			else if (instruction.opcode == 0x16) /* push ss */
			{
				cpu->rsp.l64 -= cpu->mode;
				*(uint16_t*)cpu->rsp.l64 = cpu->ss;
			}
			else if (instruction.opcode == 0x17) /* pop ss */
			{
				cpu->ss = *(uint16_t*)cpu->rsp.l64;
				cpu->rsp.l64 += cpu->mode;
			}
			else if (instruction.opcode == 0x0e) /* push cs */
			{
				cpu->rsp.l64 -= cpu->mode;
				*(uint16_t*)cpu->rsp.l64 = cpu->cs;
			}
			else if (instruction.opcode == 0x1e) /* push ds */
			{
				cpu->rsp.l64 -= cpu->mode;
				*(uint16_t*)cpu->rsp.l64 = cpu->ds;
			}
			else if (instruction.opcode == 0x1f) /* pop ds */
			{
				cpu->ds = *(uint16_t*)cpu->rsp.l64;
				cpu->rsp.l64 += cpu->mode;
			}
		}
		else if (instruction.opcode_map == NMD_X86_OPCODE_MAP_0F)
		{
			if (NMD_R(instruction.opcode) == 8 && _nmd_check_jump_condition(cpu, NMD_C(instruction.opcode))) /* conditional jump r32 */
				cpu->rip += (int32_t)(instruction.immediate);

			else if (instruction.opcode == 0xa0) /* push fs */
			{
				cpu->rsp.l64 -= cpu->mode;
				*(uint16_t*)cpu->rsp.l64 = cpu->fs;
			}
			else if (instruction.opcode == 0xa1) /* pop fs */
			{
				cpu->fs = *(uint16_t*)cpu->rsp.l64;
				cpu->rsp.l64 += cpu->mode;
			}
			else if (instruction.opcode == 0xa8) /* push gs */
			{
				cpu->rsp.l64 -= cpu->mode;
				*(uint16_t*)cpu->rsp.l64 = cpu->gs;
			}
			else if (instruction.opcode == 0xa9) /* pop gs */
			{
				cpu->gs = *(uint16_t*)cpu->rsp.l64;
				cpu->rsp.l64 += cpu->mode;
			}
		}
		else if (instruction.opcode_map == NMD_X86_OPCODE_MAP_0F38)
		{

		}
		else /* if (instruction.opcode_map == NMD_X86_OPCODE_MAP_0F_38) */
		{

		}

		/*
		if (r0)
		{
			cpu->flags.fields.ZF = (r0->l64 == 0);
			cpu->flags.fields.PF = _nmd_is_parity_even8(r0->l8);
			
		}
		*//* OF,SF,CF*/


		if (cpu->flags.fields.TF && cpu->callback)
			cpu->callback(cpu, &instruction, NMD_X86_EMULATOR_EXCEPTION_STEP);

		cpu->rip += instruction.length;

		if (max_count > 0 && ++cpu->count >= max_count)
			return true;
	}

	return true;
}