#include "HUD.h"
#include "player.h"
#include "slime.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <string>
#include <vector>

class world
{
public:
    bool Load(SDL_Renderer *renderer);
    void HandleEvent(const SDL_Event &event);
    void Update(float deltaTime);
    void Render(SDL_Renderer *renderer);

private:
    Player player;
    std::vector<Slime> slimes;
    HUD hud;
    SDL_FRect camera;
};
