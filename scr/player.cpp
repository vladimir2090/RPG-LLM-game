#include "player.h"

#include <SDL3_image/SDL_image.h>

Player::Player()
{   
    initAnimations();

    texture = NULL;
    sizeSprite = 192;
    currentIndex = 0;
    lastUpdate = 0;
    isWalk = false;
    currentAnimationY = animations.idle.y;
    rect = SDL_FRect{100, 100, sizeSprite, sizeSprite};
    srcRect = SDL_FRect{0, animations.idle.y * sizeSprite, sizeSprite, sizeSprite};
    speed = 600.0f;
}

Player::~Player()
{
    Unload();
}

void Player::initAnimations(){
    animations.idle = {2, 800, 0};
    animations.walk = {16, 94, 1}; //я протестировал сам скороть под 94 взависимости от скорости 600
}

void Player::playAnimation(const animation &animation)
{
    Uint64 now = SDL_GetTicks();

    if (currentAnimationY != animation.y) {
        currentAnimationY = animation.y;
        currentIndex = 0;
        lastUpdate = now;
        srcRect.x = 0;
        srcRect.y = animation.y * sizeSprite;
        return;
    }

    Uint64 delay = now - lastUpdate;
    if (delay < static_cast<Uint64>(animation.animationDelay)) {
        return;
    }

    lastUpdate = now;
    currentIndex++;

    if (currentIndex >= animation.frames) {
        currentIndex = 0;
    }

    srcRect.x = currentIndex * sizeSprite;
    srcRect.y = animation.y * sizeSprite;
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
    isWalk = moveUp || moveDown || moveLeft || moveRight;

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

    if (isWalk) {
        playAnimation(animations.walk);
    } else {
        playAnimation(animations.idle);
    }
}

void Player::Render(SDL_Renderer *renderer) const
{
    SDL_RenderFillRect(renderer, &rect);
    SDL_RenderTexture(renderer, texture, &srcRect, &rect);
}
