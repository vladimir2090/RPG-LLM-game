#pragma once

#include "quest.h"

#include <SDL3/SDL.h>

class HUD
{
public:
    HUD();

    void Load(int *value, float scale);
    void SetQuestLog(const QuestLog *questLogToUse);
    void Render(SDL_Renderer *renderer) const;

private:
    int *value;
    const QuestLog *questLog;
    float scale;
    float x;
    float y;
};
