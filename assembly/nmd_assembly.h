/* This is a platform independent C89 x86 assembler and disassembler library.

Features:
 - Support for x86(16/32/64). Intel and AT&T syntax.
 - No libc, dynamic memory allocation, static/global variables/state/context or runtime initialization.
 - Thread-safe by design.
 - No header files need to be included.

Setup:
Define the 'NMD_ASSEMBLY_IMPLEMENTATION' macro in one source file before the include statement to instantiate the implementation.
#define NMD_ASSEMBLY_IMPLEMENTATION
#include "nmd_assembly.h"

Interfaces(i.e the functions you call from your application):
 - The assembler is implemented by the following function:
    Assembles an instruction from a string. Returns the number of bytes written to the buffer on success, zero otherwise. Instructions can be separated using the '\n'(new line) character.
    Parameters:
     - string          [in]         A pointer to a string that represents one or more instructions in assembly language.
     - buffer          [out]        A pointer to a buffer that receives the encoded instructions.
     - buffer_size     [in]         The size of the buffer in bytes.
     - runtime_address [in]         The instruction's runtime address. You may use 'NMD_X86_INVALID_RUNTIME_ADDRESS'.
     - mode            [in]         The architecture mode. 'NMD_X86_MODE_32', 'NMD_X86_MODE_64' or 'NMD_X86_MODE_16'.
     - count           [in/out/opt] A pointer to a variable that on input is the maximum number of instructions that can be parsed(or zero for unlimited instructions), and on output is the number of instructions parsed. This parameter may be zero.
    size_t nmd_x86_assemble(const char* string, void* buffer, size_t buffer_size, uint64_t runtime_address, NMD_X86_MODE mode, size_t* const count);

 - The disassembler is composed of a decoder and a formatter implemented by these two functions respectively:
	- Decodes an instruction. Returns true if the instruction is valid, false otherwise.
      Parameters:
       - buffer      [in]  A pointer to a buffer containing an encoded instruction.
       - buffer_size [in]  The size of the buffer in bytes.
       - instruction [out] A pointer to a variable of type 'nmd_x86_instruction' that receives information about the instruction.
       - mode        [in]  The architecture mode. 'NMD_X86_MODE_32', 'NMD_X86_MODE_64' or 'NMD_X86_MODE_16'.
       - flags       [in]  A mask of 'NMD_X86_DECODER_FLAGS_XXX' that specifies which features the decoder is allowed to use. If uncertain, use 'NMD_X86_DECODER_FLAGS_MINIMAL'.
      bool nmd_x86_decode(const void* buffer, size_t buffer_size, nmd_x86_instruction* instruction, NMD_X86_MODE mode, uint32_t flags);

    - Formats an instruction. This function may access invalid memory(thus causing a crash) if you modify 'instruction' manually.
      Parameters:
       - instruction     [in]  A pointer to a variable of type 'nmd_x86_instruction' describing the instruction to be formatted.
       - buffer          [out] A pointer to buffer that receives the string. The buffer's recommended size is 128 bytes.
       - runtime_address [in]  The instruction's runtime address. You may use 'NMD_X86_INVALID_RUNTIME_ADDRESS'.
       - flags           [in]  A mask of 'NMD_X86_FORMAT_FLAGS_XXX' that specifies how the function should format the instruction. If uncertain, use 'NMD_X86_FORMAT_FLAGS_DEFAULT'.
      void nmd_x86_format(const nmd_x86_instruction* instruction, char buffer[], uint64_t runtime_address, uint32_t flags);

 - The length disassembler is implemented by the following function:
    Returns the length of the instruction if it is valid, zero otherwise.
    Parameters:
     - buffer      [in] A pointer to a buffer containing an encoded instruction.
     - buffer_size [in] The size of the buffer in bytes.
     - mode        [in] The architecture mode. 'NMD_X86_MODE_32', 'NMD_X86_MODE_64' or 'NMD_X86_MODE_16'.
    size_t nmd_x86_ldisasm(const void* buffer, size_t buffer_size, NMD_X86_MODE mode);

Enabling and disabling features of the decoder at compile-time:
To dynamically choose which features are used by the decoder, use the 'flags' parameter of nmd_x86_decode(). The less features specified in the mask, the
faster the decoder runs. By default all features are available, some can be completely disabled at compile time(thus reducing code size and increasing code speed) by defining
the following macros:
 - 'NMD_ASSEMBLY_DISABLE_DECODER_VALIDITY_CHECK': the decoder does not check if the instruction is invalid.
 - 'NMD_ASSEMBLY_DISABLE_DECODER_INSTRUCTION_ID': the decoder does not fill the 'id' variable.
 - 'NMD_ASSEMBLY_DISABLE_DECODER_CPU_FLAGS': the decoder does not fill the variables related to cpu fags.
 - 'NMD_ASSEMBLY_DISABLE_DECODER_OPERANDS': the decoder does not fill the 'num_operands' and 'operands' variable.
 - 'NMD_ASSEMBLY_DISABLE_DECODER_GROUP': the decoder does not fill the 'group' variable.
 - 'NMD_ASSEMBLY_DISABLE_DECODER_VEX': the decoder does not support VEX instructions.
 - 'NMD_ASSEMBLY_DISABLE_DECODER_EVEX': the decoder does not support EVEX instructions.
 - 'NMD_ASSEMBLY_DISABLE_DECODER_3DNOW': the decoder does not support 3DNow! instructions.

Enabling and disabling features of the formatter at compile-time:
To dynamically choose which features are used by the formatter, use the 'flags' parameter of nmd_x86_format(). The less features specified in the mask, the
faster the function runs. By default all features are available, some can be completely disabled at compile time(thus reducing code size and increasing code speed) by defining
the following macros:
 - 'NMD_ASSEMBLY_DISABLE_FORMATTER_POINTER_SIZE': the formatter does not support pointer size.
 - 'NMD_ASSEMBLY_DISABLE_FORMATTER_BYTES: the formatter does not support instruction bytes. You may define the 'NMD_X86_FORMATTER_NUM_PADDING_BYTES' macro to be the number of bytes used as space padding.
 - 'NMD_ASSEMBLY_DISABLE_FORMATTER_ATT_SYNTAX: the formatter does not support AT&T syntax.
 - 'NMD_ASSEMBLY_DISABLE_FORMATTER_UPPERCASE: the formatter does not support uppercase.
 - 'NMD_ASSEMBLY_DISABLE_FORMATTER_COMMA_SPACES: the formatter does not support comma spaces.
 - 'NMD_ASSEMBLY_DISABLE_FORMATTER_OPERATOR_SPACES: the formatter does not support operator spaces.
 - 'NMD_ASSEMBLY_DISABLE_FORMATTER_VEX': the formatter does not support VEX instructions.
 - 'NMD_ASSEMBLY_DISABLE_FORMATTER_EVEX': the formatter does not support EVEX instructions.
 - 'NMD_ASSEMBLY_DISABLE_FORMATTER_3DNOW': the formatter does not support 3DNow! instructions.

Enabling and disabling features of the length disassembler at compile-time:
Use the following macros to disable features at compile-time:
 - 'NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VALIDITY_CHECK': the length disassembler does not check if the instruction is invalid.
 - 'NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_VEX': the length disassembler does not support VEX instructions.
 - 'NMD_ASSEMBLY_DISABLE_LENGTH_DISASSEMBLER_3DNOW': the length disassembler does not support 3DNow! instructions.

Fixed width integer types:
By default the library includes <stdint.h> and <stddef.h> to include int types.
If these header-files are not available in your environment you may define the 'NMD_DEFINE_INT_TYPES' macro so the library will define them.
By defining the 'NMD_IGNORE_INT_TYPES' macro, the library will neither include nor define int types.

You may define the 'NMD_ASSEMBLY_PRIVATE' macro to mark all functions as static so that they're not visible to other translation units.

Common helper functions:
Some 'nmd' libraries utilize the same functions such as '_nmd_assembly_get_num_digits' and '_nmd_assembly_get_num_digits_hex'.
All libraries include the same implementation which internally are defined as '_nmd_[LIBRARY_NAME]_[FUNCTION_NAME]' to avoid name conflits between them.
When the function is called a macro is used in the form '_NMD_FUNCTION_NAME', this allows another function to override the default implementation, so in
the case that two 'nmd' libraries need the same function, only one implementation will be used by both libraries. You may also provide your implemetation:
Example:
size_t my_get_num_digits(int x);
#define _NMD_GET_NUM_DIGITS my_get_num_digits

Shared helper functions used by this library:
 - _NMD_GET_NUM_DIGITS()
 - _NMD_GET_NUM_DIGITS_HEX()

Conventions:
 - Every identifier uses snake case.
 - Enums and macros are uppercase, every other identifier is lowercase.
 - Non-internal identifiers start with the 'NMD_' prefix.
 - Internal identifiers start with the '_NMD_' prefix.

TODO:
 Short-Term
  - Implement instruction set extensions to the decoder : VEX, EVEX, MVEX, 3DNOW, XOP.
  - Implement x86 assembler
 Long-Term
  - Add support for other architectures(ARM, MIPS and PowerPC ?).

References:
 - Intel 64 and IA-32 Architectures. Software Developer's Manual Volume 2 (2A, 2B, 2C & 2D): Instruction Set Reference, A-Z.
   - Chapter 2 Instruction Format.
   - Chapter 3-5 Instruction set reference.
   - Appendix A Opcode Map.
   - Appendix B.16 Instruction and Formats and Encoding.
 - 3DNow! Technology Manual.
 - AMD Extensions to the 3DNow! and MMX Instruction Sets Manual.
 - Intel Architecture Instruction Set Extensions and Future Features Programming Reference.
 - Capstone Engine.
 - Zydis Disassembler.
 - VIA PadLock Programming Guide.
 - Control registers - Wikipedia.

Contributors(This may not be a complete list):
 - Nomade: Founder and maintainer.
 - Darkratos: Bug reporting and feature suggesting.
*/

#ifndef NMD_ASSEMBLY_H
#define NMD_ASSEMBLY_H

#ifndef _NMD_DEFINE_INT_TYPES
 #ifdef NMD_DEFINE_INT_TYPES
  #define _NMD_DEFINE_INT_TYPES
  #ifndef __cplusplus
   #define bool  _Bool
   #define false 0
   #define true  1
  #endif /* __cplusplus */
  typedef signed char        int8_t;
  typedef unsigned char      uint8_t;
  typedef signed short       int16_t;
  typedef unsigned short     uint16_t;
  typedef signed int         int32_t;
  typedef unsigned int       uint32_t;
  typedef signed long long   int64_t;
  typedef unsigned long long uint64_t;
  #if defined(_WIN64) && defined(_MSC_VER)
   typedef unsigned __int64 size_t;
   typedef __int64          ptrdiff_t;
  #elif (defined(_WIN32) || defined(WIN32)) && defined(_MSC_VER)
   typedef unsigned __int32 size_t
   typedef __int32          ptrdiff_t;
  #elif defined(__GNUC__) || defined(__clang__)
   #if defined(__x86_64__) || defined(__ppc64__)
    typedef unsigned long size_t
    typedef long          ptrdiff_t
   #else
    typedef unsigned int size_t
    typedef int          ptrdiff_t
   #endif
  #else
   typedef unsigned long size_t
   typedef long          ptrdiff_t
  #endif
  
 #else /* NMD_DEFINE_INT_TYPES */
  #ifndef NMD_IGNORE_INT_TYPES
    #include <stdbool.h>
    #include <stdint.h>
    #include <stddef.h>
  #endif /* NMD_IGNORE_INT_TYPES */
 #endif /* NMD_DEFINE_INT_TYPES */
#endif /* _NMD_DEFINE_INT_TYPES */

#ifndef _NMD_GET_NUM_DIGITS
#define _NMD_GET_NUM_DIGITS _nmd_assembly_get_num_digits
#endif /* _NMD_GET_NUM_DIGITS */

#ifndef _NMD_GET_NUM_DIGITS_HEX
#define _NMD_GET_NUM_DIGITS_HEX _nmd_assembly_get_num_digits_hex
#endif /* _NMD_GET_NUM_DIGITS_HEX */

#ifndef NMD_X86_FORMATTER_NUM_PADDING_BYTES
#define NMD_X86_FORMATTER_NUM_PADDING_BYTES 10
#endif /* NMD_X86_FORMATTER_NUM_PADDING_BYTES */

#define NMD_X86_INVALID_RUNTIME_ADDRESS ((uint64_t)(-1))
#define NMD_X86_MAXIMUM_INSTRUCTION_LENGTH 15
#define NMD_X86_MAXIMUM_NUM_OPERANDS 10

/* Define the api macro to potentially change functions's attributes. */
#ifndef NMD_ASSEMBLY_API
#ifdef NMD_ASSEMBLY_PRIVATE
	#define NMD_ASSEMBLY_API static
#else
	#define NMD_ASSEMBLY_API
#endif /* NMD_ASSEMBLY_PRIVATE */
#endif /* NMD_ASSEMBLY_API */

/* These flags specify how the formatter should work. */
enum NMD_X86_FORMATTER_FLAGS
{
	NMD_X86_FORMAT_FLAGS_HEX                       = (1 << 0),  /* If set, numbers are displayed in hex base, otherwise they are displayed in decimal base. */
	NMD_X86_FORMAT_FLAGS_POINTER_SIZE              = (1 << 1),  /* Pointer sizes(e.g. 'dword ptr', 'byte ptr') are displayed. */
	NMD_X86_FORMAT_FLAGS_ONLY_SEGMENT_OVERRIDE     = (1 << 2),  /* If set, only segment overrides using prefixes(e.g. '2EH', '64H') are displayed, otherwise a segment is always present before a memory operand. */
	NMD_X86_FORMAT_FLAGS_COMMA_SPACES              = (1 << 3),  /* A space is placed after a comma. */
	NMD_X86_FORMAT_FLAGS_OPERATOR_SPACES           = (1 << 4),  /* A space is placed before and after the '+' and '-' characters. */
	NMD_X86_FORMAT_FLAGS_UPPERCASE                 = (1 << 5),  /* The string is uppercase. */
	NMD_X86_FORMAT_FLAGS_0X_PREFIX                 = (1 << 6),  /* Hexadecimal numbers have the '0x'('0X' if uppercase) prefix. */
	NMD_X86_FORMAT_FLAGS_H_SUFFIX                  = (1 << 7),  /* Hexadecimal numbers have the 'h'('H' if uppercase') suffix. */
	NMD_X86_FORMAT_FLAGS_ENFORCE_HEX_ID            = (1 << 8),  /* If the HEX flag is set and either the prefix or suffix flag is also set, numbers less than 10 are displayed with preffix or suffix. */
	NMD_X86_FORMAT_FLAGS_HEX_LOWERCASE             = (1 << 9),  /* If the HEX flag is set and the UPPERCASE flag is not set, hexadecimal numbers are displayed in lowercase. */
	NMD_X86_FORMAT_FLAGS_SIGNED_NUMBER_MEMORY_VIEW = (1 << 10), /* If set, signed numbers are displayed as they are represented in memory(e.g. -1 = 0xFFFFFFFF). */
	NMD_X86_FORMAT_FLAGS_SIGNED_NUMBER_HINT_HEX    = (1 << 11), /* If set and NMD_X86_FORMAT_FLAGS_SIGNED_NUMBER_MEMORY_VIEW is also set, the number's hexadecimal representation is displayed in parenthesis. */
	NMD_X86_FORMAT_FLAGS_SIGNED_NUMBER_HINT_DEC    = (1 << 12), /* Same as NMD_X86_FORMAT_FLAGS_SIGNED_NUMBER_HINT_HEX, but the number is displayed in decimal base. */
	NMD_X86_FORMAT_FLAGS_SCALE_ONE                 = (1 << 13), /* If set, scale one is displayed. E.g. add byte ptr [eax+eax*1], al. */
	NMD_X86_FORMAT_FLAGS_BYTES                     = (1 << 14), /* The instruction's bytes are displayed before the instructions. */
	NMD_X86_FORMAT_FLAGS_ATT_SYNTAX                = (1 << 15), /* AT&T syntax is used instead of Intel's. */

	/* The formatter's default formatting style. */
	NMD_X86_FORMAT_FLAGS_DEFAULT  = (NMD_X86_FORMAT_FLAGS_HEX | NMD_X86_FORMAT_FLAGS_H_SUFFIX | NMD_X86_FORMAT_FLAGS_ONLY_SEGMENT_OVERRIDE | NMD_X86_FORMAT_FLAGS_SIGNED_NUMBER_MEMORY_VIEW | NMD_X86_FORMAT_FLAGS_SIGNED_NUMBER_HINT_DEC),
};

enum NMD_X86_DECODER_FLAGS
{
	NMD_X86_DECODER_FLAGS_VALIDITY_CHECK = (1 << 0), /* The decoder checks if the instruction is valid. */
	NMD_X86_DECODER_FLAGS_INSTRUCTION_ID = (1 << 1), /* The decoder fills the 'id' variable. */
	NMD_X86_DECODER_FLAGS_CPU_FLAGS      = (1 << 2), /* The decoder fills the variables related to cpu flags. */
	NMD_X86_DECODER_FLAGS_OPERANDS       = (1 << 3), /* The decoder fills the 'num_operands' and 'operands' variable. */
	NMD_X86_DECODER_FLAGS_GROUP          = (1 << 4), /* The decoder fills 'group' variable. */
	NMD_X86_DECODER_FLAGS_VEX            = (1 << 5), /* The decoder parses VEX instructions. */
	NMD_X86_DECODER_FLAGS_EVEX           = (1 << 6), /* The decoder parses EVEX instructions. */
	NMD_X86_DECODER_FLAGS_3DNOW          = (1 << 7), /* The decoder parses 3DNow! instructions. */

	/* These are not actual features, but rather masks of features. */
	NMD_X86_DECODER_FLAGS_NONE    = 0,
	NMD_X86_DECODER_FLAGS_MINIMAL = (NMD_X86_DECODER_FLAGS_VALIDITY_CHECK | NMD_X86_DECODER_FLAGS_VEX | NMD_X86_DECODER_FLAGS_EVEX), /* Mask that specifies minimal features to provide acurate results in any environment. */
	NMD_X86_DECODER_FLAGS_ALL     = (1 << 8) - 1, /* Mask that specifies all features. */
};

enum NMD_X86_PREFIXES
{
	NMD_X86_PREFIXES_NONE                  = 0,
	NMD_X86_PREFIXES_ES_SEGMENT_OVERRIDE   = (1 << 0),
	NMD_X86_PREFIXES_CS_SEGMENT_OVERRIDE   = (1 << 1),
	NMD_X86_PREFIXES_SS_SEGMENT_OVERRIDE   = (1 << 2),
	NMD_X86_PREFIXES_DS_SEGMENT_OVERRIDE   = (1 << 3),
	NMD_X86_PREFIXES_FS_SEGMENT_OVERRIDE   = (1 << 4),
	NMD_X86_PREFIXES_GS_SEGMENT_OVERRIDE   = (1 << 5),
	NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE = (1 << 6),
	NMD_X86_PREFIXES_ADDRESS_SIZE_OVERRIDE = (1 << 7),
	NMD_X86_PREFIXES_LOCK                  = (1 << 8),
	NMD_X86_PREFIXES_REPEAT_NOT_ZERO       = (1 << 9),
	NMD_X86_PREFIXES_REPEAT                = (1 << 10),
	NMD_X86_PREFIXES_REX_W                 = (1 << 11),
	NMD_X86_PREFIXES_REX_R                 = (1 << 12),
	NMD_X86_PREFIXES_REX_X                 = (1 << 13),
	NMD_X86_PREFIXES_REX_B                 = (1 << 14)
};

enum NMD_X86_IMM
{
	NMD_X86_IMM_NONE = 0,
	NMD_X86_IMM8     = 1,
	NMD_X86_IMM16    = 2,
	NMD_X86_IMM32    = 4,
	NMD_X86_IMM48    = 6,
	NMD_X86_IMM64    = 8,
	NMD_X86_IMM_ANY  = (NMD_X86_IMM8 | NMD_X86_IMM16 | NMD_X86_IMM32 | NMD_X86_IMM64)
};

enum NMD_X86_DISP
{
	NMD_X86_DISP_NONE        = 0,
	NMD_X86_DISP8            = 1,
	NMD_X86_DISP16           = 2,
	NMD_X86_DISP32           = 4,
	NMD_X86_DISP64           = 8,
	NMD_X86_DISP_ANY         = (NMD_X86_DISP8 | NMD_X86_DISP16 | NMD_X86_DISP32)
};

typedef union nmd_x86_modrm
{
	struct
	{
		uint8_t rm  : 3;
		uint8_t reg : 3;
		uint8_t mod : 2;
	} fields;
	uint8_t modrm;
} nmd_x86_modrm;

typedef union nmd_x86_sib
{
	struct
	{
		uint8_t base  : 3;
		uint8_t index : 3;
		uint8_t scale : 2;
	} fields;
	uint8_t sib;
} nmd_x86_sib;

typedef enum NMD_X86_MODE
{
	NMD_X86_MODE_NONE = 0, /* Invalid mode. */
	NMD_X86_MODE_16   = 2,
	NMD_X86_MODE_32   = 4,
	NMD_X86_MODE_64   = 8,
} NMD_X86_MODE;

enum NMD_X86_OPCODE_MAP
{
	NMD_X86_OPCODE_MAP_NONE = 0,
	NMD_X86_OPCODE_MAP_DEFAULT,
	NMD_X86_OPCODE_MAP_0F,
	NMD_X86_OPCODE_MAP_0F38,
	NMD_X86_OPCODE_MAP_0F3A,
	NMD_X86_OPCODE_MAP_0F0F
};

enum NMD_X86_ENCODING
{
	NMD_X86_ENCODING_NONE = 0,
	NMD_X86_ENCODING_LEGACY,  /* Legacy encoding. */
	NMD_X86_ENCODING_VEX,     /* Intel's VEX(vector extensions) coding scheme. */
	NMD_X86_ENCODING_EVEX,    /* Intel's EVEX(Enhanced vector extension) coding scheme. */
	NMD_X86_ENCODING_3DNOW,   /* AMD's 3DNow! extension. */
	NMD_X86_ENCODING_XOP,     /* AMD's XOP(eXtended Operations) instruction set. */
	/* NMD_X86_ENCODING_MVEX,    MVEX used by Intel's "Xeon Phi" ISA. */
};

typedef struct nmd_x86_vex
{
	bool R : 1;
	bool X : 1;
	bool B : 1;
	bool L : 1;
	bool W : 1;
	uint8_t pp : 2;
	uint8_t m_mmmm : 5;
	uint8_t vvvv : 4;
	uint8_t vex[3]; /* The full vex prefix. vex[0] is either C4h(3-byte VEX) or C5h(2-byte VEX).*/
} nmd_x86_vex;

enum NMD_GROUP
{
	NMD_GROUP_NONE                 = 0, /* The instruction is not part of any group. */

	NMD_GROUP_JUMP                 = (1 << 0), /* All jump instructions. */
	NMD_GROUP_CALL                 = (1 << 1), /* Call instruction. */
	NMD_GROUP_RET                  = (1 << 2), /* Return instruction. */
	NMD_GROUP_INT                  = (1 << 3), /* Interrupt instruction. */
	NMD_GROUP_PRIVILEGE            = (1 << 4), /* Privileged instruction. */
	NMD_GROUP_CONDITIONAL_BRANCH   = (1 << 5), /* Conditional branch instruction. */
	NMD_GROUP_UNCONDITIONAL_BRANCH = (1 << 6), /* Unconditional branch instruction. */
	NMD_GROUP_RELATIVE_ADDRESSING  = (1 << 7), /* Relative addressing instruction. */

	/* These are not actual groups, but rather masks of groups. */
	NMD_GROUP_BRANCH = (NMD_GROUP_CONDITIONAL_BRANCH | NMD_GROUP_UNCONDITIONAL_BRANCH), /* Mask used to check if the instruction is a branch instruction. */
	NMD_GROUP_ANY    = (1 << 8) - 1, /* Mask used to check if the instruction is part of any group. */
};

/* The enums for a some classes of registers always start at a multiple of eight */
typedef enum NMD_X86_REG
{
	NMD_X86_REG_NONE  = 0,
    
	NMD_X86_REG_IP    = 8,
	NMD_X86_REG_EIP   = 9,
	NMD_X86_REG_RIP   = 10,
    
	NMD_X86_REG_AL    = 16,
	NMD_X86_REG_CL    = 17,
	NMD_X86_REG_DL    = 18,
	NMD_X86_REG_BL    = 19,
	NMD_X86_REG_AH    = 20,
	NMD_X86_REG_CH    = 21,
	NMD_X86_REG_DH    = 22,
	NMD_X86_REG_BH    = 23,
    
	NMD_X86_REG_AX    = 24,
	NMD_X86_REG_CX    = 25,
	NMD_X86_REG_DX    = 26,
	NMD_X86_REG_BX    = 27,
	NMD_X86_REG_SP    = 28,
	NMD_X86_REG_BP    = 29,
	NMD_X86_REG_SI    = 30,
	NMD_X86_REG_DI    = 31,
    
	NMD_X86_REG_EAX   = 32,
	NMD_X86_REG_ECX   = 33,
	NMD_X86_REG_EDX   = 34,
	NMD_X86_REG_EBX   = 35,
	NMD_X86_REG_ESP   = 36,
	NMD_X86_REG_EBP   = 37,
	NMD_X86_REG_ESI   = 38,
	NMD_X86_REG_EDI   = 39,
    
	NMD_X86_REG_RAX   = 40,
	NMD_X86_REG_RCX   = 41,
	NMD_X86_REG_RDX   = 42,
	NMD_X86_REG_RBX   = 43,
	NMD_X86_REG_RSP   = 44,
	NMD_X86_REG_RBP   = 45,
	NMD_X86_REG_RSI   = 46,
	NMD_X86_REG_RDI   = 47,
    
	NMD_X86_REG_R8    = 48,
	NMD_X86_REG_R9    = 49,
	NMD_X86_REG_R10   = 50,
	NMD_X86_REG_R11   = 51,
	NMD_X86_REG_R12   = 52,
	NMD_X86_REG_R13   = 53,
	NMD_X86_REG_R14   = 54,
	NMD_X86_REG_R15   = 55,
    
	NMD_X86_REG_R8B   = 56,
	NMD_X86_REG_R9B   = 57,
	NMD_X86_REG_R10B  = 58,
	NMD_X86_REG_R11B  = 59,
	NMD_X86_REG_R12B  = 60,
	NMD_X86_REG_R13B  = 61,
	NMD_X86_REG_R14B  = 62,
	NMD_X86_REG_R15B  = 63,
    
	NMD_X86_REG_R8W   = 64,
	NMD_X86_REG_R9W   = 65,
	NMD_X86_REG_R10W  = 66,
	NMD_X86_REG_R11W  = 67,
	NMD_X86_REG_R12W  = 68,
	NMD_X86_REG_R13W  = 69,
	NMD_X86_REG_R14W  = 70,
	NMD_X86_REG_R15W  = 71,
    
	NMD_X86_REG_R8D   = 72,
	NMD_X86_REG_R9D   = 73,
	NMD_X86_REG_R10D  = 74,
	NMD_X86_REG_R11D  = 75,
	NMD_X86_REG_R12D  = 76,
	NMD_X86_REG_R13D  = 77,
	NMD_X86_REG_R14D  = 78,
	NMD_X86_REG_R15D  = 79,
    
	NMD_X86_REG_ES    = 80,
	NMD_X86_REG_CS    = 81,
	NMD_X86_REG_SS    = 82,
	NMD_X86_REG_DS    = 83,
	NMD_X86_REG_FS    = 84,
	NMD_X86_REG_GS    = 85,
    
	NMD_X86_REG_CR0   = 88,
	NMD_X86_REG_CR1   = 89,
	NMD_X86_REG_CR2   = 90,
	NMD_X86_REG_CR3   = 91,
	NMD_X86_REG_CR4   = 92,
	NMD_X86_REG_CR5   = 93,
	NMD_X86_REG_CR6   = 94,
	NMD_X86_REG_CR7   = 95,
	NMD_X86_REG_CR8   = 96,
	NMD_X86_REG_CR9   = 97,
	NMD_X86_REG_CR10  = 98,
	NMD_X86_REG_CR11  = 99,
	NMD_X86_REG_CR12  = 100,
	NMD_X86_REG_CR13  = 101,
	NMD_X86_REG_CR14  = 102,
	NMD_X86_REG_CR15  = 103,
    
	NMD_X86_REG_DR0   = 104,
	NMD_X86_REG_DR1   = 105,
	NMD_X86_REG_DR2   = 106,
	NMD_X86_REG_DR3   = 107,
	NMD_X86_REG_DR4   = 108,
	NMD_X86_REG_DR5   = 109,
	NMD_X86_REG_DR6   = 110,
	NMD_X86_REG_DR7   = 111,
	NMD_X86_REG_DR8   = 112,
	NMD_X86_REG_DR9   = 113,
	NMD_X86_REG_DR10  = 114,
	NMD_X86_REG_DR11  = 115,
	NMD_X86_REG_DR12  = 116,
	NMD_X86_REG_DR13  = 117,
	NMD_X86_REG_DR14  = 118,
	NMD_X86_REG_DR15  = 119,
    
	NMD_X86_REG_MM0   = 120,
	NMD_X86_REG_MM1   = 121,
	NMD_X86_REG_MM2   = 122,
	NMD_X86_REG_MM3   = 123,
	NMD_X86_REG_MM4   = 124,
	NMD_X86_REG_MM5   = 125,
	NMD_X86_REG_MM6   = 126,
	NMD_X86_REG_MM7   = 127,
    
	NMD_X86_REG_XMM0  = 128,
	NMD_X86_REG_XMM1  = 129,
	NMD_X86_REG_XMM2  = 130,
	NMD_X86_REG_XMM3  = 131,
	NMD_X86_REG_XMM4  = 132,
	NMD_X86_REG_XMM5  = 133,
	NMD_X86_REG_XMM6  = 134,
	NMD_X86_REG_XMM7  = 135,
	NMD_X86_REG_XMM8  = 136,
	NMD_X86_REG_XMM9  = 137,
	NMD_X86_REG_XMM10 = 138,
	NMD_X86_REG_XMM11 = 139,
	NMD_X86_REG_XMM12 = 140,
	NMD_X86_REG_XMM13 = 141,
	NMD_X86_REG_XMM14 = 142,
	NMD_X86_REG_XMM15 = 143,
	NMD_X86_REG_XMM16 = 144,
	NMD_X86_REG_XMM17 = 145,
	NMD_X86_REG_XMM18 = 146,
	NMD_X86_REG_XMM19 = 147,
	NMD_X86_REG_XMM20 = 148,
	NMD_X86_REG_XMM21 = 149,
	NMD_X86_REG_XMM22 = 150,
	NMD_X86_REG_XMM23 = 151,
	NMD_X86_REG_XMM24 = 152,
	NMD_X86_REG_XMM25 = 153,
	NMD_X86_REG_XMM26 = 154,
	NMD_X86_REG_XMM27 = 155,
	NMD_X86_REG_XMM28 = 156,
	NMD_X86_REG_XMM29 = 157,
	NMD_X86_REG_XMM30 = 158,
	NMD_X86_REG_XMM31 = 159,
    
	NMD_X86_REG_YMM0  = 160,
	NMD_X86_REG_YMM1  = 161,
	NMD_X86_REG_YMM2  = 162,
	NMD_X86_REG_YMM3  = 163,
	NMD_X86_REG_YMM4  = 164,
	NMD_X86_REG_YMM5  = 165,
	NMD_X86_REG_YMM6  = 166,
	NMD_X86_REG_YMM7  = 167,
	NMD_X86_REG_YMM8  = 168,
	NMD_X86_REG_YMM9  = 169,
	NMD_X86_REG_YMM10 = 170,
	NMD_X86_REG_YMM11 = 171,
	NMD_X86_REG_YMM12 = 172,
	NMD_X86_REG_YMM13 = 173,
	NMD_X86_REG_YMM14 = 174,
	NMD_X86_REG_YMM15 = 175,
	NMD_X86_REG_YMM16 = 176,
	NMD_X86_REG_YMM17 = 177,
	NMD_X86_REG_YMM18 = 178,
	NMD_X86_REG_YMM19 = 179,
	NMD_X86_REG_YMM20 = 180,
	NMD_X86_REG_YMM21 = 181,
	NMD_X86_REG_YMM22 = 182,
	NMD_X86_REG_YMM23 = 183,
	NMD_X86_REG_YMM24 = 184,
	NMD_X86_REG_YMM25 = 185,
	NMD_X86_REG_YMM26 = 186,
	NMD_X86_REG_YMM27 = 187,
	NMD_X86_REG_YMM28 = 188,
	NMD_X86_REG_YMM29 = 189,
	NMD_X86_REG_YMM30 = 190,
	NMD_X86_REG_YMM31 = 191,
    
	NMD_X86_REG_ZMM0  = 192,
	NMD_X86_REG_ZMM1  = 193,
	NMD_X86_REG_ZMM2  = 194,
	NMD_X86_REG_ZMM3  = 195,
	NMD_X86_REG_ZMM4  = 196,
	NMD_X86_REG_ZMM5  = 197,
	NMD_X86_REG_ZMM6  = 198,
	NMD_X86_REG_ZMM7  = 199,
	NMD_X86_REG_ZMM8  = 200,
	NMD_X86_REG_ZMM9  = 201,
	NMD_X86_REG_ZMM10 = 202,
	NMD_X86_REG_ZMM11 = 203,
	NMD_X86_REG_ZMM12 = 204,
	NMD_X86_REG_ZMM13 = 205,
	NMD_X86_REG_ZMM14 = 206,
	NMD_X86_REG_ZMM15 = 207,
	NMD_X86_REG_ZMM16 = 208,
	NMD_X86_REG_ZMM17 = 209,
	NMD_X86_REG_ZMM18 = 210,
	NMD_X86_REG_ZMM19 = 211,
	NMD_X86_REG_ZMM20 = 212,
	NMD_X86_REG_ZMM21 = 213,
	NMD_X86_REG_ZMM22 = 214,
	NMD_X86_REG_ZMM23 = 215,
	NMD_X86_REG_ZMM24 = 216,
	NMD_X86_REG_ZMM25 = 217,
	NMD_X86_REG_ZMM26 = 218,
	NMD_X86_REG_ZMM27 = 219,
	NMD_X86_REG_ZMM28 = 220,
	NMD_X86_REG_ZMM29 = 221,
	NMD_X86_REG_ZMM30 = 222,
	NMD_X86_REG_ZMM31 = 223,
    
    NMD_X86_REG_K0    = 224,
	NMD_X86_REG_K1    = 225,
	NMD_X86_REG_K2    = 226,
	NMD_X86_REG_K3    = 227,
	NMD_X86_REG_K4    = 228,
	NMD_X86_REG_K5    = 229,
	NMD_X86_REG_K6    = 230,
	NMD_X86_REG_K7    = 231,
    
	NMD_X86_REG_ST0   = 232,
	NMD_X86_REG_ST1   = 233,
	NMD_X86_REG_ST2   = 234,
	NMD_X86_REG_ST3   = 235,
	NMD_X86_REG_ST4   = 236,
	NMD_X86_REG_ST5   = 237,
	NMD_X86_REG_ST6   = 238,
	NMD_X86_REG_ST7   = 239,
} NMD_X86_REG;

/*
Credits to the capstone engine:
Some members of the enum are organized in such a way because the instruction's id parsing component of the decoder can take advantage of it.
If an instruction as marked as 'padding', it means that it's being used to fill holes between instructions organized in a special way for optimization reasons.
*/
enum NMD_X86_INSTRUCTION
{
	NMD_X86_INSTRUCTION_INVALID = 0,

	/* Optimized for opcode extension group 1. */
	NMD_X86_INSTRUCTION_ADD,
	NMD_X86_INSTRUCTION_OR,
	NMD_X86_INSTRUCTION_ADC,
	NMD_X86_INSTRUCTION_SBB,
	NMD_X86_INSTRUCTION_AND,
	NMD_X86_INSTRUCTION_SUB,
	NMD_X86_INSTRUCTION_XOR,
	NMD_X86_INSTRUCTION_CMP,

	/* Optimized for opcode extension group 2. */
	NMD_X86_INSTRUCTION_ROL,
	NMD_X86_INSTRUCTION_ROR,
	NMD_X86_INSTRUCTION_RCL,
	NMD_X86_INSTRUCTION_RCR,
	NMD_X86_INSTRUCTION_SHL,
	NMD_X86_INSTRUCTION_SHR,
	NMD_X86_INSTRUCTION_AAA, /* padding */
	NMD_X86_INSTRUCTION_SAR,

	/* Optimized for opcode extension group 3. */
	NMD_X86_INSTRUCTION_TEST,
	NMD_X86_INSTRUCTION_BLSFILL, /* pading */
	NMD_X86_INSTRUCTION_NOT,
	NMD_X86_INSTRUCTION_NEG,
	NMD_X86_INSTRUCTION_MUL,
	NMD_X86_INSTRUCTION_IMUL,
	NMD_X86_INSTRUCTION_DIV,
	NMD_X86_INSTRUCTION_IDIV,

	/* Optimized for opcode extension group 5. */
	NMD_X86_INSTRUCTION_INC,
	NMD_X86_INSTRUCTION_DEC,
	NMD_X86_INSTRUCTION_CALL,
	NMD_X86_INSTRUCTION_LCALL,
	NMD_X86_INSTRUCTION_JMP,
	NMD_X86_INSTRUCTION_LJMP,
	NMD_X86_INSTRUCTION_PUSH,

	/* Optimized for the 7th row of the 1 byte opcode map and the 8th row of the 2 byte opcode map. */
	NMD_X86_INSTRUCTION_JO,
	NMD_X86_INSTRUCTION_JNO,
	NMD_X86_INSTRUCTION_JB,
	NMD_X86_INSTRUCTION_JNB,
	NMD_X86_INSTRUCTION_JZ,
	NMD_X86_INSTRUCTION_JNZ,
	NMD_X86_INSTRUCTION_JBE,
	NMD_X86_INSTRUCTION_JA,
	NMD_X86_INSTRUCTION_JS,
	NMD_X86_INSTRUCTION_JNS,
	NMD_X86_INSTRUCTION_JP,
	NMD_X86_INSTRUCTION_JNP,
	NMD_X86_INSTRUCTION_JL,
	NMD_X86_INSTRUCTION_JGE,
	NMD_X86_INSTRUCTION_JLE,
	NMD_X86_INSTRUCTION_JG,

	/* Optimized for escape opcodes with D8 as first byte. */
	NMD_X86_INSTRUCTION_FADD,
	NMD_X86_INSTRUCTION_FMUL,
	NMD_X86_INSTRUCTION_FCOM,
	NMD_X86_INSTRUCTION_FCOMP,
	NMD_X86_INSTRUCTION_FSUB,
	NMD_X86_INSTRUCTION_FSUBR,
	NMD_X86_INSTRUCTION_FDIV,
	NMD_X86_INSTRUCTION_FDIVR,

	/* Optimized for escape opcodes with D9 as first byte. */
	NMD_X86_INSTRUCTION_FLD,
	NMD_X86_INSTRUCTION_ADOX, /* padding */
	NMD_X86_INSTRUCTION_FST,
	NMD_X86_INSTRUCTION_FSTP,
	NMD_X86_INSTRUCTION_FLDENV,
	NMD_X86_INSTRUCTION_FLDCW,
	NMD_X86_INSTRUCTION_FNSTENV,
	NMD_X86_INSTRUCTION_FNSTCW,

	NMD_X86_INSTRUCTION_FCHS,
	NMD_X86_INSTRUCTION_FABS,
	NMD_X86_INSTRUCTION_AAS, /* padding */
	NMD_X86_INSTRUCTION_ADCX, /* padding */
	NMD_X86_INSTRUCTION_FTST,
	NMD_X86_INSTRUCTION_FXAM,
	NMD_X86_INSTRUCTION_RET, /* padding */
	NMD_X86_INSTRUCTION_ENTER, /* padding */
	NMD_X86_INSTRUCTION_FLD1,
	NMD_X86_INSTRUCTION_FLDL2T,
	NMD_X86_INSTRUCTION_FLDL2E,
	NMD_X86_INSTRUCTION_FLDPI,
	NMD_X86_INSTRUCTION_FLDLG2,
	NMD_X86_INSTRUCTION_FLDLN2,
	NMD_X86_INSTRUCTION_FLDZ,
	NMD_X86_INSTRUCTION_FNOP, /* padding */
	NMD_X86_INSTRUCTION_F2XM1,
	NMD_X86_INSTRUCTION_FYL2X,
	NMD_X86_INSTRUCTION_FPTAN,
	NMD_X86_INSTRUCTION_FPATAN,
	NMD_X86_INSTRUCTION_FXTRACT,
	NMD_X86_INSTRUCTION_FPREM1,
	NMD_X86_INSTRUCTION_FDECSTP,
	NMD_X86_INSTRUCTION_FINCSTP,
	NMD_X86_INSTRUCTION_FPREM,
	NMD_X86_INSTRUCTION_FYL2XP1,
	NMD_X86_INSTRUCTION_FSQRT,
	NMD_X86_INSTRUCTION_FSINCOS,
	NMD_X86_INSTRUCTION_FRNDINT,
	NMD_X86_INSTRUCTION_FSCALE,
	NMD_X86_INSTRUCTION_FSIN,
	NMD_X86_INSTRUCTION_FCOS,

	/* Optimized for escape opcodes with DA as first byte. */
	NMD_X86_INSTRUCTION_FIADD,
	NMD_X86_INSTRUCTION_FIMUL,
	NMD_X86_INSTRUCTION_FICOM,
	NMD_X86_INSTRUCTION_FICOMP,
	NMD_X86_INSTRUCTION_FISUB,
	NMD_X86_INSTRUCTION_FISUBR,
	NMD_X86_INSTRUCTION_FIDIV,
	NMD_X86_INSTRUCTION_FIDIVR,

	NMD_X86_INSTRUCTION_FCMOVB,
	NMD_X86_INSTRUCTION_FCMOVE,
	NMD_X86_INSTRUCTION_FCMOVBE,
	NMD_X86_INSTRUCTION_FCMOVU,

	/* Optimized for escape opcodes with DB/DF as first byte. */
	NMD_X86_INSTRUCTION_FILD,
	NMD_X86_INSTRUCTION_FISTTP,
	NMD_X86_INSTRUCTION_FIST,
	NMD_X86_INSTRUCTION_FISTP,
	NMD_X86_INSTRUCTION_FBLD,
	NMD_X86_INSTRUCTION_AESKEYGENASSIST, /* padding */
	NMD_X86_INSTRUCTION_FBSTP,
	NMD_X86_INSTRUCTION_ANDN, /* padding */

	NMD_X86_INSTRUCTION_FCMOVNB,
	NMD_X86_INSTRUCTION_FCMOVNE,
	NMD_X86_INSTRUCTION_FCMOVNBE,
	NMD_X86_INSTRUCTION_FCMOVNU,
	NMD_X86_INSTRUCTION_FNCLEX,
	NMD_X86_INSTRUCTION_FUCOMI,
	NMD_X86_INSTRUCTION_FCOMI,

	/* Optimized for escape opcodes with DE as first byte. */
	NMD_X86_INSTRUCTION_FADDP,
	NMD_X86_INSTRUCTION_FMULP,
	NMD_X86_INSTRUCTION_MOVAPD, /* padding */
	NMD_X86_INSTRUCTION_BNDCN, /* padding */
	NMD_X86_INSTRUCTION_FSUBRP,
	NMD_X86_INSTRUCTION_FSUBP,
	NMD_X86_INSTRUCTION_FDIVRP,
	NMD_X86_INSTRUCTION_FDIVP,

	/* Optimized for the 15th row of the 1 byte opcode map. */
	NMD_X86_INSTRUCTION_INT1,
	NMD_X86_INSTRUCTION_BSR, /* padding */
	NMD_X86_INSTRUCTION_ADDSUBPD, /* padding */
	NMD_X86_INSTRUCTION_HLT,
	NMD_X86_INSTRUCTION_CMC,
	NMD_X86_INSTRUCTION_ADDSUBPS, /* padding */
	NMD_X86_INSTRUCTION_BLENDVPD, /* padding*/
	NMD_X86_INSTRUCTION_CLC,
	NMD_X86_INSTRUCTION_STC,
	NMD_X86_INSTRUCTION_CLI,
	NMD_X86_INSTRUCTION_STI,
	NMD_X86_INSTRUCTION_CLD,
	NMD_X86_INSTRUCTION_STD,

	/* Optimized for the 13th row of the 1 byte opcode map. */
	NMD_X86_INSTRUCTION_AAM,
	NMD_X86_INSTRUCTION_AAD,
	NMD_X86_INSTRUCTION_SALC,
	NMD_X86_INSTRUCTION_XLAT,

	/* Optimized for the 14th row of the 1 byte opcode map. */
	NMD_X86_INSTRUCTION_LOOPNE,
	NMD_X86_INSTRUCTION_LOOPE,
	NMD_X86_INSTRUCTION_LOOP,
	NMD_X86_INSTRUCTION_JRCXZ,

	/* Optimized for opcode extension group 6. */
	NMD_X86_INSTRUCTION_SLDT,
	NMD_X86_INSTRUCTION_STR,
	NMD_X86_INSTRUCTION_LLDT,
	NMD_X86_INSTRUCTION_LTR,
	NMD_X86_INSTRUCTION_VERR,
	NMD_X86_INSTRUCTION_VERW,

	/* Optimized for opcode extension group 7. */
	NMD_X86_INSTRUCTION_SGDT,
	NMD_X86_INSTRUCTION_SIDT,
	NMD_X86_INSTRUCTION_LGDT,
	NMD_X86_INSTRUCTION_LIDT,
	NMD_X86_INSTRUCTION_SMSW,
	NMD_X86_INSTRUCTION_CLWB, /* padding */
	NMD_X86_INSTRUCTION_LMSW,
	NMD_X86_INSTRUCTION_INVLPG,

	NMD_X86_INSTRUCTION_VMCALL,
	NMD_X86_INSTRUCTION_VMLAUNCH,
	NMD_X86_INSTRUCTION_VMRESUME,
	NMD_X86_INSTRUCTION_VMXOFF,

	NMD_X86_INSTRUCTION_MONITOR,
	NMD_X86_INSTRUCTION_MWAIT,
	NMD_X86_INSTRUCTION_CLAC,
	NMD_X86_INSTRUCTION_STAC,
	NMD_X86_INSTRUCTION_CBW, /* padding */
	NMD_X86_INSTRUCTION_CMPSB, /* padding */
	NMD_X86_INSTRUCTION_CMPSQ, /* padding */
	NMD_X86_INSTRUCTION_ENCLS,

	NMD_X86_INSTRUCTION_XGETBV,
	NMD_X86_INSTRUCTION_XSETBV,
	NMD_X86_INSTRUCTION_ARPL, /* padding */
	NMD_X86_INSTRUCTION_BEXTR, /* padding */
	NMD_X86_INSTRUCTION_VMFUNC,
	NMD_X86_INSTRUCTION_XEND,
	NMD_X86_INSTRUCTION_XTEST,
	NMD_X86_INSTRUCTION_ENCLU,

	NMD_X86_INSTRUCTION_VMRUN,
	NMD_X86_INSTRUCTION_VMMCALL,
	NMD_X86_INSTRUCTION_VMLOAD,
	NMD_X86_INSTRUCTION_VMSAVE,
	NMD_X86_INSTRUCTION_STGI,
	NMD_X86_INSTRUCTION_CLGI,
	NMD_X86_INSTRUCTION_SKINIT,
	NMD_X86_INSTRUCTION_INVLPGA,

	/* Optimized for the row 0x0 of the 2 byte opcode map. */
	NMD_X86_INSTRUCTION_LAR,
	NMD_X86_INSTRUCTION_LSL,
	NMD_X86_INSTRUCTION_BLCFILL, /* padding */
	NMD_X86_INSTRUCTION_SYSCALL,
	NMD_X86_INSTRUCTION_CLTS,
	NMD_X86_INSTRUCTION_SYSRET,
	NMD_X86_INSTRUCTION_INVD,
	NMD_X86_INSTRUCTION_WBINVD,
	NMD_X86_INSTRUCTION_BLCI, /* padding */
	NMD_X86_INSTRUCTION_UD2,
	NMD_X86_INSTRUCTION_PREFETCHW,
	NMD_X86_INSTRUCTION_FEMMS,

	/* Optimized for the row 0x3 of the 2 byte opcode map. */
	NMD_X86_INSTRUCTION_WRMSR,
	NMD_X86_INSTRUCTION_RDTSC,
	NMD_X86_INSTRUCTION_RDMSR,
	NMD_X86_INSTRUCTION_RDPMC,
	NMD_X86_INSTRUCTION_SYSENTER,
	NMD_X86_INSTRUCTION_SYSEXIT,
	NMD_X86_INSTRUCTION_BLCIC, /* padding */
	NMD_X86_INSTRUCTION_GETSEC,

	/* Optimized for the row 0x4 of the 2 byte opcode map. */
	NMD_X86_INSTRUCTION_CMOVO,
	NMD_X86_INSTRUCTION_CMOVNO,
	NMD_X86_INSTRUCTION_CMOVB,
	NMD_X86_INSTRUCTION_CMOVAE,
	NMD_X86_INSTRUCTION_CMOVE,
	NMD_X86_INSTRUCTION_CMOVNE,
	NMD_X86_INSTRUCTION_CMOVBE,
	NMD_X86_INSTRUCTION_CMOVA,
	NMD_X86_INSTRUCTION_CMOVS,
	NMD_X86_INSTRUCTION_CMOVNS,
	NMD_X86_INSTRUCTION_CMOVP,
	NMD_X86_INSTRUCTION_CMOVNP,
	NMD_X86_INSTRUCTION_CMOVL,
	NMD_X86_INSTRUCTION_CMOVGE,
	NMD_X86_INSTRUCTION_CMOVLE,
	NMD_X86_INSTRUCTION_CMOVG,

	/* Optimized for the row 0x9 of the 2 byte opcode map. */
	NMD_X86_INSTRUCTION_SETO,
	NMD_X86_INSTRUCTION_SETNO,
	NMD_X86_INSTRUCTION_SETB,
	NMD_X86_INSTRUCTION_SETAE,
	NMD_X86_INSTRUCTION_SETE,
	NMD_X86_INSTRUCTION_SETNE,
	NMD_X86_INSTRUCTION_SETBE,
	NMD_X86_INSTRUCTION_SETA,
	NMD_X86_INSTRUCTION_SETS,
	NMD_X86_INSTRUCTION_SETNS,
	NMD_X86_INSTRUCTION_SETP,
	NMD_X86_INSTRUCTION_SETNP,
	NMD_X86_INSTRUCTION_SETL,
	NMD_X86_INSTRUCTION_SETGE,
	NMD_X86_INSTRUCTION_SETLE,
	NMD_X86_INSTRUCTION_SETG,

	/* Optimized for the row 0xb of the 2 byte opcode map. */
	NMD_X86_INSTRUCTION_LSS,
	NMD_X86_INSTRUCTION_BTR,
	NMD_X86_INSTRUCTION_LFS,
	NMD_X86_INSTRUCTION_LGS,

	NMD_X86_INSTRUCTION_BT,
	NMD_X86_INSTRUCTION_BTC,
	NMD_X86_INSTRUCTION_BTS,

	/* Optimized for the row 0x0 of the 3 byte opcode map(38h). */
	NMD_X86_INSTRUCTION_PSHUFB,
	NMD_X86_INSTRUCTION_PHADDW,
	NMD_X86_INSTRUCTION_PHADDD,
	NMD_X86_INSTRUCTION_PHADDSW,
	NMD_X86_INSTRUCTION_PMADDUBSW,
	NMD_X86_INSTRUCTION_PHSUBW,
	NMD_X86_INSTRUCTION_PHSUBD,
	NMD_X86_INSTRUCTION_PHSUBSW,
	NMD_X86_INSTRUCTION_PSIGNB,
	NMD_X86_INSTRUCTION_PSIGNW,
	NMD_X86_INSTRUCTION_PSIGND,
	NMD_X86_INSTRUCTION_PMULHRSW,

	/* Optimized for the row 0x1 of the 3 byte opcode map(38h). */
	NMD_X86_INSTRUCTION_PABSB,
	NMD_X86_INSTRUCTION_PABSW,
	NMD_X86_INSTRUCTION_PABSD,

	/* Optimized for the row 0x2 of the 3 byte opcode map(38). */
	NMD_X86_INSTRUCTION_PMOVSXBW,
	NMD_X86_INSTRUCTION_PMOVSXBD,
	NMD_X86_INSTRUCTION_PMOVSXBQ,
	NMD_X86_INSTRUCTION_PMOVSXWD,
	NMD_X86_INSTRUCTION_PMOVSXWQ,
	NMD_X86_INSTRUCTION_PMOVZXDQ,
	NMD_X86_INSTRUCTION_CPUID, /* padding */
	NMD_X86_INSTRUCTION_BLCMSK, /* padding */
	NMD_X86_INSTRUCTION_PMULDQ,
	NMD_X86_INSTRUCTION_PCMPEQQ,
	NMD_X86_INSTRUCTION_MOVNTDQA,
	NMD_X86_INSTRUCTION_PACKUSDW,

	/* Optimized for the row 0x3 of the 3 byte opcode map(38h). */
	NMD_X86_INSTRUCTION_PMOVZXBW,
	NMD_X86_INSTRUCTION_PMOVZXBD,
	NMD_X86_INSTRUCTION_PMOVZXBQ,
	NMD_X86_INSTRUCTION_PMOVZXWD,
	NMD_X86_INSTRUCTION_PMOVZXWQ,
	NMD_X86_INSTRUCTION_PMOVSXDQ,
	NMD_X86_INSTRUCTION_BLCS, /* padding */
	NMD_X86_INSTRUCTION_PCMPGTQ,
	NMD_X86_INSTRUCTION_PMINSB,
	NMD_X86_INSTRUCTION_PMINSD,
	NMD_X86_INSTRUCTION_PMINUW,
	NMD_X86_INSTRUCTION_PMINUD,
	NMD_X86_INSTRUCTION_PMAXSB,
	NMD_X86_INSTRUCTION_PMAXSD,
	NMD_X86_INSTRUCTION_PMAXUW,
	NMD_X86_INSTRUCTION_PMAXUD,

	/* Optimized for the row 0x8 of the 3 byte opcode map(38h). */
	NMD_X86_INSTRUCTION_INVEPT,
	NMD_X86_INSTRUCTION_INVVPID,
	NMD_X86_INSTRUCTION_INVPCID,

	/* Optimized for the row 0xc of the 3 byte opcode map(38h). */
	NMD_X86_INSTRUCTION_SHA1NEXTE,
	NMD_X86_INSTRUCTION_SHA1MSG1,
	NMD_X86_INSTRUCTION_SHA1MSG2,
	NMD_X86_INSTRUCTION_SHA256RNDS2,
	NMD_X86_INSTRUCTION_SHA256MSG1,
	NMD_X86_INSTRUCTION_SHA256MSG2,

	/* Optimized for the row 0xd of the 3 byte opcode map(38h). */
	NMD_X86_INSTRUCTION_AESIMC,
	NMD_X86_INSTRUCTION_AESENC,
	NMD_X86_INSTRUCTION_AESENCLAST,
	NMD_X86_INSTRUCTION_AESDEC,
	NMD_X86_INSTRUCTION_AESDECLAST,

	/* Optimized for the row 0x0 of the 3 byte opcode map(3Ah). */
	NMD_X86_INSTRUCTION_ROUNDPS,
	NMD_X86_INSTRUCTION_ROUNDPD,
	NMD_X86_INSTRUCTION_ROUNDSS,
	NMD_X86_INSTRUCTION_ROUNDSD,
	NMD_X86_INSTRUCTION_BLENDPS,
	NMD_X86_INSTRUCTION_BLENDPD,
	NMD_X86_INSTRUCTION_PBLENDW,
	NMD_X86_INSTRUCTION_PALIGNR,

	/* Optimized for the row 0x4 of the 3 byte opcode map(3A). */
	NMD_X86_INSTRUCTION_DPPS,
	NMD_X86_INSTRUCTION_DPPD,
	NMD_X86_INSTRUCTION_MPSADBW,
	NMD_X86_INSTRUCTION_VPCMPGTQ, /* padding */
	NMD_X86_INSTRUCTION_PCLMULQDQ,

	/* Optimized for the row 0x6 of the 3 byte opcode map(3A). */
	NMD_X86_INSTRUCTION_PCMPESTRM,
	NMD_X86_INSTRUCTION_PCMPESTRI,
	NMD_X86_INSTRUCTION_PCMPISTRM,
	NMD_X86_INSTRUCTION_PCMPISTRI,

	/* Optimized for the rows 0xd, 0xe and 0xf of the 2 byte opcode map. */
	NMD_X86_INSTRUCTION_PSRLW,
	NMD_X86_INSTRUCTION_PSRLD,
	NMD_X86_INSTRUCTION_PSRLQ,
	NMD_X86_INSTRUCTION_PADDQ,
	NMD_X86_INSTRUCTION_PMULLW,
	NMD_X86_INSTRUCTION_BOUND, /* padding */
	NMD_X86_INSTRUCTION_PMOVMSKB,
	NMD_X86_INSTRUCTION_PSUBUSB,
	NMD_X86_INSTRUCTION_PSUBUSW,
	NMD_X86_INSTRUCTION_PMINUB,
	NMD_X86_INSTRUCTION_PAND,
	NMD_X86_INSTRUCTION_PADDUSB,
	NMD_X86_INSTRUCTION_PADDUSW,
	NMD_X86_INSTRUCTION_PMAXUB,
	NMD_X86_INSTRUCTION_PANDN,
	NMD_X86_INSTRUCTION_PAVGB,
	NMD_X86_INSTRUCTION_PSRAW,
	NMD_X86_INSTRUCTION_PSRAD,
	NMD_X86_INSTRUCTION_PAVGW,
	NMD_X86_INSTRUCTION_PMULHUW,
	NMD_X86_INSTRUCTION_PMULHW,
	NMD_X86_INSTRUCTION_CQO, /* padding */
	NMD_X86_INSTRUCTION_CRC32, /* padding */
	NMD_X86_INSTRUCTION_PSUBSB,
	NMD_X86_INSTRUCTION_PSUBSW,
	NMD_X86_INSTRUCTION_PMINSW,
	NMD_X86_INSTRUCTION_POR,
	NMD_X86_INSTRUCTION_PADDSB,
	NMD_X86_INSTRUCTION_PADDSW,
	NMD_X86_INSTRUCTION_PMAXSW,
	NMD_X86_INSTRUCTION_PXOR,
	NMD_X86_INSTRUCTION_LDDQU,
	NMD_X86_INSTRUCTION_PSLLW,
	NMD_X86_INSTRUCTION_PSLLD,
	NMD_X86_INSTRUCTION_PSLLQ,
	NMD_X86_INSTRUCTION_PMULUDQ,
	NMD_X86_INSTRUCTION_PMADDWD,
	NMD_X86_INSTRUCTION_PSADBW,
	NMD_X86_INSTRUCTION_BSWAP, /* padding */
	NMD_X86_INSTRUCTION_PSUBB,
	NMD_X86_INSTRUCTION_PSUBW,
	NMD_X86_INSTRUCTION_PSUBD,
	NMD_X86_INSTRUCTION_PSUBQ,
	NMD_X86_INSTRUCTION_PADDB,
	NMD_X86_INSTRUCTION_PADDW,
	NMD_X86_INSTRUCTION_PADDD,

	/* Optimized for the row 0xc of the 2 byte opcode map. */
	NMD_X86_INSTRUCTION_MOVNTI,
	NMD_X86_INSTRUCTION_PINSRW,
	NMD_X86_INSTRUCTION_PEXTRW,

	/* Optimized for opcode extension group 15. */
	NMD_X86_INSTRUCTION_FXSAVE,
	NMD_X86_INSTRUCTION_FXRSTOR,
	NMD_X86_INSTRUCTION_LDMXCSR,
	NMD_X86_INSTRUCTION_STMXCSR,
	NMD_X86_INSTRUCTION_XSAVE,
	NMD_X86_INSTRUCTION_XRSTOR,
	NMD_X86_INSTRUCTION_XSAVEOPT,
	NMD_X86_INSTRUCTION_CLFLUSH,

	NMD_X86_INSTRUCTION_RDFSBASE,
	NMD_X86_INSTRUCTION_RDGSBASE,
	NMD_X86_INSTRUCTION_WRFSBASE,
	NMD_X86_INSTRUCTION_WRGSBASE,
	NMD_X86_INSTRUCTION_CMPXCHG, /* padding */
	NMD_X86_INSTRUCTION_LFENCE,
	NMD_X86_INSTRUCTION_MFENCE,
	NMD_X86_INSTRUCTION_SFENCE,
	
	NMD_X86_INSTRUCTION_PCMPEQB,
	NMD_X86_INSTRUCTION_PCMPEQW,
	NMD_X86_INSTRUCTION_PCMPEQD,
	
	/* Optimized for the row 0x5 of the 2 byte opcode map. */
	NMD_X86_INSTRUCTION_MOVMSKPS,
	NMD_X86_INSTRUCTION_SQRTPS,
	NMD_X86_INSTRUCTION_RSQRTPS,
	NMD_X86_INSTRUCTION_RCPPS,
	NMD_X86_INSTRUCTION_ANDPS,
	NMD_X86_INSTRUCTION_ANDNPS,
	NMD_X86_INSTRUCTION_ORPS,
	NMD_X86_INSTRUCTION_XORPS,
	NMD_X86_INSTRUCTION_ADDPS,
	NMD_X86_INSTRUCTION_MULPS,
	NMD_X86_INSTRUCTION_CVTPS2PD,
	NMD_X86_INSTRUCTION_CVTDQ2PS,
	NMD_X86_INSTRUCTION_SUBPS,
	NMD_X86_INSTRUCTION_MINPS,
	NMD_X86_INSTRUCTION_DIVPS,
	NMD_X86_INSTRUCTION_MAXPS,
	
	NMD_X86_INSTRUCTION_MOVMSKPD,
	NMD_X86_INSTRUCTION_SQRTPD,
	NMD_X86_INSTRUCTION_BNDLDX, /* padding */
	NMD_X86_INSTRUCTION_BNDSTX, /* padding */
	NMD_X86_INSTRUCTION_ANDPD,
	NMD_X86_INSTRUCTION_ANDNPD,
	NMD_X86_INSTRUCTION_ORPD,
	NMD_X86_INSTRUCTION_XORPD,
	NMD_X86_INSTRUCTION_ADDPD,
	NMD_X86_INSTRUCTION_MULPD,
	NMD_X86_INSTRUCTION_CVTPD2PS,
	NMD_X86_INSTRUCTION_CVTPS2DQ,
	NMD_X86_INSTRUCTION_SUBPD,
	NMD_X86_INSTRUCTION_MINPD,
	NMD_X86_INSTRUCTION_DIVPD,
	NMD_X86_INSTRUCTION_MAXPD,
	
	NMD_X86_INSTRUCTION_BNDMOV,  /* padding */
	NMD_X86_INSTRUCTION_SQRTSS,
	NMD_X86_INSTRUCTION_RSQRTSS,
	NMD_X86_INSTRUCTION_RCPSS,
	NMD_X86_INSTRUCTION_CMPXCHG16B, /* padding */
	NMD_X86_INSTRUCTION_DAA, /* padding */
	NMD_X86_INSTRUCTION_CWD, /* padding */
	NMD_X86_INSTRUCTION_INSD, /* padding */
	NMD_X86_INSTRUCTION_ADDSS,
	NMD_X86_INSTRUCTION_MULSS,
	NMD_X86_INSTRUCTION_CVTSS2SD,
	NMD_X86_INSTRUCTION_CVTTPS2DQ,
	NMD_X86_INSTRUCTION_SUBSS,
	NMD_X86_INSTRUCTION_MINSS,
	NMD_X86_INSTRUCTION_DIVSS,
	NMD_X86_INSTRUCTION_MAXSS,
	
	NMD_X86_INSTRUCTION_BNDCL, /* padding */
	NMD_X86_INSTRUCTION_SQRTSD,
	NMD_X86_INSTRUCTION_BNDCU, /* padding */
	NMD_X86_INSTRUCTION_BNDMK, /* padding */
	NMD_X86_INSTRUCTION_CMPXCHG8B, /* padding */
	NMD_X86_INSTRUCTION_DAS, /* padding */
	NMD_X86_INSTRUCTION_CWDE, /* padding */
	NMD_X86_INSTRUCTION_INSW, /* padding */
	NMD_X86_INSTRUCTION_ADDSD,
	NMD_X86_INSTRUCTION_MULSD,
	NMD_X86_INSTRUCTION_CVTSD2SS,
	NMD_X86_INSTRUCTION_FCOMIP, /* padding */
	NMD_X86_INSTRUCTION_SUBSD,
	NMD_X86_INSTRUCTION_MINSD,
	NMD_X86_INSTRUCTION_DIVSD,
	NMD_X86_INSTRUCTION_MAXSD,
	
	/* Optimized for the row 0x6 of the 2 byte opcode map. */
	NMD_X86_INSTRUCTION_PUNPCKLBW,
	NMD_X86_INSTRUCTION_PUNPCKLWD,
	NMD_X86_INSTRUCTION_PUNPCKLDQ,
	NMD_X86_INSTRUCTION_PACKSSWB,
	NMD_X86_INSTRUCTION_PCMPGTB,
	NMD_X86_INSTRUCTION_PCMPGTW,
	NMD_X86_INSTRUCTION_PCMPGTD,
	NMD_X86_INSTRUCTION_PACKUSWB,
	NMD_X86_INSTRUCTION_PUNPCKHBW,
	NMD_X86_INSTRUCTION_PUNPCKHWD,
	NMD_X86_INSTRUCTION_PUNPCKHDQ,
	NMD_X86_INSTRUCTION_PACKSSDW,
	NMD_X86_INSTRUCTION_PUNPCKLQDQ,
	NMD_X86_INSTRUCTION_PUNPCKHQDQ,
	
	/* Optimized for AVX instructions. */
	NMD_X86_INSTRUCTION_VPSHUFB,    /* 00 */
	NMD_X86_INSTRUCTION_VPHADDW,    /* 01 */
	NMD_X86_INSTRUCTION_VPHADDD,    /* 02 */
	NMD_X86_INSTRUCTION_VPHADDSW,   /* 03 */
	NMD_X86_INSTRUCTION_VPMADDUBSW, /* 04 */
	NMD_X86_INSTRUCTION_VPHSUBW,    /* 05 */
	NMD_X86_INSTRUCTION_VPHSUBD,    /* 06 */
	NMD_X86_INSTRUCTION_VPHSUBSW,   /* 07 */
	NMD_X86_INSTRUCTION_VPSIGNB,    /* 08 */
	NMD_X86_INSTRUCTION_VPSIGNW,    /* 09 */
	NMD_X86_INSTRUCTION_VPSIGND,    /* 0A dup */
	NMD_X86_INSTRUCTION_VPMULHRSW,  /* 0B dup */

	NMD_X86_INSTRUCTION_VPHADDWQ,
	NMD_X86_INSTRUCTION_VPHADDDQ,
	NMD_X86_INSTRUCTION_BLSI,
	NMD_X86_INSTRUCTION_BLSIC,
	NMD_X86_INSTRUCTION_BLSMSK,
	NMD_X86_INSTRUCTION_BLSR,
	NMD_X86_INSTRUCTION_BSF,
	NMD_X86_INSTRUCTION_BZHI,
	NMD_X86_INSTRUCTION_CDQ,
	NMD_X86_INSTRUCTION_CDQE,
	NMD_X86_INSTRUCTION_CLFLUSHOPT,
	NMD_X86_INSTRUCTION_CMPSW,
	NMD_X86_INSTRUCTION_COMISD,
	NMD_X86_INSTRUCTION_COMISS,
	NMD_X86_INSTRUCTION_CVTDQ2PD,
	NMD_X86_INSTRUCTION_CVTPD2DQ,
	NMD_X86_INSTRUCTION_CVTSD2SI,
	NMD_X86_INSTRUCTION_CVTSI2SD,
	NMD_X86_INSTRUCTION_CVTSI2SS,
	NMD_X86_INSTRUCTION_CVTSS2SI,
	NMD_X86_INSTRUCTION_CVTTPD2DQ,
	NMD_X86_INSTRUCTION_CVTTSD2SI,
	NMD_X86_INSTRUCTION_CVTTSS2SI,
	NMD_X86_INSTRUCTION_DATA16,
	NMD_X86_INSTRUCTION_EXTRACTPS,
	NMD_X86_INSTRUCTION_EXTRQ,
	NMD_X86_INSTRUCTION_FCOMPP,
	NMD_X86_INSTRUCTION_FFREE,
	NMD_X86_INSTRUCTION_FNINIT,
	NMD_X86_INSTRUCTION_FNSTSW,
	NMD_X86_INSTRUCTION_FFREEP,
	NMD_X86_INSTRUCTION_FRSTOR,
	NMD_X86_INSTRUCTION_FNSAVE,
	NMD_X86_INSTRUCTION_FSETPM,
	NMD_X86_INSTRUCTION_FXRSTOR64,
	NMD_X86_INSTRUCTION_FXSAVE64,
	NMD_X86_INSTRUCTION_MOVAPS,
	NMD_X86_INSTRUCTION_VMOVAPD,
	NMD_X86_INSTRUCTION_VMOVAPS,
	NMD_X86_INSTRUCTION_HADDPD,
	NMD_X86_INSTRUCTION_HADDPS,
	NMD_X86_INSTRUCTION_HSUBPD,
	NMD_X86_INSTRUCTION_HSUBPS,
	NMD_X86_INSTRUCTION_IN,
	NMD_X86_INSTRUCTION_INSB,
	NMD_X86_INSTRUCTION_INSERTPS,
	NMD_X86_INSTRUCTION_INSERTQ,
	NMD_X86_INSTRUCTION_INT,
	NMD_X86_INSTRUCTION_INT3,
	NMD_X86_INSTRUCTION_INTO,
	NMD_X86_INSTRUCTION_IRET,
	NMD_X86_INSTRUCTION_IRETD,
	NMD_X86_INSTRUCTION_IRETQ,
	NMD_X86_INSTRUCTION_UCOMISD,
	NMD_X86_INSTRUCTION_UCOMISS,
	NMD_X86_INSTRUCTION_VCOMISD,
	NMD_X86_INSTRUCTION_VCOMISS,
	NMD_X86_INSTRUCTION_VCVTSD2SS,
	NMD_X86_INSTRUCTION_VCVTSI2SD,
	NMD_X86_INSTRUCTION_VCVTSI2SS,
	NMD_X86_INSTRUCTION_VCVTSS2SD,
	NMD_X86_INSTRUCTION_VCVTTSD2SI,
	NMD_X86_INSTRUCTION_VCVTTSD2USI,
	NMD_X86_INSTRUCTION_VCVTTSS2SI,
	NMD_X86_INSTRUCTION_VCVTTSS2USI,
	NMD_X86_INSTRUCTION_VCVTUSI2SD,
	NMD_X86_INSTRUCTION_VCVTUSI2SS,
	NMD_X86_INSTRUCTION_VUCOMISD,
	NMD_X86_INSTRUCTION_VUCOMISS,
	NMD_X86_INSTRUCTION_JCXZ,
	NMD_X86_INSTRUCTION_JECXZ,
	NMD_X86_INSTRUCTION_KANDB,
	NMD_X86_INSTRUCTION_KANDD,
	NMD_X86_INSTRUCTION_KANDNB,
	NMD_X86_INSTRUCTION_KANDND,
	NMD_X86_INSTRUCTION_KANDNQ,
	NMD_X86_INSTRUCTION_KANDNW,
	NMD_X86_INSTRUCTION_KANDQ,
	NMD_X86_INSTRUCTION_KANDW,
	NMD_X86_INSTRUCTION_KMOVB,
	NMD_X86_INSTRUCTION_KMOVD,
	NMD_X86_INSTRUCTION_KMOVQ,
	NMD_X86_INSTRUCTION_KMOVW,
	NMD_X86_INSTRUCTION_KNOTB,
	NMD_X86_INSTRUCTION_KNOTD,
	NMD_X86_INSTRUCTION_KNOTQ,
	NMD_X86_INSTRUCTION_KNOTW,
	NMD_X86_INSTRUCTION_KORB,
	NMD_X86_INSTRUCTION_KORD,
	NMD_X86_INSTRUCTION_KORQ,
	NMD_X86_INSTRUCTION_KORTESTB,
	NMD_X86_INSTRUCTION_KORTESTD,
	NMD_X86_INSTRUCTION_KORTESTQ,
	NMD_X86_INSTRUCTION_KORTESTW,
	NMD_X86_INSTRUCTION_KORW,
	NMD_X86_INSTRUCTION_KSHIFTLB,
	NMD_X86_INSTRUCTION_KSHIFTLD,
	NMD_X86_INSTRUCTION_KSHIFTLQ,
	NMD_X86_INSTRUCTION_KSHIFTLW,
	NMD_X86_INSTRUCTION_KSHIFTRB,
	NMD_X86_INSTRUCTION_KSHIFTRD,
	NMD_X86_INSTRUCTION_KSHIFTRQ,
	NMD_X86_INSTRUCTION_KSHIFTRW,
	NMD_X86_INSTRUCTION_KUNPCKBW,
	NMD_X86_INSTRUCTION_KXNORB,
	NMD_X86_INSTRUCTION_KXNORD,
	NMD_X86_INSTRUCTION_KXNORQ,
	NMD_X86_INSTRUCTION_KXNORW,
	NMD_X86_INSTRUCTION_KXORB,
	NMD_X86_INSTRUCTION_KXORD,
	NMD_X86_INSTRUCTION_KXORQ,
	NMD_X86_INSTRUCTION_KXORW,
	NMD_X86_INSTRUCTION_LAHF,
	NMD_X86_INSTRUCTION_LDS,
	NMD_X86_INSTRUCTION_LEA,
	NMD_X86_INSTRUCTION_LEAVE,
	NMD_X86_INSTRUCTION_LES,
	NMD_X86_INSTRUCTION_LODSB,
	NMD_X86_INSTRUCTION_LODSD,
	NMD_X86_INSTRUCTION_LODSQ,
	NMD_X86_INSTRUCTION_LODSW,
	NMD_X86_INSTRUCTION_RETF,
	NMD_X86_INSTRUCTION_XADD,
	NMD_X86_INSTRUCTION_LZCNT,
	NMD_X86_INSTRUCTION_MASKMOVDQU,
	NMD_X86_INSTRUCTION_CVTPD2PI,
	NMD_X86_INSTRUCTION_CVTPI2PD,
	NMD_X86_INSTRUCTION_CVTPI2PS,
	NMD_X86_INSTRUCTION_CVTPS2PI,
	NMD_X86_INSTRUCTION_CVTTPD2PI,
	NMD_X86_INSTRUCTION_CVTTPS2PI,
	NMD_X86_INSTRUCTION_EMMS,
	NMD_X86_INSTRUCTION_MASKMOVQ,
	NMD_X86_INSTRUCTION_MOVD,
	NMD_X86_INSTRUCTION_MOVDQ2Q,
	NMD_X86_INSTRUCTION_MOVNTQ,
	NMD_X86_INSTRUCTION_MOVQ2DQ,
	NMD_X86_INSTRUCTION_MOVQ,
	NMD_X86_INSTRUCTION_PSHUFW,
	NMD_X86_INSTRUCTION_MONTMUL,
	NMD_X86_INSTRUCTION_MOV,
	NMD_X86_INSTRUCTION_MOVABS,
	NMD_X86_INSTRUCTION_MOVBE,
	NMD_X86_INSTRUCTION_MOVDDUP,
	NMD_X86_INSTRUCTION_MOVDQA,
	NMD_X86_INSTRUCTION_MOVDQU,
	NMD_X86_INSTRUCTION_MOVHLPS,
	NMD_X86_INSTRUCTION_MOVHPD,
	NMD_X86_INSTRUCTION_MOVHPS,
	NMD_X86_INSTRUCTION_MOVLHPS,
	NMD_X86_INSTRUCTION_MOVLPD,
	NMD_X86_INSTRUCTION_MOVLPS,
	NMD_X86_INSTRUCTION_MOVNTDQ,
	NMD_X86_INSTRUCTION_MOVNTPD,
	NMD_X86_INSTRUCTION_MOVNTPS,
	NMD_X86_INSTRUCTION_MOVNTSD,
	NMD_X86_INSTRUCTION_MOVNTSS,
	NMD_X86_INSTRUCTION_MOVSB,
	NMD_X86_INSTRUCTION_MOVSD,
	NMD_X86_INSTRUCTION_MOVSHDUP,
	NMD_X86_INSTRUCTION_MOVSLDUP,
	NMD_X86_INSTRUCTION_MOVSQ,
	NMD_X86_INSTRUCTION_MOVSS,
	NMD_X86_INSTRUCTION_MOVSW,
	NMD_X86_INSTRUCTION_MOVSX,
	NMD_X86_INSTRUCTION_MOVSXD,
	NMD_X86_INSTRUCTION_MOVUPD,
	NMD_X86_INSTRUCTION_MOVUPS,
	NMD_X86_INSTRUCTION_MOVZX,
	NMD_X86_INSTRUCTION_MULX,
	NMD_X86_INSTRUCTION_NOP,
	NMD_X86_INSTRUCTION_OUT,
	NMD_X86_INSTRUCTION_OUTSB,
	NMD_X86_INSTRUCTION_OUTSD,
	NMD_X86_INSTRUCTION_OUTSW,
	NMD_X86_INSTRUCTION_PAUSE,
	NMD_X86_INSTRUCTION_PAVGUSB,
	NMD_X86_INSTRUCTION_PBLENDVB,
	NMD_X86_INSTRUCTION_PCOMMIT,
	NMD_X86_INSTRUCTION_PDEP,
	NMD_X86_INSTRUCTION_PEXT,
	NMD_X86_INSTRUCTION_PEXTRB,
	NMD_X86_INSTRUCTION_PEXTRD,
	NMD_X86_INSTRUCTION_PEXTRQ,
	NMD_X86_INSTRUCTION_PF2ID,
	NMD_X86_INSTRUCTION_PF2IW,
	NMD_X86_INSTRUCTION_PFACC,
	NMD_X86_INSTRUCTION_PFADD,
	NMD_X86_INSTRUCTION_BLENDVPS, /* padding*/
	NMD_X86_INSTRUCTION_PFCMPEQ,
	NMD_X86_INSTRUCTION_PFCMPGE,
	NMD_X86_INSTRUCTION_PFCMPGT,
	NMD_X86_INSTRUCTION_PFMAX,
	NMD_X86_INSTRUCTION_PFMIN,
	NMD_X86_INSTRUCTION_PFMUL,
	NMD_X86_INSTRUCTION_PFNACC,
	NMD_X86_INSTRUCTION_PFPNACC,
	NMD_X86_INSTRUCTION_PFRCPIT1,
	NMD_X86_INSTRUCTION_PFRCPIT2,
	NMD_X86_INSTRUCTION_PFRCP,
	NMD_X86_INSTRUCTION_PFRSQIT1,
	NMD_X86_INSTRUCTION_PFRSQRT,
	NMD_X86_INSTRUCTION_PFSUBR,
	NMD_X86_INSTRUCTION_PFSUB,
	NMD_X86_INSTRUCTION_PHMINPOSUW,
	NMD_X86_INSTRUCTION_PI2FD,
	NMD_X86_INSTRUCTION_PI2FW,
	NMD_X86_INSTRUCTION_PINSRB,
	NMD_X86_INSTRUCTION_PINSRD,
	NMD_X86_INSTRUCTION_PINSRQ,
	NMD_X86_INSTRUCTION_PMULHRW,
	NMD_X86_INSTRUCTION_PMULLD,
	NMD_X86_INSTRUCTION_POP,
	NMD_X86_INSTRUCTION_POPA,
	NMD_X86_INSTRUCTION_POPAD,
	NMD_X86_INSTRUCTION_POPCNT,
	NMD_X86_INSTRUCTION_POPF,
	NMD_X86_INSTRUCTION_POPFD,
	NMD_X86_INSTRUCTION_POPFQ,
	NMD_X86_INSTRUCTION_PREFETCH,
	NMD_X86_INSTRUCTION_PREFETCHNTA,
	NMD_X86_INSTRUCTION_PREFETCHT0,
	NMD_X86_INSTRUCTION_PREFETCHT1,
	NMD_X86_INSTRUCTION_PREFETCHT2,
	NMD_X86_INSTRUCTION_PSHUFD,
	NMD_X86_INSTRUCTION_PSHUFHW,
	NMD_X86_INSTRUCTION_PSHUFLW,
	NMD_X86_INSTRUCTION_PSLLDQ,
	NMD_X86_INSTRUCTION_PSRLDQ,
	NMD_X86_INSTRUCTION_PSWAPD,
	NMD_X86_INSTRUCTION_PTEST,
	NMD_X86_INSTRUCTION_PUSHA,
	NMD_X86_INSTRUCTION_PUSHAD,
	NMD_X86_INSTRUCTION_PUSHF,
	NMD_X86_INSTRUCTION_PUSHFD,
	NMD_X86_INSTRUCTION_PUSHFQ,
	NMD_X86_INSTRUCTION_RDRAND,
	NMD_X86_INSTRUCTION_RDPID,
	NMD_X86_INSTRUCTION_RDSEED,
	NMD_X86_INSTRUCTION_RDTSCP,
	NMD_X86_INSTRUCTION_RORX,
	NMD_X86_INSTRUCTION_RSM,
	NMD_X86_INSTRUCTION_SAHF,
	NMD_X86_INSTRUCTION_SAL,
	NMD_X86_INSTRUCTION_SARX,
	NMD_X86_INSTRUCTION_SCASB,
	NMD_X86_INSTRUCTION_SCASD,
	NMD_X86_INSTRUCTION_SCASQ,
	NMD_X86_INSTRUCTION_SCASW,
	NMD_X86_INSTRUCTION_SHA1RNDS4,
	NMD_X86_INSTRUCTION_SHLD,
	NMD_X86_INSTRUCTION_SHLX,
	NMD_X86_INSTRUCTION_SHRD,
	NMD_X86_INSTRUCTION_SHRX,
	NMD_X86_INSTRUCTION_SHUFPD,
	NMD_X86_INSTRUCTION_SHUFPS,
	NMD_X86_INSTRUCTION_STOSB,
	NMD_X86_INSTRUCTION_STOSD,
	NMD_X86_INSTRUCTION_STOSQ,
	NMD_X86_INSTRUCTION_STOSW,
	NMD_X86_INSTRUCTION_FSTPNCE,
	NMD_X86_INSTRUCTION_FXCH,
	NMD_X86_INSTRUCTION_SWAPGS,
	NMD_X86_INSTRUCTION_T1MSKC,
	NMD_X86_INSTRUCTION_TZCNT,
	NMD_X86_INSTRUCTION_TZMSK,
	NMD_X86_INSTRUCTION_FUCOMIP,
	NMD_X86_INSTRUCTION_FUCOMPP,
	NMD_X86_INSTRUCTION_FUCOMP,
	NMD_X86_INSTRUCTION_FUCOM,
	NMD_X86_INSTRUCTION_UD1,
	NMD_X86_INSTRUCTION_UNPCKHPD,
	NMD_X86_INSTRUCTION_UNPCKHPS,
	NMD_X86_INSTRUCTION_UNPCKLPD,
	NMD_X86_INSTRUCTION_UNPCKLPS,
	NMD_X86_INSTRUCTION_VADDPD,
	NMD_X86_INSTRUCTION_VADDPS,
	NMD_X86_INSTRUCTION_VADDSD,
	NMD_X86_INSTRUCTION_VADDSS,
	NMD_X86_INSTRUCTION_VADDSUBPD,
	NMD_X86_INSTRUCTION_VADDSUBPS,
	NMD_X86_INSTRUCTION_VAESDECLAST,
	NMD_X86_INSTRUCTION_VAESDEC,
	NMD_X86_INSTRUCTION_VAESENCLAST,
	NMD_X86_INSTRUCTION_VAESENC,
	NMD_X86_INSTRUCTION_VAESIMC,
	NMD_X86_INSTRUCTION_VAESKEYGENASSIST,
	NMD_X86_INSTRUCTION_VALIGND,
	NMD_X86_INSTRUCTION_VALIGNQ,
	NMD_X86_INSTRUCTION_VANDNPD,
	NMD_X86_INSTRUCTION_VANDNPS,
	NMD_X86_INSTRUCTION_VANDPD,
	NMD_X86_INSTRUCTION_VANDPS,
	NMD_X86_INSTRUCTION_VBLENDMPD,
	NMD_X86_INSTRUCTION_VBLENDMPS,
	NMD_X86_INSTRUCTION_VBLENDPD,
	NMD_X86_INSTRUCTION_VBLENDPS,
	NMD_X86_INSTRUCTION_VBLENDVPD,
	NMD_X86_INSTRUCTION_VBLENDVPS,
	NMD_X86_INSTRUCTION_VBROADCASTF128,
	NMD_X86_INSTRUCTION_VBROADCASTI32X4,
	NMD_X86_INSTRUCTION_VBROADCASTI64X4,
	NMD_X86_INSTRUCTION_VBROADCASTSD,
	NMD_X86_INSTRUCTION_VBROADCASTSS,
	NMD_X86_INSTRUCTION_VCOMPRESSPD,
	NMD_X86_INSTRUCTION_VCOMPRESSPS,
	NMD_X86_INSTRUCTION_VCVTDQ2PD,
	NMD_X86_INSTRUCTION_VCVTDQ2PS,
	NMD_X86_INSTRUCTION_VCVTPD2DQX,
	NMD_X86_INSTRUCTION_VCVTPD2DQ,
	NMD_X86_INSTRUCTION_VCVTPD2PSX,
	NMD_X86_INSTRUCTION_VCVTPD2PS,
	NMD_X86_INSTRUCTION_VCVTPD2UDQ,
	NMD_X86_INSTRUCTION_VCVTPH2PS,
	NMD_X86_INSTRUCTION_VCVTPS2DQ,
	NMD_X86_INSTRUCTION_VCVTPS2PD,
	NMD_X86_INSTRUCTION_VCVTPS2PH,
	NMD_X86_INSTRUCTION_VCVTPS2UDQ,
	NMD_X86_INSTRUCTION_VCVTSD2SI,
	NMD_X86_INSTRUCTION_VCVTSD2USI,
	NMD_X86_INSTRUCTION_VCVTSS2SI,
	NMD_X86_INSTRUCTION_VCVTSS2USI,
	NMD_X86_INSTRUCTION_VCVTTPD2DQX,
	NMD_X86_INSTRUCTION_VCVTTPD2DQ,
	NMD_X86_INSTRUCTION_VCVTTPD2UDQ,
	NMD_X86_INSTRUCTION_VCVTTPS2DQ,
	NMD_X86_INSTRUCTION_VCVTTPS2UDQ,
	NMD_X86_INSTRUCTION_VCVTUDQ2PD,
	NMD_X86_INSTRUCTION_VCVTUDQ2PS,
	NMD_X86_INSTRUCTION_VDIVPD,
	NMD_X86_INSTRUCTION_VDIVPS,
	NMD_X86_INSTRUCTION_VDIVSD,
	NMD_X86_INSTRUCTION_VDIVSS,
	NMD_X86_INSTRUCTION_VDPPD,
	NMD_X86_INSTRUCTION_VDPPS,
	NMD_X86_INSTRUCTION_VEXP2PD,
	NMD_X86_INSTRUCTION_VEXP2PS,
	NMD_X86_INSTRUCTION_VEXPANDPD,
	NMD_X86_INSTRUCTION_VEXPANDPS,
	NMD_X86_INSTRUCTION_VEXTRACTF128,
	NMD_X86_INSTRUCTION_VEXTRACTF32X4,
	NMD_X86_INSTRUCTION_VEXTRACTF64X4,
	NMD_X86_INSTRUCTION_VEXTRACTI128,
	NMD_X86_INSTRUCTION_VEXTRACTI32X4,
	NMD_X86_INSTRUCTION_VEXTRACTI64X4,
	NMD_X86_INSTRUCTION_VEXTRACTPS,
	NMD_X86_INSTRUCTION_VFMADD132PD,
	NMD_X86_INSTRUCTION_VFMADD132PS,
	NMD_X86_INSTRUCTION_VFMADDPD,
	NMD_X86_INSTRUCTION_VFMADD213PD,
	NMD_X86_INSTRUCTION_VFMADD231PD,
	NMD_X86_INSTRUCTION_VFMADDPS,
	NMD_X86_INSTRUCTION_VFMADD213PS,
	NMD_X86_INSTRUCTION_VFMADD231PS,
	NMD_X86_INSTRUCTION_VFMADDSD,
	NMD_X86_INSTRUCTION_VFMADD213SD,
	NMD_X86_INSTRUCTION_VFMADD132SD,
	NMD_X86_INSTRUCTION_VFMADD231SD,
	NMD_X86_INSTRUCTION_VFMADDSS,
	NMD_X86_INSTRUCTION_VFMADD213SS,
	NMD_X86_INSTRUCTION_VFMADD132SS,
	NMD_X86_INSTRUCTION_VFMADD231SS,
	NMD_X86_INSTRUCTION_VFMADDSUB132PD,
	NMD_X86_INSTRUCTION_VFMADDSUB132PS,
	NMD_X86_INSTRUCTION_VFMADDSUBPD,
	NMD_X86_INSTRUCTION_VFMADDSUB213PD,
	NMD_X86_INSTRUCTION_VFMADDSUB231PD,
	NMD_X86_INSTRUCTION_VFMADDSUBPS,
	NMD_X86_INSTRUCTION_VFMADDSUB213PS,
	NMD_X86_INSTRUCTION_VFMADDSUB231PS,
	NMD_X86_INSTRUCTION_VFMSUB132PD,
	NMD_X86_INSTRUCTION_VFMSUB132PS,
	NMD_X86_INSTRUCTION_VFMSUBADD132PD,
	NMD_X86_INSTRUCTION_VFMSUBADD132PS,
	NMD_X86_INSTRUCTION_VFMSUBADDPD,
	NMD_X86_INSTRUCTION_VFMSUBADD213PD,
	NMD_X86_INSTRUCTION_VFMSUBADD231PD,
	NMD_X86_INSTRUCTION_VFMSUBADDPS,
	NMD_X86_INSTRUCTION_VFMSUBADD213PS,
	NMD_X86_INSTRUCTION_VFMSUBADD231PS,
	NMD_X86_INSTRUCTION_VFMSUBPD,
	NMD_X86_INSTRUCTION_VFMSUB213PD,
	NMD_X86_INSTRUCTION_VFMSUB231PD,
	NMD_X86_INSTRUCTION_VFMSUBPS,
	NMD_X86_INSTRUCTION_VFMSUB213PS,
	NMD_X86_INSTRUCTION_VFMSUB231PS,
	NMD_X86_INSTRUCTION_VFMSUBSD,
	NMD_X86_INSTRUCTION_VFMSUB213SD,
	NMD_X86_INSTRUCTION_VFMSUB132SD,
	NMD_X86_INSTRUCTION_VFMSUB231SD,
	NMD_X86_INSTRUCTION_VFMSUBSS,
	NMD_X86_INSTRUCTION_VFMSUB213SS,
	NMD_X86_INSTRUCTION_VFMSUB132SS,
	NMD_X86_INSTRUCTION_VFMSUB231SS,
	NMD_X86_INSTRUCTION_VFNMADD132PD,
	NMD_X86_INSTRUCTION_VFNMADD132PS,
	NMD_X86_INSTRUCTION_VFNMADDPD,
	NMD_X86_INSTRUCTION_VFNMADD213PD,
	NMD_X86_INSTRUCTION_VFNMADD231PD,
	NMD_X86_INSTRUCTION_VFNMADDPS,
	NMD_X86_INSTRUCTION_VFNMADD213PS,
	NMD_X86_INSTRUCTION_VFNMADD231PS,
	NMD_X86_INSTRUCTION_VFNMADDSD,
	NMD_X86_INSTRUCTION_VFNMADD213SD,
	NMD_X86_INSTRUCTION_VFNMADD132SD,
	NMD_X86_INSTRUCTION_VFNMADD231SD,
	NMD_X86_INSTRUCTION_VFNMADDSS,
	NMD_X86_INSTRUCTION_VFNMADD213SS,
	NMD_X86_INSTRUCTION_VFNMADD132SS,
	NMD_X86_INSTRUCTION_VFNMADD231SS,
	NMD_X86_INSTRUCTION_VFNMSUB132PD,
	NMD_X86_INSTRUCTION_VFNMSUB132PS,
	NMD_X86_INSTRUCTION_VFNMSUBPD,
	NMD_X86_INSTRUCTION_VFNMSUB213PD,
	NMD_X86_INSTRUCTION_VFNMSUB231PD,
	NMD_X86_INSTRUCTION_VFNMSUBPS,
	NMD_X86_INSTRUCTION_VFNMSUB213PS,
	NMD_X86_INSTRUCTION_VFNMSUB231PS,
	NMD_X86_INSTRUCTION_VFNMSUBSD,
	NMD_X86_INSTRUCTION_VFNMSUB213SD,
	NMD_X86_INSTRUCTION_VFNMSUB132SD,
	NMD_X86_INSTRUCTION_VFNMSUB231SD,
	NMD_X86_INSTRUCTION_VFNMSUBSS,
	NMD_X86_INSTRUCTION_VFNMSUB213SS,
	NMD_X86_INSTRUCTION_VFNMSUB132SS,
	NMD_X86_INSTRUCTION_VFNMSUB231SS,
	NMD_X86_INSTRUCTION_VFRCZPD,
	NMD_X86_INSTRUCTION_VFRCZPS,
	NMD_X86_INSTRUCTION_VFRCZSD,
	NMD_X86_INSTRUCTION_VFRCZSS,
	NMD_X86_INSTRUCTION_VORPD,
	NMD_X86_INSTRUCTION_VORPS,
	NMD_X86_INSTRUCTION_VXORPD,
	NMD_X86_INSTRUCTION_VXORPS,
	NMD_X86_INSTRUCTION_VGATHERDPD,
	NMD_X86_INSTRUCTION_VGATHERDPS,
	NMD_X86_INSTRUCTION_VGATHERPF0DPD,
	NMD_X86_INSTRUCTION_VGATHERPF0DPS,
	NMD_X86_INSTRUCTION_VGATHERPF0QPD,
	NMD_X86_INSTRUCTION_VGATHERPF0QPS,
	NMD_X86_INSTRUCTION_VGATHERPF1DPD,
	NMD_X86_INSTRUCTION_VGATHERPF1DPS,
	NMD_X86_INSTRUCTION_VGATHERPF1QPD,
	NMD_X86_INSTRUCTION_VGATHERPF1QPS,
	NMD_X86_INSTRUCTION_VGATHERQPD,
	NMD_X86_INSTRUCTION_VGATHERQPS,
	NMD_X86_INSTRUCTION_VHADDPD,
	NMD_X86_INSTRUCTION_VHADDPS,
	NMD_X86_INSTRUCTION_VHSUBPD,
	NMD_X86_INSTRUCTION_VHSUBPS,
	NMD_X86_INSTRUCTION_VINSERTF128,
	NMD_X86_INSTRUCTION_VINSERTF32X4,
	NMD_X86_INSTRUCTION_VINSERTF32X8,
	NMD_X86_INSTRUCTION_VINSERTF64X2,
	NMD_X86_INSTRUCTION_VINSERTF64X4,
	NMD_X86_INSTRUCTION_VINSERTI128,
	NMD_X86_INSTRUCTION_VINSERTI32X4,
	NMD_X86_INSTRUCTION_VINSERTI32X8,
	NMD_X86_INSTRUCTION_VINSERTI64X2,
	NMD_X86_INSTRUCTION_VINSERTI64X4,
	NMD_X86_INSTRUCTION_VINSERTPS,
	NMD_X86_INSTRUCTION_VLDDQU,
	NMD_X86_INSTRUCTION_VLDMXCSR,
	NMD_X86_INSTRUCTION_VMASKMOVDQU,
	NMD_X86_INSTRUCTION_VMASKMOVPD,
	NMD_X86_INSTRUCTION_VMASKMOVPS,
	NMD_X86_INSTRUCTION_VMAXPD,
	NMD_X86_INSTRUCTION_VMAXPS,
	NMD_X86_INSTRUCTION_VMAXSD,
	NMD_X86_INSTRUCTION_VMAXSS,
	NMD_X86_INSTRUCTION_VMCLEAR,
	NMD_X86_INSTRUCTION_VMINPD,
	NMD_X86_INSTRUCTION_VMINPS,
	NMD_X86_INSTRUCTION_VMINSD,
	NMD_X86_INSTRUCTION_VMINSS,
	NMD_X86_INSTRUCTION_VMOVQ,
	NMD_X86_INSTRUCTION_VMOVDDUP,
	NMD_X86_INSTRUCTION_VMOVD,
	NMD_X86_INSTRUCTION_VMOVDQA32,
	NMD_X86_INSTRUCTION_VMOVDQA64,
	NMD_X86_INSTRUCTION_VMOVDQA,
	NMD_X86_INSTRUCTION_VMOVDQU16,
	NMD_X86_INSTRUCTION_VMOVDQU32,
	NMD_X86_INSTRUCTION_VMOVDQU64,
	NMD_X86_INSTRUCTION_VMOVDQU8,
	NMD_X86_INSTRUCTION_VMOVDQU,
	NMD_X86_INSTRUCTION_VMOVHLPS,
	NMD_X86_INSTRUCTION_VMOVHPD,
	NMD_X86_INSTRUCTION_VMOVHPS,
	NMD_X86_INSTRUCTION_VMOVLHPS,
	NMD_X86_INSTRUCTION_VMOVLPD,
	NMD_X86_INSTRUCTION_VMOVLPS,
	NMD_X86_INSTRUCTION_VMOVMSKPD,
	NMD_X86_INSTRUCTION_VMOVMSKPS,
	NMD_X86_INSTRUCTION_VMOVNTDQA,
	NMD_X86_INSTRUCTION_VMOVNTDQ,
	NMD_X86_INSTRUCTION_VMOVNTPD,
	NMD_X86_INSTRUCTION_VMOVNTPS,
	NMD_X86_INSTRUCTION_VMOVSD,
	NMD_X86_INSTRUCTION_VMOVSHDUP,
	NMD_X86_INSTRUCTION_VMOVSLDUP,
	NMD_X86_INSTRUCTION_VMOVSS,
	NMD_X86_INSTRUCTION_VMOVUPD,
	NMD_X86_INSTRUCTION_VMOVUPS,
	NMD_X86_INSTRUCTION_VMPSADBW,
	NMD_X86_INSTRUCTION_VMPTRLD,
	NMD_X86_INSTRUCTION_VMPTRST,
	NMD_X86_INSTRUCTION_VMREAD,
	NMD_X86_INSTRUCTION_VMULPD,
	NMD_X86_INSTRUCTION_VMULPS,
	NMD_X86_INSTRUCTION_VMULSD,
	NMD_X86_INSTRUCTION_VMULSS,
	NMD_X86_INSTRUCTION_VMWRITE,
	NMD_X86_INSTRUCTION_VMXON,
	NMD_X86_INSTRUCTION_VPABSB,
	NMD_X86_INSTRUCTION_VPABSD,
	NMD_X86_INSTRUCTION_VPABSQ,
	NMD_X86_INSTRUCTION_VPABSW,
	NMD_X86_INSTRUCTION_VPACKSSDW,
	NMD_X86_INSTRUCTION_VPACKSSWB,
	NMD_X86_INSTRUCTION_VPACKUSDW,
	NMD_X86_INSTRUCTION_VPACKUSWB,
	NMD_X86_INSTRUCTION_VPADDB,
	NMD_X86_INSTRUCTION_VPADDD,
	NMD_X86_INSTRUCTION_VPADDQ,
	NMD_X86_INSTRUCTION_VPADDSB,
	NMD_X86_INSTRUCTION_VPADDSW,
	NMD_X86_INSTRUCTION_VPADDUSB,
	NMD_X86_INSTRUCTION_VPADDUSW,
	NMD_X86_INSTRUCTION_VPADDW,
	NMD_X86_INSTRUCTION_VPALIGNR,
	NMD_X86_INSTRUCTION_VPANDD,
	NMD_X86_INSTRUCTION_VPANDND,
	NMD_X86_INSTRUCTION_VPANDNQ,
	NMD_X86_INSTRUCTION_VPANDN,
	NMD_X86_INSTRUCTION_VPANDQ,
	NMD_X86_INSTRUCTION_VPAND,
	NMD_X86_INSTRUCTION_VPAVGB,
	NMD_X86_INSTRUCTION_VPAVGW,
	NMD_X86_INSTRUCTION_VPBLENDD,
	NMD_X86_INSTRUCTION_VPBLENDMB,
	NMD_X86_INSTRUCTION_VPBLENDMD,
	NMD_X86_INSTRUCTION_VPBLENDMQ,
	NMD_X86_INSTRUCTION_VPBLENDMW,
	NMD_X86_INSTRUCTION_VPBLENDVB,
	NMD_X86_INSTRUCTION_VPBLENDW,
	NMD_X86_INSTRUCTION_VPBROADCASTB,
	NMD_X86_INSTRUCTION_VPBROADCASTD,
	NMD_X86_INSTRUCTION_VPBROADCASTMB2Q,
	NMD_X86_INSTRUCTION_VPBROADCASTMW2D,
	NMD_X86_INSTRUCTION_VPBROADCASTQ,
	NMD_X86_INSTRUCTION_VPBROADCASTW,
	NMD_X86_INSTRUCTION_VPCLMULQDQ,
	NMD_X86_INSTRUCTION_VPCMOV,
	NMD_X86_INSTRUCTION_VPCMPB,
	NMD_X86_INSTRUCTION_VPCMPD,
	NMD_X86_INSTRUCTION_VPCMPEQB,
	NMD_X86_INSTRUCTION_VPCMPEQD,
	NMD_X86_INSTRUCTION_VPCMPEQQ,
	NMD_X86_INSTRUCTION_VPCMPEQW,
	NMD_X86_INSTRUCTION_VPCMPESTRI,
	NMD_X86_INSTRUCTION_VPCMPESTRM,
	NMD_X86_INSTRUCTION_VPCMPGTB,
	NMD_X86_INSTRUCTION_VPCMPGTD,
	NMD_X86_INSTRUCTION_VPCMPGTW,
	NMD_X86_INSTRUCTION_VPCMPISTRI,
	NMD_X86_INSTRUCTION_VPCMPISTRM,
	NMD_X86_INSTRUCTION_VPCMPQ,
	NMD_X86_INSTRUCTION_VPCMPUB,
	NMD_X86_INSTRUCTION_VPCMPUD,
	NMD_X86_INSTRUCTION_VPCMPUQ,
	NMD_X86_INSTRUCTION_VPCMPUW,
	NMD_X86_INSTRUCTION_VPCMPW,
	NMD_X86_INSTRUCTION_VPCOMB,
	NMD_X86_INSTRUCTION_VPCOMD,
	NMD_X86_INSTRUCTION_VPCOMPRESSD,
	NMD_X86_INSTRUCTION_VPCOMPRESSQ,
	NMD_X86_INSTRUCTION_VPCOMQ,
	NMD_X86_INSTRUCTION_VPCOMUB,
	NMD_X86_INSTRUCTION_VPCOMUD,
	NMD_X86_INSTRUCTION_VPCOMUQ,
	NMD_X86_INSTRUCTION_VPCOMUW,
	NMD_X86_INSTRUCTION_VPCOMW,
	NMD_X86_INSTRUCTION_VPCONFLICTD,
	NMD_X86_INSTRUCTION_VPCONFLICTQ,
	NMD_X86_INSTRUCTION_VPERM2F128,
	NMD_X86_INSTRUCTION_VPERM2I128,
	NMD_X86_INSTRUCTION_VPERMD,
	NMD_X86_INSTRUCTION_VPERMI2D,
	NMD_X86_INSTRUCTION_VPERMI2PD,
	NMD_X86_INSTRUCTION_VPERMI2PS,
	NMD_X86_INSTRUCTION_VPERMI2Q,
	NMD_X86_INSTRUCTION_VPERMIL2PD,
	NMD_X86_INSTRUCTION_VPERMIL2PS,
	NMD_X86_INSTRUCTION_VPERMILPD,
	NMD_X86_INSTRUCTION_VPERMILPS,
	NMD_X86_INSTRUCTION_VPERMPD,
	NMD_X86_INSTRUCTION_VPERMPS,
	NMD_X86_INSTRUCTION_VPERMQ,
	NMD_X86_INSTRUCTION_VPERMT2D,
	NMD_X86_INSTRUCTION_VPERMT2PD,
	NMD_X86_INSTRUCTION_VPERMT2PS,
	NMD_X86_INSTRUCTION_VPERMT2Q,
	NMD_X86_INSTRUCTION_VPEXPANDD,
	NMD_X86_INSTRUCTION_VPEXPANDQ,
	NMD_X86_INSTRUCTION_VPEXTRB,
	NMD_X86_INSTRUCTION_VPEXTRD,
	NMD_X86_INSTRUCTION_VPEXTRQ,
	NMD_X86_INSTRUCTION_VPEXTRW,
	NMD_X86_INSTRUCTION_VPGATHERDD,
	NMD_X86_INSTRUCTION_VPGATHERDQ,
	NMD_X86_INSTRUCTION_VPGATHERQD,
	NMD_X86_INSTRUCTION_VPGATHERQQ,
	NMD_X86_INSTRUCTION_VPHADDBD,
	NMD_X86_INSTRUCTION_VPHADDBQ,
	NMD_X86_INSTRUCTION_VPHADDBW,
	NMD_X86_INSTRUCTION_VPHADDUBD,
	NMD_X86_INSTRUCTION_VPHADDUBQ,
	NMD_X86_INSTRUCTION_VPHADDUBW,
	NMD_X86_INSTRUCTION_VPHADDUDQ,
	NMD_X86_INSTRUCTION_VPHADDUWD,
	NMD_X86_INSTRUCTION_VPHADDUWQ,
	NMD_X86_INSTRUCTION_VPHADDWD,
	NMD_X86_INSTRUCTION_VPHMINPOSUW,
	NMD_X86_INSTRUCTION_VPHSUBBW,
	NMD_X86_INSTRUCTION_VPHSUBDQ,
	
	NMD_X86_INSTRUCTION_VPHSUBWD,
	
	NMD_X86_INSTRUCTION_VPINSRB,
	NMD_X86_INSTRUCTION_VPINSRD,
	NMD_X86_INSTRUCTION_VPINSRQ,
	NMD_X86_INSTRUCTION_VPINSRW,
	NMD_X86_INSTRUCTION_VPLZCNTD,
	NMD_X86_INSTRUCTION_VPLZCNTQ,
	NMD_X86_INSTRUCTION_VPMACSDD,
	NMD_X86_INSTRUCTION_VPMACSDQH,
	NMD_X86_INSTRUCTION_VPMACSDQL,
	NMD_X86_INSTRUCTION_VPMACSSDD,
	NMD_X86_INSTRUCTION_VPMACSSDQH,
	NMD_X86_INSTRUCTION_VPMACSSDQL,
	NMD_X86_INSTRUCTION_VPMACSSWD,
	NMD_X86_INSTRUCTION_VPMACSSWW,
	NMD_X86_INSTRUCTION_VPMACSWD,
	NMD_X86_INSTRUCTION_VPMACSWW,
	NMD_X86_INSTRUCTION_VPMADCSSWD,
	NMD_X86_INSTRUCTION_VPMADCSWD,
	NMD_X86_INSTRUCTION_VPMADDWD,
	NMD_X86_INSTRUCTION_VPMASKMOVD,
	NMD_X86_INSTRUCTION_VPMASKMOVQ,
	NMD_X86_INSTRUCTION_VPMAXSB,
	NMD_X86_INSTRUCTION_VPMAXSD,
	NMD_X86_INSTRUCTION_VPMAXSQ,
	NMD_X86_INSTRUCTION_VPMAXSW,
	NMD_X86_INSTRUCTION_VPMAXUB,
	NMD_X86_INSTRUCTION_VPMAXUD,
	NMD_X86_INSTRUCTION_VPMAXUQ,
	NMD_X86_INSTRUCTION_VPMAXUW,
	NMD_X86_INSTRUCTION_VPMINSB,
	NMD_X86_INSTRUCTION_VPMINSD,
	NMD_X86_INSTRUCTION_VPMINSQ,
	NMD_X86_INSTRUCTION_VPMINSW,
	NMD_X86_INSTRUCTION_VPMINUB,
	NMD_X86_INSTRUCTION_VPMINUD,
	NMD_X86_INSTRUCTION_VPMINUQ,
	NMD_X86_INSTRUCTION_VPMINUW,
	NMD_X86_INSTRUCTION_VPMOVDB,
	NMD_X86_INSTRUCTION_VPMOVDW,
	NMD_X86_INSTRUCTION_VPMOVM2B,
	NMD_X86_INSTRUCTION_VPMOVM2D,
	NMD_X86_INSTRUCTION_VPMOVM2Q,
	NMD_X86_INSTRUCTION_VPMOVM2W,
	NMD_X86_INSTRUCTION_VPMOVMSKB,
	NMD_X86_INSTRUCTION_VPMOVQB,
	NMD_X86_INSTRUCTION_VPMOVQD,
	NMD_X86_INSTRUCTION_VPMOVQW,
	NMD_X86_INSTRUCTION_VPMOVSDB,
	NMD_X86_INSTRUCTION_VPMOVSDW,
	NMD_X86_INSTRUCTION_VPMOVSQB,
	NMD_X86_INSTRUCTION_VPMOVSQD,
	NMD_X86_INSTRUCTION_VPMOVSQW,
	NMD_X86_INSTRUCTION_VPMOVSXBD,
	NMD_X86_INSTRUCTION_VPMOVSXBQ,
	NMD_X86_INSTRUCTION_VPMOVSXBW,
	NMD_X86_INSTRUCTION_VPMOVSXDQ,
	NMD_X86_INSTRUCTION_VPMOVSXWD,
	NMD_X86_INSTRUCTION_VPMOVSXWQ,
	NMD_X86_INSTRUCTION_VPMOVUSDB,
	NMD_X86_INSTRUCTION_VPMOVUSDW,
	NMD_X86_INSTRUCTION_VPMOVUSQB,
	NMD_X86_INSTRUCTION_VPMOVUSQD,
	NMD_X86_INSTRUCTION_VPMOVUSQW,
	NMD_X86_INSTRUCTION_VPMOVZXBD,
	NMD_X86_INSTRUCTION_VPMOVZXBQ,
	NMD_X86_INSTRUCTION_VPMOVZXBW,
	NMD_X86_INSTRUCTION_VPMOVZXDQ,
	NMD_X86_INSTRUCTION_VPMOVZXWD,
	NMD_X86_INSTRUCTION_VPMOVZXWQ,
	NMD_X86_INSTRUCTION_VPMULDQ,
	NMD_X86_INSTRUCTION_VPMULHUW,
	NMD_X86_INSTRUCTION_VPMULHW,
	NMD_X86_INSTRUCTION_VPMULLD,
	NMD_X86_INSTRUCTION_VPMULLQ,
	NMD_X86_INSTRUCTION_VPMULLW,
	NMD_X86_INSTRUCTION_VPMULUDQ,
	NMD_X86_INSTRUCTION_VPORD,
	NMD_X86_INSTRUCTION_VPORQ,
	NMD_X86_INSTRUCTION_VPOR,
	NMD_X86_INSTRUCTION_VPPERM,
	NMD_X86_INSTRUCTION_VPROTB,
	NMD_X86_INSTRUCTION_VPROTD,
	NMD_X86_INSTRUCTION_VPROTQ,
	NMD_X86_INSTRUCTION_VPROTW,
	NMD_X86_INSTRUCTION_VPSADBW,
	NMD_X86_INSTRUCTION_VPSCATTERDD,
	NMD_X86_INSTRUCTION_VPSCATTERDQ,
	NMD_X86_INSTRUCTION_VPSCATTERQD,
	NMD_X86_INSTRUCTION_VPSCATTERQQ,
	NMD_X86_INSTRUCTION_VPSHAB,
	NMD_X86_INSTRUCTION_VPSHAD,
	NMD_X86_INSTRUCTION_VPSHAQ,
	NMD_X86_INSTRUCTION_VPSHAW,
	NMD_X86_INSTRUCTION_VPSHLB,
	NMD_X86_INSTRUCTION_VPSHLD,
	NMD_X86_INSTRUCTION_VPSHLQ,
	NMD_X86_INSTRUCTION_VPSHLW,
	NMD_X86_INSTRUCTION_VPSHUFD,
	NMD_X86_INSTRUCTION_VPSHUFHW,
	NMD_X86_INSTRUCTION_VPSHUFLW,
	NMD_X86_INSTRUCTION_VPSLLDQ,
	NMD_X86_INSTRUCTION_VPSLLD,
	NMD_X86_INSTRUCTION_VPSLLQ,
	NMD_X86_INSTRUCTION_VPSLLVD,
	NMD_X86_INSTRUCTION_VPSLLVQ,
	NMD_X86_INSTRUCTION_VPSLLW,
	NMD_X86_INSTRUCTION_VPSRAD,
	NMD_X86_INSTRUCTION_VPSRAQ,
	NMD_X86_INSTRUCTION_VPSRAVD,
	NMD_X86_INSTRUCTION_VPSRAVQ,
	NMD_X86_INSTRUCTION_VPSRAW,
	NMD_X86_INSTRUCTION_VPSRLDQ,
	NMD_X86_INSTRUCTION_VPSRLD,
	NMD_X86_INSTRUCTION_VPSRLQ,
	NMD_X86_INSTRUCTION_VPSRLVD,
	NMD_X86_INSTRUCTION_VPSRLVQ,
	NMD_X86_INSTRUCTION_VPSRLW,
	NMD_X86_INSTRUCTION_VPSUBB,
	NMD_X86_INSTRUCTION_VPSUBD,
	NMD_X86_INSTRUCTION_VPSUBQ,
	NMD_X86_INSTRUCTION_VPSUBSB,
	NMD_X86_INSTRUCTION_VPSUBSW,
	NMD_X86_INSTRUCTION_VPSUBUSB,
	NMD_X86_INSTRUCTION_VPSUBUSW,
	NMD_X86_INSTRUCTION_VPSUBW,
	NMD_X86_INSTRUCTION_VPTESTMD,
	NMD_X86_INSTRUCTION_VPTESTMQ,
	NMD_X86_INSTRUCTION_VPTESTNMD,
	NMD_X86_INSTRUCTION_VPTESTNMQ,
	NMD_X86_INSTRUCTION_VPTEST,
	NMD_X86_INSTRUCTION_VPUNPCKHBW,
	NMD_X86_INSTRUCTION_VPUNPCKHDQ,
	NMD_X86_INSTRUCTION_VPUNPCKHQDQ,
	NMD_X86_INSTRUCTION_VPUNPCKHWD,
	NMD_X86_INSTRUCTION_VPUNPCKLBW,
	NMD_X86_INSTRUCTION_VPUNPCKLDQ,
	NMD_X86_INSTRUCTION_VPUNPCKLQDQ,
	NMD_X86_INSTRUCTION_VPUNPCKLWD,
	NMD_X86_INSTRUCTION_VPXORD,
	NMD_X86_INSTRUCTION_VPXORQ,
	NMD_X86_INSTRUCTION_VPXOR,
	NMD_X86_INSTRUCTION_VRCP14PD,
	NMD_X86_INSTRUCTION_VRCP14PS,
	NMD_X86_INSTRUCTION_VRCP14SD,
	NMD_X86_INSTRUCTION_VRCP14SS,
	NMD_X86_INSTRUCTION_VRCP28PD,
	NMD_X86_INSTRUCTION_VRCP28PS,
	NMD_X86_INSTRUCTION_VRCP28SD,
	NMD_X86_INSTRUCTION_VRCP28SS,
	NMD_X86_INSTRUCTION_VRCPPS,
	NMD_X86_INSTRUCTION_VRCPSS,
	NMD_X86_INSTRUCTION_VRNDSCALEPD,
	NMD_X86_INSTRUCTION_VRNDSCALEPS,
	NMD_X86_INSTRUCTION_VRNDSCALESD,
	NMD_X86_INSTRUCTION_VRNDSCALESS,
	NMD_X86_INSTRUCTION_VROUNDPD,
	NMD_X86_INSTRUCTION_VROUNDPS,
	NMD_X86_INSTRUCTION_VROUNDSD,
	NMD_X86_INSTRUCTION_VROUNDSS,
	NMD_X86_INSTRUCTION_VRSQRT14PD,
	NMD_X86_INSTRUCTION_VRSQRT14PS,
	NMD_X86_INSTRUCTION_VRSQRT14SD,
	NMD_X86_INSTRUCTION_VRSQRT14SS,
	NMD_X86_INSTRUCTION_VRSQRT28PD,
	NMD_X86_INSTRUCTION_VRSQRT28PS,
	NMD_X86_INSTRUCTION_VRSQRT28SD,
	NMD_X86_INSTRUCTION_VRSQRT28SS,
	NMD_X86_INSTRUCTION_VRSQRTPS,
	NMD_X86_INSTRUCTION_VRSQRTSS,
	NMD_X86_INSTRUCTION_VSCATTERDPD,
	NMD_X86_INSTRUCTION_VSCATTERDPS,
	NMD_X86_INSTRUCTION_VSCATTERPF0DPD,
	NMD_X86_INSTRUCTION_VSCATTERPF0DPS,
	NMD_X86_INSTRUCTION_VSCATTERPF0QPD,
	NMD_X86_INSTRUCTION_VSCATTERPF0QPS,
	NMD_X86_INSTRUCTION_VSCATTERPF1DPD,
	NMD_X86_INSTRUCTION_VSCATTERPF1DPS,
	NMD_X86_INSTRUCTION_VSCATTERPF1QPD,
	NMD_X86_INSTRUCTION_VSCATTERPF1QPS,
	NMD_X86_INSTRUCTION_VSCATTERQPD,
	NMD_X86_INSTRUCTION_VSCATTERQPS,
	NMD_X86_INSTRUCTION_VSHUFPD,
	NMD_X86_INSTRUCTION_VSHUFPS,
	NMD_X86_INSTRUCTION_VSQRTPD,
	NMD_X86_INSTRUCTION_VSQRTPS,
	NMD_X86_INSTRUCTION_VSQRTSD,
	NMD_X86_INSTRUCTION_VSQRTSS,
	NMD_X86_INSTRUCTION_VSTMXCSR,
	NMD_X86_INSTRUCTION_VSUBPD,
	NMD_X86_INSTRUCTION_VSUBPS,
	NMD_X86_INSTRUCTION_VSUBSD,
	NMD_X86_INSTRUCTION_VSUBSS,
	NMD_X86_INSTRUCTION_VTESTPD,
	NMD_X86_INSTRUCTION_VTESTPS,
	NMD_X86_INSTRUCTION_VUNPCKHPD,
	NMD_X86_INSTRUCTION_VUNPCKHPS,
	NMD_X86_INSTRUCTION_VUNPCKLPD,
	NMD_X86_INSTRUCTION_VUNPCKLPS,
	NMD_X86_INSTRUCTION_VZEROALL,
	NMD_X86_INSTRUCTION_VZEROUPPER,
	NMD_X86_INSTRUCTION_FWAIT,
	NMD_X86_INSTRUCTION_XABORT,
	NMD_X86_INSTRUCTION_XACQUIRE,
	NMD_X86_INSTRUCTION_XBEGIN,
	NMD_X86_INSTRUCTION_XCHG,
	NMD_X86_INSTRUCTION_XCRYPTCBC,
	NMD_X86_INSTRUCTION_XCRYPTCFB,
	NMD_X86_INSTRUCTION_XCRYPTCTR,
	NMD_X86_INSTRUCTION_XCRYPTECB,
	NMD_X86_INSTRUCTION_XCRYPTOFB,
	NMD_X86_INSTRUCTION_XRELEASE,
	NMD_X86_INSTRUCTION_XRSTOR64,
	NMD_X86_INSTRUCTION_XRSTORS,
	NMD_X86_INSTRUCTION_XRSTORS64,
	NMD_X86_INSTRUCTION_XSAVE64,
	NMD_X86_INSTRUCTION_XSAVEC,
	NMD_X86_INSTRUCTION_XSAVEC64,
	NMD_X86_INSTRUCTION_XSAVEOPT64,
	NMD_X86_INSTRUCTION_XSAVES,
	NMD_X86_INSTRUCTION_XSAVES64,
	NMD_X86_INSTRUCTION_XSHA1,
	NMD_X86_INSTRUCTION_XSHA256,
	NMD_X86_INSTRUCTION_XSTORE,
	NMD_X86_INSTRUCTION_FDISI8087_NOP,
	NMD_X86_INSTRUCTION_FENI8087_NOP,

	/* pseudo instructions */
	NMD_X86_INSTRUCTION_CMPSS,
	NMD_X86_INSTRUCTION_CMPEQSS,
	NMD_X86_INSTRUCTION_CMPLTSS,
	NMD_X86_INSTRUCTION_CMPLESS,
	NMD_X86_INSTRUCTION_CMPUNORDSS,
	NMD_X86_INSTRUCTION_CMPNEQSS,
	NMD_X86_INSTRUCTION_CMPNLTSS,
	NMD_X86_INSTRUCTION_CMPNLESS,
	NMD_X86_INSTRUCTION_CMPORDSS,

	NMD_X86_INSTRUCTION_CMPSD,
	NMD_X86_INSTRUCTION_CMPEQSD,
	NMD_X86_INSTRUCTION_CMPLTSD,
	NMD_X86_INSTRUCTION_CMPLESD,
	NMD_X86_INSTRUCTION_CMPUNORDSD,
	NMD_X86_INSTRUCTION_CMPNEQSD,
	NMD_X86_INSTRUCTION_CMPNLTSD,
	NMD_X86_INSTRUCTION_CMPNLESD,
	NMD_X86_INSTRUCTION_CMPORDSD,

	NMD_X86_INSTRUCTION_CMPPS,
	NMD_X86_INSTRUCTION_CMPEQPS,
	NMD_X86_INSTRUCTION_CMPLTPS,
	NMD_X86_INSTRUCTION_CMPLEPS,
	NMD_X86_INSTRUCTION_CMPUNORDPS,
	NMD_X86_INSTRUCTION_CMPNEQPS,
	NMD_X86_INSTRUCTION_CMPNLTPS,
	NMD_X86_INSTRUCTION_CMPNLEPS,
	NMD_X86_INSTRUCTION_CMPORDPS,

	NMD_X86_INSTRUCTION_CMPPD,
	NMD_X86_INSTRUCTION_CMPEQPD,
	NMD_X86_INSTRUCTION_CMPLTPD,
	NMD_X86_INSTRUCTION_CMPLEPD,
	NMD_X86_INSTRUCTION_CMPUNORDPD,
	NMD_X86_INSTRUCTION_CMPNEQPD,
	NMD_X86_INSTRUCTION_CMPNLTPD,
	NMD_X86_INSTRUCTION_CMPNLEPD,
	NMD_X86_INSTRUCTION_CMPORDPD,

	NMD_X86_INSTRUCTION_VCMPSS,
	NMD_X86_INSTRUCTION_VCMPEQSS,
	NMD_X86_INSTRUCTION_VCMPLTSS,
	NMD_X86_INSTRUCTION_VCMPLESS,
	NMD_X86_INSTRUCTION_VCMPUNORDSS,
	NMD_X86_INSTRUCTION_VCMPNEQSS,
	NMD_X86_INSTRUCTION_VCMPNLTSS,
	NMD_X86_INSTRUCTION_VCMPNLESS,
	NMD_X86_INSTRUCTION_VCMPORDSS,
	NMD_X86_INSTRUCTION_VCMPEQ_UQSS,
	NMD_X86_INSTRUCTION_VCMPNGESS,
	NMD_X86_INSTRUCTION_VCMPNGTSS,
	NMD_X86_INSTRUCTION_VCMPFALSESS,
	NMD_X86_INSTRUCTION_VCMPNEQ_OQSS,
	NMD_X86_INSTRUCTION_VCMPGESS,
	NMD_X86_INSTRUCTION_VCMPGTSS,
	NMD_X86_INSTRUCTION_VCMPTRUESS,
	NMD_X86_INSTRUCTION_VCMPEQ_OSSS,
	NMD_X86_INSTRUCTION_VCMPLT_OQSS,
	NMD_X86_INSTRUCTION_VCMPLE_OQSS,
	NMD_X86_INSTRUCTION_VCMPUNORD_SSS,
	NMD_X86_INSTRUCTION_VCMPNEQ_USSS,
	NMD_X86_INSTRUCTION_VCMPNLT_UQSS,
	NMD_X86_INSTRUCTION_VCMPNLE_UQSS,
	NMD_X86_INSTRUCTION_VCMPORD_SSS,
	NMD_X86_INSTRUCTION_VCMPEQ_USSS,
	NMD_X86_INSTRUCTION_VCMPNGE_UQSS,
	NMD_X86_INSTRUCTION_VCMPNGT_UQSS,
	NMD_X86_INSTRUCTION_VCMPFALSE_OSSS,
	NMD_X86_INSTRUCTION_VCMPNEQ_OSSS,
	NMD_X86_INSTRUCTION_VCMPGE_OQSS,
	NMD_X86_INSTRUCTION_VCMPGT_OQSS,
	NMD_X86_INSTRUCTION_VCMPTRUE_USSS,

	NMD_X86_INSTRUCTION_VCMPSD,
	NMD_X86_INSTRUCTION_VCMPEQSD,
	NMD_X86_INSTRUCTION_VCMPLTSD,
	NMD_X86_INSTRUCTION_VCMPLESD,
	NMD_X86_INSTRUCTION_VCMPUNORDSD,
	NMD_X86_INSTRUCTION_VCMPNEQSD,
	NMD_X86_INSTRUCTION_VCMPNLTSD,
	NMD_X86_INSTRUCTION_VCMPNLESD,
	NMD_X86_INSTRUCTION_VCMPORDSD,
	NMD_X86_INSTRUCTION_VCMPEQ_UQSD,
	NMD_X86_INSTRUCTION_VCMPNGESD,
	NMD_X86_INSTRUCTION_VCMPNGTSD,
	NMD_X86_INSTRUCTION_VCMPFALSESD,
	NMD_X86_INSTRUCTION_VCMPNEQ_OQSD,
	NMD_X86_INSTRUCTION_VCMPGESD,
	NMD_X86_INSTRUCTION_VCMPGTSD,
	NMD_X86_INSTRUCTION_VCMPTRUESD,
	NMD_X86_INSTRUCTION_VCMPEQ_OSSD,
	NMD_X86_INSTRUCTION_VCMPLT_OQSD,
	NMD_X86_INSTRUCTION_VCMPLE_OQSD,
	NMD_X86_INSTRUCTION_VCMPUNORD_SSD,
	NMD_X86_INSTRUCTION_VCMPNEQ_USSD,
	NMD_X86_INSTRUCTION_VCMPNLT_UQSD,
	NMD_X86_INSTRUCTION_VCMPNLE_UQSD,
	NMD_X86_INSTRUCTION_VCMPORD_SSD,
	NMD_X86_INSTRUCTION_VCMPEQ_USSD,
	NMD_X86_INSTRUCTION_VCMPNGE_UQSD,
	NMD_X86_INSTRUCTION_VCMPNGT_UQSD,
	NMD_X86_INSTRUCTION_VCMPFALSE_OSSD,
	NMD_X86_INSTRUCTION_VCMPNEQ_OSSD,
	NMD_X86_INSTRUCTION_VCMPGE_OQSD,
	NMD_X86_INSTRUCTION_VCMPGT_OQSD,
	NMD_X86_INSTRUCTION_VCMPTRUE_USSD,

	NMD_X86_INSTRUCTION_VCMPPS,
	NMD_X86_INSTRUCTION_VCMPEQPS,
	NMD_X86_INSTRUCTION_VCMPLTPS,
	NMD_X86_INSTRUCTION_VCMPLEPS,
	NMD_X86_INSTRUCTION_VCMPUNORDPS,
	NMD_X86_INSTRUCTION_VCMPNEQPS,
	NMD_X86_INSTRUCTION_VCMPNLTPS,
	NMD_X86_INSTRUCTION_VCMPNLEPS,
	NMD_X86_INSTRUCTION_VCMPORDPS,
	NMD_X86_INSTRUCTION_VCMPEQ_UQPS,
	NMD_X86_INSTRUCTION_VCMPNGEPS,
	NMD_X86_INSTRUCTION_VCMPNGTPS,
	NMD_X86_INSTRUCTION_VCMPFALSEPS,
	NMD_X86_INSTRUCTION_VCMPNEQ_OQPS,
	NMD_X86_INSTRUCTION_VCMPGEPS,
	NMD_X86_INSTRUCTION_VCMPGTPS,
	NMD_X86_INSTRUCTION_VCMPTRUEPS,
	NMD_X86_INSTRUCTION_VCMPEQ_OSPS,
	NMD_X86_INSTRUCTION_VCMPLT_OQPS,
	NMD_X86_INSTRUCTION_VCMPLE_OQPS,
	NMD_X86_INSTRUCTION_VCMPUNORD_SPS,
	NMD_X86_INSTRUCTION_VCMPNEQ_USPS,
	NMD_X86_INSTRUCTION_VCMPNLT_UQPS,
	NMD_X86_INSTRUCTION_VCMPNLE_UQPS,
	NMD_X86_INSTRUCTION_VCMPORD_SPS,
	NMD_X86_INSTRUCTION_VCMPEQ_USPS,
	NMD_X86_INSTRUCTION_VCMPNGE_UQPS,
	NMD_X86_INSTRUCTION_VCMPNGT_UQPS,
	NMD_X86_INSTRUCTION_VCMPFALSE_OSPS,
	NMD_X86_INSTRUCTION_VCMPNEQ_OSPS,
	NMD_X86_INSTRUCTION_VCMPGE_OQPS,
	NMD_X86_INSTRUCTION_VCMPGT_OQPS,
	NMD_X86_INSTRUCTION_VCMPTRUE_USPS,

	NMD_X86_INSTRUCTION_VCMPPD,
	NMD_X86_INSTRUCTION_VCMPEQPD,
	NMD_X86_INSTRUCTION_VCMPLTPD,
	NMD_X86_INSTRUCTION_VCMPLEPD,
	NMD_X86_INSTRUCTION_VCMPUNORDPD,
	NMD_X86_INSTRUCTION_VCMPNEQPD,
	NMD_X86_INSTRUCTION_VCMPNLTPD,
	NMD_X86_INSTRUCTION_VCMPNLEPD,
	NMD_X86_INSTRUCTION_VCMPORDPD,
	NMD_X86_INSTRUCTION_VCMPEQ_UQPD,
	NMD_X86_INSTRUCTION_VCMPNGEPD,
	NMD_X86_INSTRUCTION_VCMPNGTPD,
	NMD_X86_INSTRUCTION_VCMPFALSEPD,
	NMD_X86_INSTRUCTION_VCMPNEQ_OQPD,
	NMD_X86_INSTRUCTION_VCMPGEPD,
	NMD_X86_INSTRUCTION_VCMPGTPD,
	NMD_X86_INSTRUCTION_VCMPTRUEPD,
	NMD_X86_INSTRUCTION_VCMPEQ_OSPD,
	NMD_X86_INSTRUCTION_VCMPLT_OQPD,
	NMD_X86_INSTRUCTION_VCMPLE_OQPD,
	NMD_X86_INSTRUCTION_VCMPUNORD_SPD,
	NMD_X86_INSTRUCTION_VCMPNEQ_USPD,
	NMD_X86_INSTRUCTION_VCMPNLT_UQPD,
	NMD_X86_INSTRUCTION_VCMPNLE_UQPD,
	NMD_X86_INSTRUCTION_VCMPORD_SPD,
	NMD_X86_INSTRUCTION_VCMPEQ_USPD,
	NMD_X86_INSTRUCTION_VCMPNGE_UQPD,
	NMD_X86_INSTRUCTION_VCMPNGT_UQPD,
	NMD_X86_INSTRUCTION_VCMPFALSE_OSPD,
	NMD_X86_INSTRUCTION_VCMPNEQ_OSPD,
	NMD_X86_INSTRUCTION_VCMPGE_OQPD,
	NMD_X86_INSTRUCTION_VCMPGT_OQPD,
	NMD_X86_INSTRUCTION_VCMPTRUE_USPD,

	NMD_X86_INSTRUCTION_UD0,
	NMD_X86_INSTRUCTION_ENDBR32,
	NMD_X86_INSTRUCTION_ENDBR64,
};

enum NMD_X86_OPERAND_TYPE
{
	NMD_X86_OPERAND_TYPE_NONE = 0,
	NMD_X86_OPERAND_TYPE_REGISTER,
	NMD_X86_OPERAND_TYPE_MEMORY,
	NMD_X86_OPERAND_TYPE_IMMEDIATE,
};

typedef struct nmd_x86_memory_operand
{
	uint8_t segment;     /* The segment register. A member of 'NMD_X86_REG'. */
	uint8_t base;        /* The base register. A member of 'NMD_X86_REG'. */
	uint8_t index;       /* The index register. A member of 'NMD_X86_REG'. */
	uint8_t scale;       /* Scale(1, 2, 4 or 8). */
	int64_t disp;        /* Displacement. */
} nmd_x86_memory_operand;

enum NMD_X86_OPERAND_ACTION
{
	NMD_X86_OPERAND_ACTION_NONE = 0, /* The operand is neither read from nor written to. */

	NMD_X86_OPERAND_ACTION_READ      = (1 << 0), /* The operand is read. */
	NMD_X86_OPERAND_ACTION_WRITE     = (1 << 1), /* The operand is modified. */
	NMD_X86_OPERAND_ACTION_CONDREAD  = (1 << 2), /* The operand may be read depending on some condition(conditional read). */
	NMD_X86_OPERAND_ACTION_CONDWRITE = (1 << 3), /* The operand may be modified depending on some condition(conditional write). */

	/* These are not actual actions, but rather masks of actions. */
	NMD_X86_OPERAND_ACTION_READWRITE = (NMD_X86_OPERAND_ACTION_READ | NMD_X86_OPERAND_ACTION_WRITE),
	NMD_X86_OPERAND_ACTION_ANY_READ  = (NMD_X86_OPERAND_ACTION_READ | NMD_X86_OPERAND_ACTION_CONDREAD),
	NMD_X86_OPERAND_ACTION_ANY_WRITE = (NMD_X86_OPERAND_ACTION_WRITE | NMD_X86_OPERAND_ACTION_CONDWRITE)
};

typedef struct nmd_x86_operand
{
	uint8_t type;                  /* The operand's type. A member of 'NMD_X86_OPERAND_TYPE'. */
	/* uint8_t size;                The operand's size. (I don't really know what this `size` variable represents or how it would be useful) */
	bool is_implicit;              /* If true, the operand does not appear on the intruction's formatted form. */
	uint8_t action;                /* The action on the operand. A member of 'NMD_X86_OPERAND_ACTION'. */
	union {                        /* The operand's "raw" data. */
		uint8_t reg;               /* Register operand. A variable of type 'NMD_X86_REG' */
		int64_t imm;               /* Immediate operand. */
		nmd_x86_memory_operand mem;  /* Memory operand. */
	} fields;
} nmd_x86_operand;

typedef union nmd_x86_cpu_flags
{
	struct
	{
		uint8_t CF   : 1; /* Bit  0.    Carry Flag (CF) */
		uint8_t b1   : 1; /* Bit  1.    Reserved */
		uint8_t PF   : 1; /* Bit  2.    Parity Flag (PF) */
		uint8_t B3   : 1; /* Bit  3.    Reserved */
		uint8_t AF   : 1; /* Bit  4.    Auxiliary Carry Flag (AF) */
		uint8_t B5   : 1; /* Bit  5.    Reserved */
		uint8_t ZF   : 1; /* Bit  6.    Zero flag(ZF) */
		uint8_t SF   : 1; /* Bit  7.    Sign flag(SF) */
		uint8_t TF   : 1; /* Bit  8.    Trap flag(TF) */
		uint8_t IF   : 1; /* Bit  9.    Interrupt Enable Flag (IF) */
		uint8_t DF   : 1; /* Bit 10.    Direction Flag (DF) */
		uint8_t OF   : 1; /* Bit 11.    Overflow Flag (OF) */
		uint8_t IOPL : 2; /* Bit 12,13. I/O Privilege Level (IOPL) */
		uint8_t NT   : 1; /* Bit 14.    Nested Task (NT) */
		uint8_t B15  : 1; /* Bit 15.    Reserved */
		uint8_t RF   : 1; /* Bit 16.    Resume Flag (RF) */
		uint8_t VM   : 1; /* Bit 17.    Virtual-8086 Mode (VM) */
		uint8_t AC   : 1; /* Bit 18.    Alignment Check / Access Control (AC) */
		uint8_t VIF  : 1; /* Bit 19.    Virtual Interrupt Flag (VIF) */
		uint8_t VIP  : 1; /* Bit 20.    Virtual Interrupt Pending (VIP) */
		uint8_t ID   : 1; /* Bit 21.    ID Flag(ID) */
		uint8_t B22  : 1; /* Bit 22.    Reserved */
		uint8_t B23  : 1; /* Bit 23.    Reserved */
		uint8_t B24  : 1; /* Bit 24.    Reserved */
		uint8_t B25  : 1; /* Bit 25.    Reserved */
		uint8_t B26  : 1; /* Bit 26.    Reserved */
		uint8_t B27  : 1; /* Bit 27.    Reserved */
		uint8_t B28  : 1; /* Bit 28.    Reserved */
		uint8_t B29  : 1; /* Bit 29.    Reserved */
		uint8_t B30  : 1; /* Bit 30.    Reserved */
		uint8_t B31  : 1; /* Bit 31.    Reserved */
	} fields;
	struct
	{
		uint8_t IE  : 1; /* Bit  0.    Invalid Operation (IE) */
		uint8_t DE  : 1; /* Bit  1.    Denormalized Operand (DE) */
		uint8_t ZE  : 1; /* Bit  2.    Zero Divide (ZE) */
		uint8_t OE  : 1; /* Bit  3.    Overflow (OE) */
		uint8_t UE  : 1; /* Bit  4.    Underflow (UE) */
		uint8_t PE  : 1; /* Bit  5.    Precision (PE) */
		uint8_t SF  : 1; /* Bit  6.    Stack Fault (SF) */
		uint8_t ES  : 1; /* Bit  7.    Exception Summary Status (ES) */
		uint8_t C0  : 1; /* Bit  8.    Condition code 0 (C0) */
		uint8_t C1  : 1; /* Bit  9.    Condition code 1 (C1) */
		uint8_t C2  : 1; /* Bit 10.    Condition code 2 (C2) */
		uint8_t TOP : 3; /* Bit 11-13. Top of Stack Pointer (TOP) */
		uint8_t C3  : 1; /* Bit 14.    Condition code 3 (C3) */
		uint8_t B   : 1; /* Bit 15.    FPU Busy (B) */
	} fpu_fields;
	uint8_t l8;
	uint32_t eflags;
	uint16_t fpu_flags;
} nmd_x86_cpu_flags;

enum NMD_X86_EFLAGS
{
	NMD_X86_EFLAGS_ID   = (1 << 21),
	NMD_X86_EFLAGS_VIP  = (1 << 20),
	NMD_X86_EFLAGS_VIF  = (1 << 19),
	NMD_X86_EFLAGS_AC   = (1 << 18),
	NMD_X86_EFLAGS_VM   = (1 << 17),
	NMD_X86_EFLAGS_RF   = (1 << 16),
	NMD_X86_EFLAGS_NT   = (1 << 14),
	NMD_X86_EFLAGS_IOPL = (1 << 12) /*| (1 << 13)*/,
	NMD_X86_EFLAGS_OF   = (1 << 11),
	NMD_X86_EFLAGS_DF   = (1 << 10),
	NMD_X86_EFLAGS_IF   = (1 << 9),
	NMD_X86_EFLAGS_TF   = (1 << 8),
	NMD_X86_EFLAGS_SF   = (1 << 7),
	NMD_X86_EFLAGS_ZF   = (1 << 6),
	NMD_X86_EFLAGS_AF   = (1 << 4),
	NMD_X86_EFLAGS_PF   = (1 << 2),
	NMD_X86_EFLAGS_CF   = (1 << 0)
};

enum NMD_X86_FPU_FLAGS
{
	NMD_X86_FPU_FLAGS_C0 = (1 << 8),
	NMD_X86_FPU_FLAGS_C1 = (1 << 9),
	NMD_X86_FPU_FLAGS_C2 = (1 << 10),
	NMD_X86_FPU_FLAGS_C3 = (1 << 14)
};

typedef struct nmd_x86_instruction
{
	bool valid : 1;                                         /* If true, the instruction is valid. */
	bool has_modrm : 1;                                     /* If true, the instruction has a ModR/M byte. */
	bool has_sib : 1;                                       /* If true, the instruction has an SIB byte. */
	bool has_rex : 1;                                       /* If true, the instruction has a REX prefix */
	bool rex_w_prefix : 1;                                  /* If true, a REX.W prefix is closer to the opcode than a operand size override prefix. */
	bool repeat_prefix : 1;                                 /* If true, a 'repeat'(F3h) prefix is closer to the opcode than a 'repeat not zero'(F2h) prefix. */
	uint8_t mode;                                           /* The decoding mode. A member of 'NMD_X86_MODE'. */
	uint8_t length;                                         /* The instruction's length in bytes. */
	uint8_t opcode;                                         /* Opcode byte. */
	uint8_t opcode_size;                                    /* The opcode's size in bytes. */
	uint16_t id;                                            /* The instruction's identifier. A member of 'NMD_X86_INSTRUCTION'. */
	uint16_t prefixes;                                      /* A mask of prefixes. See 'NMD_X86_PREFIXES'. */
	uint8_t num_prefixes;                                   /* Number of prefixes. */
	uint8_t num_operands;                                   /* The number of operands. */
	uint8_t group;                                          /* The instruction's group(e.g. jmp, prvileged...). A member of 'NMD_GROUP'. */
	uint8_t buffer[NMD_X86_MAXIMUM_INSTRUCTION_LENGTH];     /* A buffer containing the full instruction. */
	nmd_x86_operand operands[NMD_X86_MAXIMUM_NUM_OPERANDS]; /* Operands. */
	nmd_x86_modrm modrm;                                    /* The Mod/RM byte. Check 'flags.fields.has_modrm'. */
	nmd_x86_sib sib;                                        /* The SIB byte. Check 'flags.fields.has_sib'. */
	uint8_t imm_mask;                                       /* A mask of one or more members of 'NMD_X86_IMM'. */
	uint8_t disp_mask;                                      /* A mask of one or more members of 'NMD_X86_DISP'. */
	uint64_t immediate;                                     /* Immediate. Check 'imm_mask'. */
	uint32_t displacement;                                  /* Displacement. Check 'disp_mask'. */
	uint8_t opcode_map;                                     /* The instruction's opcode map. A member of 'NMD_X86_OPCODE_MAP'. */
	uint8_t encoding;                                       /* The instruction's encoding. A member of 'NMD_X86_INSTRUCTION_ENCODING'. */
	nmd_x86_vex vex;                                        /* VEX prefix. */
	nmd_x86_cpu_flags modified_flags;                       /* Cpu flags modified by the instruction. */
	nmd_x86_cpu_flags tested_flags;                         /* Cpu flags tested by the instruction. */
	nmd_x86_cpu_flags set_flags;                            /* Cpu flags set by the instruction. */
	nmd_x86_cpu_flags cleared_flags;                        /* Cpu flags cleared by the instruction. */
	nmd_x86_cpu_flags undefined_flags;                      /* Cpu flags whose state is undefined. */
	uint8_t rex;                                            /* REX prefix. */
	uint8_t segment_override;                               /* The segment override prefix closest to the opcode. A member of 'NMD_X86_PREFIXES'. */
	uint16_t simd_prefix;                                   /* One of these prefixes that is the closest to the opcode: NMD_X86_PREFIXES_OPERAND_SIZE_OVERRIDE, NMD_X86_PREFIXES_LOCK, NMD_X86_PREFIXES_REPEAT_NOT_ZERO, NMD_X86_PREFIXES_REPEAT, or NMD_X86_PREFIXES_NONE. The prefixes are specified as members of the 'NMD_X86_PREFIXES' enum. */
} nmd_x86_instruction;

typedef union nmd_x86_register
{
	int8_t  h8;
	int8_t  l8;
	int16_t l16;
	int32_t l32;
	int64_t l64;
} nmd_x86_register;

typedef union nmd_x86_register_512
{
	uint64_t xmm0[2];
	uint64_t ymm0[4];
	uint64_t zmm0[8];
} nmd_x86_register_512;

/*
Assembles one or more instructions from a string. Returns the number of bytes written to the buffer on success, zero otherwise. Instructions can be separated using the '\n'(new line) character.
Parameters:
 - string          [in]         A pointer to a string that represents one or more instructions in assembly language.
 - buffer          [out]        A pointer to a buffer that receives the encoded instructions.
 - buffer_size     [in]         The size of the buffer in bytes.
 - runtime_address [in]         The instruction's runtime address. You may use 'NMD_X86_INVALID_RUNTIME_ADDRESS'.
 - mode            [in]         The architecture mode. 'NMD_X86_MODE_32', 'NMD_X86_MODE_64' or 'NMD_X86_MODE_16'.
 - count           [in/out/opt] A pointer to a variable that on input is the maximum number of instructions that can be parsed, and on output the number of instructions parsed. This parameter may be null.
*/
NMD_ASSEMBLY_API size_t nmd_x86_assemble(const char* string, void* buffer, size_t buffer_size, uint64_t runtime_address, NMD_X86_MODE mode, size_t* count);

/*
Decodes an instruction. Returns true if the instruction is valid, false otherwise.
Parameters:
 - buffer      [in]  A pointer to a buffer containing a encoded instruction.
 - buffer_size [in]  The buffer's size in bytes.
 - instruction [out] A pointer to a variable of type 'nmd_x86_instruction' that receives information about the instruction.
 - mode        [in]  The architecture mode. 'NMD_X86_MODE_32', 'NMD_X86_MODE_64' or 'NMD_X86_MODE_16'.
 - flags       [in]  A mask of 'NMD_X86_DECODER_FLAGS_XXX' that specifies which features the decoder is allowed to use. If uncertain, use 'NMD_X86_DECODER_FLAGS_MINIMAL'.
*/
NMD_ASSEMBLY_API bool nmd_x86_decode(const void* buffer, size_t buffer_size, nmd_x86_instruction* instruction, NMD_X86_MODE mode, uint32_t flags);

/*
Formats an instruction. This function may cause a crash if you modify 'instruction' manually.
Parameters:
 - instruction     [in]  A pointer to a variable of type 'nmd_x86_instruction' describing the instruction to be formatted.
 - buffer          [out] A pointer to buffer that receives the string. The buffer's recommended size is 128 bytes.
 - runtime_address [in]  The instruction's runtime address. You may use 'NMD_X86_INVALID_RUNTIME_ADDRESS'.
 - flags           [in]  A mask of 'NMD_X86_FORMAT_FLAGS_XXX' that specifies how the function should format the instruction. If uncertain, use 'NMD_X86_FORMAT_FLAGS_DEFAULT'.
*/
NMD_ASSEMBLY_API void nmd_x86_format(const nmd_x86_instruction* instruction, char* buffer, uint64_t runtime_address, uint32_t flags);

/*
Returns the instruction's length if it's valid, zero otherwise.
Parameters:
 - buffer      [in] A pointer to a buffer containing a encoded instruction.
 - buffer_size [in] The buffer's size in bytes.
 - mode        [in] The architecture mode. 'NMD_X86_MODE_32', 'NMD_X86_MODE_64' or 'NMD_X86_MODE_16'.
*/
NMD_ASSEMBLY_API size_t nmd_x86_ldisasm(const void* buffer, size_t buffer_size, NMD_X86_MODE mode);

#endif /* NMD_ASSEMBLY_H */
