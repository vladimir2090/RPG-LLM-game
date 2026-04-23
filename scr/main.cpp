#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
//надо будет поменять в будущем для более точных классов
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    (void)appstate;
    (void)argc;
    (void)argv;
    //простенький дебаг
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("GAME", 1280, 720, 0, &window, &renderer)) {
        SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    (void)appstate;

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    (void)appstate;

    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 100, 30, 200, 255);
    SDL_FRect rect;
    SDL_FRect rectBorder;

    rect.x = 100;
    rect.y = 100;
    rect.w = 100;
    rect.h = 100;
    SDL_RenderFillRect(renderer, &rect);

    rectBorder.x = 300;
    rectBorder.y = 100;
    rectBorder.w = 100;
    rectBorder.h = 100;
    SDL_RenderRect(renderer, &rectBorder);

    // разные линии
    SDL_RenderLine(renderer, 400, 100, 500, 150);

    SDL_FPoint lines[] = {
        {100, 250}, {200, 350}, {300, 400}, {350, 450}
    };
    SDL_RenderLines(renderer, lines, SDL_arraysize(lines));

    //точка
    SDL_RenderPoint(renderer, 50, 50);

    SDL_RenderPresent(renderer);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    (void)appstate;
    (void)result;

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
