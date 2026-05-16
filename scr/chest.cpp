#include "chest.h"
#include <SDL3_image/SDL_image.h>

Chest::Chest() {
    spriteAnimation.SetSpriteSize(sizeSprite);
}

bool Chest::Load(SDL_Renderer* renderer, const char* texturePath) {
    texture = IMG_LoadTexture(renderer, texturePath);
    if (!texture) {
        SDL_Log("Chest texture load failed: %s", SDL_GetError());
        return false;
    }
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    return true;
}

void Chest::Unload() {
    if (texture) { SDL_DestroyTexture(texture); texture = nullptr; }
}

int Chest::Update(const SDL_FRect& playerRect) {
    if (opened) {
        spriteAnimation.Play(animOpened, true);
        return 0;
    }
    if (isOpening) {
        bool finished = spriteAnimation.Play(animOpening, false);
        if (finished) opened = true;
        return 0;
    }

    
    spriteAnimation.Play(animClosed, true);
    
    if (SDL_HasRectIntersectionFloat(&playerRect, &rect)) {
        isOpening = true;
        spriteAnimation.Restart();
        return weaponDamage;
    }
    return 0;
}

void Chest::SetTilePosition(int tileX, int tileY, int tileSize) {
    float offsetX = (tileSize - ChestConfig::RenderWidth) * 0.5f;
    float offsetY = tileSize - ChestConfig::RenderHeight - ChestConfig::BottomMargin;

    rect = SDL_FRect{
        tileX * tileSize + offsetX,
        tileY * tileSize + offsetY,
        ChestConfig::RenderWidth,
        ChestConfig::RenderHeight
    };
}

void Chest::Render(SDL_Renderer* renderer, const SDL_FRect& camera) const {
    if (!texture) return;
    
    if (rect.x + rect.w < camera.x || rect.x > camera.x + camera.w ||
        rect.y + rect.h < camera.y || rect.y > camera.y + camera.h) {
            return;
        }
        
        SDL_FRect drawRect = rect;
        drawRect.x -= camera.x;
    drawRect.y -= camera.y;

    SDL_RenderTexture(renderer, texture, spriteAnimation.GetSourceRect(), &drawRect);
}

