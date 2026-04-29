#include "player.h"

#include <SDL3_image/SDL_image.h>

Player::Player()
    : texture(NULL),
      rect{100, 100, 192, 192},
      srcRect{0, 0, 192, 192},
      speed(600.0f)
{
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
}

void Player::Render(SDL_Renderer *renderer) const
{
    SDL_RenderFillRect(renderer, &rect);
    SDL_RenderTexture(renderer, texture, &srcRect, &rect);
}
