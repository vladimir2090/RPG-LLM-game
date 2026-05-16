#include "world.h"
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <cmath>
#include <cstdint>

const float sizeHUD = 5.0f;
const float slimeContactDamageDelay = 0.7f;
const float slimeSpawnDelay = 1.4f;
const int mapWidth = 800;
const int mapHeight = 500;
const int tileSize = 64;
const int mapOriginTileX = -mapWidth / 2;
const int mapOriginTileY = -mapHeight / 2;
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
const char *questsPath = "/home/vladimir/dev/game/assets/quests/quests.json";

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

static int FloorDiv(int value, int divisor)
{
    if (value >= 0) {
        return value / divisor;
    }

    return -((-value + divisor - 1) / divisor);
}

static int WorldToTile(float value)
{
    return static_cast<int>(std::floor(value / static_cast<float>(tileSize)));
}

static bool IsTileInMap(int tileX, int tileY)
{
    int localX = tileX - mapOriginTileX;
    int localY = tileY - mapOriginTileY;
    return localX >= 0 && localX < mapWidth && localY >= 0 && localY < mapHeight;
}

static int TileIndex(int tileX, int tileY)
{
    int localX = tileX - mapOriginTileX;
    int localY = tileY - mapOriginTileY;
    return localY * mapWidth + localX;
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
    if (!questLog.LoadFromJson(questsPath)) {
        SDL_Log("Quests were not loaded from %s", questsPath);
    }
    GenerateQuestSigns();
    GenerateInitialSlimes();
    hud.Load(player.GetHealthPointer(), sizeHUD);
    hud.SetQuestLog(&questLog);
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
    questSigns.clear();
}

void World::GenerateGround()
{
    tiles.clear();
    tiles.resize(mapWidth * mapHeight);

    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {
            int tileX = mapOriginTileX + x;
            int tileY = mapOriginTileY + y;
            float cluster = Hash01(FloorDiv(tileX, 4), FloorDiv(tileY, 4), 11);
            float detail = Hash01(tileX, tileY, 23);
            float riverCenter = std::sin(tileX * 0.055f) * 24.0f + std::sin(tileX * 0.013f + 2.0f) * 58.0f;
            float riverDistance = std::abs(static_cast<float>(tileY) - riverCenter);
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

            int tileX = mapOriginTileX + x;
            int tileY = mapOriginTileY + y;
            float chance = Hash01(tileX, tileY, 41);
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
                float offsetX = Hash01(tileX, tileY, 100 + i) * 42.0f + 8.0f;
                float offsetY = Hash01(tileX, tileY, 200 + i) * 42.0f + 10.0f;
                float width = type == VegetationType::Rye ? 34.0f : 28.0f;
                float height = type == VegetationType::Rye ? 50.0f : 34.0f;

                vegetation.push_back(VegetationPatch{
                    type,
                    SDL_FRect{
                        tileX * static_cast<float>(tileSize) + offsetX,
                        tileY * static_cast<float>(tileSize) + offsetY,
                        width,
                        height
                    },
                    Hash01(tileX, tileY, 300 + i) * 6.28f
                });
            }
        }
    }
}
void World::GenerateChests()
{
    chests.clear();
    const char* chestTexturePath = "/home/vladimir/dev/game/assets/sprites/chest.png";

    for (int chunkY = 0; chunkY < mapHeight / chunkSizeTiles; chunkY++) {
        for (int chunkX = 0; chunkX < mapWidth / chunkSizeTiles; chunkX++) {
            int worldChunkX = FloorDiv(mapOriginTileX, chunkSizeTiles) + chunkX;
            int worldChunkY = FloorDiv(mapOriginTileY, chunkSizeTiles) + chunkY;
            
            if (Hash01(worldChunkX, worldChunkY, 900) > 0.16f) continue;

            int tileX = worldChunkX * chunkSizeTiles + 3 + static_cast<int>(Hash01(worldChunkX, worldChunkY, 901) * 10.0f);
            int tileY = worldChunkY * chunkSizeTiles + 3 + static_cast<int>(Hash01(worldChunkX, worldChunkY, 902) * 10.0f);
            
            if (!IsTileInMap(tileX, tileY)) continue;

            TileType ground = tiles[TileIndex(tileX, tileY)].type;
            if (ground == TileType::Water || ground == TileType::Mud) continue;

            int damage = 4 + static_cast<int>(Hash01(worldChunkX, worldChunkY, 903) * 9.0f);

            Chest chest;
            chest.SetTilePosition(tileX, tileY, tileSize);
            chest.SetWeaponDamage(damage);
            
            if (chest.Load(renderer, chestTexturePath)) {
                chests.push_back(std::move(chest));
            }
        }
    }
}

void World::GenerateQuestSigns()
{
    questSigns.clear();

    if (questLog.GetQuestTemplateCount() <= 0) {
        return;
    }

    for (const QuestSignData &signData : questLog.GetSignsFromJson()) {
        if (!questLog.HasQuestTemplate(signData.questId)) {
            continue;
        }

        questSigns.push_back(QuestSign{
            SDL_FRect{signData.x, signData.y, 48.0f, 58.0f},
            signData.questId
        });
    }

    for (int chunkY = 0; chunkY < mapHeight / chunkSizeTiles; chunkY++) {
        for (int chunkX = 0; chunkX < mapWidth / chunkSizeTiles; chunkX++) {
            int worldChunkX = FloorDiv(mapOriginTileX, chunkSizeTiles) + chunkX;
            int worldChunkY = FloorDiv(mapOriginTileY, chunkSizeTiles) + chunkY;
            if (Hash01(worldChunkX, worldChunkY, 1700) > 0.055f) {
                continue;
            }

            int tileX = worldChunkX * chunkSizeTiles + 2 + static_cast<int>(Hash01(worldChunkX, worldChunkY, 1701) * 12.0f);
            int tileY = worldChunkY * chunkSizeTiles + 2 + static_cast<int>(Hash01(worldChunkX, worldChunkY, 1702) * 12.0f);
            if (!IsTileInMap(tileX, tileY)) {
                continue;
            }

            TileType ground = tiles[TileIndex(tileX, tileY)].type;
            if (ground == TileType::Water || ground == TileType::Mud) {
                continue;
            }

            int questIndex = static_cast<int>(Hash01(worldChunkX, worldChunkY, 1703) * questLog.GetQuestTemplateCount());
            questIndex = std::clamp(questIndex, 0, questLog.GetQuestTemplateCount() - 1);

            questSigns.push_back(QuestSign{
                SDL_FRect{
                    tileX * static_cast<float>(tileSize) + 8.0f,
                    tileY * static_cast<float>(tileSize) + 4.0f,
                    48.0f,
                    58.0f
                },
                questLog.GetQuestTemplateId(questIndex)
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

    float minX = mapOriginTileX * static_cast<float>(tileSize);
    float minY = mapOriginTileY * static_cast<float>(tileSize);
    float maxX = (mapOriginTileX + mapWidth) * static_cast<float>(tileSize) - 160.0f;
    float maxY = (mapOriginTileY + mapHeight) * static_cast<float>(tileSize) - 160.0f;
    x = std::clamp(x, minX, maxX);
    y = std::clamp(y, minY, maxY);

    int tileX = WorldToTile(x);
    int tileY = WorldToTile(y);
    if (!IsTileInMap(tileX, tileY)) {
        return;
    }

    TileType ground = tiles[TileIndex(tileX, tileY)].type;
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
    int playerChunkX = FloorDiv(WorldToTile(playerCenterX), chunkSizeTiles);
    int playerChunkY = FloorDiv(WorldToTile(playerCenterY), chunkSizeTiles);
    int count = 0;

    for (const Slime &slime : slimes) {
        if (slime.IsDead()) {
            continue;
        }

        SDL_FRect slimeRect = slime.GetHitbox();
        float slimeCenterX = slimeRect.x + slimeRect.w / 2.0f;
        float slimeCenterY = slimeRect.y + slimeRect.h / 2.0f;
        int slimeChunkX = FloorDiv(WorldToTile(slimeCenterX), chunkSizeTiles);
        int slimeChunkY = FloorDiv(WorldToTile(slimeCenterY), chunkSizeTiles);

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

    int playerChunkX = FloorDiv(WorldToTile(playerCenterX), chunkSizeTiles);
    int playerChunkY = FloorDiv(WorldToTile(playerCenterY), chunkSizeTiles);

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
        if (!IsTileInMap(tileX, tileY)) {
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
    questLog.Update(deltaTime);
    player.Update(deltaTime, moveUp, moveDown, moveLeft, moveRight, attack);
    ManageSlimePopulation(deltaTime);

    for (Slime &slime : slimes) {
        if (!slime.IsDead() && !player.IsDying()) {
            slime.Update(deltaTime, player.GetHitbox(), player.GetHealthPercent());
        }
    }

    UpdateCombat(deltaTime);
    UpdateChestsAndPickups();
    UpdateQuestSigns();
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
            bool wasAlive = !slime.IsDead();
            slime.TakeDamage(player.GetDamage());
            if (wasAlive && slime.IsDead()) {
                questLog.OnSlimeKilled();
            }
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
        int damageBonus = chest.Update(playerRect);
        if (damageBonus > 0) {
            questLog.OnChestOpened();
            weaponPickups.push_back(WeaponPickup{
                SDL_FRect{chest.GetRect().x + 6.0f, chest.GetRect().y - 24.0f, 24.0f, 24.0f},
                damageBonus,
                false
            });
        }
    }

    for (WeaponPickup &pickup : weaponPickups) {
        if (pickup.taken) continue;
        if (SDL_HasRectIntersectionFloat(&playerRect, &pickup.rect)) {
            player.AddDamage(pickup.damageBonus);
            pickup.taken = true;
            SDL_Log("Weapon picked up: +%d damage", pickup.damageBonus);
        }
    }
}

void World::UpdateQuestSigns()
{
    if (!questLog.CanStartQuest()) {
        return;
    }

    SDL_FRect playerRect = player.GetHitbox();

    for (const QuestSign &sign : questSigns) {
        if (SDL_HasRectIntersectionFloat(&playerRect, &sign.rect)) {
            if (questLog.StartQuest(sign.questId)) {
                SDL_Log("Quest started: %s", sign.questId.c_str());
            }
            return;
        }
    }
}

void World::Render(SDL_Renderer *renderer) const
{
    RenderGround(renderer);
    RenderQuestSigns(renderer);
    RenderChestsAndPickups(renderer);

    player.Render(renderer, camera, IsRectTouchingWater(player.GetHitbox()));

    for (const Slime &slime : slimes) {
        slime.Render(renderer, camera);
    }

    RenderVegetation(renderer);
    hud.Render(renderer);
}

void World::RenderChestsAndPickups(SDL_Renderer *renderer) const
{
    for (const Chest &chest : chests) {
        chest.Render(renderer, camera);
    }

    // Рендер WeaponPickup оставляем как есть (или тоже вынесем в отдельный класс)
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

void World::RenderQuestSigns(SDL_Renderer *renderer) const
{
    for (const QuestSign &sign : questSigns) {
        if (sign.rect.x + sign.rect.w < camera.x || sign.rect.x > camera.x + camera.w ||
            sign.rect.y + sign.rect.h < camera.y || sign.rect.y > camera.y + camera.h) {
            continue;
        }

        SDL_FRect post = SDL_FRect{
            sign.rect.x + 20.0f - camera.x,
            sign.rect.y + 24.0f - camera.y,
            8.0f,
            34.0f
        };
        SDL_FRect board = SDL_FRect{
            sign.rect.x - camera.x,
            sign.rect.y - camera.y,
            sign.rect.w,
            28.0f
        };
        SDL_FRect paper = SDL_FRect{
            board.x + 10.0f,
            board.y + 6.0f,
            board.w - 20.0f,
            16.0f
        };

        SDL_SetRenderDrawColor(renderer, 86, 55, 30, 255);
        SDL_RenderFillRect(renderer, &post);
        SDL_SetRenderDrawColor(renderer, 126, 82, 42, 255);
        SDL_RenderFillRect(renderer, &board);
        SDL_SetRenderDrawColor(renderer, 62, 39, 23, 255);
        SDL_RenderRect(renderer, &board);
        SDL_SetRenderDrawColor(renderer, 220, 196, 132, 255);
        SDL_RenderFillRect(renderer, &paper);
    }
}

bool World::IsRectTouchingWater(const SDL_FRect &rect) const
{
    int startX = WorldToTile(rect.x);
    int startY = WorldToTile(rect.y);
    int endX = WorldToTile(rect.x + rect.w);
    int endY = WorldToTile(rect.y + rect.h);

    for (int y = startY; y <= endY; y++) {
        for (int x = startX; x <= endX; x++) {
            if (IsTileInMap(x, y) && tiles[TileIndex(x, y)].type == TileType::Water) {
                return true;
            }
        }
    }

    return false;
}

void World::RenderGround(SDL_Renderer *renderer) const
{
    int startX = WorldToTile(camera.x) - 1;
    int startY = WorldToTile(camera.y) - 1;
    int endX = WorldToTile(camera.x + camera.w) + 2;
    int endY = WorldToTile(camera.y + camera.h) + 2;

    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            if (!IsTileInMap(x, y)) {
                SDL_SetRenderDrawColor(renderer, 13, 44, 31, 255);
                SDL_FRect voidRect = SDL_FRect{
                    x * static_cast<float>(tileSize) - camera.x,
                    y * static_cast<float>(tileSize) - camera.y,
                    static_cast<float>(tileSize),
                    static_cast<float>(tileSize)
                };
                SDL_RenderFillRect(renderer, &voidRect);
                continue;
            }

            TileType type = tiles[TileIndex(x, y)].type;
            DrawColor color = GetGroundColor(type);
            float shade = Hash01(x, y, 500) * 12.0f - 6.0f;

            if (type == TileType::Water) {
                shade += std::sin(worldTime * 3.0f + x * 0.7f + y * 0.3f) * 10.0f;
            }

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
