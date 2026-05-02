#include "HUD.h"

HUD::HUD()
{
    value = NULL;
    //отступ
    x = 16.0f;
    y = 16.0f;
}

void HUD::Load(int *globalValue, float hudScale)
{
    value = globalValue;
    scale = hudScale;
}

void HUD::Render(SDL_Renderer *renderer) const
{
    if (value == NULL) {
        return;
    }

    SDL_SetRenderScale(renderer, scale, scale);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDebugTextFormat(renderer, x / scale, y / scale, "value: %d", *value);
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
}