#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include "Input.h"
#include "UIManager.h"
#include "GameContext.h"
#include "SceneManager.h"
#include "GameplayScene.h"
#include "Assets.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 500, 500 }), "Level I");
    window.setVerticalSyncEnabled(true);
    window.setMouseCursorVisible(false);
    window.setMouseCursorGrabbed(true);

    sf::View gameView;
    auto updateView = [&window, &gameView]()
        {
            const sf::Vector2u size = window.getSize();
            const float aspect = static_cast<float>(size.x) / static_cast<float>(size.y);
            constexpr float viewHeight = 10.f * 32.f;
            gameView.setSize({ viewHeight * aspect, viewHeight });
        };
    updateView();

    Input input;
    UIManager ui;
    sf::Music music;

    if (!music.openFromFile(AssetPaths::Path("Assets/Music/Void/VoidTrack.wav")))
        std::cout << "Failed to load VoidTrack.wav\n";
    music.setLooping(true);
    music.play();
    music.setVolume(20);

    ui.Load();
    ui.AddText("Aim reticule with the mouse\nMove: WASD\nFire: LMB", 11, { 10.f, 10.f });
    ui.AddText("FPS: 0", 14, { 200.f, 20.f });
    ui.fpsTextIndex = 1;

    GameContext context{ window, gameView, input, ui, music };

    SceneManager scenes;
    scenes.ChangeScene<GameplayScene>(scenes, context, AssetPaths::Path("Assets/Map/Westarch/WA_A1.tmj"));

    float fpsTimer = 0.f;
    int frameCount = 0;
    int fps = 0;
    sf::Clock clock;

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
                music.pause();
            if (event->is<sf::Event::FocusGained>())
                music.play();

            if (const auto* key = event->getIf<sf::Event::KeyPressed>())
            {
                if (key->code == sf::Keyboard::Key::F11)
                {
                    if (window.getSize() == sf::VideoMode::getDesktopMode().size)
                        window.create(sf::VideoMode({ 500, 500 }), "Level I");
                    else
                        window.create(sf::VideoMode::getDesktopMode(), "Level I",
                            sf::Style::Default, sf::State::Fullscreen);

                    window.setVerticalSyncEnabled(true);
                    window.setMouseCursorVisible(false);
                    window.setMouseCursorGrabbed(true);
                    updateView();
                }
                else if (key->code == sf::Keyboard::Key::Escape)
                {
                    if (window.getSize() == sf::VideoMode::getDesktopMode().size)
                    {
                        window.create(sf::VideoMode({ 500, 500 }), "Level I");
                        window.setVerticalSyncEnabled(true);
                        window.setMouseCursorVisible(false);
                        window.setMouseCursorGrabbed(true);
                        updateView();
                    }
                    else
                    {
                        window.close();
                    }
                }
            }
        }

        //--- update ---
        float dt = 0.f;
        if (window.hasFocus())
        {
            dt = clock.restart().asSeconds();
            frameCount++;
            fpsTimer += dt;
            if (fpsTimer >= 1.f)
            {
                fps = frameCount;
                frameCount = 0;
                fpsTimer = 0.f;
            }
            scenes.HandleInput();
            scenes.Update(dt);
        }
        else
        {
            clock.restart();
        }

        //--- draw ---
        window.clear();
        scenes.Render(window);
        window.setView(window.getDefaultView());
        ui.texts[ui.fpsTextIndex].setString("FPS: " + std::to_string(fps));
        ui.Draw(window);
        window.display();
    }
}