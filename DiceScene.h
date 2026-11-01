#pragma once
#include "Scene.h"
#include "GameContext.h"
#include "Mesh.h"
#include "PhysicsWorld.h"

#include <SFML/Graphics.hpp>

#include <string>

class SceneManager;

// Throwable-dice prototype.
//
// A Jolt world with a static floor and one dynamic die. The die is drawn with
// the fixed-function pipeline (immediate mode plus GL lighting) because the
// real VBO/shader renderer does not exist yet.
//
// The layers stay separate on purpose: PhysicsWorld knows nothing about dice
// or meshes, Mesh knows nothing about physics, and this scene is the only
// place where the two meet.
class DiceScene : public Scene
{
public:
    DiceScene(SceneManager& sceneManager, GameContext& context,
              const std::string& returnMap, sf::Vector2f returnPlayerPos);
    ~DiceScene() override;

    void HandleInput() override;
    void Update(float dt) override;
    void Render(sf::RenderWindow& window) override;

    // Esc leaves the dice test and returns to the map, so main must not quit.
    bool ConsumesEscape() const override { return true; }

private:
    void ResetDie();
    void ThrowDie();
    void UpdateCamera();
    void ApplyMagnetHold();
    void FireDie();
    void ReportFaceWhenSettled();
    void DrawScene(sf::RenderWindow& window);

    glm::vec3 CameraEye() const;
    glm::vec3 AimDirection() const;

    SceneManager& m_sceneManager;
    GameContext& m_context;

    // Where Esc takes us back to.
    std::string m_returnMap;
    sf::Vector2f m_returnPlayerPos;

    PhysicsWorld m_physics;
    Mesh m_dieMesh;

    PhysicsWorld::BodyHandle m_die = PhysicsWorld::InvalidBody;
    PhysicsWorld::BodyHandle m_floor = PhysicsWorld::InvalidBody;

    // Index-matched to the wall table in DiceScene.cpp.
    PhysicsWorld::BodyHandle m_walls[4] = {
        PhysicsWorld::InvalidBody, PhysicsWorld::InvalidBody,
        PhysicsWorld::InvalidBody, PhysicsWorld::InvalidBody };

    glm::vec3 m_dieStart{ 0.f, 3.f, 0.f };

    // Orbit camera. The eye rides a sphere around the arena centre and always
    // looks at the pivot, so AimDirection() is simply "toward the pivot".
    float m_cameraYaw = 0.f;
    float m_cameraPitch = 0.20f;

    // Cursor tracking for the orbit. Re-armed whenever the button is released
    // so the next grab cannot jump.
    sf::Vector2i m_lastMouse{ 0, 0 };
    bool m_firstMouseSample = true;

    // Uniform scale applied to the mesh so its bounds match the physics box.
    float m_meshScale = 1.f;

    bool m_settledReported = false;

    // True while the magnet grab is held, so a fresh grab can re-arm the
    // settled report.
    bool m_holding = false;
};
