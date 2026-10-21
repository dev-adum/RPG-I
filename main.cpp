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
#include "Map.h"
#include "MapLoader.h"


int main()
{
    Input input;
    Player player;
    Enemy enemy;
    BulletSystem bulletSystem;
    UIManager ui;
    Map map;


    sf::Music VoidTrack;


    enemy.Load();
    enemy.Initialize();

    player.Load();
    player.Initialize();


    map.Initialize();


    float fpsTimer = 0.f;
    int frameCount = 0;
    int fps = 0;

    sf::View gameView;
    // 7 tiles wide x 10 tiles high (assuming 32x32 pixels per tile)
    float viewWidth = 7.f * 32.f;  // 224.f
    float viewHeight = 10.f * 32.f; // 320.f
    gameView.setSize({ viewWidth, viewHeight });




    srand(static_cast<unsigned int>(time(nullptr)));
    sf::Clock clock;


    sf::RenderWindow window(sf::VideoMode({ 500, 500 }), "Level I");
    window.setVerticalSyncEnabled(true);
    window.setMouseCursorVisible(false);
    window.setMouseCursorGrabbed(true);

    auto updateView = [&window, &gameView]()
        {
            const sf::Vector2u size = window.getSize();
            const float aspect = static_cast<float>(size.x) / static_cast<float>(size.y);

            // Keep height fixed at 10 tiles (e.g., 320 pixels), adjust width based on window aspect ratio
            constexpr float viewHeight = 10.f * 32.f;
            const float viewWidth = viewHeight * aspect;

            gameView.setSize({ viewWidth, viewHeight });
        };
    updateView();


    if (!VoidTrack.openFromFile("Assets/Music/Void/VoidTrack.wav"))
    {
        std::cout << "Failed to load VoidTrack.wav\n";
    }

    VoidTrack.setLooping(true);
    VoidTrack.play();
    VoidTrack.setVolume(20);

    ui.Load();
    ui.AddText(
        "Aim reticule with the mouse\nMove: WASD\nFire: LMB",
        11,
        { 10.f, 10.f }
    );

    ui.AddText("FPS: 0", 14, { 200.f, 20.f });
    ui.fpsTextIndex = 1;

    while (window.isOpen())
    {
        //--- events ---
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (event->is<sf::Event::Resized>())
                updateView();

            if (event->is<sf::Event::FocusLost>())
            {
                VoidTrack.pause();
            }

            // Resume music instantly when focus is gained back
            if (event->is<sf::Event::FocusGained>())
            {
                VoidTrack.play();
            }

            if (const auto* key = event->getIf<sf::Event::KeyPressed>())
            {
                // F11 still toggles fullscreen <-> windowed
                if (key->code == sf::Keyboard::Key::F11)
                {
                    if (window.getSize() == sf::VideoMode::getDesktopMode().size)
                    {
                        window.create(sf::VideoMode({ 500, 500 }), "Level I");
                    }
                    else
                    {
                        window.create(sf::VideoMode::getDesktopMode(), "Level I",
                            sf::Style::Default, sf::State::Fullscreen);
                    }

                    window.setVerticalSyncEnabled(true);
                    window.setMouseCursorVisible(false);
                    window.setMouseCursorGrabbed(true);
                    updateView();
                }
                // Escape drops you out of fullscreen if you're in it, or closes the window if you're already windowed
                else if (key->code == sf::Keyboard::Key::Escape)
                {
                    if (window.getSize() == sf::VideoMode::getDesktopMode().size)
                    {
                        // Currently in fullscreen -> revert to windowed mode
                        window.create(sf::VideoMode({ 500, 500 }), "Level I");

                        window.setVerticalSyncEnabled(true);
                        window.setMouseCursorVisible(false);
                        window.setMouseCursorGrabbed(true);
                        updateView();
                    }
                    else
                    {
                        // Currently in windowed mode -> close the application
                        window.close();
                    }
                }
            }
        }
        //--- events ---

        float delta_time = 0.f; // <--- Declared here so it's accessible everywhere below!

        // If the window isn't focused, freeze updates and skip to the next frame
        if (window.hasFocus())
        {
            delta_time = clock.restart().asSeconds();

            frameCount++;
            fpsTimer += delta_time;

            if (fpsTimer >= 1.f)
            {
                fps = frameCount;
                frameCount = 0;
                fpsTimer = 0.f;
                ui.texts[ui.fpsTextIndex].setString("FPS: " + std::to_string(fps));
            }

            sf::Vector2f dir = input.GetMovement();
            sf::Vector2f start = player.playerBox.getPosition();
            sf::Vector2f clampedMouse = player.GetSmoothedAim(window, gameView, delta_time);
            sf::Vector2f aimDir = Math::NormalizeVector(clampedMouse - start);

            player.Update(dir, aimDir, delta_time, gameView, enemy, bulletSystem, map.md);
            player.reticuleSprite.setPosition(clampedMouse);

            bulletSystem.Update(delta_time, enemy, input);
            enemy.Update(delta_time, player.playerBox.getPosition());
            map.Update(delta_time);
        }
        else
        {
            clock.restart(); // Keeps the clock fresh so delta_time doesn't spike on return
        }

        //--- draw ---
        window.clear();

        // Center the camera on the player's current position so it tracks them across the map
        gameView.setCenter(player.playerBox.getPosition());

        window.setView(gameView);
        map.Draw(window);
        enemy.Draw(window);
        player.Draw(window);
        bulletSystem.Draw(window);
        window.setView(window.getDefaultView());

        ui.texts[ui.fpsTextIndex].setString("FPS: " + std::to_string(fps));
        ui.Draw(window);

        window.display();
    }
}