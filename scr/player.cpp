#include "player.h"

#include <SDL3_image/SDL_image.h>

Player::Player()
{   
    idle.animationDelay = 100;
    idle.frames = 4;
    idle.y = 0;
    texture = NULL;
    sizeSprite = 192;
    currentIndex = 0;
    lastUpdate = 0;
    rect = SDL_FRect{100, 100, sizeSprite, sizeSprite};
    srcRect = SDL_FRect{0, idle.y * sizeSprite, sizeSprite, sizeSprite};
    speed = 600.0f;
}

Player::~Player()
{
    Unload();
}

bool Player::Load(SDL_Renderer *renderer, const char *texturePath)
{
    texture = IMG_LoadTexture(renderer, texturePath);
    if (texture == NULL) {
        SDL_Log("IMG_LoadTexture failed: %s", SDL_GetError());
        return false;
    }

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    return true;
}

void Player::Unload()
{
    SDL_DestroyTexture(texture);
    texture = NULL;
}

void Player::Update(float deltaTime, bool moveUp, bool moveDown, bool moveLeft, bool moveRight)
{
    float step = speed * deltaTime;

    if (moveUp) {
        rect.y -= step;
    }
    if (moveDown) {
        rect.y += step;
    }
    if (moveLeft) {
        rect.x -= step;
    }
    if (moveRight) {
        rect.x += step;
    }

    Uint64 now = SDL_GetTicks();
    Uint64 delay = now - lastUpdate;
    if(delay >= static_cast<Uint64>(idle.animationDelay)){
        lastUpdate = now;
        currentIndex++;
        if (currentIndex >= idle.frames){
            currentIndex = 0;
        }
        srcRect.x = currentIndex * sizeSprite;
        srcRect.y = idle.y * sizeSprite;
    }
}

void Player::Render(SDL_Renderer *renderer) const
{
    SDL_RenderFillRect(renderer, &rect);
    SDL_RenderTexture(renderer, texture, &srcRect, &rect);
}
