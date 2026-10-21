#include "Enemy.h"
#include "Assets.h"

Enemy::Enemy()
    : enemySprite(idleTexture),
    hitSound1(hitBuffer1)
{
}

void Enemy::Load()
{
    if (!idleTexture.loadFromFile(AssetPaths::Path("Assets/Enemy/Textures/Enemy_Walk.png")))
    {
        std::cout << "Failed to load Enemy_Walk.png\n";
    }

    if (!hitTexture.loadFromFile(AssetPaths::Path("Assets/Enemy/Textures/Enemy_Hit.png")))
    {
        std::cout << "Failed to load Enemy_Hit.png\n";
    }

    if (!hitBuffer1.loadFromFile(AssetPaths::Path("Assets/Enemy/SFX/Enemy_Hit.wav")))
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
    // Enemy_Walk.png is byte-identical in layout to Player_Walk.png,
    // so the same slicing table applies.
    // Row 0 (Y = 0)
    animManager.AddRow("Walk_East", 384, 0, 64, 64, 2, 0.1f);
    animManager.AddRow("Walk_SouthEast", 256, 0, 64, 64, 2, 0.1f);
    animManager.AddRow("Walk_South", 128, 0, 64, 64, 2, 0.1f);
    animManager.AddRow("Walk_SouthWest", 0, 0, 64, 64, 2, 0.1f);

    // Row 1 (Y = 64)
    animManager.AddRow("Walk_West", 384, 64, 64, 64, 2, 0.1f);
    animManager.AddRow("Walk_NorthWest", 256, 64, 64, 64, 2, 0.1f);
    animManager.AddRow("Walk_North", 128, 64, 64, 64, 2, 0.1f);
    animManager.AddRow("Walk_NorthEast", 0, 64, 64, 64, 2, 0.1f);

    animManager.AddRow("Hit_South", 0, 0, 64, 64, 4, 0.08f);
}

void Enemy::Update(float delta_time, sf::Vector2f playerPos, const MapData& mapData)
{
    // --- 1. HEALTH BAR TRACKING ---
    sf::Vector2f currentPos = enemySprite.getPosition();

    enemyHealthBarBack.setPosition({ currentPos.x - 15.f, currentPos.y - 40.f });
    enemyHealthBarFront.setPosition({ currentPos.x - 15.f, currentPos.y - 40.f });

    float healthPercent = static_cast<float>(health) / maxHealth;
    enemyHealthBarFront.setSize({ 30.f * healthPercent, 3.f });

    // --- 2. FINITE STATE MACHINE (FSM) AI ---
    std::string animState = "Walk_South";

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
            sf::Vector2f normalizedDir = Math::NormalizeVector(dirToPlayer);
            sf::Vector2f move = normalizedDir * moveSpeed * delta_time;

            // --- TILE COLLISION (same 4-corner test the player uses) ---
            sf::Vector2f candidate = currentPos + move;
            bool blocked = false;

            if (mapData.GetTileSizeX() > 0 && mapData.GetTileSizeY() > 0)
            {
                sf::FloatRect box(
                    { candidate.x - enemyBoxSize.x / 2.f, candidate.y - enemyBoxSize.y / 2.f },
                    { static_cast<float>(enemyBoxSize.x), static_cast<float>(enemyBoxSize.y) });

                sf::Vector2f corners[4] = {
                    { box.position.x, box.position.y },
                    { box.position.x + box.size.x, box.position.y },
                    { box.position.x, box.position.y + box.size.y },
                    { box.position.x + box.size.x, box.position.y + box.size.y }
                };

                const auto& layout = mapData.GetData();

                for (const auto& corner : corners)
                {
                    int gridX = static_cast<int>(corner.x / mapData.GetTileSizeX());
                    int gridY = static_cast<int>(corner.y / mapData.GetTileSizeY());

                    if (gridY >= 0 && gridY < static_cast<int>(layout.size()) &&
                        gridX >= 0 && gridX < static_cast<int>(layout[gridY].size()))
                    {
                        if (mapData.IsTileSolid(layout[gridY][gridX]))
                        {
                            blocked = true;
                            break;
                        }
                    }
                    else
                    {
                        blocked = true;
                        break;
                    }
                }
            }

            if (!blocked)
            {
                currentPos = candidate;
                enemySprite.setPosition(currentPos);
            }

            // Walk animation matching the chase direction
            animState = "Walk_" + Math::Get8WayDirection(dirToPlayer);
        }
        else
        {
            animState = "Walk_South";
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