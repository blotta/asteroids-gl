#pragma once

#include "SDL3/SDL_stdinc.h"
#include "vmath.h"
#include <SDL3/SDL.h>

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

void shape_rectangle(Shape* s, float width, float height)
{
    s->kind = SHAPEKIND_RECTANGLE;
    s->rect.size.x = width;
    s->rect.size.y = height;
}

void shape_circle(Shape* s, float radius)
{
    s->kind = SHAPEKIND_CIRCLE;
    s->circle.radius = radius;
}

void shape_polygon(Shape* s, Vec2* vertices, int vertex_count)
{
    SDL_assert(vertex_count >= 3 && vertex_count <= SHAPE_POLYGON_MAX_VERTICES);
    s->kind = SHAPEKIND_POLYGON;
    s->polygon.vertex_count = vertex_count;
    for (int i = 0; i < vertex_count; i++)
    {
        s->polygon.vertices[i] = vertices[i];
    }
}

static int _compare_floats(const void* a, const void* b)
{
    float fa = *(const float*)a;
    float fb = *(const float*)b;
    return (fa > fb) - (fa < fb);
}

static int _compare_vec_angle(const void* a, const void* b)
{
    Vec2* va = (Vec2*)a;
    Vec2* vb = (Vec2*)b;

    float angleA = atan2f(va->y, va->x);
    float angleB = atan2f(vb->y, vb->x);

    return (angleA > angleB) - (angleA < angleB);
}

// Fisher-Yates shuffle
static void shufflef(float* arr, int n)
{
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        float tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

// Generates a convex polygon centered around (0,0) using Valtr's proof
// taken from https://cglab.ca/~sander/misc/ConvexGeneration/convex.html
// converted from Java
void polygon_generate_convex(Vec2* out, int n, float max_radius)
{
    SDL_assert(n >= 3);

    // 1. Generate two lists of random x and y coordinates
    float xPool[n];
    float yPool[n];
    for (int i = 0; i < n; i++)
    {
        xPool[i] = SDL_randf();
        yPool[i] = SDL_randf();
    }

    // 2. Sort them
    qsort(xPool, n, sizeof(float), _compare_floats);
    qsort(yPool, n, sizeof(float), _compare_floats);

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
        if (SDL_randf() > 0.5f)
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
        if (SDL_randf() > 0.5f)
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
    shufflef(yVec, yVec_count);

    // 7. Combine the paired up components into vectors
    Vec2 vec[n];
    for (int i = 0; i < n; i++) {
        vec[i] = vec2(xVec[i], yVec[i]);
    }

    // 8. Sort vectors by angle
    qsort(vec, n, sizeof(Vec2), _compare_vec_angle);

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

        minPolygonX = fminf(minPolygonX, x);
        minPolygonY = fminf(minPolygonY, y);
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

void shape_polygon_generate_convex(Shape* shape, int count, float max_radius)
{
    SDL_assert(count >= 3 && count <= SHAPE_POLYGON_MAX_VERTICES);
    Vec2 vertices[count];
    polygon_generate_convex(vertices, count, max_radius);
    shape_polygon(shape, vertices, count);
}

float shape_polygon_area(Vec2* v, int count)
{
    float area = 0.0f;
    for (int i = 0; i < count; i++)
    {
        int j = (i + 1) % count;
        area += vec2_cross(v[i], v[j]);
    }
    return area * 0.5f;
}

// returns the inner angle of each vertex
void polygon_angles(Vec2* vertices, int count, float* out_angles)
{
    SDL_assert(count >= 3);

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

        float angle = atan2f(det, dot);

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
    SDL_assert(count >= 3);
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
