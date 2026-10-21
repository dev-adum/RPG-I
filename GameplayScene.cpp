#include "GameplayScene.h"
#include "SceneManager.h"
#include <iostream>

GameplayScene::GameplayScene(SceneManager& sceneManager, const std::string& mapFilename)
    : m_sceneManager(sceneManager), m_mapFilename(mapFilename)
{
    // 1. Load the map
    m_mapLoader.Load(m_mapFilename, m_mapData);

    // 2. Automatically find the player spawn point inside the map's objects
    bool spawned = false;
    for (const auto& obj : m_mapData.GetObjects())
    {
        if (obj.name == "Spawn" || obj.name == "PlayerSpawn")
        {
            // Position the player at this object's coordinates from Tiled
            m_player.playerBox.setPosition({ obj.bounds.position.x, obj.bounds.position.y });
            spawned = true;
            break;
        }
    }

    // Fallback default position if you forgot to place a spawn object in Tiled
    if (!spawned)
    {
        m_player.playerBox.setPosition({ 100.f, 100.f });
    }

}

void GameplayScene::HandleInput()
{
    // If your player handles its own input polling or event listening, call it here:
    // m_player.HandleInput();
}

void GameplayScene::Update(float dt)
{
    // --- Object Layer Door Collision Check ---
    sf::FloatRect playerBounds = m_player.playerBox.getGlobalBounds();

    for (const auto& obj : m_mapData.GetObjects())
    {
        if (obj.name == "Door")
        {
            if (playerBounds.findIntersection(obj.bounds))
            {
                std::cout << "Door triggered! Loading: " << obj.targetMap << std::endl;

                // Just change the scene with the 2 standard arguments!
                m_sceneManager.ChangeScene<GameplayScene>(m_sceneManager, obj.targetMap);
                break;
            }
        }
    }
}


void GameplayScene::Render(sf::RenderWindow& window)
{
    // --- Render your map tiles here ---
    // (Loop through m_mapData.GetData() and draw tiles)

    // Render your actual player
    // window.draw(m_player);
}