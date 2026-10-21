#pragma once
#include "Scene.h"
#include "MapData.h"
#include "MapLoader.h"
#include "Player.h" // Include your actual player header
#include <string>
#include <SFML/Graphics.hpp>

class SceneManager;

class GameplayScene : public Scene {
private:
    SceneManager& m_sceneManager;
    std::string m_mapFilename;
    sf::Vector2f spawnPos = { 100.f, 100.f };
    MapData m_mapData;
    MapLoader m_mapLoader;

    Player m_player; // <--- Declare your player here!

public:
    GameplayScene(SceneManager& sceneManager, const std::string& mapFilename);

    void HandleInput() override;
    void Update(float dt) override;
    void Render(sf::RenderWindow& window) override;
};