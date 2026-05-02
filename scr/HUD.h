#pragma once

#include <SDL3/SDL.h>

class HUD
{
public:
    HUD();

    void Load(int *value, float scale);
    void Render(SDL_Renderer *renderer) const;

private:
    int *value;
    float scale;
    float x;
    float y;
};