#pragma once

#include <string>
#include <vector>

enum class QuestKind
{
    OpenChests,
    KillSlimes
};

struct QuestSignData
{
    std::string questId;
    float x;
    float y;
};

struct Quest
{
    std::string id;
    std::string title;
    std::string completeTitle;
    std::string objective;
    std::string hint;
    std::string completeHint;
    QuestKind kind;
    int required;
    int progress;
    bool complete;
    float completeTimer;
    float visibility;
};

class QuestLog
{
public:
    bool LoadFromJson(const char *path);
    void Update(float deltaTime);

    bool StartQuest(const std::string &questId);
    bool CanStartQuest() const;
    bool HasQuestTemplate(const std::string &questId) const;

    void OnChestOpened();
    void OnSlimeKilled();

    const std::vector<Quest> &GetQuests() const;
    int GetQuestTemplateCount() const;
    const std::string &GetQuestTemplateId(int index) const;
    const std::vector<QuestSignData> &GetSignsFromJson() const;

private:
    Quest *FindQuestTemplate(const std::string &questId);
    const Quest *FindQuestTemplate(const std::string &questId) const;
    bool IsQuestRunning(const std::string &questId) const;
    void AddProgress(QuestKind kind);

    std::vector<Quest> questTemplates;
    std::vector<QuestSignData> signsFromJson;
    std::vector<Quest> activeQuests;
};
