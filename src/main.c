#include <assert.h>
#include <glad/glad.h>
#include <stddef.h>
#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include "vmath.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

const int LOGICAL_WIDTH = 800;
const int LOGICAL_HEIGHT = 600;

const char* vertex_shader_source = "#version 330 core\n"
                                   "layout (location = 0) in vec3 aPos;\n"
                                   "layout (location = 1) in vec2 aUV;\n"
                                   "layout (location = 2) in vec4 aColor;\n"
                                   "out vec2 vUV;\n"
                                   "out vec4 vColor;\n"
                                   "void main() {\n"
                                   "   vUV = aUV;\n"
                                   "   vColor = aColor;\n"
                                   "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
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
    COUNT_UNIFORMS,
} UNIFORMS;
static const char* uniform_names[COUNT_UNIFORMS] = {
    [U_TIME] = "time"
};

typedef struct
{
    Vec3 pos;
    Vec2 uv;
    Vec4 color;
} Vertex;

#define VERTEX_BUF_CAP 1024
typedef struct
{
    GLuint shader_program;
    GLint uniforms[COUNT_UNIFORMS];
    GLuint vao;
    GLuint vbo;
    Vertex vertex_buf[VERTEX_BUF_CAP];
    size_t vertex_buf_sz;
} Renderer;

int renderer_init(Renderer* r)
{
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    GLint vertex_compiled;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_compiled);
    if (vertex_compiled != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetShaderInfoLog(vertex_shader, 1024, &log_length, message);
        SDL_Log("Error compiling vertex shader: %s", message);
        return -1;
    }

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    GLint fragment_compiled;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_compiled);
    if (fragment_compiled != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetShaderInfoLog(fragment_shader, 1024, &log_length, message);
        SDL_Log("Error compiling fragment shader: %s", message);
        return -1;
    }

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

    r->vertex_buf_sz = 0;

    glGenBuffers(1, &r->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(r->vertex_buf), r->vertex_buf, GL_DYNAMIC_DRAW);

    glGenVertexArrays(1, &r->vao);
    glBindVertexArray(r->vao);

    glVertexAttribPointer(VA_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(VA_POS);

    glVertexAttribPointer(VA_UV, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(VA_UV);

    glVertexAttribPointer(VA_COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(VA_COLOR);

    r->uniforms[U_TIME] = glGetUniformLocation(r->shader_program, uniform_names[U_TIME]);

    return 1;
}

void renderer_push_vertex(Renderer* r, Vec3 pos, Vec2 uv, Vec4 color)
{
    assert(r->vertex_buf_sz < VERTEX_BUF_CAP);
    r->vertex_buf[r->vertex_buf_sz].pos = pos;
    r->vertex_buf[r->vertex_buf_sz].uv = uv;
    r->vertex_buf[r->vertex_buf_sz].color = color;
    r->vertex_buf_sz += 1;
}

void renderer_push_rect(Renderer* r, SDL_FRect rect)
{
    Vec4 color = vec4(1.f, 1.f, 1.f, 1.f);
    renderer_push_vertex(r, vec3(rect.x, rect.y, 0), vec2(0.f, 0.f), color);          // a
    renderer_push_vertex(r, vec3(rect.x + rect.w, rect.y, 0), vec2(1.f, 0.f), color); // b
    renderer_push_vertex(r, vec3(rect.x, rect.y + rect.h, 0), vec2(0.f, 1.f), color); // c

    renderer_push_vertex(r, vec3(rect.x, rect.y + rect.h, 0), vec2(0.f, 1.f), color);          // c
    renderer_push_vertex(r, vec3(rect.x + rect.w, rect.y, 0), vec2(1.f, 0.f), color);          // b
    renderer_push_vertex(r, vec3(rect.x + rect.w, rect.y + rect.h, 0), vec2(1.f, 1.f), color); // d
}

void renderer_sync_buf(Renderer* r)
{
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * r->vertex_buf_sz, r->vertex_buf);
}

void renderer_sync_uniforms(Renderer* r, GLfloat time)
{
    glUniform1f(r->uniforms[U_TIME], time);
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

    // SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    // SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    // SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_Log("Scale: %f", scale);

    SDL_Window* window = SDL_CreateWindow("ASTEROIDS!", (int)(LOGICAL_WIDTH * scale), (int)(LOGICAL_HEIGHT * scale),
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!window)
    {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    state->window = window;

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context)
    {
        SDL_Log("Couldn't create OpenGL Context: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    state->gl_context = gl_context;

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        SDL_Log("Couldn't initialize GLAD");
        return SDL_APP_FAILURE;
    }

    int w, h;
    SDL_GetWindowSizeInPixels(window, &w, &h);
    glViewport(0, 0, w, h);

    if (!renderer_init(&state->renderer)) {
        return SDL_APP_FAILURE;
    }

    renderer_push_rect(&state->renderer, (SDL_FRect){-0.5f, -0.5f, 1.f, 1.f});
    renderer_push_rect(&state->renderer, (SDL_FRect){-0.25f, -0.25f, 1.f, 1.f});
    renderer_sync_buf(&state->renderer);

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
        return SDL_APP_SUCCESS; /* end the program, reporting success to the OS.
                                 */
    case SDL_EVENT_WINDOW_RESIZED:
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
        int w, h;
        SDL_GetWindowSizeInPixels(state->window, &w, &h);
        glViewport(0, 0, w, h);
        SDL_Log("resize/pixel size changed: %dx%d", w, h);
        break;
    }
    }

    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void* appstate)
{
    AppState* state = (AppState*)appstate;

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(state->renderer.shader_program);
    glBindVertexArray(state->renderer.vao);

    renderer_sync_uniforms(&state->renderer, SDL_GetTicks()/1000.f);

    // glDrawArrays(GL_TRIANGLES, 0, 4);
    glDrawArrays(GL_TRIANGLES, 0, state->renderer.vertex_buf_sz);

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
