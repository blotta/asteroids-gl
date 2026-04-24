#pragma once

#include <SDL3/SDL.h>

#define PI_D SDL_PI_D
#define PI_F SDL_PI_F
#define RadToDeg ((float)(180.0 / PI_D))
#define DegToRad ((float)(PI_D / 180.0))

#define sinF SDL_sinf
#define cosF SDL_cosf
#define atan2F SDL_atan2f

typedef union Vec2 {
    struct
    {
        float x, y;
    };

    float elements[2];

} Vec2;

typedef union Vec3 {
    struct
    {
        float x, y, z;
    };

    float elements[3];

} Vec3;

typedef union Vec4 {
    struct
    {
        float x, y, z, w;
    };

    struct
    {
        float r, g, b, a;
    };

    float elements[4];

} Vec4;

typedef union Mat4 {
    float elements[4][4];
    Vec4 columns[4];
} Mat4;

typedef union RectF {
    struct
    {
        Vec2 pos;
        Vec2 size;
    };
    struct
    {
        float x;
        float y;
        float w;
        float h;
    };
} RectF;

static inline Vec2 vec2(float x, float y)
{
    Vec2 v;
    v.x = x;
    v.y = y;
    return v;
}

#define VEC2_ZERO vec2(0.f, 0.f)
#define VEC2_ONE vec2(1.f, 1.f)
#define VEC2_LEFT vec2(-1.f, 0.f)
#define VEC2_RIGHT vec2(1.f, 0.f)
#define VEC2_UP vec2(0.f, 1.f)
#define VEC2_DOWN vec2(0.f, -1.f)
#define VEC2_UV_BL vec2(0.f, 0.f)
#define VEC2_UV_BR vec2(1.f, 0.f)
#define VEC2_UV_TL vec2(0.f, 1.f)
#define VEC2_UV_TR vec2(1.f, 1.f)


static inline float vec2_dot(Vec2 a, Vec2 b)
{
    return (a.x * b.x) + (a.y * b.y);
}

static inline float vec2_length_squared(Vec2 a)
{
    return vec2_dot(a, a);
}

static inline float vec2_length(Vec2 a)
{
    return SDL_sqrtf(vec2_length_squared(a));
}

static inline Vec2 vec2_add(Vec2 a, Vec2 b)
{
    Vec2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

static inline Vec2 vec2_sub(Vec2 a, Vec2 b)
{
    Vec2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

static inline Vec2 vec2_mulf(Vec2 a, float b)
{
    Vec2 result;
    result.x = a.x * b;
    result.y = a.y * b;

    return result;
}

static inline float vec2_cross(Vec2 a, Vec2 b)
{
    return a.x * b.y - a.y * b.x;
}

static inline Vec2 vec2_normalized(Vec2 a)
{
    float len = vec2_length(a);
    return vec2_mulf(a, 1.f / len);
}

// ccw normal
static inline Vec2 vec2_normal_ccw(Vec2 v)
{
    return vec2(-v.y, v.x);
}

// normalized ccw normal
static inline Vec2 vec2_normal_ccw_n(Vec2 v)
{
    return vec2_normalized(vec2(-v.y, v.x));
}

// cw normal
static inline Vec2 vec2_normal_cw(Vec2 v)
{
    return vec2(v.y, -v.x);
}

// normalized cw normal
static inline Vec2 vec2_normal_cw_n(Vec2 v)
{
    return vec2_normalized(vec2(v.y, -v.x));
}


static inline Vec2 vec2_rotate(Vec2 v, float angle)
{
    float sinA = sinF(angle);
    float cosA = cosF(angle);

    return vec2(v.x * cosA - v.y * sinA, v.x * sinA + v.y * cosA);
}

static inline Vec3 vec3(float x, float y, float z)
{
    Vec3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

static inline Vec3 vec3_v2(Vec2 a)
{
    Vec3 v;
    v.x = a.x;
    v.y = a.y;
    v.z = 0.f;
    return v;
}

static inline Vec4 vec4(float x, float y, float z, float w)
{
    Vec4 v;
    v.x = x;
    v.y = y;
    v.z = z;
    v.w = w;
    return v;
}

static inline RectF rectf(float x, float y, float w, float h)
{
    RectF r;
    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;
    return r;
}

static inline Mat4 mat4_diagonal(float diagonal)
{
    Mat4 result = {0};
    result.elements[0][0] = diagonal;
    result.elements[1][1] = diagonal;
    result.elements[2][2] = diagonal;
    result.elements[3][3] = diagonal;

    return result;
}

// Produces a right-handed orthographic projection matrix with Z ranging from -1 to 1 (the GL convention).
// Left, Right, Bottom, and Top specify the coordinates of their respective clipping planes.
// Near and Far specify the distances to the near and far clipping planes.
static inline Mat4 mat4_orthographic_rh_no(float left, float right, float bottom, float top, float near, float far)
{
    Mat4 result = {0};

    result.elements[0][0] = 2.0f / (right - left);
    result.elements[1][1] = 2.0f / (top - bottom);
    result.elements[2][2] = 2.0f / (near - far);
    result.elements[3][3] = 1.0f;

    result.elements[3][0] = (left + right) / (left - right);
    result.elements[3][1] = (bottom + top) / (bottom - top);
    result.elements[3][2] = (near + far) / (near - far);

    return result;
}
