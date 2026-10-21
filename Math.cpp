#include "Math.h"
#include <cmath>
#include <string>

sf::Vector2f Math::NormalizeVector(sf::Vector2f vector)
{
    float m = std::sqrt(vector.x * vector.x + vector.y * vector.y);

    sf::Vector2f normalizedVector;

    if (m != 0.f)
    {
        normalizedVector.x = vector.x / m;
        normalizedVector.y = vector.y / m;
    }
    else
    {
        normalizedVector = { 0.f, 0.f };
    }

    return normalizedVector;
}

bool Math::DetectCollision(sf::FloatRect rect1, sf::FloatRect rect2)
{
    if (rect1.position.x < rect2.position.x + rect2.size.x &&
        rect1.position.x + rect1.size.x > rect2.position.x &&
        rect1.position.y < rect2.position.y + rect2.size.y &&
        rect1.position.y + rect1.size.y > rect2.position.y)
        return true;

    return false;
}

std::string Math::Get8WayDirection(sf::Vector2f dir)
{
    if (dir.x == 0.f && dir.y == 0.f) return "South"; // Default fallback

    float angle = std::atan2(dir.y, dir.x) * 180.f / 3.14159f;
    if (angle < 0.f) angle += 360.f;

    int index = static_cast<int>(std::floor((angle + 22.5f) / 45.f)) % 8;

    const std::string directions[8] = {
        "East", "SouthEast", "South", "SouthWest",
        "West", "NorthWest", "North", "NorthEast"
    };

    return directions[index];
}