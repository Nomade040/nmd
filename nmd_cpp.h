/* This is a platform independent C preprocessor(cpp) */

#ifndef NMD_CPP_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

enum NMD_CPP_FLAG
{
	NMD_CPP_FLAG_NONE     = 0,
	NMD_CPP_FLAG_TRIGRAPH = (1 << 0)
};

enum NMD_CPP_ERROR
{
	NMD_CPP_ERROR_NONE = 0,
	NMD_CPP_ERROR_UNTERMINATED_COMMENT
};

/*
Preprocesses a C/C++ source file. The function returns zero on success, otherwise the return value is one of 'NMD_CPP_ERROR_XXX'.
The following transformations are performed:
 - Transform trigraphs
 - Remove single and multi line comments
 - Merge split lines
 - Transform two or more adjacent spaces into a single space
 - Transform two or more adjacent new lines into a single new line
 - Transform tabs into spaces

Expansion operations(TODO):
 - Macro expansion
 - Include replacement
 - Add new line at the end if there isn't one

A valid new line is either '\n' or '\r\n'.
Parameters:
 source [in] A pointer to a buffer that contains the source file.
 flags  [in] A mask of 'NMD_CPP_FLAGS_XXX'
*/
int nmd_cpp(char* source, uint32_t flags);

#ifdef NMD_CPP_IMPLEMENTATION

bool _nmd_strcmp_str(const char* s, const char* s2)
{
	while (*s2)
	{
		if (*s++ != *s2++)
			return false;
	}

	return true;
}

/*
Preprocesses a C/C++ source file. The function returns zero on success, otherwise the return value is one of 'NMD_CPP_ERROR_XXX'.
The following transformations are performed:
 - Transform trigraphs
 - Remove single and multi line comments
 - Merge split lines
 - Transform two or more adjacent spaces into a single space
 - Transform two or more adjacent new lines into a single new line
 - Transform tabs into spaces

Expansion operations(TODO):
 - Macro expansion
 - Include replacement
 - Add new line at the end if there isn't one

A valid new line is either '\n' or '\r\n'.
Parameters:
 source [in] A pointer to a buffer that contains the source file.
 flags  [in] A mask of 'NMD_CPP_FLAGS_XXX'
*/
int nmd_cpp(char* source, uint32_t flags)
{
	uint8_t between_quotes = 0x00;
	char last_char = '\0';
	char* out = source;

	while (*source)
	{
		/* Determine if we're between quotes or not */
		if(between_quotes)
		{
			if (*source == between_quotes && last_char != '\\')
				between_quotes = 0x00;
		}
		else if (*source == '\'' || *source == '"')
			between_quotes = *source;

		/* Skip if we're between quotes */
		if (between_quotes)
		{
			last_char = *source;
			*out++ = *source++;
			continue;
		}

		if (_nmd_strcmp_str(source, "#include"))
		{
			const char* header_begin = source + 8;
			while (*header_begin != '<' && *header_begin != '"')
				header_begin++;
			const char end_delimiter = *header_begin == '<' ? '>' : '"';
			header_begin++;

			const char* header_end = header_begin;
			while (*header_end != end_delimiter)
				header_end++;

			char header[256];
			char* tmp = header;
			for (; header_begin != header_end;)
				*tmp++ = *header_begin++;
			*tmp = '\0';

			if (end_delimiter == '"')
			{
				FILE* f = fopen(header, "rb");
				if (!f)
				{
					printf("failed to open \"%s\"\n", header);
					return false;
				}
			}
			int a = 0;
		}

		/* Do transformations */
		if (flags & NMD_CPP_FLAG_TRIGRAPH && source[0] == '?' && source[1] == '?') /* Handle trigraphs */
		{
			switch (source[2])
			{
			case '=': *out = '#'; break;
			case '/': *out = '\\'; break;
			case '\'': *out = '^'; break;
			case '(': *out = '['; break;
			case ')': *out = ']'; break;
			case '!': *out = '|'; break;
			case '<': *out = '{'; break;
			case '>': *out = '}'; break;
			case '-': *out = '~'; break;
			default: source += 2; continue;
			}
			
			source += 3;
			out++;
		}
		else if (source[0] == '/') /* Handle comments */
		{
			if (source[1] == '/') /* Single-line comment */
			{
				/* Iterate until a new line or EOF */
				source += 2;
				while (*source && source[0] != '\n')
					source++;
			}
			else if (source[1] == '*') /* Multi-line comment */
			{
				source += 2;
				while (*source && source[0] != '*' && source[1] != '/')
					source++;

				/* Check for unterminated commented */
				if (!*source)
					return NMD_CPP_ERROR_UNTERMINATED_COMMENT;

				source += 2;
			}
			else
				*out++ = *source++;
		}
		else if (source[0] == '\\') /* Handle split lines */
		{
			if (source[1] == '\n')
				source += 2;
			else if (source[1] == '\r' && source[2] == '\n')
				source += 3;
			else
				*out++ = *source++;
		}
		else
		{
			if (*source == '\t') { *out++ = ' '; } /* Convert tab to space */
			else if (last_char == ' ' && *source == ' ') {} /* Do not add space if the previous character is also a space */
			else if (last_char == '\n' && *source == '\n') {} /* Do not add a new line if the previous character is also a new line */
			else if (last_char == '\n' && source[0] == '\r' && source[1] == '\n') { source++; } /* Do not add a new line if the previous character is also a new line */
			else
				*out++ = *source;

			last_char = *source++;
		}
	}

	*out = '\0';

	return NMD_CPP_ERROR_NONE;
}

#endif /* NMD_CPP_IMPLEMENTATION */

#endif /* NMD_CPP_H */