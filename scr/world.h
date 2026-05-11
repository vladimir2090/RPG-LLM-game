#pragma once

#include "HUD.h"
#include "player.h"
#include "slime.h"

#include <SDL3/SDL.h>
#include <vector>

enum class TileType
{
    Grass,
    DarkGrass,
    Dirt,
    Moss,
    Mud
};

struct Tile
{
    TileType type;
};

enum class VegetationType
{
    TallGrass,
    Rye
};

struct VegetationPatch
{
    VegetationType type;
    SDL_FRect rect;
    float phase;
};

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
    void GenerateGround();
    void GenerateVegetation();
    void GenerateSlimes(SDL_Renderer *renderer);
    void UpdateCamera();
    void UpdateCombat(float deltaTime);
    void RenderGround(SDL_Renderer *renderer) const;
    void RenderVegetation(SDL_Renderer *renderer) const;

    Player player;
    std::vector<Slime> slimes;
    std::vector<Tile> tiles;
    std::vector<VegetationPatch> vegetation;
    HUD hud;
    SDL_FRect camera;
    float slimeContactDamageCooldown;
    bool playerAttackHitDone;
    float worldTime;
};
