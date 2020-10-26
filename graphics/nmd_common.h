#ifndef NMD_COMMON_H
#define NMD_COMMON_H

#include "nmd_graphics.h"
#include "stb_truetype.h"

#define NMD_PI  3.141592653f
#define NMD_2PI 6.283185306f

#define NMD_CLAMP(x, low, high) ((x < low) ? low : (x > high) ? high : x)
#define NMD_MIN(a, b) (a < b ? a : b)
#define NMD_MAX(a, b) (a > b ? a : b)

#define NMD_CIRCLE_AUTO_SEGMENT_MIN 12
#define NMD_CIRCLE_AUTO_SEGMENT_MAX 512
#define NMD_CIRCLE_AUTO_SEGMENT_CALC(radius, max_error) NMD_CLAMP(NMD_2PI / NMD_ACOS(((radius) - max_error) / (radius)), NMD_CIRCLE_AUTO_SEGMENT_MIN, NMD_CIRCLE_AUTO_SEGMENT_MAX)

#define _NMD_OFFSETOF(TYPE, NAME) (&((TYPE*)0)->NAME)

extern nmd_context _nmd_context;
extern const uint8_t nmd_karla_ttf_regular[14824];

#endif /* NMD_COMMON_H */