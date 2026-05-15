#include "world.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

const float sizeHUD = 5.0f;
const float slimeContactDamageDelay = 0.7f;
const float slimeSpawnDelay = 1.4f;
const int mapWidth = 800;
const int mapHeight = 500;
const int tileSize = 64;
const int chunkSizeTiles = 16;
const int activeChunkRadius = 2;
const int desiredNearbySlimes = 28;
const int maxSlimes = 180;
const float minSpawnDistance = 720.0f;
const float maxSpawnDistance = 2300.0f;
const float despawnDistance = 3400.0f;
const char *playerTexturePath = "/home/vladimir/dev/game/assets/sprites/knight.png";
const char *slimeTexturePath = "/home/vladimir/dev/game/assets/sprites/slime_green.png";
const char *slimeBrainPath = "/home/vladimir/dev/game/ai/models/slime_weights.json";

struct DrawColor
{
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
};

static float Hash01(int x, int y, int seed)
{
    uint32_t value = static_cast<uint32_t>(x) * 374761393u;
    value += static_cast<uint32_t>(y) * 668265263u;
    value += static_cast<uint32_t>(seed) * 2246822519u;
    value = (value ^ (value >> 13)) * 1274126177u;
    value ^= value >> 16;
    return static_cast<float>(value & 0x00FFFFFFu) / static_cast<float>(0x01000000u);
}

static DrawColor GetGroundColor(TileType type)
{
    switch (type) {
        //как нарисую текстурки сделаю парс png как в player.cpp
        case TileType::Grass:
            return DrawColor{52, 139, 55, 255};
        case TileType::DarkGrass:
            return DrawColor{35, 105, 48, 255};
        case TileType::Dirt:
            return DrawColor{117, 91, 55, 255};
        case TileType::Moss:
            return DrawColor{78, 128, 63, 255};
        case TileType::Mud:
            return DrawColor{84, 72, 50, 255};
        case TileType::Sand:
            return DrawColor{154, 136, 86, 255};
        case TileType::Water:
            return DrawColor{42, 101, 150, 255};
        default:
            return DrawColor{52, 139, 55, 255};
    }
}

World::World()
{
    renderer = NULL;
    camera = SDL_FRect{0.0f, 0.0f, 1920.0f, 1080.0f};
    slimeContactDamageCooldown = 0.0f;
    slimeSpawnTimer = 0.0f;
    playerAttackHitDone = false;
    worldTime = 0.0f;
}

bool World::Load(SDL_Renderer *rendererToUse)
{
    renderer = rendererToUse;

    if (!player.Load(renderer, playerTexturePath)) {
        return false;
    }

    GenerateGround();
    GenerateVegetation();
    GenerateChests();
    GenerateInitialSlimes();
    hud.Load(player.GetHealthPointer(), sizeHUD);
    return true;
}

void World::Unload()
{
    player.Unload();
    for (Slime &slime : slimes) {
        slime.Unload();
    }
    slimes.clear();
    tiles.clear();
    vegetation.clear();
    chests.clear();
    weaponPickups.clear();
}

void World::GenerateGround()
{
    tiles.clear();
    tiles.resize(mapWidth * mapHeight);

    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {
            float cluster = Hash01(x / 4, y / 4, 11);
            float detail = Hash01(x, y, 23);
            float riverCenter = mapHeight * 0.48f + std::sin(x * 0.055f) * 24.0f + std::sin(x * 0.013f + 2.0f) * 58.0f;
            float riverDistance = std::abs(static_cast<float>(y) - riverCenter);
            TileType type = TileType::Grass;

            if (riverDistance < 2.4f) {
                type = TileType::Water;
            } else if (riverDistance < 4.2f) {
                type = TileType::Sand;
            } else if (cluster < 0.12f) {
                type = detail < 0.75f ? TileType::Dirt : TileType::Mud;
            } else if (cluster < 0.22f) {
                type = TileType::Moss;
            } else if (cluster > 0.82f) {
                type = TileType::DarkGrass;
            } else if (detail > 0.94f) {
                type = TileType::Dirt;
            }

            tiles[y * mapWidth + x] = Tile{type};
        }
    }
}

void World::GenerateVegetation()
{
    vegetation.clear();

    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {
            TileType ground = tiles[y * mapWidth + x].type;
            if (ground == TileType::Dirt || ground == TileType::Mud ||
                ground == TileType::Sand || ground == TileType::Water) {
                continue;
            }

            float chance = Hash01(x, y, 41);
            int patchCount = 0;
            VegetationType type = VegetationType::TallGrass;

            if (ground == TileType::DarkGrass && chance < 0.55f) {
                patchCount = 2;
                type = VegetationType::TallGrass;
            } else if (ground == TileType::Moss && chance < 0.30f) {
                patchCount = 1;
                type = VegetationType::TallGrass;
            } else if (chance > 0.92f) {
                patchCount = 3;
                type = VegetationType::Rye;
            } else if (chance < 0.20f) {
                patchCount = 1;
                type = VegetationType::TallGrass;
            }

            for (int i = 0; i < patchCount; i++) {
                float offsetX = Hash01(x, y, 100 + i) * 42.0f + 8.0f;
                float offsetY = Hash01(x, y, 200 + i) * 42.0f + 10.0f;
                float width = type == VegetationType::Rye ? 34.0f : 28.0f;
                float height = type == VegetationType::Rye ? 50.0f : 34.0f;

                vegetation.push_back(VegetationPatch{
                    type,
                    SDL_FRect{
                        x * static_cast<float>(tileSize) + offsetX,
                        y * static_cast<float>(tileSize) + offsetY,
                        width,
                        height
                    },
                    Hash01(x, y, 300 + i) * 6.28f
                });
            }
        }
    }
}

void World::GenerateChests()
{
    chests.clear();

    for (int chunkY = 0; chunkY < mapHeight / chunkSizeTiles; chunkY++) {
        for (int chunkX = 0; chunkX < mapWidth / chunkSizeTiles; chunkX++) {
            if (Hash01(chunkX, chunkY, 900) > 0.16f) {
                continue;
            }

            int tileX = chunkX * chunkSizeTiles + 3 + static_cast<int>(Hash01(chunkX, chunkY, 901) * 10.0f);
            int tileY = chunkY * chunkSizeTiles + 3 + static_cast<int>(Hash01(chunkX, chunkY, 902) * 10.0f);
            if (tileX < 0 || tileX >= mapWidth || tileY < 0 || tileY >= mapHeight) {
                continue;
            }

            TileType ground = tiles[tileY * mapWidth + tileX].type;
            if (ground == TileType::Water || ground == TileType::Mud) {
                continue;
            }

            int damage = 4 + static_cast<int>(Hash01(chunkX, chunkY, 903) * 9.0f);
            chests.push_back(Chest{
                SDL_FRect{
                    tileX * static_cast<float>(tileSize) + 14.0f,
                    tileY * static_cast<float>(tileSize) + 18.0f,
                    36.0f,
                    30.0f
                },
                false,
                damage
            });
        }
    }
}

void World::GenerateInitialSlimes()
{
    slimes.clear();
    slimes.reserve(maxSlimes);

    for (int i = 0; i < 12; i++) {
        float angle = Hash01(i, 0, 1200) * 6.283185f;
        float distance = minSpawnDistance + Hash01(i, 0, 1201) * 900.0f;
        SDL_FRect playerRect = player.GetHitbox();
        float playerCenterX = playerRect.x + playerRect.w / 2.0f;
        float playerCenterY = playerRect.y + playerRect.h / 2.0f;
        SpawnSlimeAt(playerCenterX + std::cos(angle) * distance, playerCenterY + std::sin(angle) * distance);
    }
}

void World::SpawnSlimeAt(float x, float y)
{
    if (renderer == NULL || static_cast<int>(slimes.size()) >= maxSlimes) {
        return;
    }

    x = std::clamp(x, 0.0f, mapWidth * static_cast<float>(tileSize) - 160.0f);
    y = std::clamp(y, 0.0f, mapHeight * static_cast<float>(tileSize) - 160.0f);

    int tileX = std::clamp(static_cast<int>(x / tileSize), 0, mapWidth - 1);
    int tileY = std::clamp(static_cast<int>(y / tileSize), 0, mapHeight - 1);
    TileType ground = tiles[tileY * mapWidth + tileX].type;
    if (ground == TileType::Water) {
        return;
    }

    Slime slime;
    slime.SetPosition(x, y);

    if (!slime.Load(renderer, slimeTexturePath)) {
        SDL_Log("Slime texture was not loaded");
        return;
    }

    if (!slime.LoadBrain(slimeBrainPath)) {
        SDL_Log("Slime brain was not loaded, using patrol fallback");
    }

    slimes.push_back(std::move(slime));
}

int World::CountNearbySlimes() const
{
    SDL_FRect playerRect = player.GetHitbox();
    float playerCenterX = playerRect.x + playerRect.w / 2.0f;
    float playerCenterY = playerRect.y + playerRect.h / 2.0f;
    int playerChunkX = static_cast<int>(playerCenterX / (chunkSizeTiles * tileSize));
    int playerChunkY = static_cast<int>(playerCenterY / (chunkSizeTiles * tileSize));
    int count = 0;

    for (const Slime &slime : slimes) {
        if (slime.IsDead()) {
            continue;
        }

        SDL_FRect slimeRect = slime.GetHitbox();
        float slimeCenterX = slimeRect.x + slimeRect.w / 2.0f;
        float slimeCenterY = slimeRect.y + slimeRect.h / 2.0f;
        int slimeChunkX = static_cast<int>(slimeCenterX / (chunkSizeTiles * tileSize));
        int slimeChunkY = static_cast<int>(slimeCenterY / (chunkSizeTiles * tileSize));

        if (std::abs(slimeChunkX - playerChunkX) <= activeChunkRadius &&
            std::abs(slimeChunkY - playerChunkY) <= activeChunkRadius) {
            count++;
        }
    }

    return count;
}

void World::ManageSlimePopulation(float deltaTime)
{
    SDL_FRect playerRect = player.GetHitbox();
    float playerCenterX = playerRect.x + playerRect.w / 2.0f;
    float playerCenterY = playerRect.y + playerRect.h / 2.0f;

    slimes.erase(
        std::remove_if(slimes.begin(), slimes.end(), [&](const Slime &slime) {
            SDL_FRect slimeRect = slime.GetHitbox();
            float slimeCenterX = slimeRect.x + slimeRect.w / 2.0f;
            float slimeCenterY = slimeRect.y + slimeRect.h / 2.0f;
            float dx = slimeCenterX - playerCenterX;
            float dy = slimeCenterY - playerCenterY;
            return std::sqrt(dx * dx + dy * dy) > despawnDistance;
        }),
        slimes.end()
    );

    slimeSpawnTimer -= deltaTime;
    if (slimeSpawnTimer > 0.0f || CountNearbySlimes() >= desiredNearbySlimes) {
        return;
    }

    int playerChunkX = static_cast<int>(playerCenterX / (chunkSizeTiles * tileSize));
    int playerChunkY = static_cast<int>(playerCenterY / (chunkSizeTiles * tileSize));

    for (int attempt = 0; attempt < 16; attempt++) {
        float roll = Hash01(static_cast<int>(worldTime * 10.0f), attempt, 1300);
        int offsetX = static_cast<int>(roll * (activeChunkRadius * 2 + 1)) - activeChunkRadius;
        int offsetY = static_cast<int>(Hash01(static_cast<int>(worldTime * 10.0f), attempt, 1301) * (activeChunkRadius * 2 + 1)) - activeChunkRadius;
        if (offsetX == 0 && offsetY == 0) {
            offsetX = activeChunkRadius;
        }

        int chunkX = playerChunkX + offsetX;
        int chunkY = playerChunkY + offsetY;
        int tileX = chunkX * chunkSizeTiles + static_cast<int>(Hash01(chunkX, chunkY, 1302 + attempt) * chunkSizeTiles);
        int tileY = chunkY * chunkSizeTiles + static_cast<int>(Hash01(chunkX, chunkY, 1303 + attempt) * chunkSizeTiles);
        if (tileX < 0 || tileX >= mapWidth || tileY < 0 || tileY >= mapHeight) {
            continue;
        }

        float x = tileX * static_cast<float>(tileSize) + 8.0f;
        float y = tileY * static_cast<float>(tileSize) + 8.0f;
        float dx = x - playerCenterX;
        float dy = y - playerCenterY;
        float distance = std::sqrt(dx * dx + dy * dy);
        if (distance < minSpawnDistance || distance > maxSpawnDistance) {
            continue;
        }

        SpawnSlimeAt(x, y);
        slimeSpawnTimer = slimeSpawnDelay;
        return;
    }

    slimeSpawnTimer = slimeSpawnDelay * 0.5f;
}

void World::Update(float deltaTime, bool moveUp, bool moveDown, bool moveLeft, bool moveRight, bool attack)
{
    worldTime += deltaTime;
    player.Update(deltaTime, moveUp, moveDown, moveLeft, moveRight, attack);
    ManageSlimePopulation(deltaTime);

    for (Slime &slime : slimes) {
        if (!slime.IsDead() && !player.IsDying()) {
            slime.Update(deltaTime, player.GetHitbox(), player.GetHealthPercent());
        }
    }

    UpdateCombat(deltaTime);
    UpdateChestsAndPickups();
    UpdateCamera();
}

void World::UpdateCamera()
{
    SDL_FRect playerRect = player.GetHitbox();
    camera.x = playerRect.x + playerRect.w / 2 - camera.w / 2;
    camera.y = playerRect.y + playerRect.h / 2 - camera.h / 2;
}

void World::UpdateCombat(float deltaTime)
{
    if (slimeContactDamageCooldown > 0.0f) {
        slimeContactDamageCooldown -= deltaTime;
    }

    if (player.IsDead() || player.IsDying()) {
        return;
    }

    SDL_FRect playerRect = player.GetHitbox();
    SDL_FRect attackRect = player.GetAttackRect();
    bool attackHitSomething = false;

    for (Slime &slime : slimes) {
        if (slime.IsDead()) {
            continue;
        }

        SDL_FRect slimeRect = slime.GetHitbox();

        if (SDL_HasRectIntersectionFloat(&playerRect, &slimeRect) && slimeContactDamageCooldown <= 0.0f) {
            player.TakeDamage(slime.GetDamage());
            slimeContactDamageCooldown = slimeContactDamageDelay;
        }

        if (player.IsAttacking() && !playerAttackHitDone && SDL_HasRectIntersectionFloat(&attackRect, &slimeRect)) {
            slime.TakeDamage(player.GetDamage());
            attackHitSomething = true;
        }
    }

    if (player.IsAttacking()) {
        playerAttackHitDone = playerAttackHitDone || attackHitSomething;
    } else {
        playerAttackHitDone = false;
    }
}

void World::UpdateChestsAndPickups()
{
    SDL_FRect playerRect = player.GetHitbox();

    for (Chest &chest : chests) {
        if (chest.opened) {
            continue;
        }

        if (SDL_HasRectIntersectionFloat(&playerRect, &chest.rect)) {
            chest.opened = true;
            weaponPickups.push_back(WeaponPickup{
                SDL_FRect{chest.rect.x + 6.0f, chest.rect.y - 24.0f, 24.0f, 24.0f},
                chest.weaponDamage,
                false
            });
        }
    }

    for (WeaponPickup &pickup : weaponPickups) {
        if (pickup.taken) {
            continue;
        }

        if (SDL_HasRectIntersectionFloat(&playerRect, &pickup.rect)) {
            player.AddDamage(pickup.damageBonus);
            pickup.taken = true;
            SDL_Log("Weapon picked up: +%d damage", pickup.damageBonus);
        }
    }
}

void World::Render(SDL_Renderer *renderer) const
{
    RenderGround(renderer);
    RenderChestsAndPickups(renderer);

    player.Render(renderer, camera);

    for (const Slime &slime : slimes) {
        slime.Render(renderer, camera);
    }

    RenderVegetation(renderer);
    hud.Render(renderer);
}

void World::RenderChestsAndPickups(SDL_Renderer *renderer) const
{
    for (const Chest &chest : chests) {
        if (chest.rect.x + chest.rect.w < camera.x || chest.rect.x > camera.x + camera.w ||
            chest.rect.y + chest.rect.h < camera.y || chest.rect.y > camera.y + camera.h) {
            continue;
        }

        SDL_FRect drawRect = chest.rect;
        drawRect.x -= camera.x;
        drawRect.y -= camera.y;

        if (chest.opened) {
            SDL_SetRenderDrawColor(renderer, 90, 57, 32, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 139, 88, 42, 255);
        }
        SDL_RenderFillRect(renderer, &drawRect);

        SDL_SetRenderDrawColor(renderer, 68, 42, 24, 255);
        SDL_RenderRect(renderer, &drawRect);
    }

    for (const WeaponPickup &pickup : weaponPickups) {
        if (pickup.taken ||
            pickup.rect.x + pickup.rect.w < camera.x || pickup.rect.x > camera.x + camera.w ||
            pickup.rect.y + pickup.rect.h < camera.y || pickup.rect.y > camera.y + camera.h) {
            continue;
        }

        SDL_FRect drawRect = pickup.rect;
        drawRect.x -= camera.x;
        drawRect.y -= camera.y;

        SDL_SetRenderDrawColor(renderer, 210, 210, 220, 255);
        SDL_RenderFillRect(renderer, &drawRect);
        SDL_SetRenderDrawColor(renderer, 65, 70, 85, 255);
        SDL_RenderLine(renderer, drawRect.x + 4.0f, drawRect.y + 20.0f, drawRect.x + 20.0f, drawRect.y + 4.0f);
    }
}

void World::RenderGround(SDL_Renderer *renderer) const
{
    int startX = std::max(0, static_cast<int>(camera.x / tileSize) - 1);
    int startY = std::max(0, static_cast<int>(camera.y / tileSize) - 1);
    int endX = std::min(mapWidth, static_cast<int>((camera.x + camera.w) / tileSize) + 2);
    int endY = std::min(mapHeight, static_cast<int>((camera.y + camera.h) / tileSize) + 2);

    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            TileType type = tiles[y * mapWidth + x].type;
            DrawColor color = GetGroundColor(type);
            float shade = Hash01(x, y, 500) * 12.0f - 6.0f;

            SDL_SetRenderDrawColor(
                renderer,
                static_cast<Uint8>(std::clamp(static_cast<int>(color.r + shade), 0, 255)),
                static_cast<Uint8>(std::clamp(static_cast<int>(color.g + shade), 0, 255)),
                static_cast<Uint8>(std::clamp(static_cast<int>(color.b + shade), 0, 255)),
                color.a
            );

            SDL_FRect rect = SDL_FRect{
                x * static_cast<float>(tileSize) - camera.x,
                y * static_cast<float>(tileSize) - camera.y,
                static_cast<float>(tileSize),
                static_cast<float>(tileSize)
            };
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

void World::RenderVegetation(SDL_Renderer *renderer) const
{
    SDL_FRect playerRect = player.GetHitbox();
    float playerCenterX = playerRect.x + playerRect.w / 2.0f;
    float playerCenterY = playerRect.y + playerRect.h / 2.0f;

    for (const VegetationPatch &patch : vegetation) {
        if (patch.rect.x + patch.rect.w < camera.x || patch.rect.x > camera.x + camera.w ||
            patch.rect.y + patch.rect.h < camera.y || patch.rect.y > camera.y + camera.h) {
            continue;
        }

        float patchCenterX = patch.rect.x + patch.rect.w / 2.0f;
        float patchCenterY = patch.rect.y + patch.rect.h / 2.0f;
        float dx = patchCenterX - playerCenterX;
        float dy = patchCenterY - playerCenterY;
        float distance = std::sqrt(dx * dx + dy * dy);
        float influence = std::max(0.0f, 1.0f - distance / 130.0f);
        float wind = std::sin(worldTime * 2.2f + patch.phase) * 4.0f;
        float bendX = wind;

        if (distance > 0.001f) {
            bendX += dx / distance * influence * 24.0f;
        }

        DrawColor color = patch.type == VegetationType::Rye
            ? DrawColor{196, 172, 78, 230}
            : DrawColor{83, 171, 75, 220};
        int bladeCount = patch.type == VegetationType::Rye ? 7 : 6;

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

        for (int i = 0; i < bladeCount; i++) {
            float t = bladeCount <= 1 ? 0.0f : static_cast<float>(i) / static_cast<float>(bladeCount - 1);
            float baseX = patch.rect.x + t * patch.rect.w;
            float baseY = patch.rect.y + patch.rect.h;
            float bladeHeight = patch.rect.h * (0.75f + Hash01(static_cast<int>(patch.rect.x), i, 700) * 0.35f);
            float tipX = baseX + bendX * (0.45f + t * 0.35f);
            float tipY = baseY - bladeHeight;

            SDL_RenderLine(
                renderer,
                baseX - camera.x,
                baseY - camera.y,
                tipX - camera.x,
                tipY - camera.y
            );
        }
    }
}

bool World::IsPlayerDead() const
{
    return player.IsDead();
}
