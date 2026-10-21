#pragma once
#include <SFML/Graphics.hpp>

class Scene {
public:
    virtual ~Scene() = default;

    virtual void HandleInput() = 0;
    virtual void Update(float dt) = 0;
    virtual void Render(sf::RenderWindow& window) = 0;
};