#include "HUD.h"

#include <algorithm>
#include <string>

HUD::HUD()
{
    value = NULL;
    questLog = NULL;
    scale = 1.0f;
    //отступ
    x = 16.0f;
    y = 16.0f;
}

void HUD::Load(int *globalValue, float hudScale)
{
    value = globalValue;
    scale = hudScale;
}

void HUD::SetQuestLog(const QuestLog *questLogToUse)
{
    questLog = questLogToUse;
}

void HUD::Render(SDL_Renderer *renderer) const
{
    if (value == NULL) {
        return;
    }

    SDL_SetRenderScale(renderer, scale, scale);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDebugTextFormat(renderer, x / scale, y / scale, "value: %d", *value);
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);

    if (questLog == NULL || questLog->GetQuests().empty()) {
        return;
    }

    float visibleQuests = 0.0f;
    for (const Quest &quest : questLog->GetQuests()) {
        visibleQuests += quest.visibility;
    }

    if (visibleQuests <= 0.01f) {
        return;
    }

    int windowWidth = 0;
    int windowHeight = 0;
    SDL_GetCurrentRenderOutputSize(renderer, &windowWidth, &windowHeight);

    float panelWidth = 390.0f;
    float blockHeight = 78.0f;
    float panelHeight = 22.0f + blockHeight * visibleQuests;
    SDL_FRect panel = SDL_FRect{
        static_cast<float>(windowWidth) - panelWidth - 24.0f,
        static_cast<float>(windowHeight) - panelHeight - 24.0f,
        panelWidth,
        panelHeight
    };

    SDL_SetRenderDrawColor(renderer, 57, 43, 30, 220);
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawColor(renderer, 130, 98, 58, 255);
    SDL_RenderRect(renderer, &panel);

    float textScale = 1.35f;
    SDL_SetRenderScale(renderer, textScale, textScale);

    float textX = (panel.x + 14.0f) / textScale;
    float textY = (panel.y + 12.0f) / textScale;

    for (const Quest &quest : questLog->GetQuests()) {
        if (quest.visibility <= 0.01f) {
            continue;
        }

        std::string title = quest.complete ? quest.completeTitle : quest.title;
        std::string objective = quest.objective + ": " +
            std::to_string(quest.progress) + "/" + std::to_string(quest.required);
        std::string hint = quest.complete ? quest.completeHint : quest.hint;

        SDL_SetRenderDrawColor(renderer, 255, 235, 176, 255);
        SDL_RenderDebugText(renderer, textX, textY, title.c_str());
        SDL_SetRenderDrawColor(renderer, 230, 220, 190, 255);
        SDL_RenderDebugText(renderer, textX, textY + 18.0f, objective.c_str());
        SDL_SetRenderDrawColor(renderer, 190, 210, 185, 255);
        SDL_RenderDebugText(renderer, textX, textY + 36.0f, hint.c_str());

        textY += blockHeight * quest.visibility / textScale;
    }

    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
}
