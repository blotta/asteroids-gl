#include <HandmadeMath.h>

typedef HMM_Vec3 Vec3;
typedef HMM_Vec2 Vec2;
typedef HMM_Vec4 Vec4;

static inline Vec2 vec2(float x, float y)
{
    Vec2 v;
    v.X = x;
    v.Y = y;
    return v;
}
static inline Vec3 vec3(float x, float y, float z)
{
    Vec3 v;
    v.X = x;
    v.Y = y;
    v.Z = z;
    return v;
}
static inline Vec4 vec4(float x, float y, float z, float w)
{
    Vec4 v;
    v.X = x;
    v.Y = y;
    v.Z = z;
    v.W = w;
    return v;
}
