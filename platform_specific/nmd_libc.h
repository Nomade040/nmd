/* This is a platform independent C89 mini libc library.

Most of the functions were found somewhere in the internet. I just put them together.
I did however make the string and memory related functions.

Setup:
Define the 'NMD_LIBC_IMPLEMENTATION' macro in one source file before the include statement to instantiate the implementation.
#define NMD_LIBC_IMPLEMENTATION
#include "nmd_libc.h"

String related
 - int nmd_sprintf(char* buf, const char* fmt, ...)
 - int nmd_snprintf(char* buf, size_t n, const char* fmt, ...)
 - int nmd_vsprintf(char* buf, const char* fmt, va_list va)
 - int nmd_vsnprintf(char* buf, size_t n, const char* fmt, va_list va)

Memory related
 - void* nmd_malloc(size_t size)
 - void nmd_free(void* ptr)
 - void* nmd_memset(void* ptr, int value, size_t size)
 - void* nmd_memcpy(void* destination, void* source, size_t size)
 - size_t nmd_strlen(const char* str)

Math related
 - double nmd_sqrt(double x)
 - double nmd_scalbn(double x, int n)
 - double nmd_ldexp(double x, int n)
 - double nmd_frexp(double x, int* e)
 - double nmd_pow(double base, double exponent)
 - double nmd_ceil(double x)
 - float  nmd_ceilf(float x)
 - double nmd_floor(double x)
 - float  nmd_floorf(float x)
 - double nmd_fabs(double x)
 - double nmd_fmod(double numer, double denom)
 - double nmd_hypot(double x, double y)
 - double nmd_abs(double x)
 - float  nmd_fabs(float x)
 - double nmd_sin(double x)
 - float  nmd_sinf(float x)
 - double nmd_cos(double x)
 - float  nmd_cosf(float x)
 - float  nmd_asinf(float x)
 - float  nmd_acosf(float x)
 - double nmd_atan(double x)
 - double nmd_atan2(double y, double x)
 - float  nmd_atanf(float x)
 - float  nmd_atan2f(float y, float x)

TODO:
 - Add floating-point support for sprintf functions
 - Finish implementation of sub-specifier for the sprintf functions

Define the 'NMD_LIBC_DEFINE_INT_TYPES' macro to tell the library to define fixed width int types such as 'uint8_t', 'uint32_t', etc. Be aware, 
this feature uses platform dependent macros.

*/

#ifndef NMD_LIBC_H
#define NMD_LIBC_H

#ifdef NMD_LIBC_DEFINE_INT_TYPES

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

#else

/* Dependencies when 'NMD_LIBC_DEFINE_INT_TYPES' is not defined. */
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#endif /* NMD_LIBC_DEFINE_INT_TYPES */

/* Required for 'va_list', 'va_start()', 'va_arg()' and 'va_end()'. */
#include <stdarg.h>

#ifndef _NMD_GET_NUM_DIGITS
#define _NMD_GET_NUM_DIGITS _nmd_libc_get_num_digits
#endif /* _NMD_GET_NUM_DIGITS */

#ifndef _NMD_GET_NUM_DIGITS_HEX
#define _NMD_GET_NUM_DIGITS_HEX _nmd_libc_get_num_digits_hex
#endif /* _NMD_GET_NUM_DIGITS_HEX */

int nmd_sprintf(char* buf, const char* fmt, ...);
int nmd_snprintf(char* buf, size_t n, const char* fmt, ...);
int nmd_vsprintf(char* buf, const char* fmt, va_list va);
int nmd_vsnprintf(char* buf, size_t n, const char* fmt, va_list va);

void* nmd_malloc(size_t size);
void nmd_free(void* address);
void* nmd_memcpy(void* destination, void* source, size_t size);
size_t nmd_strlen(const char* str);

double nmd_abs(double x);
double nmd_sqrt(double x);
double nmd_scalbn(double x, int n);
double nmd_ldexp(double x, int n);
double nmd_frexp(double x, int* e);
double nmd_pow(double base, double exponent);
double nmd_fabs(double x);
double nmd_fmod(double numer, double denom);
double nmd_hypot(double x, double y);
double nmd_ceil(double x);
float nmd_ceilf(float x);
double nmd_floor(double x);
float nmd_floorf(float x);
double nmd_fabs(double x);
float nmd_fabsf(float x);
double nmd_sin(double x);
float nmd_sinf(float x);
double nmd_cos(double x);
float nmd_cosf(float x);
float nmd_asinf(float x);
float nmd_acosf(float x);
double nmd_atan(double x);
double nmd_atan2(double y, double x);
float nmd_atanf(float x);
float nmd_atan2f(float y, float x);

#define nmd_memset(ptr, value, num) { size_t _nmd_index = num-1; while(_nmd_index){((uint8_t*)ptr)[_nmd_index--] = value;}}
#ifdef NMD_LIBC_IMPLEMENTATION

size_t _nmd_libc_get_num_digits_hex(uint64_t n)
{
	size_t num_digits = 0;
	for (; n > 0; n /= 16)
		num_digits++;

	return num_digits;
}

size_t _nmd_libc_get_num_digits(uint64_t n)
{
	if (n == 0)
		return 1;

	size_t num_digits = 0;
	for (; n > 0; n /= 10)
		num_digits++;

	return num_digits;
}

size_t _nmd_libc_get_num_digits_octal(uint64_t n)
{
	size_t num_digits = 0;
	for (; n > 0; n /= 8)
		num_digits++;

	return num_digits;
}

int nmd_sprintf(char* buf, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	const int size = nmd_vsnprintf(buf, -1, fmt, va);
	va_end(va);

	return size;
}

int nmd_snprintf(char* buf, size_t n, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	const int size = nmd_vsnprintf(buf, n, fmt, va);
	va_end(va);

	return size;
}

int nmd_vsprintf(char* buf, const char* fmt, va_list va)
{
	return nmd_vsnprintf(buf, -1, fmt, va);
}

void _nmd_sprintf_append_uint(uint64_t x, char** p_buf, size_t* p_i, size_t n)
{
	size_t num_digits = _NMD_GET_NUM_DIGITS(x);
	const size_t offset = num_digits;

	char* buf = *p_buf;
	size_t i = *p_i;
		
	if (i + num_digits < n)
	{
		if (x == 0)
			*buf = '0';
		else
		{
			for (; x > 0; x /= 10)
				buf[--num_digits] = (char)('0' + x % 10);
		}
		*p_buf += offset;
	}

	*p_i += offset;
}

int nmd_vsnprintf(char* buf, size_t n, const char* fmt, va_list va)
{
	/* Return if 'n' is zero. Decrement 'n' because of the new line. */
	if (n-- == 0)
		return 0;

	/* 'i' is the length of the string constructed according to 'fmt'. It's not how many character have/will be written to 'buf'. */
	size_t i = 0;
	for (; *fmt; fmt++)
	{
		/* Format specifier handler */
		if (*fmt == '%')
		{
			fmt++;

			/* Parse flag sub-specifier */
			bool left_justify_flag = false;
			bool space_flag = false;
			bool force_sign_flag = false;
			bool prefix_flag = false;
			bool left_pad_flag = false;
			for (;; fmt++)
			{
				switch (*fmt)
				{
				case '-': left_justify_flag = true; continue; /* Left-justify */
				case '+': force_sign_flag = true; continue; /* Force sign */
				case ' ': space_flag = true; continue; /* Space */
				case '#': prefix_flag = true; continue; /* Prefix(0, 0x, 0X) */
				case '0': left_pad_flag = true; continue; /* Left-pad */
				}
				break; /* Stop parsing when an invalid flag is detected */
			}

			/* Parse padding sub-specifier */
			size_t width = 0;
			if (*fmt == '*') /* Width specified by argument */
			{
				width = va_arg(va, size_t);
				fmt++;
			}
			else
			{
				for (; *fmt >= '0' && *fmt <= '9'; fmt++)
				{
					width += (uint8_t)(*fmt - '0'); /* Add digit to the unit's place */
					width *= 10; /* Move all digits by one place to the left */
				}
			}

			/* Parse precision sub-specifier */
			size_t precision = 6; /* 6 is the default precision specified by the standard */
			if (*fmt == '.')
			{
				fmt++;

				if (*fmt == '*') /* Precision specified as argument */
				{
					precision = va_arg(va, size_t);
					fmt++;
				}
				else /* Precision specified in format string */
				{
					precision = 0;
					size_t place = 0;
					for (; *fmt >= '0' && *fmt <= '9'; fmt++, place++)
						precision += (uint8_t)(*fmt - '0') * nmd_pow(10, place);
				}
			}
			
			/* Parse length sub-specifier */
			enum NMD_LENGTH
			{
				NMD_LENGTH_NONE = 0,
				NMD_LENGTH_hh   = 1, /* hh */
				NMD_LENGTH_h    = 2, /* h  */
				NMD_LENGTH_l    = 3, /* l  */
				NMD_LENGTH_ll   = 4, /* ll */
				NMD_LENGTH_j    = 5, /* j  */
				NMD_LENGTH_z    = 6, /* z  */
				NMD_LENGTH_t    = 7, /* t  */
				NMD_LENGTH_L    = 8, /* L  */
			};
			uint8_t length = NMD_LENGTH_NONE;
			switch (*fmt)
			{
			case 'h': /* h or hh */
				if (fmt[1] == 'h')
				{
					length = NMD_LENGTH_hh;
					fmt += 2;
				}
				else
				{
					length = NMD_LENGTH_h;
					fmt++;
				}
				break;
			case 'l': /* l or ll */
				if (fmt[1] == 'l')
				{
					length = NMD_LENGTH_ll;
					fmt += 2;
				}
				else
				{
					length = NMD_LENGTH_l;
					fmt++;
				}
				break;
			case 'j': length = NMD_LENGTH_j; fmt++; break;
			case 'z': length = NMD_LENGTH_z; fmt++; break;
			case 't': length = NMD_LENGTH_t; fmt++; break;
			case 'L': length = NMD_LENGTH_L; fmt++; break;
			}

			if (*fmt == 'u' || *fmt == 'd' || *fmt == 'i') /* Signed/Unsigned decimal integer specifier */
			{
				uint32_t x;
				if (*fmt == 'd' || *fmt == 'i')
				{
					int32_t num = va_arg(va, int32_t);
					if (num < 0)
					{
						if (i < n)
							*buf++ = '-';
						i++;

						/* Convert to positive using two's complement */
						x = ~num + 1;
					}
					else
					{
						if (space_flag || force_sign_flag)
						{
							if (i < n)
								*buf++ = space_flag ? ' ' : '+';
							i++;
						}
						x = num;
					}
				}
				else
					x = va_arg(va, uint32_t);

				_nmd_sprintf_append_uint(x, &buf, &i, n);
			}
			else if (*fmt == 'x' || *fmt == 'X') /* Hexadecimal integer specifier(lowercase or uppercase) */
			{
				uint32_t x = va_arg(va, uint32_t);

				size_t num_digits = _NMD_GET_NUM_DIGITS_HEX(x);
				const size_t offset = num_digits;

				if (prefix_flag)
				{
					if (i + 2 <= n)
					{
						*buf++ = '0';
						*buf++ = *fmt;
					}
					i += 2;
				}

				if (i + num_digits <= n)
				{
					const char base_char = *fmt == 'x' ? 0x57 : 0x37;
					for (; x > 0; x /= 16)
					{
						size_t num = x % 16;
						buf[--num_digits] = (char)((num > 9 ? base_char : '0') + num);
					}
					buf += offset;
				}

				i += offset;
			}
			else if (*fmt == 'c') /* Chracter specifier */
			{
				const char c = va_arg(va, char);
				if(i < n)
					*buf++ = c;
				i++;
			}
			else if (*fmt == 's') /* String specifier */
			{
				const char* s = va_arg(va, const char*);
				for (; *s; s++, i++)
				{
					if (i < n)
						*buf++ = *s;
				}
			}
			else if (*fmt == 'p') /* Pointer address specifier */
			{
				uintptr_t x = va_arg(va, uintptr_t);

				if (i + sizeof(uintptr_t) * 2 <= n)
				{
					size_t num_chars = sizeof(uintptr_t) * 2;

					/* Add the address*/
					for (; x > 0; x /= 16)
					{
						size_t num = x % 16;
						buf[ --num_chars] = (char)((num > 9 ? 0x37 : '0') + num);
					}

					/* Add leading zeros */
					while(num_chars > 0)
						buf[--num_chars] = '0';

					buf += sizeof(uintptr_t) * 2;
				}

				i += sizeof(uintptr_t) * 2;
			}
			else if (*fmt == 'f' || *fmt == 'F') /* Floating-point specifier */
			{
				double x = va_arg(va, double);

				/* Add minus sign if negative */
				if((*(uint64_t*)(&x)) & ((uint64_t)1<<63))
				{
					if (i < n)
						*buf++ = '-';
					i++;
					x = -x;
				}

				/* Add integer part*/
				uint32_t int_part = (uint32_t)x;
				_nmd_sprintf_append_uint(int_part, &buf, &i, n);

				/* Add fractional part */
				if (precision > 0)
				{
					/* Add dot */
					if (i++ < n)
						*buf++ = '.';

					uint32_t fractional_part = (x - (double)int_part) * nmd_pow(10, precision);
					if (fractional_part == 0)
					{
						const size_t num = precision < (n - i) ? precision : (n - i);
						size_t j = 0;
						for (; j < num; j++)
							*buf++ = '0';
						i += precision;
					}
					else
						_nmd_sprintf_append_uint(fractional_part, &buf, &i, n);
				}
			}
			else if (*fmt == 'o') /* Unsigned octal specifier */
			{
				uint32_t x = va_arg(va, uint32_t);

				size_t num_digits = _nmd_libc_get_num_digits_octal(x);
				const size_t offset = num_digits;

				if (i + num_digits < n)
				{
					for (; x > 0; x /= 8)
						buf[--num_digits] = (char)('0' + x % 8);
					buf += offset;
				}

				i += offset;
			}
			else if (*fmt == '%')
			{
				if(i < n)
					*buf++ = '%';
				i++;
			}
			else if (*fmt == 'n') /* Number of characters written */
			{
				unsigned int* p_num_chars = va_arg(va, unsigned int*);
				*p_num_chars = i;
			}
			else
				fmt--;
		}
		else if(i++ < n) /* Check if we can add a character to 'buf', then increment 'i' */
			*buf++ = *fmt;
	}

	/* Place null terminator charater */
	*buf = '\0';

	return i;
}

#ifdef _WIN32
typedef struct _nmd_block_header
{
	/*
	Most significant bit indicates if the block is used(1) or not(0).
	'next' is null(not considering the most significant bit) if it's the last block in the linked list.
	*/
	struct _nmd_block_header* next; 

	size_t size;
} _nmd_block_header;

_nmd_block_header* _nmd_first_block = 0;

#define _NMD_IS_MOST_SIGNIFCANT_BIT_SET(x) (((uintptr_t)(x)) & (((uintptr_t)1)<<(sizeof(uintptr_t)*8-1)))
#define _NMD_SET_MOST_SIGNIFICANT_BIT(x) (((uintptr_t)(x))|(((uintptr_t)1)<<(sizeof(uintptr_t)*8-1)))
#define _NMD_CLEAR_MOST_SIGNIFICANT_BIT(x) (((uintptr_t)(x))&(~(((uintptr_t)1)<<(sizeof(uintptr_t)*8-1))))
#define _NMD_ROUND_POINTER_ALIGN(n) ((n) + sizeof(uintptr_t) - (((n)&(sizeof(uintptr_t)-1)) ? ((n) & (sizeof(uintptr_t)-1)) : sizeof(uintptr_t)))
/*#define _NMD_ROUND_PAGE_BOUNDARY(n) ((n) + 0x1000 - ((n) & 0xfff))*/

#pragma comment(lib, "ntdll.lib")
extern "C" uint32_t __stdcall NtAllocateVirtualMemory(void* ProcessHandle, void** BaseAddress, uint32_t ZeroBits, size_t* RegionSize, uint32_t AllocationType, uint32_t Protect);

_nmd_block_header* _nmd_alloc_region(size_t size)
{
	_nmd_block_header* block = 0;
	
	/* The requested size is 'size + sizeof(_nmd_block_header)' because the memory region must have at least one block header */
	size_t memory_region_size = size + sizeof(_nmd_block_header);
	if (NtAllocateVirtualMemory((void*)(-1), (void**)&block, NULL, &memory_region_size, 0x00002000 | 0x00001000/*MEM_RESERVE | MEM_COMMIT*/, 0x04/*PAGE_READWRITE*/))
		return 0;

	/* Check for a rare case where the memory region only has one block */
	if (memory_region_size - (size + sizeof(_nmd_block_header)) <= sizeof(_nmd_block_header))
	{
		/* Indicate that the next block must be in another memory region */
		block->next = (_nmd_block_header*)_NMD_SET_MOST_SIGNIFICANT_BIT(0);

		/* Give all the space available on this region to the block */
		block->size = memory_region_size - sizeof(_nmd_block_header);
	}
	else
	{
		/* Calculate the address of the next block in this memory region */
		_nmd_block_header* next_block = (_nmd_block_header*)((uint8_t*)block + sizeof(_nmd_block_header) + size);

		/* Set the remaining space available on this region to the next block */
		next_block->size = memory_region_size - (size + sizeof(_nmd_block_header));

		/* Link this block and the next block. Also set the most significant bit indicating that this block is not free */
		block->next = (_nmd_block_header*)_NMD_SET_MOST_SIGNIFICANT_BIT(next_block);

		/* Set the block size(with padding) */
		block->size = size;
	}	

	return block;
}

void* nmd_malloc(size_t size)
{
	/* Calculate the size of the memory block with padding */
	size = _NMD_ROUND_POINTER_ALIGN(size);

	/* Initialize a memory region when this function is called for the first time */
	if (!_nmd_first_block)
	{
		if (!(_nmd_first_block = _nmd_alloc_region(size)))
			return 0;
		return (uint8_t*)_nmd_first_block + sizeof(_nmd_block_header);
	}

	/* Try to find a free block which is big enough for the user */
	_nmd_block_header* block = _nmd_first_block;
	while (true)
	{
		/* Check if the block is free and if it is big enough */
		if (!_NMD_IS_MOST_SIGNIFCANT_BIT_SET(block->next) && size <= block->size)
		{
			/* Set the most significant bit indicating that this block is not free anymore */
			block->next = (_nmd_block_header*)_NMD_SET_MOST_SIGNIFICANT_BIT(block->next);

			return (uint8_t*)block + sizeof(_nmd_block_header);
		}

		/* Get the next block. Stop parsing if it's null */
		_nmd_block_header* next_block = (_nmd_block_header*)_NMD_CLEAR_MOST_SIGNIFICANT_BIT(block->next);
		if (!next_block)
			break;

		block = next_block;
	}

	/* There're no free blocks which are big enough. Allocate a new memory region */
	if(!(block->next = _nmd_alloc_region(size)))
		return 0;
	return (uint8_t*)block->next + sizeof(_nmd_block_header);
}

void nmd_free(void* ptr)
{
	if (!_nmd_first_block)
		return;

	_nmd_block_header* block = _nmd_first_block;
	_nmd_block_header* last_block = block;
	while (true)
	{
		/* Check if the block is used */
		if (_NMD_IS_MOST_SIGNIFCANT_BIT_SET(block->next) && (uint8_t*)block + sizeof(_nmd_block_header) == ptr)
		{
			/* Set the block as free */
			block->next = (_nmd_block_header*)_NMD_CLEAR_MOST_SIGNIFICANT_BIT(block->next);

			/* Check if the current block and the next block are adjacent and if the next block is free. If so, merge them */
			if ((uint8_t*)block->next == (uint8_t*)block + sizeof(_nmd_block_header) + block->size && !_NMD_IS_MOST_SIGNIFCANT_BIT_SET(block->next->next))
			{
				block->size += sizeof(_nmd_block_header) + block->next->size;
				block->next = (_nmd_block_header*)_NMD_CLEAR_MOST_SIGNIFICANT_BIT(block->next->next);
			}
			
			/* Check if the last block is free and if the current block and the last block are adjacent. If so, merge them */
			if (!_NMD_IS_MOST_SIGNIFCANT_BIT_SET(last_block->next) && (uint8_t*)last_block->next == (uint8_t*)last_block + sizeof(_nmd_block_header) + last_block->size)
			{
				last_block->size += sizeof(_nmd_block_header) + block->size;
				last_block->next = block->next;
			}

			return;
		}

		last_block = block;
		_nmd_block_header* next_block = (_nmd_block_header*)_NMD_CLEAR_MOST_SIGNIFICANT_BIT(block->next);
		if (!next_block)
			return;
		block = next_block;
	}
}

#endif

void* nmd_memcpy(void* destination, void* source, size_t size)
{
	size_t i = 0;
	for (; i < size; i++)
		((uint8_t*)destination)[i] = ((uint8_t*)source)[i];

	return destination;
}

size_t nmd_strlen(const char* str)
{
	const char* const str_begin = str;
	while (*str)
		str++;

	return (size_t)(str - str_begin);
}

double nmd_hypot(double x, double y)
{
	return nmd_sqrt(x * x + y * y);
}

#ifndef DBL_EPSILON
#define DBL_EPSILON 2.2204460492503131e-016
#endif

#if FLT_EVAL_METHOD==0 || FLT_EVAL_METHOD==1
#define EPS DBL_EPSILON
#elif FLT_EVAL_METHOD==2
#define EPS LDBL_EPSILON
#endif
static const double _nmd_toint = 1/EPS;

/* Taken from https://git.musl-libc.org/cgit/musl/tree/src/math/ceil.c */
double nmd_ceil(double x)
{
	union { double f; uint64_t i; } u = { x };
	int e = u.i >> 52 & 0x7ff;
	double y;

	if (e >= 0x3ff + 52 || x == 0)
		return x;
	/* y = int(x) - x, where int(x) is an integer neighbor of x */
	if (u.i >> 63)
		y = x - _nmd_toint + _nmd_toint - x;
	else
		y = x + _nmd_toint - _nmd_toint - x;
	/* special case because of non-nearest rounding modes */
	if (e <= 0x3ff - 1) {
		/*FORCE_EVAL(y);*/
		return u.i >> 63 ? -0.0 : 1;
	}
	if (y < 0)
		return x + y + 1;
	return x + y;
}

/* Taken from https://git.musl-libc.org/cgit/musl/tree/src/math/ceilf.c */
float nmd_ceilf(float x)
{
	union { float f; uint32_t i; } u = { x };
	int e = (int)(u.i >> 23 & 0xff) - 0x7f;
	uint32_t m;

	if (e >= 23)
		return x;
	if (e >= 0) {
		m = 0x007fffff >> e;
		if ((u.i & m) == 0)
			return x;
		/*FORCE_EVAL(x + 0x1p120f);*/
		if (u.i >> 31 == 0)
			u.i += m;
		u.i &= ~m;
	}
	else {
		/*FORCE_EVAL(x + 0x1p120f);*/
		if (u.i >> 31)
			u.f = -0.0;
		else if (u.i << 1)
			u.f = 1.0;
	}
	return u.f;
}

/* Taken from https://git.musl-libc.org/cgit/musl/tree/src/math/floor.c */
double nmd_floor(double x)
{
	union { double f; uint64_t i; } u = { x };
	int e = u.i >> 52 & 0x7ff;
	double y;

	if (e >= 0x3ff + 52 || x == 0)
		return x;
	/* y = int(x) - x, where int(x) is an integer neighbor of x */
	if (u.i >> 63)
		y = x - _nmd_toint + _nmd_toint - x;
	else
		y = x + _nmd_toint - _nmd_toint - x;
	/* special case because of non-nearest rounding modes */
	if (e <= 0x3ff - 1) {
		/*FORCE_EVAL(y);*/
		return u.i >> 63 ? -1 : 0;
	}
	if (y > 0)
		return x + y - 1;
	return x + y;
}

/* Taken from https://git.musl-libc.org/cgit/musl/tree/src/math/floorf.c */
float nmd_floorf(float x)
{
	union { float f; uint32_t i; } u = { x };
	int e = (int)(u.i >> 23 & 0xff) - 0x7f;
	uint32_t m;

	if (e >= 23)
		return x;
	if (e >= 0) {
		m = 0x007fffff >> e;
		if ((u.i & m) == 0)
			return x;
		/*FORCE_EVAL(x + 0x1p120f);*/
		if (u.i >> 31)
			u.i += m;
		u.i &= ~m;
	}
	else {
		/*FORCE_EVAL(x + 0x1p120f);*/
		if (u.i >> 31 == 0)
			u.i = 0;
		else if (u.i << 1)
			u.f = -1.0;
	}
	return u.f;
}

double nmd_abs(double x)
{
	return x >= 0 ? x : -x;
}

/*
float nmd_fabsf(float x)
{
	return x >= 0 ? x : -x;
}*/

/* Taken from https://git.musl-libc.org/cgit/musl/tree/src/math/scalbn.c */
double nmd_scalbn(double x, int n)
{
	union {double f; uint64_t i;} u;
	double y = x;

	if (n > 1023) {
		y *= 0x1p1023;
		n -= 1023;
		if (n > 1023) {
			y *= 0x1p1023;
			n -= 1023;
			if (n > 1023)
				n = 1023;
		}
	} else if (n < -1022) {
		/* make sure final n < -53 to avoid double
		   rounding in the subnormal range */
		y *= 0x1p-1022 * 0x1p53;
		n += 1022 - 53;
		if (n < -1022) {
			y *= 0x1p-1022 * 0x1p53;
			n += 1022 - 53;
			if (n < -1022)
				n = -1022;
		}
	}
	u.i = (uint64_t)(0x3ff+n)<<52;
	x = y * u.f;
	return x;
}

/* Taken from https://git.musl-libc.org/cgit/musl/tree/src/math/ldexp.c */
double nmd_ldexp(double x, int n)
{
	return nmd_scalbn(x, n);
}

/* Taken from https://git.musl-libc.org/cgit/musl/tree/src/math/frexp.c */
double nmd_frexp(double x, int* e)
{
	union { double d; uint64_t i; } y = { x };
	int ee = y.i >> 52 & 0x7ff;

	if (!ee) {
		if (x) {
			x = nmd_frexp(x * 0x1p64, e);
			*e -= 64;
		}
		else *e = 0;
		return x;
	}
	else if (ee == 0x7ff) {
		return x;
	}

	*e = ee - 0x3fe;
	y.i &= 0x800fffffffffffffull;
	y.i |= 0x3fe0000000000000ull;
	return y.d;
}

/* Taken from: https://www.codeproject.com/Articles/69941/Best-Square-Root-Method-Algorithm-Function-Precisi */
double nmd_sqrt(double x)
{
	if (x <= 0.0)
		return 0.0;

	float guess = 1;

	while (nmd_abs((guess * guess) / x - 1.0) >= 0.0001)
		guess = ((x / guess) + guess) / 2;

	return guess;
}

/* Taken from ftp://ftp.update.uu.se/pub/pdp11/rt/cmath/pow.c */
/* p o w . c

title	pow	x raised to power y
index	x raised to power y
usage
.s
double x, y, f, pow();
.br
f = pow(x, y);
.s
description
.s
Returns value of x raised to power y
.s
diagnostics
.s
There are three error possible error messages from this function.
.s
If the x argument is negative the message 'pow arg negative',
followed by the value of x, is written to stderr.The value
of pow for | x | is returned.
.s
If x = 0.0 and y <= 0.0 or if result overflows the message 'pow
overflow', followed by the value of y, is written to stderr.
The value of HUGE is returned.
.s
If the result underflows and if warnings are enabled(normally not),
the message 'pow underflow', followed by the value of y, is written
to stderr.The value of 0 is returned.
.s
The suggestion of Cody and Waite, that the domain be reduced to
simplify the overflow test, has been adopted, consequently overflow
is reported if the result would exceed HUGE * 2 * *(-1 / 16).
2 * *(-1 / 16) is approximately 0.9576.
.s
internal
.s
Algorithm from Cody and Waite pp. 84 - 124.  This algorithm required
two auxiliary programs POWGA1 and POWGA2 to calculate, respectively,
the arrays _nmd_a1[] and _nmd_a2[] used to represent the powers of 2 * *(-1 / 16)
to more than machine precision.
The source code for these programs are in the files POWGA1.AUXand
POWGA2.AUX.The octal table on page 98 of Cody and Waite is in the
file POWOCT.DAT which is required on stdin by POWGA2.
.s
author
.s
Hamish Ross.
.s
date
.s
27 - Jan - 85 */
#define _NMD_MAXEXP 2031		/* (MAX_EXP * 16) - 1			*/
#define _NMD_MINEXP -2047		/* (MIN_EXP * 16) - 1			*/

static double _nmd_a1[] = {
	1.0,
	0.95760328069857365,
	0.91700404320467123,
	0.87812608018664974,
	0.84089641525371454,
	0.80524516597462716,
	0.77110541270397041,
	0.73841307296974966,
	0.70710678118654752,
	0.67712777346844637,
	0.64841977732550483,
	0.62092890603674203,
	0.59460355750136054,
	0.56939431737834583,
	0.54525386633262883,
	0.52213689121370692,
	0.50000000000000000
};
static double _nmd_a2[] = {
	 0.24114209503420288E-17,
	 0.92291566937243079E-18,
	-0.15241915231122319E-17,
	-0.35421849765286817E-17,
	-0.31286215245415074E-17,
	-0.44654376565694490E-17,
	 0.29306999570789681E-17,
	 0.11260851040933474E-17
};
static double _nmd_p1 = 0.833333333333332114e-1;
static double _nmd_p2 = 0.125000000005037992e-1;
static double _nmd_p3 = 0.223214212859242590e-2;
static double _nmd_p4 = 0.434457756721631196e-3;
static double _nmd_q1 = 0.693147180559945296e0;
static double _nmd_q2 = 0.240226506959095371e0;
static double _nmd_q3 = 0.555041086640855953e-1;
static double _nmd_q4 = 0.961812905951724170e-2;
static double _nmd_q5 = 0.133335413135857847e-2;
static double _nmd_q6 = 0.154002904409897646e-3;
static double _nmd_q7 = 0.149288526805956082e-4;
static double _nmd_k = 0.442695040888963407;
double nmd_pow(double base, double exponent)
{
	double x = base;
	double y = exponent;
	
	double  g, r, u1, u2, v, w, w1, w2, y1, y2, z;
	int iw1, m, p;

	if (y == 0.0)
		return(1.0);
	if (x <= 0.0) {
		if (x == 0.0) {
			if (y > 0.0)
				return(x);
			//cmemsg(FP_POWO, &y);
			//return HUGE;
			return 1e300;
		}
		else {
			//cmemsg(FP_POWN, &x);
			x = -x;
		}
	}
	g = nmd_frexp(x, &m);
	p = 0;
	if (g <= _nmd_a1[8])
		p = 8;
	if (g <= _nmd_a1[p + 4])
		p += 4;
	if (g <= _nmd_a1[p + 2])
		p += 2;
	p++;
	z = ((g - _nmd_a1[p]) - _nmd_a2[p / 2]) / (g + _nmd_a1[p]);
	z += z;
	v = z * z;
	r = (((_nmd_p4 * v + _nmd_p3) * v + _nmd_p2) * v + _nmd_p1) * v * z;
	r += _nmd_k * r;
	u2 = (r + z * _nmd_k) + z;
	u1 = 0.0625 * (double)(16 * m - p);
	y1 = 0.0625 * (double)((int)(16.0 * y));
	y2 = y - y1;
	w = u2 * y + u1 * y2;
	w1 = 0.0625 * (double)((int)(16.0 * w));
	w2 = w - w1;
	w = w1 + u1 * y1;
	w1 = 0.0625 * (double)((int)(16.0 * w));
	w2 += (w - w1);
	w = 0.0625 * (double)((int)(16.0 * w2));
	iw1 = 16.0 * (w1 + w);
	w2 -= w;
	while (w2 > 0.0) {
		iw1++;
		w2 -= 0.0625;
	}
	if (iw1 > _NMD_MAXEXP) {
		//cmemsg(FP_POWO, &y);
		//return HUGE;
		return 1e300;
	}
	if (iw1 < _NMD_MINEXP) {
		//cmemsg(FP_POWU, &y);
		return 0.0;
	}
	m = iw1 / 16;
	if (iw1 >= 0)
		m++;
	p = 16 * m - iw1;
	z = ((((((_nmd_q7 * w2 + _nmd_q6) * w2 + _nmd_q5) * w2 + _nmd_q4) * w2 + _nmd_q3) * w2 + _nmd_q2) * w2 + _nmd_q1) * w2;
	z = _nmd_a1[p] + _nmd_a1[p] * z;
	return nmd_ldexp(z, m);
}

/* Taken from https://git.musl-libc.org/cgit/musl/tree/src/math/fabs.c */
double nmd_fabs(double x)
{
	union { double f; uint64_t i; } u = { x };
	u.i &= (uint64_t)(-1) / 2;
	return u.f;
}

#define _NMD_FP_NAN       0
#define _NMD_FP_INFINITE  1
#define _NMD_FP_ZERO      2
#define _NMD_FP_SUBNORMAL 3
#define _NMD_FP_NORMAL    4

int _nmd_fpclassify(double x)
{
	union { double f; uint64_t i; } u = { x };
	int e = u.i >> 52 & 0x7ff;
	if (!e) return u.i << 1 ? _NMD_FP_SUBNORMAL : _NMD_FP_ZERO;
	if (e == 0x7ff) return u.i << 12 ? _NMD_FP_NAN : _NMD_FP_INFINITE;
	return _NMD_FP_NORMAL;
}

bool _nmd_isnan(double x)
{
	return _nmd_fpclassify(x) == _NMD_FP_NAN;
}

/* Taken from https://git.musl-libc.org/cgit/musl/tree/src/math/fmod.c */
double nmd_fmod(double numer, double denom)
{
	double x = numer;
	double y = denom;

	union { double f; uint64_t i; } ux = { x }, uy = { y };
	int ex = ux.i >> 52 & 0x7ff;
	int ey = uy.i >> 52 & 0x7ff;
	int sx = ux.i >> 63;
	uint64_t i;

	/* in the followings uxi should be ux.i, but then gcc wrongly adds */
	/* float load/store to inner loops ruining performance and code size */
	uint64_t uxi = ux.i;

	if (uy.i << 1 == 0 || _nmd_isnan(y) || ex == 0x7ff)
		return (x * y) / (x * y);
	if (uxi << 1 <= uy.i << 1) {
		if (uxi << 1 == uy.i << 1)
			return 0 * x;
		return x;
	}

	/* normalize x and y */
	if (!ex) {
		for (i = uxi << 12; i >> 63 == 0; ex--, i <<= 1);
		uxi <<= -ex + 1;
	}
	else {
		uxi &= (uint64_t)(-1) >> 12;
		uxi |= 1ULL << 52;
	}
	if (!ey) {
		for (i = uy.i << 12; i >> 63 == 0; ey--, i <<= 1);
		uy.i <<= -ey + 1;
	}
	else {
		uy.i &= (uint64_t)(-1) >> 12;
		uy.i |= 1ULL << 52;
	}

	/* x mod y */
	for (; ex > ey; ex--) {
		i = uxi - uy.i;
		if (i >> 63 == 0) {
			if (i == 0)
				return 0 * x;
			uxi = i;
		}
		uxi <<= 1;
	}
	i = uxi - uy.i;
	if (i >> 63 == 0) {
		if (i == 0)
			return 0 * x;
		uxi = i;
	}
	for (; uxi >> 52 == 0; uxi <<= 1, ex--);

	/* scale result */
	if (ex > 0) {
		uxi -= 1ULL << 52;
		uxi |= (uint64_t)ex << 52;
	}
	else {
		uxi >>= -ex + 1;
	}
	uxi |= (uint64_t)sx << 63;
	ux.i = uxi;
	return ux.f;
}

/* Taken from https://web.eecs.utk.edu/~azh/blog/cosine.html */

//float _nmd_absf(float a) { *((unsigned long*)&a) &= ~(1UL << 31); return a; }
float _nmd_cos_table[] = { 1.0000000000f,0.9999500004f,0.9998000067f,0.9995500337f,0.9992001067f,0.9987502604f,0.9982005399f,0.9975510003f,0.9968017063f,0.9959527330f,0.9950041653f,0.9939560980f,0.9928086359f,0.9915618937f,0.9902159962f,0.9887710779f,0.9872272834f,0.9855847669f,0.9838436928f,0.9820042351f,0.9800665778f,0.9780309147f,0.9758974493f,0.9736663950f,0.9713379749f,0.9689124217f,0.9663899781f,0.9637708964f,0.9610554383f,0.9582438755f,0.9553364891f,0.9523335699f,0.9492354181f,0.9460423435f,0.9427546655f,0.9393727128f,0.9358968237f,0.9323273456f,0.9286646356f,0.9249090599f,0.9210609940f,0.9171208228f,0.9130889403f,0.9089657497f,0.9047516632f,0.9004471024f,0.8960524975f,0.8915682882f,0.8869949228f,0.8823328586f,0.8775825619f,0.8727445076f,0.8678191797f,0.8628070705f,0.8577086814f,0.8525245221f,0.8472551110f,0.8419009752f,0.8364626499f,0.8309406791f,0.8253356149f,0.8196480178f,0.8138784567f,0.8080275083f,0.8020957579f,0.7960837985f,0.7899922315f,0.7838216659f,0.7775727188f,0.7712460150f,0.7648421873f,0.7583618760f,0.7518057291f,0.7451744023f,0.7384685587f,0.7316888689f,0.7248360107f,0.7179106696f,0.7109135380f,0.7038453157f,0.6967067093f,0.6894984330f,0.6822212073f,0.6748757601f,0.6674628258f,0.6599831459f,0.6524374682f,0.6448265472f,0.6371511442f,0.6294120266f,0.6216099683f,0.6137457495f,0.6058201566f,0.5978339823f,0.5897880250f,0.5816830895f,0.5735199861f,0.5652995312f,0.5570225468f,0.5486898606f,0.5403023059f,0.5318607214f,0.5233659513f,0.5148188450f,0.5062202572f,0.4975710479f,0.4888720819f,0.4801242290f,0.4713283642f,0.4624853669f,0.4535961214f,0.4446615167f,0.4356824463f,0.4266598079f,0.4175945040f,0.4084874409f,0.3993395294f,0.3901516843f,0.3809248244f,0.3716598723f,0.3623577545f,0.3530194012f,0.3436457463f,0.3342377271f,0.3247962844f,0.3153223624f,0.3058169084f,0.2962808729f,0.2867152096f,0.2771208751f,0.2674988286f,0.2578500325f,0.2481754517f,0.2384760534f,0.2287528078f,0.2190066871f,0.2092386659f,0.1994497210f,0.1896408313f,0.1798129777f,0.1699671429f,0.1601043116f,0.1502254699f,0.1403316058f,0.1304237087f,0.1205027694f,0.1105697798f,0.1006257334f,0.0906716245f,0.0807084485f,0.0707372017f,0.0607588812f,0.0507744849f,0.0407850112f,0.0307914591f,0.0207948278f,0.0107961171f,0.0007963267f,-0.0092035433f,-0.0192024929f,-0.0291995223f,-0.0391936318f,-0.0491838219f,-0.0591690937f,-0.0691484487f,-0.0791208888f,-0.0890854169f,-0.0990410366f,-0.1089867522f,-0.1189215693f,-0.1288444943f,-0.1387545350f,-0.1486507003f,-0.1585320006f,-0.1683974479f,-0.1782460556f,-0.1880768389f,-0.1978888146f,-0.2076810016f,-0.2174524207f,-0.2272020947f,-0.2369290487f,-0.2466323100f,-0.2563109082f,-0.2659638756f,-0.2755902468f,-0.2851890592f,-0.2947593530f,-0.3043001711f,-0.3138105594f,-0.3232895669f,-0.3327362457f,-0.3421496512f,-0.3515288419f,-0.3608728801f,-0.3701808314f,-0.3794517648f,-0.3886847534f,-0.3978788738f,-0.4070332067f,-0.4161468365f,-0.4252188521f,-0.4342483461f,-0.4432344157f,-0.4521761621f,-0.4610726914f,-0.4699231137f,-0.4787265442f,-0.4874821023f,-0.4961889127f,-0.5048461046f,-0.5134528123f,-0.5220081752f,-0.5305113376f,-0.5389614494f,-0.5473576655f,-0.5556991463f,-0.5639850576f,-0.5722145709f,-0.5803868632f,-0.5885011173f,-0.5965565217f,-0.6045522711f,-0.6124875657f,-0.6203616120f,-0.6281736227f,-0.6359228166f,-0.6436084187f,-0.6512296605f,-0.6587857799f,-0.6662760213f,-0.6736996356f,-0.6810558805f,-0.6883440204f,-0.6955633265f,-0.7027130768f,-0.7097925564f,-0.7168010573f,-0.7237378787f,-0.7306023269f,-0.7373937155f,-0.7441113654f,-0.7507546047f,-0.7573227692f,-0.7638152021f,-0.7702312540f,-0.7765702835f,-0.7828316566f,-0.7890147472f,-0.7951189370f,-0.8011436155f,-0.8070881804f,-0.8129520371f,-0.8187345993f,-0.8244352887f,-0.8300535352f,-0.8355887771f,-0.8410404608f,-0.8464080412f,-0.8516909815f,-0.8568887534f,-0.8620008371f,-0.8670267214f,-0.8719659039f,-0.8768178904f,-0.8815821959f,-0.8862583439f,-0.8908458668f,-0.8953443058f,-0.8997532112f,-0.9040721420f,-0.9083006664f,-0.9124383614f,-0.9164848133f,-0.9204396176f,-0.9243023786f,-0.9280727102f,-0.9317502353f,-0.9353345861f,-0.9388254043f,-0.9422223407f,-0.9455250556f,-0.9487332188f,-0.9518465095f,-0.9548646164f,-0.9577872376f,-0.9606140808f,-0.9633448634f,-0.9659793124f,-0.9685171642f,-0.9709581651f,-0.9733020711f,-0.9755486476f,-0.9776976700f,-0.9797489236f,-0.9817022030f,-0.9835573130f,-0.9853140682f,-0.9869722927f,-0.9885318208f,-0.9899924966f,-0.9913541739f,-0.9926167167f,-0.9937799986f,-0.9948439034f,-0.9958083245f,-0.9966731657f,-0.9974383404f,-0.9981037721f,-0.9986693942f,-0.9991351503f,-0.9995009936f,-0.9997668877f,-0.9999328059f,-0.9999987317f,-0.9999646585f,-0.9998305896f,-0.9995965385f,-0.9992625285f,-0.9988285932f,-0.9982947758f,-0.9976611298f,-0.9969277185f,-0.9960946152f,-0.9951619033f,-0.9941296761f,-0.9929980367f,-0.9917670983f,-0.9904369841f,-0.9890078270f,-0.9874797699f,-0.9858529657f,-0.9841275770f,-0.9823037763f,-0.9803817461f,-0.9783616786f,-0.9762437757f,-0.9740282492f,-0.9717153207f,-0.9693052215f,-0.9667981926f,-0.9641944846f,-0.9614943581f,-0.9586980828f,-0.9558059386f,-0.9528182146f,-0.9497352095f,-0.9465572318f,-0.9432845990f,-0.9399176387f,-0.9364566873f,-0.9329020910f,-0.9292542053f,-0.9255133950f,-0.9216800341f,-0.9177545060f,-0.9137372031f,-0.9096285274f,-0.9054288895f,-0.9011387095f,-0.8967584163f,-0.8922884481f,-0.8877292518f,-0.8830812833f,-0.8783450074f,-0.8735208977f,-0.8686094366f,-0.8636111154f,-0.8585264337f,-0.8533559002f,-0.8481000317f,-0.8427593540f,-0.8373344010f,-0.8318257152f,-0.8262338476f,-0.8205593573f,-0.8148028118f,-0.8089647866f,-0.8030458657f,-0.7970466408f,-0.7909677119f,-0.7848096869f,-0.7785731816f,-0.7722588196f,-0.7658672324f,-0.7593990591f,-0.7528549466f,-0.7462355491f,-0.7395415288f,-0.7327735549f,-0.7259323042f,-0.7190184609f,-0.7120327164f,-0.7049757692f,-0.6978483250f,-0.6906510966f,-0.6833848036f,-0.6760501727f,-0.6686479374f,-0.6611788378f,-0.6536436209f,-0.6460430401f,-0.6383778556f,-0.6306488339f,-0.6228567478f,-0.6150023765f,-0.6070865055f,-0.5991099264f,-0.5910734368f,-0.5829778403f,-0.5748239465f,-0.5666125708f,-0.5583445344f,-0.5500206639f,-0.5416417918f,-0.5332087560f,-0.5247223998f,-0.5161835718f,-0.5075931258f,-0.4989519210f,-0.4902608213f,-0.4815206960f,-0.4727324191f,-0.4638968693f,-0.4550149301f,-0.4460874899f,-0.4371154413f,-0.4280996815f,-0.4190411121f,-0.4099406390f,-0.4007991721f,-0.3916176256f,-0.3823969177f,-0.3731379704f,-0.3638417097f,-0.3545090650f,-0.3451409698f,-0.3357383608f,-0.3263021782f,-0.3168333656f,-0.3073328700f,-0.2978016413f,-0.2882406328f,-0.2786508004f,-0.2690331031f,-0.2593885028f,-0.2497179638f,-0.2400224533f,-0.2303029407f,-0.2205603980f,-0.2107957994f,-0.2010101215f,-0.1912043427f,-0.1813794436f,-0.1715364067f,-0.1616762164f,-0.1517998585f,-0.1419083208f,-0.1320025924f,-0.1220836638f,-0.1121525269f,-0.1022101749f,-0.0922576020f,-0.0822958034f,-0.0723257753f,-0.0623485146f,-0.0523650192f,-0.0423762873f,-0.0323833178f,-0.0223871100f,-0.0123886635f,-0.0023889781f,0.0076109461f,0.0176101093f,0.0276075115f,0.0376021529f,0.0475930341f,0.0575791561f,0.0675595202f,0.0775331285f,0.0874989834f,0.0974560886f,0.1074034482f,0.1173400676f,0.1272649530f,0.1371771121f,0.1470755536f,0.1569592876f,0.1668273259f,0.1766786815f,0.1865123694f,0.1963274063f,0.2061228105f,0.2158976027f,0.2256508053f,0.2353814430f,0.2450885427f,0.2547711338f,0.2644282480f,0.2740589195f,0.2836621855f,0.2932370854f,0.3027826619f,0.3122979603f,0.3217820292f,0.3312339202f,0.3406526881f,0.3500373910f,0.3593870904f,0.3687008515f,0.3779777427f,0.3872168365f,0.3964172089f,0.4055779400f,0.4146981136f,0.4237768177f,0.4328131445f,0.4418061903f,0.4507550559f,0.4596588463f,0.4685166713f,0.4773276450f,0.4860908863f,0.4948055189f,0.5034706714f,0.5120854772f,0.5206490750f,0.5291606082f,0.5376192258f,0.5460240820f,0.5543743362f,0.5626691534f,0.5709077042f,0.5790891647f,0.5872127167f,0.5952775480f,0.6032828520f,0.6112278282f,0.6191116822f,0.6269336255f,0.6346928759f,0.6423886576f,0.6500202010f,0.6575867429f,0.6650875267f,0.6725218022f,0.6798888262f,0.6871878618f,0.6944181793f,0.7015790554f,0.7086697743f,0.7156896268f,0.7226379109f,0.7295139318f,0.7363170019f,0.7430464410f,0.7497015760f,0.7562817415f,0.7627862794f,0.7692145394f,0.7755658785f,0.7818396617f,0.7880352616f,0.7941520586f,0.8001894411f,0.8061468053f,0.8120235554f,0.8178191040f,0.8235328712f,0.8291642859f,0.8347127848f,0.8401778132f,0.8455588245f,0.8508552806f,0.8560666518f,0.8611924172f,0.8662320640f,0.8711850883f,0.8760509948f,0.8808292970f,0.8855195169f,0.8901211857f,0.8946338431f,0.8990570378f,0.9033903276f,0.9076332791f,0.9117854680f,0.9158464792f,0.9198159064f,0.9236933529f,0.9274784307f,0.9311707615f,0.9347699760f,0.9382757143f,0.9416876258f,0.9450053693f,0.9482286131f,0.9513570348f,0.9543903216f,0.9573281701f,0.9601702867f,0.9629163869f,0.9655661964f,0.9681194501f,0.9705758926f,0.9729352783f,0.9751973713f,0.9773619454f,0.9794287841f,0.9813976807f,0.9832684384f,0.9850408701f,0.9867147985f,0.9882900563f,0.9897664858f,0.9911439396f,0.9924222797f,0.9936013785f,0.9946811180f,0.9956613902f,0.9965420970f,0.9973231505f,0.9980044725f,0.9985859949f,0.9990676595f,0.9994494182f,0.9997312328f,0.9999130751f,0.9999949269f,1.0f };

#define _NMD_PI  3.14159265358979323
#define _NMD_PIF 3.1415926535f

#define _NMD_HALF_PI 1.5707963267948966
#define _NMD_HALF_PIF 1.5707963267f

#define _NMD_TWO_PI  6.283185307179586
#define _NMD_TWO_PIF 6.2831853071f

#define _NMD_MOD(x, y) ((x) - (int)((x) / (y)) * (y))
#define _NMD_LERP(w, v1, v2) ((1.0 - (w)) * (v1) + (w) * (v2))

double nmd_sin(double x)
{
	return nmd_cos(x - _NMD_HALF_PI);
}

float nmd_sinf(float x)
{
	return nmd_cosf(x - _NMD_HALF_PIF);
}

double nmd_cos(double x)
{
	x = _NMD_MOD(x, _NMD_TWO_PI);
	char sign = 1;
	if (x > _NMD_PI)
	{
		x -= _NMD_PI;
		sign = -1;
	}
	double xx = x * x;

	return sign * (1 - ((xx) / (2)) + ((xx * xx) / (24)) - ((xx * xx * xx) / (720)) + ((xx * xx * xx * xx) / (40320)) - ((xx * xx * xx * xx * xx) / (3628800)) + ((xx * xx * xx * xx * xx * xx) / (479001600)) - ((xx * xx * xx * xx * xx * xx * xx) / (87178291200)) + ((xx * xx * xx * xx * xx * xx * xx * xx) / (20922789888000)) - ((xx * xx * xx * xx * xx * xx * xx * xx * xx) / (6402373705728000)) + ((xx * xx * xx * xx * xx * xx * xx * xx * xx * xx) / (2432902008176640000)));
}

float nmd_cosf(float x)
{
	/* Implementation without interpolation. Faster but less accurate
	x = nmd_fabs(x);
	x = _NMD_MOD(x, _NMD_TWO_PIF);
	return _nmd_cos_table[(int)(x * 100 + 0.5)];
	*/

	x = nmd_fabs(x);
	x = _NMD_MOD(x, _NMD_TWO_PIF);
	float i = x * 100.0;
	int index = (int)i;
	return _NMD_LERP(i - index, _nmd_cos_table[index], _nmd_cos_table[index + 1]);
}

/* Taken from https://developer.download.nvidia.com/cg/asin.html */
float nmd_asinf(float x)
{
	float negate = float(x < 0);
	x = nmd_fabs(x);
	float ret = -0.0187293;
	ret *= x;
	ret += 0.0742610;
	ret *= x;
	ret -= 0.2121144;
	ret *= x;
	ret += 1.5707288;
	ret = 3.14159265358979 * 0.5 - nmd_sqrt(1.0 - x) * ret;
	return ret - 2 * negate * ret;
}

/* Taken from https://developer.download.nvidia.com/cg/acos.html */
float nmd_acosf(float x)
{
	float negate = float(x < 0);
	x = nmd_fabs(x);
	float ret = -0.0187293;
	ret = ret * x;
	ret = ret + 0.0742610;
	ret = ret * x;
	ret = ret - 0.2121144;
	ret = ret * x;
	ret = ret + 1.5707288;
	ret = ret * nmd_sqrt(1.0 - x);
	ret = ret - 2 * negate * ret;
	return negate * 3.14159265358979 + ret;
}

/* Implementation taken from https://opensource.apple.com/source/Libm/Libm-315/Source/Intel/nmd_atan.c */
/*	Declare certain constants volatile to force the compiler to access them
	when we reference them.  This in turn forces arithmetic operations on them
	to be performed at run time (or as if at run time).  We use such operations
	to generate exceptions such as underflow or inexact.
*/
static volatile const double _nmd_Tiny = 0x1p-1022;

/*	double nmd_atan(double x).

	(This routine appears below, following subroutines.)

	Notes:

		Citations in parentheses below indicate the source of a requirement.

		"C" stands for ISO/IEC 9899:TC2.

		The Open Group specification (IEEE Std 1003.1, 2004 edition) adds no
		requirements since it defers to C and requires errno behavior only if
		we choose to support it by arranging for "math_errhandling &
		MATH_ERRNO" to be non-zero, which we do not.

	Return value:

		For arctangent of +/- zero, return zero with same sign (C F.9 12 and
		F.9.1.3).

		For arctangent of +/- infinity, return +/- pi/2 (C F.9.1.3).

		For a NaN, return the same NaN (C F.9 11 and 13).  (If the NaN is a
		signalling NaN, we return the "same" NaN quieted.)

		Otherwise:

			If the rounding mode is round-to-nearest, return arctangent(x)
			faithfully rounded.  This is not proven but seems likely.
			Generally, the largest source of errors is the evaluation of the
			polynomial using double precision.  Some analysis might bound this
			and prove faithful rounding.  The largest observed error is .814
			ULP.

			Return a value in [-pi/2, +pi/2] (C 7.12.4.3 3).
		
			Not implemented:  In other rounding modes, return arctangent(x)
			possibly with slightly worse error, not necessarily honoring the
			rounding mode (Ali Sazegari narrowing C F.9 10).

	Exceptions:

		Raise underflow for a denormal result (C F.9 7 and Draft Standard for
		Floating-Point Arithmetic P754 Draft 1.2.5 9.5).  If the input is the
		smallest normal, underflow may or may not be raised.  This is stricter
		than the older 754 standard.

		May or may not raise inexact, even if the result is exact (C F.9 8).

		Raise invalid if the input is a signalling NaN (C 5.2.4.2.2 3, in spite
		of C 4.2.1), but not if the input is a quiet NaN (C F.9 11).

		May not raise exceptions otherwise (C F.9 9).

	Properties:

		Not proven:  Monotonic.
*/


// Return arctangent(x) given that 2 < x, with the same properties as nmd_atan.
static double _nmd_Tail(double x)
{
	{
		static const double HalfPi = 0x3.243f6a8885a308d313198a2e037ap-1;

		// For large x, generate inexact and return pi/2.
		if (0x1p53 <= x)
			return HalfPi + _nmd_Tiny;
		
		if (_nmd_isnan(x))
			return x - x;
	}

	static const double p03 = -0x1.5555555554A51p-2;
	static const double p05 = +0x1.999999989EBCAp-3;
	static const double p07 = -0x1.249248E1422E3p-3;
	static const double p09 = +0x1.C71C5EDFED480p-4;
	static const double p11 = -0x1.745B7F2D72663p-4;
	static const double p13 = +0x1.3AFD7A0E6EB75p-4;
	static const double p15 = -0x1.104146B1A1AE8p-4;
	static const double p17 = +0x1.D78252FA69C1Cp-5;
	static const double p19 = -0x1.81D33E401836Dp-5;
	static const double p21 = +0x1.007733E06CEB3p-5;
	static const double p23 = -0x1.83DAFDA7BD3FDp-7;

	static const double p000 = +0x1.921FB54442D18p0;
	static const double p001 = +0x1.1A62633145C07p-54;

	double y = 1/x;

	// Square y.
	double y2 = y * y;

	return p001 - ((((((((((((
		+ p23) * y2
		+ p21) * y2
		+ p19) * y2
		+ p17) * y2
		+ p15) * y2
		+ p13) * y2
		+ p11) * y2
		+ p09) * y2
		+ p07) * y2
		+ p05) * y2
		+ p03) * y2 * y + y) + p000;
}


/*	Return arctangent(x) given that 0x1p-27 < |x| <= 1/2, with the same
	properties as nmd_atan.
*/
static double _nmd_atani0(double x)
{
	static const double p03 = -0x1.555555555551Bp-2;
	static const double p05 = +0x1.99999999918D8p-3;
	static const double p07 = -0x1.2492492179CA3p-3;
	static const double p09 = +0x1.C71C7096C2725p-4;
	static const double p11 = -0x1.745CF51795B21p-4;
	static const double p13 = +0x1.3B113F18AC049p-4;
	static const double p15 = -0x1.10F31279EC05Dp-4;
	static const double p17 = +0x1.DFE7B9674AE37p-5;
	static const double p19 = -0x1.A38CF590469ECp-5;
	static const double p21 = +0x1.56CDB5D887934p-5;
	static const double p23 = -0x1.C0EB85F543412p-6;
	static const double p25 = +0x1.4A9F5C4724056p-7;

	// Square x.
	double x2 = x * x;

	return ((((((((((((
		+ p25) * x2
		+ p23) * x2
		+ p21) * x2
		+ p19) * x2
		+ p17) * x2
		+ p15) * x2
		+ p13) * x2
		+ p11) * x2
		+ p09) * x2
		+ p07) * x2
		+ p05) * x2
		+ p03) * x2 * x + x;
}


/*	Return arctangent(x) given that 1/2 < x <= 3/4, with the same properties as
	nmd_atan.
*/
static double _nmd_atani1(double x)
{
	static const double p00 = +0x1.1E00BABDEFED0p-1;
	static const double p01 = +0x1.702E05C0B8155p-1;
	static const double p02 = -0x1.4AF2B78215A1Bp-2;
	static const double p03 = +0x1.5D0B7E9E69054p-6;
	static const double p04 = +0x1.A1247CA5D9475p-4;
	static const double p05 = -0x1.519E110F61B54p-4;
	static const double p06 = +0x1.A759263F377F2p-7;
	static const double p07 = +0x1.094966BE2B531p-5;
	static const double p08 = -0x1.09BC0AB7F914Cp-5;
	static const double p09 = +0x1.FF3B7C531AA4Ap-8;
	static const double p10 = +0x1.950E69DCDD967p-7;
	static const double p11 = -0x1.D88D31ABC3AE5p-7;
	static const double p12 = +0x1.10F3E20F6A2E2p-8;

	double y = x - 0x1.4000000000027p-1;

	return ((((((((((((
		+ p12) * y
		+ p11) * y
		+ p10) * y
		+ p09) * y
		+ p08) * y
		+ p07) * y
		+ p06) * y
		+ p05) * y
		+ p04) * y
		+ p03) * y
		+ p02) * y
		+ p01) * y
		+ p00;
}


/*	Return arctangent(x) given that 3/4 < x <= 1, with the same properties as
	nmd_atan.
*/
static double _nmd_atani2(double x)
{
	static const double p00 = +0x1.700A7C580EA7Ep-01;
	static const double p01 = +0x1.21FB781196AC3p-01;
	static const double p02 = -0x1.1F6A8499714A2p-02;
	static const double p03 = +0x1.41B15E5E8DCD0p-04;
	static const double p04 = +0x1.59BC93F81895Ap-06;
	static const double p05 = -0x1.63B543EFFA4EFp-05;
	static const double p06 = +0x1.C90E92AC8D86Cp-06;
	static const double p07 = -0x1.91F7E2A7A338Fp-08;
	static const double p08 = -0x1.AC1645739E676p-08;
	static const double p09 = +0x1.152311B180E6Cp-07;
	static const double p10 = -0x1.265EF51B17DB7p-08;
	static const double p11 = +0x1.CA7CDE5DE9BD7p-14;

	double y = x - 0x1.c0000000f4213p-1;

	return (((((((((((
		+ p11) * y
		+ p10) * y
		+ p09) * y
		+ p08) * y
		+ p07) * y
		+ p06) * y
		+ p05) * y
		+ p04) * y
		+ p03) * y
		+ p02) * y
		+ p01) * y
		+ p00;
}


/*	Return arctangent(x) given that 1 < x <= 4/3, with the same properties as
	nmd_atan.
*/
static double _nmd_atani3(double x)
{
	static const double p00 = +0x1.B96E5A78C5C40p-01;
	static const double p01 = +0x1.B1B1B1B1B1B3Dp-02;
	static const double p02 = -0x1.AC97826D58470p-03;
	static const double p03 = +0x1.3FD2B9F586A67p-04;
	static const double p04 = -0x1.BC317394714B7p-07;
	static const double p05 = -0x1.2B01FC60CC37Ap-07;
	static const double p06 = +0x1.73A9328786665p-07;
	static const double p07 = -0x1.C0B993A09CE31p-08;
	static const double p08 = +0x1.2FCDACDD6E5B5p-09;
	static const double p09 = +0x1.CBD49DA316282p-13;
	static const double p10 = -0x1.0120E602F6336p-10;
	static const double p11 = +0x1.A89224FF69018p-11;
	static const double p12 = -0x1.883D8959134B3p-12;

	double y = x - 0x1.2aaaaaaaaaa96p0;

	return ((((((((((((
		+ p12) * y
		+ p11) * y
		+ p10) * y
		+ p09) * y
		+ p08) * y
		+ p07) * y
		+ p06) * y
		+ p05) * y
		+ p04) * y
		+ p03) * y
		+ p02) * y
		+ p01) * y
		+ p00;
}


/*	Return arctangent(x) given that 4/3 < x <= 5/3, with the same properties as
	nmd_atan.
*/
static double _nmd_atani4(double x)
{
	static const double p00 = +0x1.F730BD281F69Dp-01;
	static const double p01 = +0x1.3B13B13B13B0Cp-02;
	static const double p02 = -0x1.22D719C06115Ep-03;
	static const double p03 = +0x1.C963C83985742p-05;
	static const double p04 = -0x1.135A0938EC462p-06;
	static const double p05 = +0x1.13A254D6E5B7Cp-09;
	static const double p06 = +0x1.DFAA5E77B7375p-10;
	static const double p07 = -0x1.F4AC1342182D2p-10;
	static const double p08 = +0x1.25BAD4D85CBE1p-10;
	static const double p09 = -0x1.E4EEF429EB680p-12;
	static const double p10 = +0x1.B4E30D1BA3819p-14;
	static const double p11 = +0x1.0280537F097F3p-15;

	double y = x - 0x1.8000000000003p0;

	return (((((((((((
		+ p11) * y
		+ p10) * y
		+ p09) * y
		+ p08) * y
		+ p07) * y
		+ p06) * y
		+ p05) * y
		+ p04) * y
		+ p03) * y
		+ p02) * y
		+ p01) * y
		+ p00;
}

/*	Return arctangent(x) given that 5/3 < x <= 2, with the same properties as
	nmd_atan.
*/
static double _nmd_atani5(double x)
{
	static const double p00 = +0x1.124A85750FB5Cp+00;
	static const double p01 = +0x1.D59AE78C11C49p-03;
	static const double p02 = -0x1.8AD3C44F10DC3p-04;
	static const double p03 = +0x1.2B090AAD5F9DCp-05;
	static const double p04 = -0x1.881EC3D15241Fp-07;
	static const double p05 = +0x1.8CB82A74E0699p-09;
	static const double p06 = -0x1.3182219E21362p-12;
	static const double p07 = -0x1.2B9AD13DB35A8p-12;
	static const double p08 = +0x1.10F884EAC0E0Ap-12;
	static const double p09 = -0x1.3045B70E93129p-13;
	static const double p10 = +0x1.00B6A460AC05Dp-14;

	double y = x - 0x1.d555555461337p0;

	return ((((((((((
		+ p10) * y
		+ p09) * y
		+ p08) * y
		+ p07) * y
		+ p06) * y
		+ p05) * y
		+ p04) * y
		+ p03) * y
		+ p02) * y
		+ p01) * y
		+ p00;
}


// See documentation above.
double nmd_atan(double x)
{
	if (x < 0)
		if (x < -1)
			if (x < -5/3.)
				if (x < -2)
					return -_nmd_Tail(-x);
				else
					return -_nmd_atani5(-x);
			else
				if (x < -4/3.)
					return -_nmd_atani4(-x);
				else
					return -_nmd_atani3(-x);
		else
			if (x < -.5)
				if (x < -.75)
					return -_nmd_atani2(-x);
				else
					return -_nmd_atani1(-x);
			else
				if (x < -0x1.d12ed0af1a27fp-27)
					return _nmd_atani0(x);
				else
					if (x <= -0x1p-1022)
						// Generate inexact and return x.
						return (_nmd_Tiny + 1) * x;
					else
						if (x == 0)
							return x;
						else
							// Generate underflow and return x.
							return x*_nmd_Tiny + x;
	else
		if (x <= +1)
			if (x <= +.5)
				if (x <= +0x1.d12ed0af1a27fp-27)
					if (x < +0x1p-1022)
						if (x == 0)
							return x;
						else
							// Generate underflow and return x.
							return x*_nmd_Tiny + x;
					else
						// Generate inexact and return x.
						return (_nmd_Tiny + 1) * x;
				else
					return _nmd_atani0(x);
			else
				if (x <= +.75)
					return +_nmd_atani1(+x);
				else
					return +_nmd_atani2(+x);
		else
			if (x <= +5/3.)
				if (x <= +4/3.)
					return +_nmd_atani3(+x);
				else
					return +_nmd_atani4(+x);
			else
				if (x <= +2)
					return +_nmd_atani5(+x);
				else
					return +_nmd_Tail(+x);
}

/* Taken from https://en.wikipedia.org/wiki/Atan2s */
double nmd_atan2(double y, double x)
{
	if (x > 0.0)
		return nmd_atan(y / x);
	else if (x < 0.0 && y >= 0.0)
		return nmd_atan(y / x) + _NMD_PI;
	else if (x < 0.0 && y < 0.0)
		return nmd_atan(y / x) - _NMD_PI;
	else if (x == 0.0)
	{
		if (y > 0.0)
			return _NMD_HALF_PI;
		else if (y < 0.0)
			return -_NMD_HALF_PI;
		else
			return 0.0; /* Undefined */
	}
}

/* nmd_atanf() and nmd_atan2f() were taken from https://www.dsprelated.com/showarticle/1052.php */
float nmd_atanf(float x)
{
	const float n1 = 0.97239411f;
	const float n2 = -0.19194795f;
	return (n1 + n2 * x * x) * x;
}

float nmd_atan2f(float y, float x)
{
	const float n1 = 0.97239411f;
	const float n2 = -0.19194795f;
	float result = 0.0f;
	if (x != 0.0f)
	{
		const union { float flVal; uint32_t nVal; } tYSign = { y };
		const union { float flVal; uint32_t nVal; } tXSign = { x };
		if (nmd_fabs(x) >= nmd_fabs(y))
		{
			union { float flVal; uint32_t nVal; } tOffset = { _NMD_PIF };
			// Add or subtract PI based on y's sign.
			tOffset.nVal |= tYSign.nVal & 0x80000000u;
			// No offset if x is positive, so multiply by 0 or based on x's sign.
			tOffset.nVal *= tXSign.nVal >> 31;
			result = tOffset.flVal;
			const float z = y / x;
			result += (n1 + n2 * z * z) * z;
		}
		else // Use atan(y/x) = pi/2 - atan(x/y) if |y/x| > 1.
		{
			union { float flVal; uint32_t nVal; } tOffset = { _NMD_HALF_PIF };
			// Add or subtract PI/2 based on y's sign.
			tOffset.nVal |= tYSign.nVal & 0x80000000u;
			result = tOffset.flVal;
			const float z = x / y;
			result -= (n1 + n2 * z * z) * z;
		}
	}
	else if (y > 0.0f)
	{
		result = _NMD_HALF_PIF;
	}
	else if (y < 0.0f)
	{
		result = -_NMD_HALF_PIF;
	}
	return result;
}

#endif /* NMD_LIBC_IMPLEMENTATION */

#endif /* NMD_LIBC_H */
