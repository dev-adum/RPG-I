#pragma once
#include <SFML/Graphics.hpp>

class Bullet
{

public:

    Bullet();

    void Load();
    void Initialize();
    void Update();
    void Draw();

    float life = 3.f;
    float bulletSpeed = 0.075f;
   
    
    
    sf::RectangleShape shape;
    sf::Vector2f direction;

   


};

