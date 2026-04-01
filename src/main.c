#include "SDL3/SDL_events.h"
#include "SDL3/SDL_video.h"
#include <glad/glad.h>
#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

const int LOGICAL_WIDTH = 800;
const int LOGICAL_HEIGHT = 600;

typedef struct {
  SDL_Window *window;
  SDL_GLContext gl_context;
} AppState;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
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

  SDL_Window *window = SDL_CreateWindow(
      "ASTEROIDS!", (int)(LOGICAL_WIDTH * scale), (int)(LOGICAL_HEIGHT * scale),
      SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
  if (!window) {
    SDL_Log("Couldn't create window: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_GLContext gl_context = SDL_GL_CreateContext(window);
  if (!gl_context) {
    SDL_Log("Couldn't create OpenGL Context: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
    SDL_Log("Couldn't initialize GLAD");
    return SDL_APP_FAILURE;
  }

  int w, h;
  SDL_GetWindowSizeInPixels(window, &w, &h);
  glViewport(0, 0, w, h);

  AppState *state = (AppState *)SDL_calloc(1, sizeof(AppState));
  state->window = window;
  state->gl_context = gl_context;
  *appstate = state;

  return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  AppState *state = (AppState *)appstate;

  switch (event->type) {
  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS; /* end the program, reporting success to the OS. */
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
SDL_AppResult SDL_AppIterate(void *appstate) {
  AppState *state = (AppState *)appstate;

  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  SDL_GL_SwapWindow(state->window);

  return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  if (appstate) {
    AppState *state = (AppState *)appstate;
    SDL_GL_DestroyContext(state->gl_context);
    SDL_DestroyWindow(state->window);
    SDL_free(state);
  }
  SDL_Quit();
}
