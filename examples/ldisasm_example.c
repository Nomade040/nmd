#define NMD_LDISASM_IMPLEMENTATION
#include "../nmd_ldisasm.h"
#include <stdio.h>
int main()
{
	const uint8_t buffer[] = { 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0x81, 0xEC, 0x70, 0x01, 0x00, 0x00, 0xA1, 0x60, 0x33, 0x82, 0x77, 0x33, 0xC4 };

	size_t i = 0;
	size_t length = 0;
	for (; i < sizeof(buffer); i += length)
	{
		if (!(length = nmd_x86_ldisasm(buffer + i, NMD_LDISASM_X86_MODE_32)))
			break;

		printf("%d\n", (uint32_t)length);
	}
}