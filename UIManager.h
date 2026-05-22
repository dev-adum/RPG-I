#pragma once
#include <SFML/Graphics.hpp>


class UIManager
{
public:

    sf::Font font;
    std::vector<sf::Text> texts;

    void Load();

    void AddText(
        const std::string& str,
        unsigned int size,
        sf::Vector2f pos
    );

    void Draw(sf::RenderWindow& window);
};
