#include "slime.h"

#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

//чтение весов
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

static std::string ExtractJsonArray(const std::string &json, const std::string &key)
{
    size_t keyPosition = json.find("\"" + key + "\"");
    if (keyPosition == std::string::npos) {
        return "";
    }

    size_t arrayStart = json.find('[', keyPosition);
    if (arrayStart == std::string::npos) {
        return "";
    }

    int depth = 0;
    for (size_t i = arrayStart; i < json.size(); i++) {
        if (json[i] == '[') {
            depth++;
        } else if (json[i] == ']') {
            depth--;
            if (depth == 0) {
                return json.substr(arrayStart, i - arrayStart + 1);
            }
        }
    }

    return "";
}

static std::vector<float> ParseNumbers(const std::string &text)
{
    std::vector<float> numbers;
    std::regex numberRegex(R"([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)");

    for (std::sregex_iterator it(text.begin(), text.end(), numberRegex), end; it != end; ++it) {
        numbers.push_back(std::stof(it->str()));
    }

    return numbers;
}

Slime::Slime()
{
    initAnimations();

    texture = NULL;
    sizeSprite = 144;
    spriteAnimation.SetSpriteSize(sizeSprite);
    rect = SDL_FRect{500, 500, sizeSprite, sizeSprite};
    speed = 120.0f;
    isWalk = true;
    lookLeft = true;
    health = 30;
    moveDirection = -1;
    patrolStartX = rect.x;
    patrolDistance = 180.0f;
    brainLoaded = false;
}

Slime::~Slime()
{
    Unload();
}

void Slime::initAnimations()
{
    animations.walk = {4, 120, 1};
}

bool Slime::Load(SDL_Renderer *renderer, const char *texturePath)
{
    texture = IMG_LoadTexture(renderer, texturePath);
    if (texture == NULL) {
        SDL_Log("IMG_LoadTexture failed: %s", SDL_GetError());
        return false;
    }

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    return true;
}

bool Slime::LoadBrain(const char *weightsPath)
{
    std::string json = ReadTextFile(weightsPath);
    if (json.empty()) {
        SDL_Log("Slime brain weights not found: %s", weightsPath);
        return false;
    }

    std::vector<float> fc1WeightValues = ParseNumbers(ExtractJsonArray(json, "fc1.weight"));
    std::vector<float> fc1BiasValues = ParseNumbers(ExtractJsonArray(json, "fc1.bias"));
    std::vector<float> fc2WeightValues = ParseNumbers(ExtractJsonArray(json, "fc2.weight"));
    std::vector<float> fc2BiasValues = ParseNumbers(ExtractJsonArray(json, "fc2.bias"));

    if (fc1WeightValues.size() != 40 || fc1BiasValues.size() != 8 ||
        fc2WeightValues.size() != 16 || fc2BiasValues.size() != 2) {
        SDL_Log("Slime brain weights have wrong shape");
        return false;
    }

    int index = 0;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 5; col++) {
            fc1Weight[row][col] = fc1WeightValues[index++];
        }
    }

    for (int i = 0; i < 8; i++) {
        fc1Bias[i] = fc1BiasValues[i];
    }

    index = 0;
    for (int row = 0; row < 2; row++) {
        for (int col = 0; col < 8; col++) {
            fc2Weight[row][col] = fc2WeightValues[index++];
        }
    }

    for (int i = 0; i < 2; i++) {
        fc2Bias[i] = fc2BiasValues[i];
    }

    brainLoaded = true;
    return true;
}

void Slime::Unload()
{
    SDL_DestroyTexture(texture);
    texture = NULL;
}
//надо сделать загрузку от deltaTime
void Slime::Update(float deltaTime, const SDL_FRect &playerRect, float playerPower)
{
    if (brainLoaded) {
        SDL_FPoint move = PredictMove(playerRect, playerPower);
        rect.x += move.x * speed * deltaTime;
        rect.y += move.y * speed * deltaTime;

        if (move.x < -0.01f) {
            lookLeft = true;
        } else if (move.x > 0.01f) {
            lookLeft = false;
        }
    } else {
        float step = speed * deltaTime * moveDirection;
        rect.x += step;

        if (rect.x <= patrolStartX - patrolDistance) {
            moveDirection = 1;
            lookLeft = false;
        }
        if (rect.x >= patrolStartX + patrolDistance) {
            moveDirection = -1;
            lookLeft = true;
        }
    }

    if (isWalk) {
        spriteAnimation.Play(animations.walk, true);
    } else {
        spriteAnimation.Play(animations.idle, true);
    }
}
//предиктим
SDL_FPoint Slime::PredictMove(const SDL_FRect &playerRect, float playerPower) const
{
    float slimeCenterX = rect.x + rect.w / 2;
    float slimeCenterY = rect.y + rect.h / 2;
    float playerCenterX = playerRect.x + playerRect.w / 2;
    float playerCenterY = playerRect.y + playerRect.h / 2;

    float dx = (playerCenterX - slimeCenterX) / 1000.0f;
    float dy = (playerCenterY - slimeCenterY) / 1000.0f;
    float distance = std::min(std::sqrt(dx * dx + dy * dy), 1.0f);
    float slimeHealth = health / 30.0f;

    if (playerPower > 0.8f || slimeHealth < 0.25f) {
        return NormalizeMove(-dx, -dy);
    }

    float input[5] = {dx, dy, distance, playerPower, slimeHealth};
    float hidden[8];

    for (int row = 0; row < 8; row++) {
        float value = fc1Bias[row];
        for (int col = 0; col < 5; col++) {
            value += fc1Weight[row][col] * input[col];
        }

        hidden[row] = std::max(value, 0.0f);
    }

    float output[2];
    for (int row = 0; row < 2; row++) {
        float value = fc2Bias[row];
        for (int col = 0; col < 8; col++) {
            value += fc2Weight[row][col] * hidden[col];
        }

        output[row] = value;
    }

    return NormalizeMove(output[0], output[1]);
}

SDL_FPoint Slime::NormalizeMove(float x, float y) const
{
    float length = std::sqrt(x * x + y * y);
    if (length < 0.001f) {
        return SDL_FPoint{0.0f, 0.0f};
    }

    return SDL_FPoint{x / length, y / length};
}

void Slime::Render(SDL_Renderer *renderer, const SDL_FRect &camera) const
{
    SDL_FRect drawRect = rect;
    drawRect.x -= camera.x;
    drawRect.y -= camera.y;

    SDL_FlipMode flip = lookLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderTextureRotated(renderer, texture, spriteAnimation.GetSourceRect(), &drawRect, 0.0, NULL, flip);
}

SDL_FRect Slime::GetRect() const
{
    return rect;
}

int *Slime::GetHealthPointer()
{
    return &health;
}
