#include "Player.h"
#include "Enemy.h"
#include "BulletSystem.h"
#include "Input.h"
#include "UIManager.h"
#include "Math.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <math.h>
#include <cstdlib>
#include <ctime>


int main()
{ 
    Input input;
    Player player;
    Enemy enemy;
    BulletSystem bulletSystem;
    UIManager ui;
  
    enemy.Load();
    enemy.Initialize();

    player.Load();
    player.Initialize();
    
    ui.Load();

    sf::Vector2f lastDir(1.f, 0.f);
    
    
    float fpsTimer = 0.f;
    int frameCount = 0;
    int fps = 0;


    const float WORLD_W = 600.f;
    const float WORLD_H = 600.f;
    sf::View gameView(sf::FloatRect({ 0.f, 0.f }, { WORLD_W, WORLD_H }));

  
    float maxAimDistance = 150.f;

    srand(static_cast<unsigned int>(time(nullptr)));
    sf::Clock clock;


    const sf::Vector2f view_size = { 600.f, 600.f };
    sf::RenderWindow window(sf::VideoMode({ 600, 600 }), "Level I");
    window.setVerticalSyncEnabled(true);
   
    sf::Vector2u windowSize = window.getSize();
   

    float zoom = 0.5f; // 0.5 = zoom in (2x bigger visuals)
    gameView.setSize({ 600.f * zoom, 600.f * zoom });


   
    ui.AddText(
        "Aim reticule with the mouse\nMove: WASD\nFire: LMB",
        11,
        { 10.f, 10.f }
    );
           
    ui.AddText("FPS: 0", 14, { 200.f, 20.f });
    ui.fpsTextIndex = 1;
    
    while (window.isOpen())
    {  
        float const delta_time = clock.restart().asSeconds();
        
        frameCount++;
        fpsTimer += delta_time;

        if (fpsTimer >= 1.f)
        {
            fps = frameCount;
            frameCount = 0;
            fpsTimer = 0.f;
        }

        sf::Vector2f dir = input.GetMovement();
        
        sf::Vector2f start = player.playerBox.getPosition();
        sf::Vector2f clampedMouse = player.GetClampedAim(window, maxAimDistance);
        sf::Vector2f aimDir = Math::NormalizeVector(clampedMouse - start);
        
        player.Update(dir, aimDir, delta_time, gameView, enemy);
        
        player.reticuleSprite.setPosition(clampedMouse);
        
        bulletSystem.Update(delta_time, enemy);
        enemy.Update(delta_time);

      
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* resized = event->getIf<sf::Event::Resized>())
            {
                float windowRatio =
                    static_cast<float>(resized->size.x) /
                    static_cast<float>(resized->size.y);

                float viewRatio = WORLD_W / WORLD_H;

                float sizeX = 1.f;
                float sizeY = 1.f;

                float posX = 0.f;
                float posY = 0.f;

                if (windowRatio > viewRatio)
                {
                    sizeX = viewRatio / windowRatio;
                    posX = (1.f - sizeX) * 0.5f;
                }
                else
                {
                    sizeY = windowRatio / viewRatio;
                    posY = (1.f - sizeY) * 0.5f;
                }

                gameView.setViewport(
                    sf::FloatRect(
                        { posX, posY },
                        { sizeX, sizeY }
                    )
                );
            }
        }

   
        if (input.FirePressed())
        {
            if (bulletSystem.Shoot(start, aimDir))
            {
                sf::Sound* s = (rand() % 2 == 0)
                    ? &player.fireSound1
                    : &player.fireSound2;

                s->setVolume(player.bulletVolume);
                s->play();
            }
        }
        
        
        window.clear();
        
        window.setView(gameView);
       
        enemy.Draw(window);
        player.Draw(window);
        bulletSystem.Draw(window);
        window.setView(window.getDefaultView());
       
        ui.texts[ui.fpsTextIndex].setString("FPS: " + std::to_string(fps));
        ui.Draw(window);
       
        window.display();
        window.setMouseCursorVisible(false);
    }
}