#pragma once

#include "SDL3/SDL_assert.h"
#include "vmath.h"
#include "geometry.h"
#include <math.h>

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

static int s_next_body_id = 0;

void ph_body_init(PHBody* body, Shape* shapes, int shape_count)
{
    body->id = s_next_body_id++;
    body->position = vec2(0.f, 0.f);
    body->velocity = vec2(0.f, 0.f);
    body->angle = 0.f;
    body->angular_velocity = 0.f;
    body->shape_count = shape_count;
    SDL_assert(shape_count > 0 && shape_count <= PHBODY_MAX_SHAPES);
    for (int i = 0; i < shape_count; i++)
    {
        body->shapes[i] = shapes[i];
    }
}

void ph_body_add_shape(PHBody* body, Shape* shape)
{
    SDL_assert(body->shape_count < PHBODY_MAX_SHAPES);
    body->shapes[body->shape_count++] = *shape;
}

RectF ph_bounding_box(PHBody* b)
{
    SDL_assert(b->shape_count > 0);

    float minx = INFINITY, miny = INFINITY, maxx = -INFINITY, maxy = -INFINITY;

    for (int i = 0; i < b->shape_count; i++)
    {
        Shape* shape = &b->shapes[i];
        Vec2 bottomleft = vec2(INFINITY, INFINITY);
        Vec2 topright = vec2(-INFINITY, -INFINITY);

        switch (shape->kind)
        {
        case SHAPEKIND_RECTANGLE: {
            bottomleft = vec2(b->position.x - shape->rect.size.x / 2.f, b->position.y - shape->rect.size.y / 2.f);
            bottomleft = vec2_rotate(bottomleft, b->angle);
            topright = vec2(b->position.x + shape->rect.size.x / 2.f, b->position.y + shape->rect.size.y / 2.f);
            topright = vec2_rotate(topright, b->angle);
        }
        break;
        case SHAPEKIND_CIRCLE: {
            bottomleft = vec2(b->position.x - shape->circle.radius, b->position.y - shape->circle.radius);
            topright = vec2(b->position.x + shape->circle.radius, b->position.y + shape->circle.radius);
        }
        break;
        case SHAPEKIND_POLYGON: {
            for (int j = 0; j < shape->polygon.vertex_count; j++)
            {
                Vec2 v = shape->polygon.vertices[j];
                v = vec2_rotate(v, b->angle);
                v = vec2_add(v, b->position);
                bottomleft = vec2(fminf(bottomleft.x, v.x), fminf(bottomleft.y, v.y));
                topright = vec2(fmaxf(topright.x, v.x), fmaxf(topright.y, v.y));
            }
        }
        break;
        }

        float shape_minx = fminf(bottomleft.x, topright.x);
        float shape_miny = fminf(bottomleft.y, topright.y);
        float shape_maxx = fmaxf(bottomleft.x, topright.x);
        float shape_maxy = fmaxf(bottomleft.y, topright.y);

        minx = fminf(minx, shape_minx);
        miny = fminf(miny, shape_miny);
        maxx = fmaxf(maxx, shape_maxx);
        maxy = fmaxf(maxy, shape_maxy);
    }

    SDL_assert(minx != INFINITY && miny != INFINITY && maxx != -INFINITY && maxy != -INFINITY);
    SDL_assert(minx < maxx && miny < maxy);

    return rectf(minx, miny, maxx - minx, maxy - miny);
}
