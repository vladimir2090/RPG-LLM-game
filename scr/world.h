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
    Mud,
    Sand,
    Water
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

struct Chest
{
    SDL_FRect rect;
    bool opened;
    int weaponDamage;
};

struct WeaponPickup
{
    SDL_FRect rect;
    int damageBonus;
    bool taken;
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
    void GenerateChests();
    void GenerateInitialSlimes();
    void SpawnSlimeAt(float x, float y);
    void ManageSlimePopulation(float deltaTime);
    int CountNearbySlimes() const;
    void UpdateCamera();
    void UpdateCombat(float deltaTime);
    void UpdateChestsAndPickups();
    bool IsRectTouchingWater(const SDL_FRect &rect) const;
    void RenderGround(SDL_Renderer *renderer) const;
    void RenderChestsAndPickups(SDL_Renderer *renderer) const;
    void RenderVegetation(SDL_Renderer *renderer) const;

    SDL_Renderer *renderer;
    Player player;
    std::vector<Slime> slimes;
    std::vector<Tile> tiles;
    std::vector<VegetationPatch> vegetation;
    std::vector<Chest> chests;
    std::vector<WeaponPickup> weaponPickups;
    HUD hud;
    SDL_FRect camera;
    float slimeContactDamageCooldown;
    float slimeSpawnTimer;
    bool playerAttackHitDone;
    float worldTime;
};
