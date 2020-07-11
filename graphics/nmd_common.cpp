#include "nmd_common.hpp"

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
	Vec2 Vec2::operator+(const Vec2& other) const { return Vec2(this->x + other.x, this->y + other.x); }
	Vec2 Vec2::operator-(const Vec2& other) const { return Vec2(this->x - other.x, this->y - other.x); }
	Vec2 Vec2::operator*(const Vec2& other) const { return Vec2(this->x * other.x, this->y * other.x); }

	Vec2 Vec2::Clamp(const Vec2& x, const Vec2& low, const Vec2& high) { return Vec2(NMD_CLAMP(x.x, low.x, high.x), NMD_CLAMP(x.y, low.y, high.y)); }
	Vec2 Vec2::Min(const Vec2& lhs, const Vec2& rhs) { return Vec2(NMD_MIN(lhs.x, rhs.x), NMD_MIN(lhs.y, rhs.y)); }
	Vec2 Vec2::Max(const Vec2& lhs, const Vec2& rhs) { return Vec2(NMD_MAX(lhs.x, rhs.x), NMD_MAX(lhs.y, rhs.y)); }

	bool IsPointInRect(const Vec4& rect, const Vec2& p) { return p.x >= rect.pos.x && p.x <= rect.pos.x + rect.size.x && p.y >= rect.pos.y && p.y <= rect.pos.y + rect.size.y; }

#ifndef NMD_GRAPHICS_DISABLE_DEFAULT_FONT
	extern const uint8_t karla_ttf_regular[14824];
#endif // NMD_GRAPHICS_DISABLE_DEFAULT_FONT
} // namespace nmd
