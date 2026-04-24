#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>

//надо будет поменять в будущем для более точных классов
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
bool turnRect = false;
float speedRect = 1;
const Uint64 frameDelayMs = 16;

static bool IsMoveLeftKey(SDL_Keycode key) //смотреть ~44 строчку
{
    return key == SDLK_A || key == 0x0444 || key == 0x0424;  // A, Cyrillic 'ф'/'Ф'
}

//расшифровка кнопок мышки
static const char *GetMouseButtonName(Uint8 button)
{
    switch (button) {
        case SDL_BUTTON_LEFT:
            return "left";
        case SDL_BUTTON_MIDDLE:
            return "middle";
        case SDL_BUTTON_RIGHT:
            return "right";
        case SDL_BUTTON_X1:
            return "x1";
        case SDL_BUTTON_X2:
            return "x2";
        default:
            return "unknown";
    }
}

//объекты глобальные
SDL_FRect rect = {100, 100, 100, 100};

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

    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;

        /*
        case SDL_EVENT_KEY_DOWN:
            std::cout << "key pressed: "
                      << SDL_GetKeyName(event->key.key)
                      << " code=" << event->key.key
                      << std::endl;

            if (IsMoveLeftKey(event->key.key)) {
                std::cout << "move left pressed" << std::endl;
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            std::cout << "mouse pressed: "
                      << GetMouseButtonName(event->button.button)
                      << " code=" << static_cast<int>(event->button.button)
                      << " clicks=" << static_cast<int>(event->button.clicks)
                      << " x=" << event->button.x
                      << " y=" << event->button.y
                      << std::endl;
            break;
        */

        default:
            break;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    (void)appstate;

    Uint64 frameStart = SDL_GetTicks();

    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 100, 30, 200, 255);

    if (rect.x >= 1080 || rect.x < 100){
        turnRect = !turnRect;
    }
    
    rect.x = turnRect ? rect.x + speedRect : rect.x -speedRect;
    
    SDL_RenderFillRect(renderer, &rect);
    /*
    SDL_FRect rectBorder;
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
    */

    SDL_RenderPresent(renderer);

    //учитываем время логики и рендера, чтобы не спать лишние 16 мс каждый кадр
    Uint64 frameTime = SDL_GetTicks() - frameStart;
    if (frameTime < frameDelayMs) {
        SDL_Delay(static_cast<Uint32>(frameDelayMs - frameTime));
    }

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
