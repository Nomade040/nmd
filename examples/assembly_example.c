#define NMD_ASSEMBLY_IMPLEMENTATION
#include "../nmd_assembly.h"
#include <stdio.h>
int main()
{
	const uint8_t buffer[] = { 0x33, 0xC0, 0x40, 0xC3, 0x8B, 0x65, 0xE8 };
	const uint8_t* const buffer_end = buffer + sizeof(buffer);

	nmd_x86_instruction instruction;
	char formatted_instruction[128];

	size_t i = 0;
	for (; i < sizeof(buffer); i += instruction.length)
	{
		if (!nmd_x86_decode(buffer + i, buffer_end - (buffer + i), &instruction, NMD_X86_MODE_32, NMD_X86_DECODER_FLAGS_MINIMAL))
			break;

		nmd_x86_format(&instruction, formatted_instruction, NMD_X86_INVALID_RUNTIME_ADDRESS, NMD_X86_FORMAT_FLAGS_DEFAULT);

		printf("%s\n", formatted_instruction);
	}
}