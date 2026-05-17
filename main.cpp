#include "Player.h"
#include "Bullet.h"
#include "BulletSystem.h"
#include "Input.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <math.h>
#include <cstdlib>
#include <ctime>



sf::Vector2f NormalizeVector(sf::Vector2f vector)
{
    float m = std::sqrt(vector.x * vector.x + vector.y * vector.y);

    sf::Vector2f normalizedVector;

    normalizedVector.x = vector.x / m;
    normalizedVector.y = vector.y / m;

    return normalizedVector;
}



int main()
{ 
    Input input;
    Player player;
    BulletSystem bulletSystem;

    sf::Vector2f lastDir(1.f, 0.f);

    float fireCooldown = 0.25f;
    float fireTimer = 0.f;
    float maxAimDistance = 150.f;

    player.Load();
    player.Initialize();

    srand(time(nullptr));


    const sf::Vector2f view_size = { 500.f, 500.f };
    sf::RenderWindow window(sf::VideoMode({ 500, 500 }), "Level I");
    sf::View view(sf::FloatRect({ 0.f, 0.f }, { 500.f, 500.f }));
    window.setView(view);
    sf::Vector2u windowSize = window.getSize();


   

    sf::Font font;
    if (!font.openFromFile("Assets/Fonts/Helvetica.ttf"))
    {
       return EXIT_FAILURE;
   }

    sf::Clock clock;
   

    sf::Text text(font);
    text.setString("Move: Arrows/WASD  \n Fire: LMB or 'F'");
    text.setCharacterSize(7);
    text.setPosition({ 30,30 });

            
    
    while (window.isOpen())
    {
        
        sf::Vector2f clampedMouse = player.GetClampedAim(window, maxAimDistance);
        player.targetSprite.setPosition(clampedMouse);
        sf::Vector2f dir = input.GetMovement();
        float const delta_time = clock.restart().asSeconds();
        sf::Vector2f start = player.playerSprite.getPosition();
        sf::Vector2f aimDir = NormalizeVector(clampedMouse - start);
        
        bulletSystem.Update(delta_time);
        player.Update(dir, aimDir, delta_time);

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (event->is<sf::Event::Resized>())
            {
                auto size = event->getIf<sf::Event::Resized>()->size;

                sf::View view(sf::FloatRect({ 0.f, 0.f }, { 100.f, 100.f }));

                float windowAspect = float(size.x) / float(size.y);
                float worldAspect = 1.f;

                if (windowAspect > worldAspect)
                    view.setSize({ 500.f * windowAspect, 500.f });
                else
                    view.setSize({ 500.f, 500.f / windowAspect });

                view.setCenter({ 150.f, 150.f });
                window.setView(view);
            
            
            
            }
        }
       


        bulletSystem.Update(delta_time);

   
        static float fireTimer = 0.f;

        fireTimer += delta_time;

        bool fireHeld = input.FirePressed();

        if (fireHeld && fireTimer >= fireCooldown)
        {
            fireTimer = 0.f;

            bulletSystem.Shoot(start, aimDir);

            sf::Sound* s = (rand() % 2 == 0)
                ? &player.fireSound1
                : &player.fireSound2;

            s->setVolume(player.bulletVolume);
            s->stop();
            s->play();
        }


        
        window.clear();
        
        window.draw(player.playerSprite);
        window.draw(player.targetSprite);
        window.draw(text);
        bulletSystem.Draw(window);
        window.display();
        window.setView(window.getView());
        window.setMouseCursorVisible(false);
    }
}