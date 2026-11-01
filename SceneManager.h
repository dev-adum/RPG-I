#pragma once
#include "Scene.h"
#include <memory>
#include <utility>
#include <functional>
#include <algorithm>
#include <cstdint>

class SceneManager {
private:
    std::unique_ptr<Scene> m_currentScene;
    std::function<std::unique_ptr<Scene>()> m_pendingScene;

    // --- Transition fade ---
    // A full-screen black overlay is drawn on top of the scene. When a scene
    // change is requested we fade OUT to black, swap the scene at full black,
    // then fade IN. Scenes never see this: they just call RequestScene().
    enum class FadeState { Idle, Out, In };

    FadeState m_fadeState = FadeState::Idle;
    float m_fadeT = 0.f;              // 0 = clear, 1 = fully black
    float m_fadeDuration = 0.48f;     // seconds per half (cinematic, +25% pace)
    sf::RectangleShape m_fadeOverlay; // black, screen-space
    sf::RectangleShape m_fadeSolid;   // flat black under the vignette

    // --- Radial vignette (Phase 2) ---
    // A soft radial mask darkens from the edges inward as the fade progresses.
    // Drawn ON TOP of the flat black rect, so at t=1 the screen is fully black
    // everywhere (the flat rect guarantees it) while the edges lead the way in.
    // If the shader fails to compile we simply keep the flat fade.
    sf::Shader m_vignetteShader;
    bool  m_vignetteReady = false;
    float m_vignetteInner = 0.15f;    // where darkening begins (0 = centre)
    float m_vignetteOuter = 1.0f;     // where it is fully dark (corner)

    static const char* VignetteFragmentSource() {
        return
            "uniform vec2  u_resolution;\n"
            "uniform float u_progress;\n"
            "uniform float u_inner;\n"
            "uniform float u_outer;\n"
            "void main()\n"
            "{\n"
            "    vec2 uv = gl_FragCoord.xy / u_resolution;\n"
            "    vec2 p  = (uv - 0.5) * 2.0;\n"
            "    p.x *= u_resolution.x / u_resolution.y;\n"
            "    float dist = length(p) / length(vec2(u_resolution.x / u_resolution.y, 1.0));\n"
            "    float mask = smoothstep(u_inner, u_outer, dist);\n"
            "    gl_FragColor = vec4(0.0, 0.0, 0.0, mask * u_progress);\n"
            "}\n";
    }

public:
    SceneManager() {
        m_fadeOverlay.setFillColor(sf::Color::Black);
        m_fadeOverlay.setPosition({ 0.f, 0.f });
        m_fadeSolid.setFillColor(sf::Color::Black);
        m_fadeSolid.setPosition({ 0.f, 0.f });

        m_vignetteReady = m_vignetteShader.loadFromMemory(
            VignetteFragmentSource(), sf::Shader::Type::Fragment);
    }

    // Changes the active scene, forwarding any constructor arguments directly to it
    template <typename T, typename... Args>
    void ChangeScene(Args&&... args) {
        m_currentScene = std::make_unique<T>(std::forward<Args>(args)...);
    }

    // Queue a scene switch. The switch itself happens once the fade-out has
    // reached full black, so the scene that requested it is never destroyed
    // mid-method and the swap is never visible.
    void RequestScene(std::function<std::unique_ptr<Scene>()> factory) {
        m_pendingScene = std::move(factory);
    }

    void Update(float dt) {
        // A request starts the fade-out (only from Idle, so re-requests during
        // a transition can't restart or fight it).
        if (m_pendingScene && m_fadeState == FadeState::Idle) {
            m_fadeState = FadeState::Out;
            m_fadeT = 0.f;
        }

        if (m_fadeState == FadeState::Out) {
            m_fadeT += dt / m_fadeDuration;
            if (m_fadeT >= 1.f) {
                m_fadeT = 1.f;

                // Screen is black: swap now, so the change is invisible.
                if (m_pendingScene) {
                    m_currentScene = m_pendingScene();
                    m_pendingScene = nullptr;
                }
                m_fadeState = FadeState::In;
            }
        }
        else if (m_fadeState == FadeState::In) {
            m_fadeT -= dt / m_fadeDuration;
            if (m_fadeT <= 0.f) {
                m_fadeT = 0.f;
                m_fadeState = FadeState::Idle;
            }
        }

        if (m_currentScene) {
            m_currentScene->Update(dt);
        }
    }

    void Render(sf::RenderWindow& window) {
        if (m_currentScene) {
            m_currentScene->Render(window);
        }
    }

    // Draw the fade overlay in SCREEN space (not the gameplay view), so the
    // black never scrolls with the camera. Call after Render().
    void RenderFade(sf::RenderWindow& window) {
        if (m_fadeState == FadeState::Idle && m_fadeT <= 0.f)
            return;

        window.setView(window.getDefaultView());

        const sf::Vector2u size = window.getSize();
        const sf::Vector2f sizeF(static_cast<float>(size.x), static_cast<float>(size.y));
        m_fadeOverlay.setSize(sizeF);
        m_fadeSolid.setSize(sizeF);

        const float t = std::clamp(m_fadeT, 0.f, 1.f);

        // 1. Flat black underneath: guarantees the screen is fully black at t=1,
        //    so the scene swap is never visible through the vignette's centre.
        const auto solidAlpha = static_cast<std::uint8_t>(t * 255.f);
        m_fadeSolid.setFillColor(sf::Color(0, 0, 0, solidAlpha));
        window.draw(m_fadeSolid);

        // 2. Radial vignette on top: the cinematic part. Edges darken first,
        //    the centre holds longest. Falls back to nothing if it didn't compile.
        if (m_vignetteReady) {
            m_vignetteShader.setUniform("u_resolution", sf::Glsl::Vec2(sizeF));
            m_vignetteShader.setUniform("u_progress", t);
            m_vignetteShader.setUniform("u_inner", m_vignetteInner);
            m_vignetteShader.setUniform("u_outer", m_vignetteOuter);

            m_fadeOverlay.setFillColor(sf::Color(0, 0, 0, 255));
            window.draw(m_fadeOverlay, sf::RenderStates(&m_vignetteShader));
        }
    }

    void HandleInput() {
        if (m_currentScene)
            m_currentScene->HandleInput();
    }

    // True if the active scene wants to handle Esc itself (e.g. the menu),
    // so main should not treat Esc as "quit".
    bool CurrentSceneConsumesEscape() const {
        return m_currentScene && m_currentScene->ConsumesEscape();
    }

};