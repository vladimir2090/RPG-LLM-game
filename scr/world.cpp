#include "world.h"

const float sizeHUD = 5.0f;
const float slimeContactDamageDelay = 0.7f;
const char *playerTexturePath = "/home/vladimir/dev/game/assets/sprites/knight.png";
const char *slimeTexturePath = "/home/vladimir/dev/game/assets/sprites/slime_green.png";
const char *slimeBrainPath = "/home/vladimir/dev/game/ai/models/slime_weights.json";

World::World()
{
    camera = SDL_FRect{0.0f, 0.0f, 1920.0f, 1080.0f};
    slimeContactDamageCooldown = 0.0f;
    playerAttackHitDone = false;
}

bool World::Load(SDL_Renderer *renderer)
{
    if (!player.Load(renderer, playerTexturePath)) {
        return false;
    }

    GenerateSlimes(renderer);
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
}

void World::GenerateSlimes(SDL_Renderer *renderer)
{
    slimes.clear();
    slimes.reserve(3);

    const SDL_FPoint spawnPoints[] = {
        SDL_FPoint{500.0f, 500.0f},
        SDL_FPoint{850.0f, 420.0f},
        SDL_FPoint{300.0f, 760.0f}
    };

    for (const SDL_FPoint &spawnPoint : spawnPoints) {
        Slime slime;
        slime.SetPosition(spawnPoint.x, spawnPoint.y);

        if (!slime.Load(renderer, slimeTexturePath)) {
            SDL_Log("Slime texture was not loaded");
            continue;
        }

        if (!slime.LoadBrain(slimeBrainPath)) {
            SDL_Log("Slime brain was not loaded, using patrol fallback");
        }

        slimes.push_back(std::move(slime));
    }
}

void World::Update(float deltaTime, bool moveUp, bool moveDown, bool moveLeft, bool moveRight, bool attack)
{
    player.Update(deltaTime, moveUp, moveDown, moveLeft, moveRight, attack);

    for (Slime &slime : slimes) {
        if (!slime.IsDead() && !player.IsDying()) {
            slime.Update(deltaTime, player.GetHitbox(), player.GetHealthPercent());
        }
    }

    UpdateCombat(deltaTime);
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

void World::Render(SDL_Renderer *renderer) const
{
    player.Render(renderer, camera);

    for (const Slime &slime : slimes) {
        slime.Render(renderer, camera);
    }

    hud.Render(renderer);
}

bool World::IsPlayerDead() const
{
    return player.IsDead();
}
