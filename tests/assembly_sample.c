#define NMD_ASSEMBLY_IMPLEMENTATION
#include "../nmd_assembly.h"
#include <stdio.h>
int main()
{
	const uint8_t buffer[] = { 0x33, 0xC0, 0x40, 0xC3, 0x8B, 0x65, 0xE8 };
	const uint8_t* const bufferEnd = buffer + sizeof(buffer);

	NMD_X86Instruction instruction;
	char formattedInstruction[128];

	size_t i = 0;
	for (; i < sizeof(buffer); i += instruction.length)
	{
		if (!nmd_x86_decode_buffer(buffer + i, bufferEnd - (buffer + i), &instruction, NMD_X86_MODE_32, NMD_X86_FEATURE_FLAGS_MINIMAL))
			break;

		nmd_x86_format_instruction(&instruction, formattedInstruction, NMD_X86_INVALID_RUNTIME_ADDRESS, NMD_X86_FORMAT_FLAGS_DEFAULT);

		printf("%s\n", formattedInstruction);
	}
}