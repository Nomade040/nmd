#include "../nmd_assembly.h"
#include <gtest\gtest.h>

NMD_X86Instruction instruction;

TEST(ValidityAndLengthCheck, OneOpcodeMap)
{
	const uint8_t oneByteInstructions[] = {0xcc,0x90,0xc3,0x9c,0x9d,0xcb,0xcb,0xc9,0xf1,0x06,0x16,0x1e,0x0e,0x07,0x17,0x1f,0x27,0x37,0x2f,0x3f,0xd7,0x9b,0xf4,0xf5,0x9e,0x9f,0xce,0xcf,0x98,0x99,0xd6,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd};
	for (size_t i = 0; i < sizeof(oneByteInstructions); i++)
	{
		EXPECT_EQ(nmd_x86_decode_buffer(&oneByteInstructions[i], 1, &instruction, NMD_X86_MODE_32, NMD_X86_FEATURE_FLAGS_MINIMAL), true);
		EXPECT_EQ(instruction.length, 1);
		EXPECT_EQ(instruction.opcodeMap, NMD_X86_OPCODE_MAP_DEFAULT);
	}

	const uint8_t twoByteInstructions[] = { 0x05,0x06,0x07,0x08,0x09,0x0b,0x0e,0x30,0x31,0x32,0x33,0x34,0x35,0x37,0x77,0xa0,0xa1,0xa2,0xa8,0xa9,0xaa };
	uint8_t buffer[2] = { 0x0f, 0x00 };
	for (size_t i = 0; i < sizeof(twoByteInstructions); i++)
	{
		buffer[1] = twoByteInstructions[i];
		EXPECT_EQ(nmd_x86_decode_buffer(buffer, 2, &instruction, NMD_X86_MODE_32, NMD_X86_FEATURE_FLAGS_MINIMAL), true);
		EXPECT_EQ(instruction.length, 2);
		EXPECT_EQ(instruction.opcodeMap, NMD_X86_OPCODE_MAP_0F);
	}
}

int test(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}