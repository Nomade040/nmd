#ifndef NMD_COMMON_H
#define NMD_COMMON_H

#include "nmd_graphics.hpp"

#define NMD_PI  3.141592653f
#define NMD_2PI 6.283185306f

#define NMD_CLAMP(x, low, high) ((x < low) ? low : (x > high) ? high : x)
#define NMD_MIN(a, b) (a < b ? a : b)
#define NMD_MAX(a, b) (a > b ? a : b)

#define CIRCLE_AUTO_SEGMENT_MIN 12
#define CIRCLE_AUTO_SEGMENT_MAX 512
#define CIRCLE_AUTO_SEGMENT_CALC(radius, maxError) NMD_CLAMP(NMD_2PI / acosf((radius - maxError) / radius), CIRCLE_AUTO_SEGMENT_MIN, CIRCLE_AUTO_SEGMENT_MAX)

namespace nmd
{
#ifndef NMD_GRAPHICS_DISABLE_DEFAULT_FONT
	extern const uint8_t karla_ttf_regular[14824];
#endif // NMD_GRAPHICS_DISABLE_DEFAULT_FONT
} // namespace nmd

#endif NMD_COMMON_H // NMD_COMMON_H