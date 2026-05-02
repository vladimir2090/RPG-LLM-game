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
    //может зря я накинул 2 bool для атаки??
    isAttacking = false;
    attackWasPressed = false;
    lookLeft = false;
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
    animations.atack = {4, 90, 5}; //атака
}

bool Player::playAnimation(const animation &animation, bool loop)
{
    Uint64 now = SDL_GetTicks();    //надо будет оптимизировать это

    if (currentAnimationY != animation.y) {
        currentAnimationY = animation.y;
        currentIndex = 0;
        lastUpdate = now;
        srcRect.x = 0;
        srcRect.y = animation.y * sizeSprite;
        return false;
    }

    Uint64 delay = now - lastUpdate;
    if (delay < static_cast<Uint64>(animation.animationDelay)) {
        return false;
    }

    lastUpdate = now;
    currentIndex++;
    
    //если атака уже идёт, новая не начинается
    if (currentIndex >= animation.frames) {
        if (!loop) {
            currentIndex = animation.frames - 1;
            srcRect.x = currentIndex * sizeSprite;
            srcRect.y = animation.y * sizeSprite;
            return true;
        }

        currentIndex = 0;
    }

    srcRect.x = currentIndex * sizeSprite;
    srcRect.y = animation.y * sizeSprite;
    return false;
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

void Player::Update(float deltaTime, bool moveUp, bool moveDown, bool moveLeft, bool moveRight, bool attack)
{
    float step = speed * deltaTime;
    isWalk = moveUp ||  moveRight || moveDown || moveLeft;

    if (moveUp) {
        rect.y -= step;
    }
    if (moveDown) {
        rect.y += step;
    }
    if (moveLeft) {
        rect.x -= step;
        lookLeft = true;
    }
    if (moveRight) {
        rect.x += step;
        lookLeft = false;
    }

    if (attack && !attackWasPressed && !isAttacking) {
        isAttacking = true;
        currentAnimationY = -1;
    }

    if (isAttacking) {
        bool attackFinished = playAnimation(animations.atack, false);
        if (attackFinished) {
            isAttacking = false;
            currentAnimationY = -1;
        }
    } else if (isWalk) {
        playAnimation(animations.walk, true);
    } else {
        playAnimation(animations.idle, true);
    }

    attackWasPressed = attack;
}

void Player::Render(SDL_Renderer *renderer) const
{
    SDL_RenderFillRect(renderer, &rect);
    SDL_FlipMode flip = lookLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderTextureRotated(renderer, texture, &srcRect, &rect, 0.0, NULL, flip);
}
