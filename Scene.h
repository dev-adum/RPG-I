#pragma once
#include <SFML/Graphics.hpp>

class Scene {
public:
    virtual ~Scene() = default;

    virtual void HandleInput() = 0;
    virtual void Update(float dt) = 0;
    virtual void Render(sf::RenderWindow& window) = 0;

    // If true, the scene consumes Esc itself, so main should not treat Esc
    // as "quit". Default false = Esc quits as before.
    virtual bool ConsumesEscape() const { return false; }
};