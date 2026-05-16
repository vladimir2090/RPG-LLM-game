#pragma once
#include "animation.h"
#include <SDL3/SDL.h>

struct ChestConfig {
    static constexpr float RenderWidth  = 102.0f;
    static constexpr float RenderHeight = 102.0f;
    static constexpr float BottomMargin = 16.0f;
};

class Chest {
public:
    Chest();
    bool Load(SDL_Renderer* renderer, const char* texturePath);
    void Unload();
    
    int Update(const SDL_FRect& playerRect);
    void Render(SDL_Renderer* renderer, const SDL_FRect& camera) const;
    
    void SetTilePosition(int tileX, int tileY, int tileSize);
    
    void SetWeaponDamage(int dmg) { weaponDamage = dmg; }
    const SDL_FRect& GetRect() const { return rect; }
    bool IsOpened() const { return opened; }

private:
    SDL_FRect rect;
    SDL_Texture* texture{nullptr};
    
    AnimationClip animClosed{1, 200, 0};
    AnimationClip animOpening{1, 100, 0};
    AnimationClip animOpened{1, 200, 0};
    
    Animation spriteAnimation;
    bool opened{false};
    bool isOpening{false};
    int weaponDamage{0};
    float sizeSprite{144.0f};
};