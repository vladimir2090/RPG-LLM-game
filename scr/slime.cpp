#include "slime.h"

#include <SDL3_image/SDL_image.h>

Slime::Slime()
{
    initAnimations();

    texture = NULL;
    sizeSprite = 144;
    spriteAnimation.SetSpriteSize(sizeSprite);
    rect = SDL_FRect{500, 500, sizeSprite, sizeSprite};
    speed = 120.0f;
    isWalk = true;
    lookLeft = true;
    health = 30;
    moveDirection = -1;
    patrolStartX = rect.x;
    patrolDistance = 180.0f;
}

Slime::~Slime()
{
    Unload();
}

void Slime::initAnimations()
{
    animations.walk = {4, 120, 1};
}

bool Slime::Load(SDL_Renderer *renderer, const char *texturePath)
{
    texture = IMG_LoadTexture(renderer, texturePath);
    if (texture == NULL) {
        SDL_Log("IMG_LoadTexture failed: %s", SDL_GetError());
        return false;
    }

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    return true;
}

void Slime::Unload()
{
    SDL_DestroyTexture(texture);
    texture = NULL;
}

void Slime::Update(float deltaTime)
{
    float step = speed * deltaTime * moveDirection;
    rect.x += step;

    if (rect.x <= patrolStartX - patrolDistance) {
        moveDirection = 1;
        lookLeft = false;
    }
    if (rect.x >= patrolStartX + patrolDistance) {
        moveDirection = -1;
        lookLeft = true;
    }

    // как сделать прыжки??
    if (isWalk) {
        spriteAnimation.Play(animations.walk, true);
    } else {
        spriteAnimation.Play(animations.idle, true);
    }
}

void Slime::Render(SDL_Renderer *renderer) const
{
    SDL_FlipMode flip = lookLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderTextureRotated(renderer, texture, spriteAnimation.GetSourceRect(), &rect, 0.0, NULL, flip);
}

int *Slime::GetHealthPointer()
{
    return &health;
}
