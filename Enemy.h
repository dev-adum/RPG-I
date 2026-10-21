#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include "Math.h"
#include "Animation.h"
#include "MapData.h"

class Enemy
{
public:
    Enemy();

    void Load();
    void Initialize();
    void Update(float delta_time, sf::Vector2f playerPos, const MapData& mapData);
    void TakeDamage(int damage);
    void Draw(sf::RenderWindow& window);

    sf::RectangleShape enemyBox;
    sf::Vector2i enemyBoxSize;
    sf::Sprite enemySprite;

    // --- NEW: Animation Manager for the Enemy ---
    AnimationManager animManager;

    // Health & Combat
    int maxHealth = 100;
    int health = 100;
    bool isHit = false;
    bool dead = false;

    // UI Bars
    sf::RectangleShape enemyHealthBarBack;
    sf::RectangleShape enemyHealthBarFront;

private:
    sf::Texture idleTexture;
    sf::Texture hitTexture;
    sf::SoundBuffer hitBuffer1;
    sf::Sound hitSound1;

    // AI & Movement variables
    float moveSpeed = 50.f;
    float aggroRange = 80.f;
    float enemy_hit_timer = 0.f;
    float hit_duration = 0.2f;
};