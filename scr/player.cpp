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
    isHit = false;
    isDying = false;
    isDead = false;
    damage = 20;
    lookLeft = false;
    rect = SDL_FRect{100, 100, sizeSprite, sizeSprite};
    speed = 600.0f;
    maxHealth = 100;
    health = maxHealth;
}

Player::~Player()
{
    Unload();
}

void Player::initAnimations(){
    animations.idle = {2, 800, 0};
    animations.walk = {16, 94, 1};
    animations.atack = {4, 90, 5};
    animations.hit = {4, 90, 3};
    animations.death = {4, 90, 4}; 
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
    if (isDead) {
        return;
    }

    if (isDying) {
        bool deathFinished = spriteAnimation.Play(animations.death, false);
        if (deathFinished) {
            isDead = true;
        }
        return;
    }

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

    if (isHit) {
        bool hitFinished = spriteAnimation.Play(animations.hit, false);
        if (hitFinished) {
            isHit = false;
            spriteAnimation.Restart();
        }
    } else if (isAttacking) {
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

    if (health < 0 && !isDying) {
        isDying = true;
        isAttacking = false;
        isWalk = false;
        spriteAnimation.Restart();
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

SDL_FRect Player::GetHitbox() const
{
    return SDL_FRect{
        rect.x + 56.0f,
        rect.y + 64.0f,
        rect.w - 112.0f,
        rect.h - 76.0f
    };
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

float Player::GetHealthPercent() const
{
    if (maxHealth <= 0) {
        return 0.0f;
    }

    return static_cast<float>(health) / static_cast<float>(maxHealth);
}

int Player::GetDamage() const
{
    return damage;
}

bool Player::IsAttacking() const
{
    return isAttacking && !isDying && !isDead;
}

bool Player::IsDying() const
{
    return isDying;
}

bool Player::IsDead() const
{
    return isDead;
}

void Player::TakeDamage(int incomingDamage)
{
    if (isDying || isDead) {
        return;
    }

    health -= incomingDamage;

    if (health < 0) {
        isDying = true;
        isAttacking = false;
        isHit = false;
        isWalk = false;
        spriteAnimation.Restart();
    } else {
        isHit = true;
        isAttacking = false;
        spriteAnimation.Restart();
    }
}
