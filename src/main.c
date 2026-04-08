#include <assert.h>
#include <glad/glad.h>
#include <stddef.h>
#include <stdint.h>
#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include "vmath.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

const int LOGICAL_WIDTH = 1024;
const int LOGICAL_HEIGHT = 576;
#define USE_VSYNC 1
#define TARGET_FPS 60

#define COLOR_WHITE vec4(1.f, 1.f, 1.f, 1.f)
#define COLOR_BLACK vec4(0.f, 0.f, 0.f, 1.f)
#define COLOR_CYAN vec4(0.f, 1.f, 1.f, 1.f)
#define COLOR_RED vec4(1.f, 0.f, 0.f, 1.f)
#define COLOR_GREEN vec4(0.f, 1.f, 0.f, 1.f)
#define COLOR_BLUE vec4(0.f, 0.f, 1.f, 1.f)

const char* vertex_shader_source = "#version 330 core\n"
                                   "layout (location = 0) in vec3 aPos;\n"
                                   "layout (location = 1) in vec2 aUV;\n"
                                   "layout (location = 2) in vec4 aColor;\n"
                                   // "uniform mat4 uModel;\n"
                                   "uniform mat4 uView;\n"
                                   "uniform mat4 uProjection;\n"
                                   "out vec2 vUV;\n"
                                   "out vec4 vColor;\n"
                                   "void main() {\n"
                                   "   vUV = aUV;\n"
                                   "   vColor = aColor;\n"
                                   "   gl_Position = uProjection * uView * vec4(aPos, 1.0);\n"
                                   "}\0";

const char* fragment_shader_source = "#version 330 core\n"
                                     "uniform float time;\n"
                                     "in vec2 vUV;\n"
                                     "in vec4 vColor;\n"
                                     "out vec4 FragColor;\n"
                                     "void main() {\n"
                                     "   vec4 c = vec4(vColor.rgb, vColor.a);\n"
                                     // "   float strength = (sin(time) * 0.5) + 0.5;\n"
                                     // "   c.rgb *= strength;\n"
                                     "   FragColor = c;\n"
                                     "}\n\0";

typedef enum
{
    VA_POS = 0,
    VA_UV,
    VA_COLOR,
    COUNT_VAS,
} VERT_ATTRIB;

typedef enum
{
    U_TIME = 0,
    // U_MODEL,
    U_VIEW,
    U_PROJECTION,
    U_COLOR,
    COUNT_UNIFORMS,
} UNIFORMS;

// clang-format off
static const char* uniform_names[COUNT_UNIFORMS] = {
    [U_TIME] = "time",
    // [U_MODEL] = "uModel",
    [U_VIEW] = "uView",
    [U_PROJECTION] = "uProjection",
    [U_COLOR] = "uColor",
};
// clang-format on

typedef struct
{
    Vec3 pos;
    Vec2 uv;
    Vec4 color;
} Vertex;

Vertex vertex(Vec3 pos, Vec2 uv, Vec4 color)
{
    return (Vertex){pos, uv, color};
}

GLuint compile_shader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE)
    {
        GLchar message[1024];
        GLsizei log_length = 0;
        glGetShaderInfoLog(shader, 1024, &log_length, message);
        SDL_Log("Shader compilation error: %s", message);
        return -1;
    }
    return shader;
}

#define VERTEX_BUF_CAP 1024
typedef struct
{
    GLuint shader_program;
    GLint uniforms[COUNT_UNIFORMS];
    GLuint vao;
    GLuint vbo;

    Vertex vertex_buf[VERTEX_BUF_CAP];
    size_t vertex_buf_sz;

    Mat4 view;
    Mat4 projection;
} Renderer;

void renderer_begin(Renderer* r)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(r->shader_program);
    glBindVertexArray(r->vao);

    r->vertex_buf_sz = 0;
}

void renderer_push_vertex_values(Renderer* r, Vec3 pos, Vec2 uv, Vec4 color)
{
    assert(r->vertex_buf_sz < VERTEX_BUF_CAP);
    r->vertex_buf[r->vertex_buf_sz].pos = pos;
    r->vertex_buf[r->vertex_buf_sz].uv = uv;
    r->vertex_buf[r->vertex_buf_sz].color = color;
    r->vertex_buf_sz += 1;
}

void renderer_push_vertex(Renderer* r, Vertex v)
{
    assert(r->vertex_buf_sz < VERTEX_BUF_CAP);
    r->vertex_buf[r->vertex_buf_sz] = v;
    r->vertex_buf_sz += 1;
}

void renderer_end(Renderer* r)
{
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * r->vertex_buf_sz, r->vertex_buf);
    glDrawArrays(GL_TRIANGLES, 0, (GLint)r->vertex_buf_sz);
}

int renderer_init(Renderer* r)
{
    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);

    if (!vertex_shader || !fragment_shader)
        return -1;

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    GLint shader_program_linked;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &shader_program_linked);
    if (!shader_program_linked)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetProgramInfoLog(shader_program, 1024, &log_length, message);
        SDL_Log("Error linking shader program: %s", message);
        return -1;
    }
    r->shader_program = shader_program;

    for (int i = 0; i < COUNT_UNIFORMS; i++)
    {
        r->uniforms[i] = glGetUniformLocation(r->shader_program, uniform_names[i]);
    }

    glGenBuffers(1, &r->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(Vertex) * VERTEX_BUF_CAP), NULL,
                 GL_DYNAMIC_DRAW); // only allocate

    glGenVertexArrays(1, &r->vao);
    glBindVertexArray(r->vao);
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);

    glVertexAttribPointer(VA_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(VA_POS);

    glVertexAttribPointer(VA_UV, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(VA_UV);

    glVertexAttribPointer(VA_COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(VA_COLOR);

    r->view = HMM_M4D(1.f);
    // orthographic projection: origin at bottom-left
    r->projection = Orthographic_RH_NO(0.f, (float)LOGICAL_WIDTH,  // left, right
                                       0.f, (float)LOGICAL_HEIGHT, // bottom, top
                                       -1.f, 1.f);                 // near, far

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return 1;
}

void renderer_push_triangle(Renderer* r, Vertex a, Vertex b, Vertex c)
{
    renderer_push_vertex(r, a);
    renderer_push_vertex(r, b);
    renderer_push_vertex(r, c);
}

// bl, br, tr, tl
void renderer_push_quad(Renderer* r, Vertex a, Vertex b, Vertex c, Vertex d)
{
    renderer_push_vertex(r, a);
    renderer_push_vertex(r, b);
    renderer_push_vertex(r, c);

    renderer_push_vertex(r, c);
    renderer_push_vertex(r, d);
    renderer_push_vertex(r, a);
}

void update_viewport(SDL_Window* window)
{
    int w, h;
    SDL_GetWindowSizeInPixels(window, &w, &h);

    float target_aspect = (float)LOGICAL_WIDTH / (float)LOGICAL_HEIGHT;
    float window_aspect = (float)w / (float)h;

    int vp_w, vp_h;
    if (window_aspect > target_aspect)
    {
        /* Window is too wide → pillarbox (bars on left/right) */
        vp_h = h;
        vp_w = (int)(h * target_aspect);
    }
    else
    {
        /* Window is too tall → letterbox (bars on top/bottom) */
        vp_w = w;
        vp_h = (int)(w / target_aspect);
    }

    int vp_x = (w - vp_w) / 2;
    int vp_y = (h - vp_h) / 2;

    glViewport(vp_x, vp_y, vp_w, vp_h);
    SDL_Log("resize/pixel size changed: %dx%d", w, h);
}

static Uint8 _key_state[SDL_SCANCODE_COUNT];

void input_update(void)
{
    // 0 not pressed
    // 1 pressed
    // 2 just pressed
    // 3 just released

    for (Uint16 i = 0; i < SDL_SCANCODE_COUNT; i++)
    {
        // just pressed becomes pressed
        if (_key_state[i] == 2)
            _key_state[i] = 1;

        // just released becomes not pressed
        if (_key_state[i] == 3)
            _key_state[i] = 0;
    }
}

void input_handle_event(SDL_Event* event)
{
    switch (event->type)
    {
    case SDL_EVENT_KEY_DOWN: {
        if (_key_state[event->key.scancode] == 0)
            _key_state[event->key.scancode] = 2; // just pressed
    }
    break;
    case SDL_EVENT_KEY_UP: {
        _key_state[event->key.scancode] = 3; // just released
    }
    break;
    }
}

bool input_key_pressed(SDL_Scancode scancode)
{
    return _key_state[scancode] == 1 || _key_state[scancode] == 2;
}

bool input_key_just_pressed(SDL_Scancode scancode)
{
    return _key_state[scancode] == 2;
}

bool input_key_just_released(SDL_Scancode scancode)
{
    return _key_state[scancode] == 3;
}

#define MIN_ASTEROID_POINTS 6
#define MAX_ASTEROID_POINTS 20
typedef struct
{
    Vec2 pos;
    Vec2 vel;
    float base_radius;
    Vec4 color;
    Vec2 points[MAX_ASTEROID_POINTS];
    int point_count;
} Asteroid;

typedef struct
{
    Vec2 pos;
    float angle;
    float size;
    Vec2 vel;
    Vec2 acc;
    Vec4 color;
} Ship;

#define MAX_ASTEROIDS 100
typedef struct
{
    Ship ship;
    Asteroid asteroids[MAX_ASTEROIDS];
    int asteroid_count;
} Game;

void game_init(Game* game)
{
    // ship
    game->ship.pos = vec2(LOGICAL_WIDTH / 2.f, LOGICAL_HEIGHT / 2.f);
    game->ship.angle = SDL_PI_F / 2.f;
    game->ship.vel = vec2(0.f, 0.f);
    game->ship.acc = vec2(0.f, 0.f);
    game->ship.size = 15.f;
    game->ship.color = COLOR_WHITE;

    // asteroids
    float max_speed = 100.f;
    // add 10 asteroids
    for (int i = 0; i < 10; i++)
    {
        Asteroid a;
        a.pos = vec2(SDL_randf() * LOGICAL_WIDTH, SDL_randf() * LOGICAL_HEIGHT);
        a.vel = vec2(SDL_randf() * max_speed - max_speed / 2.f, SDL_randf() * max_speed - max_speed / 2.f);
        a.base_radius = 5.f + SDL_randf() * 30.f;
        a.color = COLOR_WHITE;
        a.point_count = MIN_ASTEROID_POINTS + SDL_rand(MAX_ASTEROID_POINTS - MIN_ASTEROID_POINTS);
        for (int p = 0; p < a.point_count; p++)
        {
            // place points around pos
            float offset = a.base_radius + (SDL_randf() * 2.f - 1.f) * a.base_radius / 4.f;
            float angle = ((float)p / a.point_count) * 2.f * SDL_PI_F;
            a.points[p] = vec2(offset * cos(angle), offset * sin(angle));
        }
        game->asteroids[i] = a;
        game->asteroid_count++;
    }
}

void game_update(Game* game, float dt)
{
    bool input_thrust = input_key_pressed(SDL_SCANCODE_W);
    float input_rot = 0;
    if (input_key_pressed(SDL_SCANCODE_A))
    {
        input_rot = 1.f;
    }
    if (input_key_pressed(SDL_SCANCODE_D))
    {
        input_rot = -1.f;
    }

    // ship
    Ship* ship = &game->ship;
    ship->angle += input_rot * 2.5f * dt;

    if (input_thrust)
    {
        float thrust = 1000.f;
        ship->acc.X += cos(ship->angle) * thrust * dt;
        ship->acc.Y += sin(ship->angle) * thrust * dt;
    }

    ship->vel.X += ship->acc.X * dt;
    ship->vel.Y += ship->acc.Y * dt;
    ship->vel = HMM_MulV2F(ship->vel, 0.99f); // linear damping
    ship->acc = HMM_MulV2F(ship->acc, 0.9f);
    ship->pos.X += ship->vel.X * dt;
    ship->pos.Y += ship->vel.Y * dt;

    // wrap ship around screen edges
    if (ship->pos.X + ship->size < 0)
        ship->pos.X = LOGICAL_WIDTH + ship->size;
    if (ship->pos.X - ship->size > LOGICAL_WIDTH)
        ship->pos.X = -ship->size;
    if (ship->pos.Y + ship->size < 0)
        ship->pos.Y = LOGICAL_HEIGHT + ship->size;
    if (ship->pos.Y - ship->size > LOGICAL_HEIGHT)
        ship->pos.Y = -ship->size;

    // asteroids
    for (int i = 0; i < game->asteroid_count; i++)
    {
        Asteroid* a = &game->asteroids[i];
        a->pos.X += a->vel.X * dt;
        a->pos.Y += a->vel.Y * dt;

        // wrap around screen edges
        if (a->pos.X + a->base_radius < 0)
            a->pos.X = LOGICAL_WIDTH + a->base_radius;
        if (a->pos.X - a->base_radius > LOGICAL_WIDTH)
            a->pos.X = -a->base_radius;
        if (a->pos.Y + a->base_radius < 0)
            a->pos.Y = LOGICAL_HEIGHT + a->base_radius;
        if (a->pos.Y - a->base_radius > LOGICAL_HEIGHT)
            a->pos.Y = -a->base_radius;
    }
}

void game_draw(Game* game, Renderer* renderer)
{
    // ship
    Ship* ship = &game->ship;
    Vec2 p1 = HMM_RotateV2(vec2(ship->size, 0), ship->angle);
    Vec2 p2 = HMM_RotateV2(vec2(ship->size, 0), ship->angle + 135 * HMM_DegToRad);
    Vec2 p3 = HMM_RotateV2(vec2(ship->size, 0), ship->angle - 135 * HMM_DegToRad);
    renderer_push_triangle(renderer,
                           vertex(vec3(ship->pos.X + p1.X, ship->pos.Y + p1.Y, 0), vec2(1.f, 0.5f), ship->color),
                           vertex(vec3(ship->pos.X + p2.X, ship->pos.Y + p2.Y, 0), vec2(0, 1.f), ship->color),
                           vertex(vec3(ship->pos.X + p3.X, ship->pos.Y + p3.Y, 0), vec2(0, 0), ship->color));

    // asteroids
    for (int i = 0; i < game->asteroid_count; i++)
    {
        Asteroid* ast = &game->asteroids[i];
        for (int i = 0; i < ast->point_count; i++)
        {
            Vec2 p1 = ast->points[i];
            Vec2 p2 = ast->points[(i + 1) % ast->point_count];
            Vertex va = vertex(vec3(ast->pos.X, ast->pos.Y, 0), vec2(.5f, .5f), ast->color);
            Vertex vb = vertex(vec3(ast->pos.X + p1.X, ast->pos.Y + p1.Y, 0), vec2(1, 0), ast->color);
            Vertex vc = vertex(vec3(ast->pos.X + p2.X, ast->pos.Y + p2.Y, 0), vec2(1, 1), ast->color);
            renderer_push_triangle(renderer, va, vb, vc);
        }
    }
}

typedef struct
{
    double dt;
    Uint64 now;
    Uint64 last;

    Uint64 frame_last;
    Uint64 frame_delay;
    Uint64 frame_time;

    Uint32 frame_rate;
    Uint32 frame_count;
} FrameTiming;

void frame_timing_init(FrameTiming* time, int frame_rate)
{
    time->frame_rate = frame_rate;
    time->frame_delay = SDL_NS_PER_SECOND / frame_rate;
    time->last = SDL_GetTicksNS();
    time->frame_last = time->last;
}

void frame_timing_update(FrameTiming* time)
{
    time->now = SDL_GetTicksNS();
    time->dt = (float)((double)(time->now - time->last) / (double)SDL_NS_PER_SECOND);
    time->last = time->now;

    time->frame_count++;
    if (time->now - time->frame_last >= SDL_NS_PER_SECOND)
    {
        time->frame_rate = time->frame_count;
        time->frame_count = 0;
        time->frame_last = time->now;
    }
}

void frame_timing_update_late(FrameTiming* time, bool should_delay)
{
    time->frame_time = SDL_GetTicksNS() - time->now;

    if (should_delay && time->frame_delay > time->frame_time)
    {
        Uint64 requested_delay = time->frame_delay - time->frame_time;
        Uint64 before_delay = SDL_GetTicksNS();
        SDL_DelayPrecise(requested_delay);
        Uint64 actual_delay = SDL_GetTicksNS() - before_delay;
        Uint64 diff = actual_delay - requested_delay;
        Uint64 diff_ms = SDL_NS_TO_MS(diff);
        if (diff_ms > 0)
            SDL_Log("Delay diff: %llu ms", diff_ms);
    }
}

typedef struct
{
    SDL_Window* window;
    SDL_GLContext gl_context;
    Renderer renderer;
    Game game;
    bool use_vsync;
    FrameTiming time;
} AppState;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    (void)argc; // silences -Wunused-parameter
    (void)argv;

    AppState* state = (AppState*)SDL_calloc(1, sizeof(AppState));

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_Log("Scale: %f", scale);

    state->window = SDL_CreateWindow("ASTEROIDS!", (int)(LOGICAL_WIDTH * scale), (int)(LOGICAL_HEIGHT * scale),
                                     SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!state->window)
    {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    state->gl_context = SDL_GL_CreateContext(state->window);
    if (!state->gl_context)
    {
        SDL_Log("Couldn't create OpenGL Context: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        SDL_Log("Couldn't initialize GLAD");
        return SDL_APP_FAILURE;
    }

    SDL_DisplayID display_id = SDL_GetDisplayForWindow(state->window);
    const SDL_DisplayMode* display_mode = SDL_GetDesktopDisplayMode(display_id);
    // Try to use vsync first
    if (USE_VSYNC && SDL_GL_SetSwapInterval(1))
    {
        SDL_Log("Using VSync at %f Hz", display_mode->refresh_rate);
        state->use_vsync = true;
    }
    else
    {
        state->use_vsync = false;
        SDL_Log("Unable to set VSync! Error: %s\n", SDL_GetError());
        SDL_GL_SetSwapInterval(0);
        SDL_Log("Falling back to VSync Off at %d Hz", TARGET_FPS);
    }
    frame_timing_init(&state->time, TARGET_FPS);

    update_viewport(state->window);

    if (!renderer_init(&state->renderer))
    {
        return SDL_APP_FAILURE;
    }

    game_init(&state->game);

    *appstate = state;

    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void* appstate)
{
    AppState* state = (AppState*)appstate;

    frame_timing_update(&state->time);

    input_update();
    game_update(&state->game, state->time.dt);

    // uniforms
    glad_glUniform1f(state->renderer.uniforms[U_TIME], (float)SDL_GetTicks() / 1000.f);
    glUniformMatrix4fv(state->renderer.uniforms[U_VIEW], 1, GL_FALSE, (float*)&state->renderer.view);
    glUniformMatrix4fv(state->renderer.uniforms[U_PROJECTION], 1, GL_FALSE, (float*)&state->renderer.projection);
    renderer_begin(&state->renderer);

    game_draw(&state->game, &state->renderer);

    renderer_end(&state->renderer);
    SDL_GL_SwapWindow(state->window);

    frame_timing_update_late(&state->time, !state->use_vsync);

    SDL_Log("FPS: %d", state->time.frame_rate);

    return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    input_handle_event(event);

    AppState* state = (AppState*)appstate;

    switch (event->type)
    {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS; /* end the program, reporting success to the OS. */
    case SDL_EVENT_WINDOW_RESIZED:
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
        update_viewport(state->window);
        break;
    }
    }

    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    (void)result;
    if (appstate)
    {
        AppState* state = (AppState*)appstate;
        SDL_GL_DestroyContext(state->gl_context);
        SDL_DestroyWindow(state->window);
        glDeleteVertexArrays(1, &state->renderer.vao);
        glDeleteProgram(state->renderer.shader_program);
        SDL_free(state);
    }
    SDL_Quit();
}
