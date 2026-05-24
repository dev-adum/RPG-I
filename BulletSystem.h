#pragma once
#include <SFML/Graphics.hpp>
#include "Enemy.h"
#include "Input.h"

struct Bullet
{
    sf::RectangleShape shape;
    sf::Vector2f direction;
    float bulletSpeed = 20.f;
    float life = 2.f;
    bool dead = false;
};

class BulletSystem
{
public:
    void Update(float delta_time, Enemy& enemy);
    void Draw(sf::RenderWindow& window);

    bool Shoot(sf::Vector2f start, sf::Vector2f dir);

private:
    std::vector<Bullet> bullets;

    float fireCooldown = .1f;
    float fireTimer = 0.f;
};