#pragma once
#include <SFML/Graphics.hpp>

class Input
{
public:
    sf::Vector2f GetMovement();

    // Movement speed multiplier applied while Shift is held.
    float sprintMultiplier = 2.0f;

};

