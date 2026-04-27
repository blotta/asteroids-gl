#ifndef GAME_LIB_H
#define GAME_LIB_H

#include "game_lib_config.h"

#ifndef LIB_DEF
#define LIB_DEF static inline
#endif

/*
 *
 * GAME MATH
 *
 */

#define PI_D 3.141592653589793238462643383279502884
#define PI_F 3.141592653589793238462643383279502884F
#define TWO_PI_D (2.0 * PI_D)
#define TWO_PI_F ((float)(TWO_PI_D))
#define RadToDeg ((float)(180.0 / PI_D))
#define DegToRad ((float)(PI_D / 180.0))

// Scalar

LIB_DEF float min_f(float a, float b);
LIB_DEF float max_f(float a, float b);
LIB_DEF float min_f3(float a, float b, float c);
LIB_DEF float max_f3(float a, float b, float c);
LIB_DEF float min_f4(float a, float b, float c, float d);
LIB_DEF float max_f4(float a, float b, float c, float d);
LIB_DEF float min_fv(float* v, int count);
LIB_DEF float max_fv(float* v, int count);

void shuffle_fv(float* arr, int n);
LIB_DEF void sort_fv(float* values, size_t count);

// Vectors

typedef union {
    struct
    {
        float x, y;
    };

    float elements[2];

} Vec2;

typedef union {
    struct
    {
        float x, y, z;
    };

    float elements[3];

} Vec3;

typedef union {
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

typedef union {
    float elements[4][4];
    Vec4 columns[4];
} Mat4;

typedef union {
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
} Rect;

LIB_DEF Vec2 vec2(float x, float y);

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

LIB_DEF float vec2_dot(Vec2 a, Vec2 b);
LIB_DEF float vec2_length_squared(Vec2 a);
LIB_DEF float vec2_length(Vec2 a);
LIB_DEF Vec2 vec2_add(Vec2 a, Vec2 b);
LIB_DEF Vec2 vec2_sub(Vec2 a, Vec2 b);
LIB_DEF Vec2 vec2_mulf(Vec2 a, float b);
LIB_DEF float vec2_angle(Vec2 v);
LIB_DEF float vec2_cross(Vec2 a, Vec2 b);
LIB_DEF Vec2 vec2_normalized(Vec2 a);
LIB_DEF Vec2 vec2_normal_ccw(Vec2 v);
// normalized ccw normal
LIB_DEF Vec2 vec2_normal_ccw_n(Vec2 v);
LIB_DEF Vec2 vec2_normal_cw(Vec2 v);
// normalized cw normal
LIB_DEF Vec2 vec2_normal_cw_n(Vec2 v);
LIB_DEF Vec2 vec2_rotated(Vec2 v, float angle);
LIB_DEF Vec2 vec2_project(Vec2 v, Vec2 onto);

LIB_DEF Vec3 vec3(float x, float y, float z);
LIB_DEF Vec3 vec3_v2(Vec2 a);
LIB_DEF float vec3_dot(Vec3 a, Vec3 b);
LIB_DEF Vec3 vec3_mulf(Vec3 a, float b);
LIB_DEF float vec3_length_squared(Vec3 v);
LIB_DEF float vec3_length(Vec3 v);
LIB_DEF Vec3 vec3_normalized(Vec3 v);

LIB_DEF Vec4 vec4(float x, float y, float z, float w);
LIB_DEF Vec4 vec4_linear_combine_mat4(Vec4 left, Mat4 right);

LIB_DEF Rect rect(float x, float y, float w, float h);

LIB_DEF Mat4 mat4(float diagonal);
LIB_DEF Mat4 mat4_translation(Vec3 translation);
LIB_DEF Mat4 mat4_rotation(float angle, Vec3 axis);
LIB_DEF Mat4 mat4_mul_m4(Mat4 left, Mat4 right);

// Produces a right-handed orthographic projection matrix with Z ranging from -1 to 1 (the GL convention).
// Left, Right, Bottom, and Top specify the coordinates of their respective clipping planes.
// Near and Far specify the distances to the near and far clipping planes.
LIB_DEF Mat4 mat4_orthographic_rh_no(float left, float right, float bottom, float top, float near, float far);

// General geometry

typedef struct {
    float left;
    float right;
    float top;
    float bottom;
} AABB;

Vec2 rect_center(Rect rect, Vec2 anchor, float angle);
AABB rect_aabb(Rect rect, Vec2 anchor, float angle);
float polygon_area(Vec2* vertices, int count);
void polygon_angles(Vec2* vertices, int count, float* out_angles);
bool polygon_is_convex(Vec2* vertices, int count);

// Generates a convex polygon centered around (0,0) using Valtr's proof
// taken from https://cglab.ca/~sander/misc/ConvexGeneration/convex.html
// converted from Java
void polygon_generate_convex(Vec2* out, int n, float max_radius);

// --- GAME MATH IMPLEMENTATION --- //
#ifdef GAME_LIB_IMPLEMENTATION

/*
 * Scalar
 */

float min_f(float a, float b)
{
    return (a < b) ? a : b;
}

float max_f(float a, float b)
{
    return (a < b) ? b : a;
}

float min_f3(float a, float b, float c)
{
    return min_f(min_f(a, b), c);
}

float min_f4(float a, float b, float c, float d)
{
    return min_f(min_f(a, b), min_f(c, d));
}

float max_f3(float a, float b, float c)
{
    return max_f(max_f(a, b), c);
}

float max_f4(float a, float b, float c, float d)
{
    return max_f(max_f(a, b), max_f(c, d));
}

float min_fv(float* v, int count)
{
    float min = v[0];
    for (int i = 1; i < count; i++)
    {
        if (v[i] < min)
        {
            min = v[i];
        }
    }
    return min;
}

float max_fv(float* v, int count)
{
    float max = v[0];
    for (int i = 1; i < count; i++)
    {
        if (v[i] > max)
        {
            max = v[i];
        }
    }
    return max;
}

// Fisher-Yates shuffle
void shuffle_fv(float* arr, int n)
{
    for (int i = n - 1; i > 0; i--)
    {
        int j = rand_n(i + 1);
        float tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

int _fn_compare_f(const void* a, const void* b)
{
    float fa = *(const float*)a;
    float fb = *(const float*)b;
    return (fa > fb) - (fa < fb);
}
void sort_fv(float* values, size_t count)
{
    sort_custom((void*)values, count, sizeof(float), _fn_compare_f);
}

/*
 * Vec2
 */
Vec2 vec2(float x, float y)
{
    Vec2 v;
    v.x = x;
    v.y = y;
    return v;
}

float vec2_dot(Vec2 a, Vec2 b)
{
    return (a.x * b.x) + (a.y * b.y);
}

float vec2_length_squared(Vec2 a)
{
    return vec2_dot(a, a);
}

float vec2_length(Vec2 a)
{
    return sqrt_f(vec2_length_squared(a));
}

Vec2 vec2_add(Vec2 a, Vec2 b)
{
    Vec2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

Vec2 vec2_sub(Vec2 a, Vec2 b)
{
    Vec2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

Vec2 vec2_mulf(Vec2 a, float b)
{
    Vec2 result;
    result.x = a.x * b;
    result.y = a.y * b;

    return result;
}

float vec2_angle(Vec2 v)
{
    return atan2_f(v.y, v.x);
}

float vec2_cross(Vec2 a, Vec2 b)
{
    return a.x * b.y - a.y * b.x;
}

Vec2 vec2_normalized(Vec2 a)
{
    float len = vec2_length(a);
    ASSERT(len > 0.f);
    return vec2_mulf(a, 1.f / len);
}

Vec2 vec2_normal_ccw(Vec2 v)
{
    return vec2(-v.y, v.x);
}

Vec2 vec2_normal_ccw_n(Vec2 v)
{
    return vec2_normalized(vec2(-v.y, v.x));
}

Vec2 vec2_normal_cw(Vec2 v)
{
    return vec2(v.y, -v.x);
}

Vec2 vec2_normal_cw_n(Vec2 v)
{
    return vec2_normalized(vec2(v.y, -v.x));
}

Vec2 vec2_rotated(Vec2 v, float angle)
{
    float sinA = sin_f(angle);
    float cosA = cos_f(angle);

    return vec2(v.x * cosA - v.y * sinA, v.x * sinA + v.y * cosA);
}

Vec2 vec2_project(Vec2 v, Vec2 onto)
{
    onto = vec2_normalized(onto);
    float scalar_projection = vec2_dot(v, onto);
    onto = vec2_mulf(onto, scalar_projection);
    return onto;
}

/*
 * Vec3
 */

Vec3 vec3(float x, float y, float z)
{
    Vec3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

Vec3 vec3_v2(Vec2 a)
{
    Vec3 v;
    v.x = a.x;
    v.y = a.y;
    v.z = 0.f;
    return v;
}

float vec3_dot(Vec3 a, Vec3 b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

Vec3 vec3_mulf(Vec3 v, float a)
{
    Vec3 result;
    result.x = v.x * a;
    result.y = v.y * a;
    result.z = v.z * a;
    return result;
}

float vec3_length_squared(Vec3 a)
{
    return vec3_dot(a, a);
}

float vec3_length(Vec3 v)
{
    return sqrt_f(vec3_length_squared(v));
}

Vec3 vec3_normalized(Vec3 v)
{
    float len = vec3_length(v);
    ASSERT(len != 0.0f);
    return vec3_mulf(v, 1.0f / len);
}

/*
 * Vec4
 */

Vec4 vec4(float x, float y, float z, float w)
{
    Vec4 v;
    v.x = x;
    v.y = y;
    v.z = z;
    v.w = w;
    return v;
}

Vec4 vec4_linear_combine_mat4(Vec4 left, Mat4 right)
{
    Vec4 result;

    result.x = left.elements[0] * right.columns[0].x;
    result.y = left.elements[0] * right.columns[0].y;
    result.z = left.elements[0] * right.columns[0].z;
    result.w = left.elements[0] * right.columns[0].w;

    result.x += left.elements[1] * right.columns[1].x;
    result.y += left.elements[1] * right.columns[1].y;
    result.z += left.elements[1] * right.columns[1].z;
    result.w += left.elements[1] * right.columns[1].w;

    result.x += left.elements[2] * right.columns[2].x;
    result.y += left.elements[2] * right.columns[2].y;
    result.z += left.elements[2] * right.columns[2].z;
    result.w += left.elements[2] * right.columns[2].w;

    result.x += left.elements[3] * right.columns[3].x;
    result.y += left.elements[3] * right.columns[3].y;
    result.z += left.elements[3] * right.columns[3].z;
    result.w += left.elements[3] * right.columns[3].w;

    return result;
}

/*
 * Mat4
 */

Mat4 mat4(float diagonal)
{
    Mat4 result = {0};
    result.elements[0][0] = diagonal;
    result.elements[1][1] = diagonal;
    result.elements[2][2] = diagonal;
    result.elements[3][3] = diagonal;

    return result;
}

Mat4 mat4_translation(Vec3 translation)
{
    Mat4 result = mat4(1.0f);
    result.elements[3][0] = translation.x;
    result.elements[3][1] = translation.y;
    result.elements[3][2] = translation.z;

    return result;
}

Mat4 mat4_rotation(float angle, Vec3 axis)
{
    Mat4 result = mat4(1.0f);

    axis = vec3_normalized(axis);

    float sintheta = sin_f(angle);
    float costheta = cos_f(angle);
    float cosvalue = 1.0f - costheta;

    result.elements[0][0] = (axis.x * axis.x * cosvalue) + costheta;
    result.elements[0][1] = (axis.x * axis.y * cosvalue) + (axis.z * sintheta);
    result.elements[0][2] = (axis.x * axis.z * cosvalue) - (axis.y * sintheta);

    result.elements[1][0] = (axis.y * axis.x * cosvalue) - (axis.z * sintheta);
    result.elements[1][1] = (axis.y * axis.y * cosvalue) + costheta;
    result.elements[1][2] = (axis.y * axis.z * cosvalue) + (axis.x * sintheta);

    result.elements[2][0] = (axis.z * axis.x * cosvalue) + (axis.y * sintheta);
    result.elements[2][1] = (axis.z * axis.y * cosvalue) - (axis.x * sintheta);
    result.elements[2][2] = (axis.z * axis.z * cosvalue) + costheta;

    return result;
}

Mat4 mat4_mul_m4(Mat4 left, Mat4 right)
{
    Mat4 result;
    result.columns[0] = vec4_linear_combine_mat4(right.columns[0], left);
    result.columns[1] = vec4_linear_combine_mat4(right.columns[1], left);
    result.columns[2] = vec4_linear_combine_mat4(right.columns[2], left);
    result.columns[3] = vec4_linear_combine_mat4(right.columns[3], left);

    return result;
}

Vec4 mat4_mul_v4(Mat4 matrix, Vec4 vector)
{
    return vec4_linear_combine_mat4(vector, matrix);
}

Mat4 mat4_orthographic_rh_no(float left, float right, float bottom, float top, float near, float far)
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


/*
 * General geometry
 */

Rect rect(float x, float y, float w, float h)
{
    Rect r;
    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;
    return r;
}

Vec2 rect_center(Rect rect, Vec2 anchor, float angle)
{
    Vec2 center = vec2(rect.w * 0.5f, rect.h * 0.5f);
    Vec2 pivot = vec2(anchor.x * rect.w, anchor.y * rect.h);
    Vec2 p = vec2_sub(center, pivot);
    p = vec2_rotated(p, angle);
    p = vec2_add(p, vec2(rect.x, rect.y));
    return p;
}

AABB rect_aabb(Rect rect, Vec2 anchor, float angle)
{
    float cos_a = cos_f(angle);
    float sin_a = sin_f(angle);

    float hw = rect.w * 0.5f;
    float hh = rect.h * 0.5f;

    float ex = abs_f(hw * cos_a) + abs_f(hh * sin_a);
    float ey = abs_f(hw * sin_a) + abs_f(hh * cos_a);

    Vec2 center = rect_center(rect, anchor, angle);

    AABB result = {0};
    result.left = center.x - ex;
    result.right = center.x + ex;
    result.top = center.y + ey;
    result.bottom = center.y - ey;

    return result;
}

float polygon_area(Vec2* vertices, int count)
{
    float area = 0.0f;
    for (int i = 0; i < count; i++)
    {
        int j = (i + 1) % count;
        area += vec2_cross(vertices[i], vertices[j]);
    }
    return area * 0.5f;
}

// returns the inner angle of each vertex
void polygon_angles(Vec2* vertices, int count, float* out_angles)
{
    ASSERT(count >= 3);

    for (int i = 0; i < count; i++)
    {
        int prev = (i - 1 + count) % count;
        int next = (i + 1) % count;
        Vec2 v1 = vertices[prev];
        Vec2 v2 = vertices[i];
        Vec2 v3 = vertices[next];

        Vec2 a = vec2_sub(v2, v1);
        Vec2 b = vec2_sub(v3, v2);

        float dot = vec2_dot(a, b);
        float det = vec2_cross(a, b);

        float angle = atan2_f(det, dot);

        angle = PI_F - angle; // interior angle
        while (angle < 0)
        {
            angle += 2.0f * PI_F;
        }
        out_angles[i] = angle;
    }
}

bool polygon_is_convex(Vec2* vertices, int count)
{
    ASSERT(count >= 3);
    if (count == 3)
    {
        return true;
    }

    float angles[count];
    polygon_angles(vertices, count, angles);
    for (int i = 0; i < count; i++)
    {
        if (angles[i] > PI_F)
        {
            return false;
        }
    }
    return true;
}

static int _compare_vec_angle(const void* a, const void* b)
{
    Vec2* va = (Vec2*)a;
    Vec2* vb = (Vec2*)b;

    float angleA = atan2_f(va->y, va->x);
    float angleB = atan2_f(vb->y, vb->x);

    return (angleA > angleB) - (angleA < angleB);
}

void polygon_generate_convex(Vec2* out, int n, float max_radius)
{
    ASSERT(n >= 3);

    // 1. Generate two lists of random x and y coordinates
    float xPool[n];
    float yPool[n];
    for (int i = 0; i < n; i++)
    {
        xPool[i] = rand_0_1f();
        yPool[i] = rand_0_1f();
    }

    // 2. Sort them
    sort_fv(xPool, n);
    sort_fv(yPool, n);

    // 3. Isolate the extreme points
    float minX = xPool[0];
    float maxX = xPool[n - 1];
    float minY = yPool[0];
    float maxY = yPool[n - 1];

    // 4. Randomly divide the interior points into two chains & 5. Extract the vector components
    float xVec[n];
    int xVec_count = 0;
    float yVec[n];
    int yVec_count = 0;

    float lastTop = minX;
    float lastBot = minX;

    for (int i = 1; i < n - 1; i++)
    {
        float x = xPool[i];
        if (rand_0_1f() > 0.5f)
        {
            xVec[xVec_count++] = x - lastTop;
            lastTop = x;
        }
        else
        {
            xVec[xVec_count++] = lastBot - x;
            lastBot = x;
        }
    }

    xVec[xVec_count++] = maxX - lastTop;
    xVec[xVec_count++] = lastBot - maxX;

    float lastLeft = minY;
    float lastRight = minY;

    for (int i = 1; i < n - 1; i++)
    {
        float y = yPool[i];
        if (rand_0_1f() > 0.5f)
        {
            yVec[yVec_count++] = y - lastLeft;
            lastLeft = y;
        }
        else
        {
            yVec[yVec_count++] = lastRight - y;
            lastRight = y;
        }
    }

    yVec[yVec_count++] = maxY - lastLeft;
    yVec[yVec_count++] = lastRight - maxY;

    // 6. Randomly pair up the x and y components
    shuffle_fv(yVec, yVec_count);

    // 7. Combine the paired up components into vectors
    Vec2 vec[n];
    for (int i = 0; i < n; i++)
    {
        vec[i] = vec2(xVec[i], yVec[i]);
    }

    // 8. Sort vectors by angle
    sort_custom(vec, n, sizeof(Vec2), _compare_vec_angle);

    // 9. Lay them end to end to form a polygon
    float x = 0;
    float y = 0;
    float minPolygonX = 0;
    float minPolygonY = 0;
    Vec2 points[n];
    int points_count = 0;

    for (int i = 0; i < n; i++)
    {
        points[points_count++] = vec2(x, y);
        x += vec[i].x;
        y += vec[i].y;

        minPolygonX = min_f(minPolygonX, x);
        minPolygonY = min_f(minPolygonY, y);
    }

    // 10. Move the polygon to the original min and max coordinates
    float xShift = minX - minPolygonX;
    float yShift = minY - minPolygonY;

    for (int i = 0; i < n; i++)
    {
        Vec2* p = &points[i];
        points[i].x = p->x + xShift;
        points[i].y = p->y + yShift;
    }

    // 11. (OWN) center the polygon around 0,0
    Vec2 centroid = vec2(0, 0);
    for (int i = 0; i < n; i++)
    {
        centroid.x += points[i].x;
        centroid.y += points[i].y;
    }
    centroid.x /= (float)n;
    centroid.y /= (float)n;
    for (int i = 0; i < n; i++)
    {
        points[i].x -= centroid.x;
        points[i].y -= centroid.y;
    }

    // 12. (OWN) Scale and output
    for (int i = 0; i < n; i++)
    {
        out[i] = vec2_mulf(points[i], max_radius);
    }
}

#endif // GAME MATH IMPLEMENTATION

/*
 *
 * SHAPES
 *
 */

typedef enum
{
    SHAPEKIND_POLYGON,
    SHAPEKIND_CIRCLE,
    SHAPEKIND_RECTANGLE
} ShapeKind;

#define SHAPE_POLYGON_MAX_VERTICES 20

typedef struct
{
    ShapeKind kind;
    union {
        struct
        {
            int vertex_count;
            Vec2 vertices[SHAPE_POLYGON_MAX_VERTICES];
        } polygon;

        struct
        {
            float radius;
        } circle;

        struct
        {
            Vec2 size;
        } rect;
    };
} Shape;

void shape_rectangle(Shape* s, float width, float height);
void shape_circle(Shape* s, float radius);
void shape_polygon(Shape* s, Vec2* vertices, int vertex_count);
void shape_polygon_generate_convex(Shape* shape, int count, float max_radius);

// SHAPES IMPLEMENTATION
#ifdef GAME_LIB_IMPLEMENTATION

void shape_rectangle(Shape* s, float width, float height)
{
    ASSERT(width > 0.0f && height > 0.0f);
    s->kind = SHAPEKIND_RECTANGLE;
    s->rect.size.x = width;
    s->rect.size.y = height;
}

void shape_circle(Shape* s, float radius)
{
    ASSERT(radius > 0.0f);
    s->kind = SHAPEKIND_CIRCLE;
    s->circle.radius = radius;
}

void shape_polygon(Shape* s, Vec2* vertices, int vertex_count)
{
    ASSERT(vertex_count >= 3 && vertex_count <= SHAPE_POLYGON_MAX_VERTICES);
    s->kind = SHAPEKIND_POLYGON;
    s->polygon.vertex_count = vertex_count;
    for (int i = 0; i < vertex_count; i++)
    {
        s->polygon.vertices[i] = vertices[i];
    }
}

void shape_polygon_generate_convex(Shape* shape, int count, float max_radius)
{
    ASSERT(count >= 3 && count <= SHAPE_POLYGON_MAX_VERTICES);
    Vec2 vertices[count];
    polygon_generate_convex(vertices, count, max_radius);
    shape_polygon(shape, vertices, count);
}

#endif // SHAPES_IMPLEMENTATION


/*
 *
 * PHYSICS
 *
 */

#define PHBODY_MAX_SHAPES 8
typedef struct
{
    int id;
    Vec2 position;
    Vec2 velocity;
    float angle;
    float angular_velocity;
    Shape shapes[PHBODY_MAX_SHAPES];
    int shape_count;
} PHBody;

void ph_body_init(PHBody* body, Shape* shapes, int shape_count);
void ph_body_add_shape(PHBody* body, Shape* shape);
AABB ph_bounding_box(PHBody* b);

// PHYSICS IMPLEMENTATION
#ifdef GAME_LIB_IMPLEMENTATION

static int s_next_body_id = 0;

void ph_body_init(PHBody* body, Shape* shapes, int shape_count)
{
    body->id = s_next_body_id++;
    body->position = vec2(0.f, 0.f);
    body->velocity = vec2(0.f, 0.f);
    body->angle = 0.f;
    body->angular_velocity = 0.f;
    body->shape_count = shape_count;
    ASSERT(shape_count > 0 && shape_count <= PHBODY_MAX_SHAPES);
    for (int i = 0; i < shape_count; i++)
    {
        body->shapes[i] = shapes[i];
    }
}

void ph_body_add_shape(PHBody* body, Shape* shape)
{
    ASSERT(body->shape_count < PHBODY_MAX_SHAPES);
    body->shapes[body->shape_count++] = *shape;
}

AABB ph_bounding_box(PHBody* b)
{
    ASSERT(b->shape_count > 0);

    AABB bbox;
    bbox.left = INFINITY;
    bbox.right = -INFINITY;
    bbox.top = -INFINITY;
    bbox.bottom = INFINITY;

    for (int i = 0; i < b->shape_count; i++)
    {
        Shape* shape = &b->shapes[i];

        switch (shape->kind)
        {
        case SHAPEKIND_RECTANGLE: {
            float hw = shape->rect.size.x * 0.5f;
            float hh = shape->rect.size.y * 0.5f;
            Rect rc = rect(b->position.x - hw, b->position.y - hh, shape->rect.size.x, shape->rect.size.y);
            bbox = rect_aabb(rc, vec2(0.5f, 0.5f), b->angle);
        }
        break;
        case SHAPEKIND_CIRCLE: {
            bbox.left = b->position.x - shape->circle.radius;
            bbox.right = b->position.x + shape->circle.radius;
            bbox.top = b->position.y + shape->circle.radius;
            bbox.bottom = b->position.y - shape->circle.radius;
        }
        break;
        case SHAPEKIND_POLYGON: {
            for (int j = 0; j < shape->polygon.vertex_count; j++)
            {
                Vec2 v = shape->polygon.vertices[j];
                v = vec2_rotated(v, b->angle);
                v = vec2_add(v, b->position);
                bbox.left = min_f(bbox.left, v.x);
                bbox.right = max_f(bbox.right, v.x);
                bbox.bottom = min_f(bbox.bottom, v.y);
                bbox.top = max_f(bbox.top, v.y);
            }
        }
        break;
        }
    }

    return bbox;
}

#endif // PHYSICS IMPLEMENTATION

#endif // GAME_LIB_H
