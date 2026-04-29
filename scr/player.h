#pragma once
#include <SDL3/SDL.h>

typedef struct
{
    int frames;
    int animationDelay;
    int y;
} animation;


class Player
{
public:
    Player();
    ~Player();

    bool Load(SDL_Renderer *renderer, const char *texturePath);
    void Unload();
    void Update(float deltaTime, bool moveUp, bool moveDown, bool moveLeft, bool moveRight);
    void Render(SDL_Renderer *renderer) const;

private:
    SDL_FRect rect;
    SDL_FRect srcRect;
    SDL_Texture *texture;
    float sizeSprite;
    float speed;
    animation idle;
    int currentIndex;
    Uint64 lastUpdate;
};
