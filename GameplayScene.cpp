#include "GameplayScene.h"
#include "SceneManager.h"
#include "Math.h"
#include <iostream>
#include <memory>

GameplayScene::GameplayScene(SceneManager& sceneManager, GameContext& context,
    const std::string& mapFilename, sf::Vector2f spawnPoint)
    : m_sceneManager(sceneManager), m_context(context), m_mapFilename(mapFilename)
{
    m_map.Load(m_mapFilename);

    m_player.Load();
    m_player.Initialize();

    m_enemy.Load();
    m_enemy.Initialize();

    // Spawn priority: explicit door spawn > "Spawn" object > default
    if (spawnPoint.x >= 0.f && spawnPoint.y >= 0.f)
    {
        m_player.playerBox.setPosition(spawnPoint);
        return;
    }

    bool spawned = false;
    for (const auto& obj : m_map.md.GetObjects())
    {
        if (obj.name == "Spawn" || obj.name == "PlayerSpawn")
        {
            m_player.playerBox.setPosition({ obj.bounds.position.x, obj.bounds.position.y });
            spawned = true;
            break;
        }
    }
    if (!spawned)
        m_player.playerBox.setPosition({ 400.f, 300.f });
}

void GameplayScene::HandleInput()
{
}

void GameplayScene::Update(float dt)
{
    sf::Vector2f dir = m_context.input.GetMovement();

    // Mouse aiming removed. Player::Update now fires along its facing direction.
    m_player.Update(dir, m_player.facingDir, dt, m_context.view, m_enemy, m_bullets, m_map.md);

    // Camera follows the player AFTER movement, so it can never move the player
    // and is never stale by a frame.
    m_context.view.setCenter(m_player.playerBox.getPosition());

    m_bullets.Update(dt, m_enemy, m_context.input);
    m_enemy.Update(dt, m_player.playerBox.getPosition(), m_map.md);
    m_map.Update(dt);

    // --- door trigger (deferred switch) ---
    if (!m_doorTriggered)
    {
        const sf::FloatRect playerBounds = m_player.playerBox.getGlobalBounds();

        for (const auto& obj : m_map.md.GetObjects())
        {
            if (obj.name == "Door" && playerBounds.findIntersection(obj.bounds))
            {
                m_doorTriggered = true;

                const std::string target = obj.targetMap;
                const sf::Vector2f spawn = obj.spawnPos;
                SceneManager& sm = m_sceneManager;
                GameContext& ctx = m_context;

                m_sceneManager.RequestScene([&sm, &ctx, target, spawn]()
                {
                    return std::make_unique<GameplayScene>(sm, ctx, target, spawn);
                });
                break;
            }
        }
    }
}

void GameplayScene::Render(sf::RenderWindow& window)
{
    window.setView(m_context.view);

    m_map.Draw(window);
    m_enemy.Draw(window);
    m_player.Draw(window);
    m_bullets.Draw(window);
}