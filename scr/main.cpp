#define SDL_MAIN_USE_CALLBACKS 1
#include "player.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>

//надо будет поменять в будущем для более точных классов
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static Player player;

const Uint64 targetFps = 60;
const Uint64 targetFrameNs = 1000000000 / targetFps;
//выглядит страшно, но единственное что придумал это через bool привязать действия и сделать case для смены значения bool
bool moveUp = false;
bool moveDown = false;
bool moveLeft = false;
bool moveRight = false;

enum MoveAction
{
    MoveNone,
    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight
};

static MoveAction GetMoveActionFromKey(SDL_Keycode key)
{
    switch (key) {
        case SDLK_W:
        case 0x0446:  // Cyrillic 'ц'
        case 0x0426:  // Cyrillic 'Ц'
            return MoveUp;

        case SDLK_S:
        case 0x044b:  // Cyrillic 'ы'
        case 0x042b:  // Cyrillic 'Ы'
            return MoveDown;

        case SDLK_A:
        case 0x0444:  // Cyrillic 'ф'
        case 0x0424:  // Cyrillic 'Ф'
            return MoveLeft;

        case SDLK_D:
        case 0x0432:  // Cyrillic 'в'
        case 0x0412:  // Cyrillic 'В'
            return MoveRight;

        default:
            return MoveNone;
    }
}

static void SetMoveState(MoveAction action, bool pressed)
{
    switch (action) {
        case MoveUp:
            moveUp = pressed;
            break;
        case MoveDown:
            moveDown = pressed;
            break;
        case MoveLeft:
            moveLeft = pressed;
            break;
        case MoveRight:
            moveRight = pressed;
            break;
        default:
            break;
    }
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

    if (!player.Load(renderer, "/home/vladimir/dev/game/assets/sprites/knight.png")) {
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
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
            bool pressed = event->type == SDL_EVENT_KEY_DOWN;
            MoveAction action = GetMoveActionFromKey(event->key.key);

            std::cout << (pressed ? "key pressed: " : "key released: ")
                      << SDL_GetKeyName(event->key.key)
                      << " code=" << event->key.key
                      << std::endl;

            SetMoveState(action, pressed);
            break;
        }
        
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            std::cout << "mouse pressed: "
                      << GetMouseButtonName(event->button.button)
                      << " code=" << static_cast<int>(event->button.button)
                      << " clicks=" << static_cast<int>(event->button.clicks)
                      << " x=" << event->button.x
                      << " y=" << event->button.y
                      << std::endl;
            break;

        default:
            break;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    (void)appstate;

    static Uint64 lastFrameNs = 0;
    Uint64 frameStartNs = SDL_GetTicksNS();
    if (lastFrameNs == 0) {
        lastFrameNs = frameStartNs;
    }

    float deltaTime = (frameStartNs - lastFrameNs) / 1000000000.0f;
    lastFrameNs = frameStartNs;

    if (deltaTime > 0.05f) {
        deltaTime = 0.05f;
    }

    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
    SDL_RenderClear(renderer);

    player.Update(deltaTime, moveUp, moveDown, moveLeft, moveRight);
    player.Render(renderer);

    SDL_RenderPresent(renderer);

    //спим только оставшееся время кадра, чтобы держать targetFps
    Uint64 frameTimeNs = SDL_GetTicksNS() - frameStartNs;
    if (frameTimeNs < targetFrameNs) {
        SDL_DelayNS(targetFrameNs - frameTimeNs);
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    (void)appstate;
    (void)result;

    player.Unload();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
