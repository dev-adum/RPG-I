#pragma once
#include <SFML/Graphics.hpp>
#include "Bullet.h"
#include "Player.h"

class BulletSystem
{
public:
    std::vector<Bullet> bullets;

    float fireTimer = 0.f;
    float fireCooldown = 0.5f;

    void Update(float delta_time);
    void Shoot(sf::Vector2f start, sf::Vector2f dir);
    void Draw(sf::RenderWindow& window);
};