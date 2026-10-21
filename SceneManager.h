#pragma once
#include "Scene.h"
#include <memory>
#include <utility>
#include <functional>

class SceneManager {
private:
    std::unique_ptr<Scene> m_currentScene;
    std::function<std::unique_ptr<Scene>()> m_pendingScene;

public:
    SceneManager() = default;

    // Changes the active scene, forwarding any constructor arguments directly to it
    template <typename T, typename... Args>
    void ChangeScene(Args&&... args) {
        m_currentScene = std::make_unique<T>(std::forward<Args>(args)...);
    }

    // Queue a scene switch. Applied at the start of the next Update,
    // so the scene that requested it is never destroyed mid-method.
    void RequestScene(std::function<std::unique_ptr<Scene>()> factory) {
        m_pendingScene = std::move(factory);
    }

    void Update(float dt) {
        if (m_pendingScene) {
            m_currentScene = m_pendingScene();
            m_pendingScene = nullptr;
        }
        if (m_currentScene) {
            m_currentScene->Update(dt);
        }
    }

    void Render(sf::RenderWindow& window) {
        if (m_currentScene) {
            m_currentScene->Render(window);
        }
    }
    
    void HandleInput() {
        if (m_currentScene)
            m_currentScene->HandleInput();
    }

};