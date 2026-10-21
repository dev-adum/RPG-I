#include "Enemy.h"

Enemy::Enemy()
    : enemySprite(idleTexture),
    hitSound1(hitBuffer1)
{
}

void Enemy::Load()
{
    if (!idleTexture.loadFromFile("Assets/Enemy/Textures/Enemy_Idle.png"))
    {
        std::cout << "Failed to load Enemy_Idle.png\n";
    }

    if (!hitTexture.loadFromFile("Assets/Enemy/Textures/Enemy_Hit.png"))
    {
        std::cout << "Failed to load Enemy_Hit.png\n";
    }

    if (!hitBuffer1.loadFromFile("Assets/Enemy/SFX/Enemy_Hit.wav"))
    {
        std::cout << "Failed to load Enemy_Hit.wav\n";
    }
}

void Enemy::Initialize()
{
    enemyBoxSize = sf::Vector2i({ 16, 32 });

    enemyBox.setFillColor(sf::Color::Transparent);
    enemyBox.setOutlineColor(sf::Color::Transparent);
    enemyBox.setOutlineThickness(1);
    enemyBox.setSize(sf::Vector2f(enemyBoxSize.x, enemyBoxSize.y));
    enemyBox.setOrigin({ 8, 16 });

    enemyHealthBarBack.setSize({ 30.f, 3.f });
    enemyHealthBarBack.setFillColor(sf::Color::Red);

    enemyHealthBarFront.setSize({ 30.f, 3.f });
    enemyHealthBarFront.setFillColor(sf::Color::Green);

    enemySprite.setOrigin({ 32.f, 32.f });
    enemySprite.setPosition({ 150.f, 150.f });

    hitSound1.setVolume(30.f);

    // --- SETUP ENEMY ANIMATIONS VIA ANIMATION MANAGER ---
    // Assuming your enemy spritesheet has rows for Idle and Hit
    animManager.AddRow("Idle_South", 0, 0, 64, 64, 4, 0.15f);
    animManager.AddRow("Hit_South", 0, 0, 64, 64, 4, 0.08f);
}

void Enemy::Update(float delta_time, sf::Vector2f playerPos)
{
    // --- 1. HEALTH BAR TRACKING ---
    sf::Vector2f currentPos = enemySprite.getPosition();

    enemyHealthBarBack.setPosition({ currentPos.x - 15.f, currentPos.y - 40.f });
    enemyHealthBarFront.setPosition({ currentPos.x - 15.f, currentPos.y - 40.f });

    float healthPercent = static_cast<float>(health) / maxHealth;
    enemyHealthBarFront.setSize({ 30.f * healthPercent, 3.f });

    // --- 2. FINITE STATE MACHINE (FSM) AI ---
    std::string animState = "Idle_South";

    if (isHit)
    {
        enemy_hit_timer += delta_time;
        animState = "Hit_South";

        if (enemy_hit_timer >= hit_duration)
        {
            isHit = false;
            enemy_hit_timer = 0.f;
        }
    }
    else
    {
        // Calculate vector from enemy to player
        sf::Vector2f dirToPlayer = playerPos - currentPos;
        float distance = std::sqrt(dirToPlayer.x * dirToPlayer.x + dirToPlayer.y * dirToPlayer.y);

        // Check if player is within aggro range to Chase
        if (distance < aggroRange && distance > 5.f)
        {
            // Normalize direction vector for movement
            sf::Vector2f normalizedDir = Math::NormalizeVector(dirToPlayer);
            currentPos += normalizedDir * moveSpeed * delta_time;
            enemySprite.setPosition(currentPos);

            // Use our modular math helper to map direction to animation name!
            std::string facingDir = Math::Get8WayDirection(dirToPlayer);
            animState = "Idle_South"; // Change to "Walk_" + facingDir once you have walk sheets!
        }
        else
        {
            animState = "Idle_South";
        }
    }

    // --- 3. UPDATE ANIMATIONS & HITBOX ---
    animManager.Play(animState);
    animManager.Update(delta_time, enemySprite);

    enemyBox.setPosition(enemySprite.getPosition());
}

void Enemy::TakeDamage(int damage)
{
    health -= damage;
    isHit = true;
    enemy_hit_timer = 0.f;
    hitSound1.play();

    if (health <= 0)
    {
        health = 0;
        dead = true;
        std::cout << "Enemy Dead\n";
    }
}

void Enemy::Draw(sf::RenderWindow& window)
{
    window.draw(enemySprite);
    window.draw(enemyBox);
    window.draw(enemyHealthBarBack);
    window.draw(enemyHealthBarFront);
}