#pragma once
#include "Scene.h"
#include "GameContext.h"
#include <string>
#include <SFML/Graphics.hpp>

class SceneManager;

// The pause menu.
//
// Entered from GameplayScene when M is pressed. The screen dips to black via
// SceneManager's existing fade (this scene is swapped in at full black), so the
// panel needs no entrance animation of its own - it is simply there.
//
// Closing (M or Esc) requests a return to GameplayScene, restoring the map and
// the player position we were opened from, so the player does not respawn.
class MenuScene : public Scene {
private:
    SceneManager& m_sceneManager;
    GameContext& m_context;

    // Where to return to.
    std::string m_returnMap;
    sf::Vector2f m_returnPlayerPos;

    // Panel geometry, recomputed on resize.
    sf::Vector2f m_panelSize;
    sf::Vector2f m_panelPos;

    sf::RectangleShape m_border;   // lighter blue frame
    sf::VertexArray    m_fill;     // 4-corner diagonal gradient

    void Layout();
    void RequestReturn();

public:
    MenuScene(SceneManager& sceneManager, GameContext& context,
        const std::string& returnMap, sf::Vector2f returnPlayerPos);

    void HandleInput() override;
    void Update(float dt) override;
    void Render(sf::RenderWindow& window) override;

    // Esc closes the menu, so main must not also treat it as "quit".
    bool ConsumesEscape() const override { return true; }
};
