#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <unordered_map>

// Represents a single animation sequence (e.g., "Walk_South" or "Attack")
struct Animation {
    std::vector<sf::IntRect> frames;
    float frameDuration = 0.1f; // Speed of animation (seconds per frame)
    bool loop = true;
};

// Helper struct to manage and play multiple animations on a sprite
struct AnimationManager {
    std::unordered_map<std::string, Animation> animations;
    std::string currentAnim = "";
    size_t frameIndex = 0;
    float timer = 0.0f;
    bool isPlaying = true;

    // Add an animation to the manager
    void AddAnimation(const std::string& name, const Animation& anim) {
        animations[name] = anim;
    }

    // Helper to automatically slice a row of frames from a spritesheet
    void AddRow(const std::string& name, int startX, int startY, int frameWidth, int frameHeight, int frameCount, float speed = 0.1f) {
        Animation anim;
        anim.frameDuration = speed;
        for (int i = 0; i < frameCount; ++i) {
            anim.frames.push_back(sf::IntRect(
                { startX + (i * frameWidth), startY },
                { frameWidth, frameHeight }
            ));
        }
        animations[name] = anim;
    }

    // Call this in your entity's Update() function
    void Update(float delta_time, sf::Sprite& sprite) {
        if (!isPlaying || currentAnim.empty() || animations.find(currentAnim) == animations.end())
            return;

        Animation& anim = animations[currentAnim];
        if (anim.frames.empty()) return;

        timer += delta_time;
        if (timer >= anim.frameDuration) {
            timer = 0.0f;
            if (anim.loop) {
                frameIndex = (frameIndex + 1) % anim.frames.size();
            }
            else {
                if (frameIndex + 1 < anim.frames.size()) {
                    frameIndex++;
                }
            }
        }

        sprite.setTextureRect(anim.frames[frameIndex]);
    }

    // Switch to a new animation state
    void Play(const std::string& name) {
        if (currentAnim != name) {
            currentAnim = name;
            frameIndex = 0;
            timer = 0.0f;
            isPlaying = true;
        }
    }
};