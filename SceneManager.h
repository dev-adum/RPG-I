#pragma once
#include "Scene.h"
#include <memory>
#include <utility>

class SceneManager {
private:
    std::unique_ptr<Scene> m_currentScene;

public:
    SceneManager() = default;

    // Changes the active scene, forwarding any constructor arguments directly to it
    template <typename T, typename... Args>
    void ChangeScene(Args&&... args) {
        m_currentScene = std::make_unique<T>(std::forward<Args>(args)...);
    }

    void Update(float dt) {
        if (m_currentScene) {
            m_currentScene->Update(dt);
        }
    }

    void Render(sf::RenderWindow& window) {
        if (m_currentScene) {
            m_currentScene->Render(window);
        }
    }
};