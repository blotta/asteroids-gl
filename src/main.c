#include <assert.h>
#include <glad/glad.h>
#include <stddef.h>
#include <stdint.h>
#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include "vmath.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

const int LOGICAL_WIDTH = 800;
const int LOGICAL_HEIGHT = 600;

#define COLOR_WHITE vec4(1.f, 1.f, 1.f, 1.f)

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
                                     "   float strength = (sin(time) * 0.5) + 0.5;\n"
                                     "   c.rgb *= strength;\n"
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

typedef struct
{
    SDL_Window* window;
    SDL_GLContext gl_context;
    Renderer renderer;

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

    update_viewport(state->window);

    if (!renderer_init(&state->renderer))
    {
        return SDL_APP_FAILURE;
    }

    *appstate = state;

    return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
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

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void* appstate)
{
    AppState* state = (AppState*)appstate;
    float time = (float)SDL_GetTicks() / 1000.f;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(state->renderer.shader_program);
    glBindVertexArray(state->renderer.vao);

    // uniforms
    glad_glUniform1f(state->renderer.uniforms[U_TIME], time);
    glUniformMatrix4fv(state->renderer.uniforms[U_VIEW], 1, GL_FALSE, (float*)&state->renderer.view);
    glUniformMatrix4fv(state->renderer.uniforms[U_PROJECTION], 1, GL_FALSE, (float*)&state->renderer.projection);

    // begin drawing
    renderer_begin(&state->renderer);

    // Vec2 pos = vec2(LOGICAL_WIDTH / 2.f, LOGICAL_HEIGHT / 2.f);
    // float size = (600.f / 2) - 2;
    // float wh = LOGICAL_WIDTH / 2.f;
    // float hh = LOGICAL_HEIGHT / 2.f;
    // renderer_push_triangle(&state->renderer,
    //                        vertex(vec3(pos.X - size, pos.Y - size, 0.f), vec2(-1.f, -1.f), COLOR_WHITE),
    //                        vertex(vec3(pos.X + size, pos.Y - size, 0.f), vec2(1.f, -1.f), COLOR_WHITE),
    //                        vertex(vec3(pos.X, pos.Y + size, 0.f), vec2(0.f, 1.f), COLOR_WHITE));

    // renderer_push_quad(&state->renderer, vertex(vec3(pos.X - wh, pos.Y - hh, 0.f), vec2(-1.f, -1.f), COLOR_WHITE),
    //                    vertex(vec3(pos.X + wh, pos.Y - hh, 0.f), vec2(1.f, -1.f), COLOR_WHITE),
    //                    vertex(vec3(pos.X + wh, pos.Y + hh, 0.f), vec2(1.f, 1.f), COLOR_WHITE),
    //                    vertex(vec3(pos.X - wh, pos.Y + hh, 0.f), vec2(0.f, 1.f), COLOR_WHITE));

    // draw 50 random quads
    for (int i = 0; i < 50; i++)
    {
        Vec2 rand_pos = vec2((float)rand() / RAND_MAX * LOGICAL_WIDTH, (float)rand() / RAND_MAX * LOGICAL_HEIGHT);
        float rand_wh = (float)rand() / RAND_MAX * 10.f;
        float rand_hh = (float)rand() / RAND_MAX * 10.f;
        renderer_push_quad(&state->renderer,
                           vertex(vec3(rand_pos.X - rand_wh, rand_pos.Y - rand_hh, 0.f), vec2(-1.f, -1.f), COLOR_WHITE),
                           vertex(vec3(rand_pos.X + rand_wh, rand_pos.Y - rand_hh, 0.f), vec2(1.f, -1.f), COLOR_WHITE),
                           vertex(vec3(rand_pos.X + rand_wh, rand_pos.Y + rand_hh, 0.f), vec2(1.f, 1.f), COLOR_WHITE),
                           vertex(vec3(rand_pos.X - rand_wh, rand_pos.Y + rand_hh, 0.f), vec2(0.f, 1.f), COLOR_WHITE));
    }

    renderer_end(&state->renderer);

    SDL_GL_SwapWindow(state->window);

    return SDL_APP_CONTINUE; /* carry on with the program! */
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
