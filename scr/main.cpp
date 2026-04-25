#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>

//надо будет поменять в будущем для более точных классов
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

float speedRect = 10;
const Uint64 frameDelayMs = 16;
//выглядит страшно, но единственное что придумал это через bool привязать действия и сделать case для смены значения bool 
bool moveUp = false;
bool moveDown = false;
bool moveLeft = false;
bool moveRight = false;

//объекты глобальные
SDL_FRect rect = {130, 190, 100, 100};

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

    Uint64 frameStart = SDL_GetTicks();

    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
    SDL_RenderClear(renderer);

    if (moveUp) {
        rect.y -= speedRect;
    }
    if (moveDown) {
        rect.y += speedRect;
    }
    if (moveLeft) {
        rect.x -= speedRect;
    }
    if (moveRight) {
        rect.x += speedRect;
    }

    SDL_RenderFillRect(renderer, &rect);

    SDL_Texture* player = IMG_LoadTexture(renderer, "/home/vladimir/dev/game/assets/custom/knight-main.png");
    SDL_RenderTexture(renderer, player, NULL, &rect);

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
