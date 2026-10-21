#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Input.h"
#include "UIManager.h"

// Everything a scene might need that main owns.
struct GameContext {
    sf::RenderWindow& window;
    sf::View& view;
    Input& input;
    UIManager& ui;
    sf::Music& music;
};