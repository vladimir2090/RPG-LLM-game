#include "quest.h"

#include <algorithm>
#include <fstream>
#include <regex>
#include <sstream>

const int maxActiveQuests = 3;
const float completeTextTime = 1.4f;
const float hudAnimationSpeed = 4.5f;

// ---------------- JSON helpers ----------------

static std::string ReadTextFile(const char *path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

static std::string JsonString(const std::string &object, const std::string &key)
{
    std::regex regex("\"" + key + "\"\\s*:\\s*\"([^\"]*)\"");
    std::smatch match;
    if (!std::regex_search(object, match, regex)) {
        return "";
    }

    return match[1].str();
}

static int JsonInt(const std::string &object, const std::string &key, int fallback)
{
    std::regex regex("\"" + key + "\"\\s*:\\s*(-?\\d+)");
    std::smatch match;
    if (!std::regex_search(object, match, regex)) {
        return fallback;
    }

    return std::stoi(match[1].str());
}

static float JsonFloat(const std::string &object, const std::string &key, float fallback)
{
    std::regex regex("\"" + key + "\"\\s*:\\s*(-?\\d+(?:\\.\\d+)?)");
    std::smatch match;
    if (!std::regex_search(object, match, regex)) {
        return fallback;
    }

    return std::stof(match[1].str());
}

static bool ParseQuestKind(const std::string &text, QuestKind *kind)
{
    if (text == "open_chests") {
        *kind = QuestKind::OpenChests;
        return true;
    }

    if (text == "kill_slimes") {
        *kind = QuestKind::KillSlimes;
        return true;
    }

    return false;
}

// ---------------- Loading ----------------

bool QuestLog::LoadFromJson(const char *path)
{
    std::string json = ReadTextFile(path);
    if (json.empty()) {
        return false;
    }

    questTemplates.clear();
    signsFromJson.clear();
    activeQuests.clear();

    std::regex questRegex("\\{[^\\{\\}]*\"id\"[^\\{\\}]*\\}");
    for (std::sregex_iterator it(json.begin(), json.end(), questRegex), end; it != end; ++it) {
        std::string object = it->str();

        QuestKind kind = QuestKind::OpenChests;
        if (!ParseQuestKind(JsonString(object, "type"), &kind)) {
            continue;
        }

        Quest quest;
        quest.id = JsonString(object, "id");
        quest.title = JsonString(object, "title");
        quest.completeTitle = JsonString(object, "complete_title");
        quest.objective = JsonString(object, "objective");
        quest.hint = JsonString(object, "hint");
        quest.completeHint = JsonString(object, "complete_hint");
        quest.kind = kind;
        quest.required = JsonInt(object, "required", 1);
        quest.progress = 0;
        quest.complete = false;
        quest.completeTimer = 0.0f;
        quest.visibility = 0.0f;

        if (quest.completeTitle.empty()) {
            quest.completeTitle = "Complete";
        }
        if (quest.completeHint.empty()) {
            quest.completeHint = "Done.";
        }

        bool valid = !quest.id.empty() && !quest.title.empty() &&
            !quest.objective.empty() && !quest.hint.empty() &&
            quest.required > 0;

        if (valid) {
            questTemplates.push_back(quest);
        }
    }

    std::regex signRegex("\\{[^\\{\\}]*\"quest_id\"[^\\{\\}]*\\}");
    for (std::sregex_iterator it(json.begin(), json.end(), signRegex), end; it != end; ++it) {
        std::string object = it->str();
        QuestSignData sign;
        sign.questId = JsonString(object, "quest_id");
        sign.x = JsonFloat(object, "x", 0.0f);
        sign.y = JsonFloat(object, "y", 0.0f);

        if (!sign.questId.empty()) {
            signsFromJson.push_back(sign);
        }
    }

    return !questTemplates.empty();
}

// ---------------- Starting quests ----------------

bool QuestLog::StartQuest(const std::string &questId)
{
    if (!CanStartQuest() || IsQuestRunning(questId)) {
        return false;
    }

    const Quest *questTemplate = FindQuestTemplate(questId);
    if (questTemplate == NULL) {
        return false;
    }

    activeQuests.push_back(*questTemplate);
    return true;
}

bool QuestLog::CanStartQuest() const
{
    return static_cast<int>(activeQuests.size()) < maxActiveQuests;
}

bool QuestLog::HasQuestTemplate(const std::string &questId) const
{
    return FindQuestTemplate(questId) != NULL;
}

// ---------------- Game events ----------------

void QuestLog::OnChestOpened()
{
    AddProgress(QuestKind::OpenChests);
}

void QuestLog::OnSlimeKilled()
{
    AddProgress(QuestKind::KillSlimes);
}

void QuestLog::AddProgress(QuestKind kind)
{
    for (Quest &quest : activeQuests) {
        if (quest.complete || quest.kind != kind) {
            continue;
        }

        quest.progress++;
        if (quest.progress >= quest.required) {
            quest.progress = quest.required;
            quest.complete = true;
            quest.completeTimer = completeTextTime;
        }
    }
}

// ---------------- Frame update ----------------

void QuestLog::Update(float deltaTime)
{
    for (Quest &quest : activeQuests) {
        float targetVisibility = 1.0f;

        if (quest.complete) {
            quest.completeTimer -= deltaTime;
            if (quest.completeTimer <= 0.0f) {
                targetVisibility = 0.0f;
            }
        }

        if (quest.visibility < targetVisibility) {
            quest.visibility = std::min(targetVisibility, quest.visibility + hudAnimationSpeed * deltaTime);
        } else if (quest.visibility > targetVisibility) {
            quest.visibility = std::max(targetVisibility, quest.visibility - hudAnimationSpeed * deltaTime);
        }
    }

    activeQuests.erase(
        std::remove_if(activeQuests.begin(), activeQuests.end(), [](const Quest &quest) {
            return quest.complete && quest.completeTimer <= 0.0f && quest.visibility <= 0.01f;
        }),
        activeQuests.end()
    );
}

// ---------------- Simple getters ----------------

const std::vector<Quest> &QuestLog::GetQuests() const
{
    return activeQuests;
}

int QuestLog::GetQuestTemplateCount() const
{
    return static_cast<int>(questTemplates.size());
}

const std::string &QuestLog::GetQuestTemplateId(int index) const
{
    return questTemplates[index].id;
}

const std::vector<QuestSignData> &QuestLog::GetSignsFromJson() const
{
    return signsFromJson;
}

Quest *QuestLog::FindQuestTemplate(const std::string &questId)
{
    for (Quest &quest : questTemplates) {
        if (quest.id == questId) {
            return &quest;
        }
    }

    return NULL;
}

const Quest *QuestLog::FindQuestTemplate(const std::string &questId) const
{
    for (const Quest &quest : questTemplates) {
        if (quest.id == questId) {
            return &quest;
        }
    }

    return NULL;
}

bool QuestLog::IsQuestRunning(const std::string &questId) const
{
    for (const Quest &quest : activeQuests) {
        if (quest.id == questId) {
            return true;
        }
    }

    return false;
}
