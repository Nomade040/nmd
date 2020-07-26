#ifndef NMD_COMMON_H
#define NMD_COMMON_H

#include "nmd_graphics.h"

#define NMD_PI  3.141592653f
#define NMD_2PI 6.283185306f

#define NMD_CLAMP(x, low, high) ((x < low) ? low : (x > high) ? high : x)
#define NMD_MIN(a, b) (a < b ? a : b)
#define NMD_MAX(a, b) (a > b ? a : b)

#define NMD_CIRCLE_AUTO_SEGMENT_MIN 12
#define NMD_CIRCLE_AUTO_SEGMENT_MAX 512
#define NMD_CIRCLE_AUTO_SEGMENT_CALC(radius, maxError) NMD_CLAMP(NMD_2PI / NMD_ACOS((radius - maxError) / radius), NMD_CIRCLE_AUTO_SEGMENT_MIN, NMD_CIRCLE_AUTO_SEGMENT_MAX)

extern nmd_context _nmd_context;
extern const uint8_t nmd_karla_ttf_regular[14824];

void _nmd_push_remaining_draw_commands();

#endif /* NMD_COMMON_H */