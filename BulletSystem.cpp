#include "BulletSystem.h"
#include "Math.h"
#include <algorithm>

bool BulletSystem::Shoot(sf::Vector2f start, sf::Vector2f dir)
{
    if (fireTimer < fireCooldown)
        return false;

    fireTimer = 0.f;

    Bullet b;
    b.shape.setSize({ 2.f, 2.f });
    b.shape.setPosition(start);
    b.direction = dir;

    bullets.push_back(b);

    return true;
}

void BulletSystem::Update(float delta_time, Enemy& enemy)
{
    fireTimer += delta_time;

    for (auto& b : bullets)
    {
        b.shape.move(b.direction * b.bulletSpeed);
        b.life -= delta_time;

        if (Math::DetectCollision(
            b.shape.getGlobalBounds(),
            enemy.enemyBox.getGlobalBounds()))
        {
            enemy.hitSound1.play();

            enemy.isHit = true;
            enemy.enemy_hit_timer = 0.f;

            b.dead = true;
        }
    }

    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(),
            [](const Bullet& b)
            {
                return b.life <= 0.f || b.dead;
            }),
        bullets.end()
    );
}

void BulletSystem::Draw(sf::RenderWindow& window)
{
    for (auto& b : bullets)
        window.draw(b.shape);
}