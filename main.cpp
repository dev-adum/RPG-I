#include "Player.h"
#include "Bullet.h"
#include "FadingSound.h"
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

    Player player;
    FadingSound sound;

    Bullet bullet;
    std::vector<Bullet> bullets;
    sf::Vector2f lastDir(1.f, 0.f);

    float fireCooldown = 0.25f;
    float fireTimer = 0.f;
    float maxAimDistance = 150.f;

    player.Load();
    player.Initialize();

    bullet.Initialize();
   
    
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
    sf::Vector2f dir(0.f, 0.f);

    sf::Text text(font);
    text.setString("Move: Arrows/WASD  \n Fire: LMB or 'F'");
    text.setCharacterSize(7);
    text.setPosition({ 30,30 });

            
    
    while (window.isOpen())
    {
        
        dir = sf::Vector2f(0.f, 0.f);
        float const delta_time = clock.restart().asSeconds();


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

        
        sf::Vector2f clampedMouse = player.GetClampedAim(window, maxAimDistance);
        player.targetSprite.setPosition(clampedMouse);
      

            if
                (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            {
                dir.y -= 1.f;
            }

            if
                (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            {
                dir.y += 1.f;
            }

            if
                (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            {
                dir.x += 1.f;
            }

            if
                (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            {
                dir.x -= 1.f;
            }

            float fireCooldown = 0.25f;
           


            fireTimer += delta_time;

            if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F) || sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
                && fireTimer >= fireCooldown)
            {
                fireTimer = 0.f;

                Bullet b;
                b.Initialize();
                
                sf::Vector2f startPos = player.playerSprite.getPosition();

                sf::Vector2f dir = NormalizeVector(clampedMouse - startPos);

                b.shape.setPosition(startPos);
                b.direction = dir;

                bullets.push_back(b);

                player.fireSound1.play();
            }

            sf::Vector2f aimDir = NormalizeVector(clampedMouse - player.playerSprite.getPosition());

            player.Update(dir, aimDir, delta_time);






        for (auto& b : bullets)
        {
            b.shape.move(b.direction * bullet.bulletSpeed);
            b.life -= delta_time;
        }

        bullets.erase(
            std::remove_if(bullets.begin(), bullets.end(),
                [](const Bullet& b) { return b.life <= 0.f; }),
            bullets.end()
        );

     
        
        window.clear();
        window.draw(player.playerSprite);
        window.draw(player.targetSprite);
        window.draw(text);
        
        for (auto& b : bullets)
        {
            window.draw(b.shape);
        }
     
        window.display();
        window.setView(window.getView());
        window.setMouseCursorVisible(false);
    }
}