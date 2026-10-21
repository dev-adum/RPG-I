#pragma once
#include "Scene.h"
#include "GameContext.h"
#include "Map.h"
#include "Player.h"
#include "Enemy.h"
#include "BulletSystem.h"
#include <string>
#include <SFML/Graphics.hpp>

class SceneManager;

class GameplayScene : public Scene {
private:
    SceneManager& m_sceneManager;
    GameContext& m_context;
    std::string m_mapFilename;

    Map m_map;
    Player m_player;
    Enemy m_enemy;
    BulletSystem m_bullets;

    bool m_doorTriggered = false;

public:
    GameplayScene(SceneManager& sceneManager, GameContext& context,
        const std::string& mapFilename,
        sf::Vector2f spawnPoint = { -1.f, -1.f });

    void HandleInput() override;
    void Update(float dt) override;
    void Render(sf::RenderWindow& window) override;
};