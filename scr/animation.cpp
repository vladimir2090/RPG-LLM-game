#include "animation.h"

Animation::Animation()
{
    sizeSprite = 0.0f;
    currentIndex = 0;
    lastUpdate = 0;
    currentAnimationY = -1;
    srcRect = SDL_FRect{0, 0, 0, 0};
}

void Animation::SetSpriteSize(float spriteSize)
{
    sizeSprite = spriteSize;
    srcRect.w = sizeSprite;
    srcRect.h = sizeSprite;
}

void Animation::Restart()
{
    currentIndex = 0;
    lastUpdate = 0;
    currentAnimationY = -1;
}

bool Animation::Play(const AnimationClip &clip, bool loop)
{
    Uint64 now = SDL_GetTicks();

    if (currentAnimationY != clip.y) {
        currentAnimationY = clip.y;
        currentIndex = 0;
        lastUpdate = now;
        srcRect.x = 0;
        srcRect.y = clip.y * sizeSprite;
        return false;
    }

    if (now - lastUpdate < static_cast<Uint64>(clip.animationDelay)) {return false;}

    lastUpdate = now;
    currentIndex++;

    if (currentIndex >= clip.frames) {
        if (!loop) {
            currentIndex = clip.frames - 1;
            srcRect.x = currentIndex * sizeSprite;
            srcRect.y = clip.y * sizeSprite;
            return true;
        }

        currentIndex = 0;
    }

    srcRect.x = currentIndex * sizeSprite;
    srcRect.y = clip.y * sizeSprite;
    return false;
}

const SDL_FRect *Animation::GetSourceRect() const
{
    return &srcRect;
}
