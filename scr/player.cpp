#include "player.h"
#include <SDL3_image/SDL_image.h>
#include <algorithm>

Player::Player()
{   
    //надо дслеать красиво
    initAnimations();

    texture = NULL;
    sizeSprite = 192;
    spriteAnimation.SetSpriteSize(sizeSprite);
    isWalk = false;
    isAttacking = false;
    attackWasPressed = false;
    damage = 20;
    lookLeft = false;
    rect = SDL_FRect{100, 100, sizeSprite, sizeSprite};
    speed = 600.0f;
    health = 100;
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
        spriteAnimation.Restart();
    }

    if (isAttacking) {
        bool attackFinished = spriteAnimation.Play(animations.atack, false);
        if (attackFinished) {
            isAttacking = false;
            spriteAnimation.Restart();
        }
    } else if (isWalk) {
        spriteAnimation.Play(animations.walk, true);
    } else {
        spriteAnimation.Play(animations.idle, true);
    }

    attackWasPressed = attack;

    if (health <= 0) {
        health = 0;
    }
}

void Player::Render(SDL_Renderer *renderer, const SDL_FRect &camera) const
{
    SDL_FRect drawRect = rect;
    drawRect.x -= camera.x;
    drawRect.y -= camera.y;

    SDL_RenderFillRect(renderer, &drawRect);
    SDL_FlipMode flip = lookLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderTextureRotated(renderer, texture, spriteAnimation.GetSourceRect(), &drawRect, 0.0, NULL, flip);
}

SDL_FRect Player::GetRect() const
{
    return rect;
}

SDL_FRect Player::GetAttackRect() const
{
    float attackWidth = rect.w * 0.55f;
    float attackHeight = rect.h * 0.55f;
    float attackY = rect.y + (rect.h - attackHeight) / 2.0f;
    float attackX = lookLeft ? rect.x - attackWidth : rect.x + rect.w;

    return SDL_FRect{attackX, attackY, attackWidth, attackHeight};
}

int *Player::GetHealthPointer()
{
    return &health;
}

int Player::GetDamage() const
{
    return damage;
}

bool Player::IsAttacking() const
{
    return isAttacking;
}

bool Player::IsDead() const
{
    return health <= 0;
}

void Player::TakeDamage(int incomingDamage)
{
    health = std::max(0, health - incomingDamage);
}
