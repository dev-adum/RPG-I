#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include "Math.h"
#include "Enemy.h"
#include "BulletSystem.h"
#include "MapData.h"
#include "Animation.h" 

class Enemy;

class Player
{
public:
    Player();

    void Load();
    void Initialize();
    void Update(sf::Vector2f dir, sf::Vector2f aimDir, float delta_time, const sf::View& view, Enemy& enemy, BulletSystem& bulletSystem, const MapData& mapData);
    void Draw(sf::RenderWindow& window);

    // Clean position getter using your authoritative hitbox
    sf::Vector2f getPosition() const {
        return playerBox.getPosition();
    }

    sf::Vector2f GetSmoothedAim(sf::RenderWindow& window, const sf::View& view, float delta_time);

    sf::Vector2f smoothedAimOffset = { 0.f, 0.f };
    bool isReticuleInitialized = false;

    sf::Vector2f aimOffset = { 0.f, 0.f };
    sf::Vector2i lastMousePos;
    bool firstMouseCheck = true;

    sf::RectangleShape playerBox;
    sf::Vector2i playerBoxSize;

    sf::Texture playerTexture;
    sf::Sprite playerSprite;

    sf::Texture reticuleTexture;
    sf::Sprite reticuleSprite;

    sf::SoundBuffer walkBuffer;
    sf::SoundBuffer fireBuffer1;
    sf::SoundBuffer fireBuffer2;

    sf::Sound fireSound1;
    sf::Sound fireSound2;
    sf::Sound walkSound;

    // --- ANIMATION MANAGER ---
    AnimationManager animManager;

    std::vector<sf::IntRect> Reticule;
    unsigned int targ_idx = 0;
    float reticule_anim_timer = 0.0f;
    float const reticule_anim_speed = 0.8f;

    float maxAimDistance = 100.f;
    float moveSpeed = 100.f;

    bool isFiring = false;
    float bulletVolume = 20.f;
};