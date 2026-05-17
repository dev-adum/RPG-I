#include "BulletSystem.h"

void BulletSystem::Update(float dt)
{
    fireTimer += dt;

    for (auto& b : bullets)
    {
        b.shape.move(b.direction * b.bulletSpeed);
        b.life -= dt;
    }

    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(),
            [](const Bullet& b) { return b.life <= 0.f; }),
        bullets.end()
    );
}

void BulletSystem::Shoot(sf::Vector2f start, sf::Vector2f dir)
{
    if (fireTimer < fireCooldown)
        return;

    fireTimer = 0.f;

    Bullet b;
    b.Initialize();

    b.shape.setPosition(start);
    b.direction = dir;

    bullets.push_back(b);
}


void BulletSystem::Draw(sf::RenderWindow& window)
{
    for (auto& b : bullets)
        window.draw(b.shape);
}