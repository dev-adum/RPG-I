#include "UIManager.h"
#include <iostream>

void UIManager::Load()
{
    if (!font.openFromFile("Assets/Fonts/Helvetica.ttf"))
    {
        std::cout << "Failed to load font\n";
    }
}

void UIManager::AddText(
    const std::string& str, unsigned int size, sf::Vector2f texpos)
{
    sf::Text text(font);

    text.setString(str);
    text.setCharacterSize(size);
    text.setPosition(texpos);
    text.setScale({ 1.f, 1.f });
    texts.push_back(text);
   
}

void UIManager::Draw(sf::RenderWindow& window)
{
    for (auto& text : texts)
    {
        window.draw(text);
    }
}