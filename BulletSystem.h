#pragma once
#include <SFML/Graphics.hpp>
#include "Enemy.h"
#include "Math.h"

class BulletSystem
{
public:
    void Update(float delta_time, Enemy& enemy);
    void Shoot(sf::Vector2f start, sf::Vector2f dir);
    void Draw(sf::RenderWindow& window);

private:
    struct Bullet
    {
        sf::RectangleShape shape;
        sf::Vector2f direction;
        float bulletSpeed = 10.f;
        float life = 3.f;
        bool dead = false;
    };

    std::vector<Bullet> bullets;


    float fireTimer = 0.f;
    float fireCooldown = 0.25f;
};