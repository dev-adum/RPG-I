#pragma once
#include <SFML/Audio.hpp>
#include <memory>

class FadingSound
{
public:
    FadingSound() = default;

    void setBuffer(const sf::SoundBuffer& buffer)
    {
        sound = std::make_unique<sf::Sound>(buffer);
    }

    void play()
    {
        if (!sound) return;

        sound->setVolume(100.f);
        sound->play();

        active = true;
        playTime = 0.f;
    }

    void update(float dt)
    {
        if (!active || !sound) return;

        playTime += dt;

        if (playTime >= maxDuration)
        {
            sound->stop();
            active = false;
            return;
        }

        float remaining = maxDuration - playTime;

        if (remaining < 0.2f)
        {
            float v = sound->getVolume();
            v -= fadeSpeed * dt;

            if (v <= 0.f)
            {
                sound->stop();
                active = false;
            }
            else
            {
                sound->setVolume(v);
            }
        }
    }

    void setMaxDuration(float d) { maxDuration = d; }
    void setFadeSpeed(float s) { fadeSpeed = s; }

private:
    std::unique_ptr<sf::Sound> sound;

    float fadeSpeed = 250.f;
    bool active = false;
    float playTime = 0.f;
    float maxDuration = 1.f;
};