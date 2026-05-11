#pragma once

#include "HUD.h"
#include "player.h"
#include "slime.h"

#include <SDL3/SDL.h>
#include <vector>

class World
{
public:
    World();

    bool Load(SDL_Renderer *renderer);
    void Unload();
    void Update(float deltaTime, bool moveUp, bool moveDown, bool moveLeft, bool moveRight, bool attack);
    void Render(SDL_Renderer *renderer) const;
    bool IsPlayerDead() const;

private:
    void GenerateSlimes(SDL_Renderer *renderer);
    void UpdateCamera();
    void UpdateCombat(float deltaTime);

    Player player;
    std::vector<Slime> slimes;
    HUD hud;
    SDL_FRect camera;
    float slimeContactDamageCooldown;
    bool playerAttackHitDone;
};
