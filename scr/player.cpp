#include "player.h"
#include <SDL3_image/SDL_image.h>

Player::Player()
{   
    //надо дслеать красиво
    initAnimations();

    texture = NULL;
    sizeSprite = 192;
    spriteAnimation.SetSpriteSize(sizeSprite);
    isWalk = false;
    //может зря я накинул 2 bool для атаки??
    isAttacking = false;
    attackWasPressed = false;
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

    if (health <= 0){
        //а как игру то сейвить и перса килять???
        //я конечно могу сделать рогалик, но стоит?
    } 
}

void Player::Render(SDL_Renderer *renderer) const
{
    SDL_RenderFillRect(renderer, &rect);
    SDL_FlipMode flip = lookLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderTextureRotated(renderer, texture, spriteAnimation.GetSourceRect(), &rect, 0.0, NULL, flip);
}

int *Player::GetHealthPointer()
{
    return &health;
}
