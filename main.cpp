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
    // The 3D dice scene needs a depth buffer, and SFML's defaults request
    // none (depthBits is 0). The settings also have to be passed to every
    // create() call, because recreating the window for fullscreen would
    // otherwise silently drop the depth buffer again.
    sf::ContextSettings windowSettings;
    windowSettings.depthBits = 24;
    windowSettings.stencilBits = 8;
    windowSettings.antiAliasingLevel = 4;

    sf::RenderWindow window(sf::VideoMode({ 500, 500 }), "Level I",
        sf::Style::Default, sf::State::Windowed, windowSettings);
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
    ui.AddText(
        "Move: WASD\nFire: LMB or 'Space'\nRun: WASD+Shift\nMenu: 'M'\nFullscreen: 'F11'/'Esc'\nDice test: 'F'\nMagnet: hold 'E'\nOrbit: hold MMB", 11, { 10.f, 10.f });
    ui.AddText("FPS: 0", 18, { 200.f, 20.f });
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
        // Clear this frame's discrete presses BEFORE polling, so
        // HandleEvent() can set them during the poll below.
        input.BeginFrame();

        //--- events ---
        // Poll only: record input here, but do NOT act on discrete presses
        // inside this loop. An action like window.create() can queue further
        // events, and the flag stays set for the whole frame - checking it
        // per-event would fire it many times per press.
        while (const std::optional event = window.pollEvent())
        {
            input.HandleEvent(*event);

            if (event->is<sf::Event::Closed>())
                window.close();

            if (event->is<sf::Event::Resized>())
                updateView();

            if (event->is<sf::Event::FocusLost>())
                music.pause();
            if (event->is<sf::Event::FocusGained>())
                music.play();
        }

        //--- discrete actions: once per frame, after polling ---
        if (input.WasFullscreenPressed())
        {
            if (window.getSize() == sf::VideoMode::getDesktopMode().size)
                window.create(sf::VideoMode({ 500, 500 }), "Level I",
                    sf::Style::Default, sf::State::Windowed, windowSettings);
            else
                window.create(sf::VideoMode::getDesktopMode(), "Level I",
                    sf::Style::Default, sf::State::Fullscreen, windowSettings);

            window.setVerticalSyncEnabled(true);
            window.setMouseCursorVisible(false);
            window.setMouseCursorGrabbed(true);
            updateView();
        }
        else if (input.WasEscapePressed() && !scenes.CurrentSceneConsumesEscape())
        {
            if (window.getSize() == sf::VideoMode::getDesktopMode().size)
            {
                window.create(sf::VideoMode({ 500, 500 }), "Level I",
                    sf::Style::Default, sf::State::Windowed, windowSettings);
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
        scenes.RenderFade(window);
        window.setView(window.getDefaultView());
        ui.texts[ui.fpsTextIndex].setString("FPS: " + std::to_string(fps));
        ui.Draw(window);
        window.display();
    }
}