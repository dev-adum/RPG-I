#include "DiceScene.h"

#include "GameplayScene.h"
#include "SceneManager.h"
#include "Assets.h"

#include <SFML/OpenGL.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <random>

namespace
{
    // Size of the die in metres, across opposite faces. The physics box and
    // the rendered mesh are both derived from this, so they cannot drift.
    constexpr float kDieSize = 1.f;

    constexpr float kFloorHalfExtent = 10.f;
    constexpr float kFloorHalfThickness = 0.5f;

    // Walls: a four-sided arena so a fired die ricochets instead of leaving.
    constexpr float kWallHeight = 4.f;
    constexpr float kWallHalfThickness = 0.25f;

    const float kWallOffset = kFloorHalfExtent + kWallHalfThickness;
    const float kWallHalfHeight = kWallHeight * 0.5f;

    struct WallDef
    {
        glm::vec3 centre;
        glm::vec3 halfExtent;
    };

    // Inner faces sit exactly on the floor edge, so the arena is seamless.
    const WallDef kWalls[4] = {
        { {  kWallOffset, kWallHalfHeight, 0.f },
          { kWallHalfThickness, kWallHalfHeight, kWallOffset } },
        { { -kWallOffset, kWallHalfHeight, 0.f },
          { kWallHalfThickness, kWallHalfHeight, kWallOffset } },
        { { 0.f, kWallHalfHeight,  kWallOffset },
          { kWallOffset, kWallHalfHeight, kWallHalfThickness } },
        { { 0.f, kWallHalfHeight, -kWallOffset },
          { kWallOffset, kWallHalfHeight, kWallHalfThickness } },
    };

    // Orbit camera. The eye rides a sphere around the arena centre and always
    // looks at the pivot, so the aim direction is simply "toward the pivot"
    // and the whole arena stays framed.
    const glm::vec3 kCameraPivot(0.f, 1.f, 0.f);
    constexpr float kCameraRadius = 7.f;
    constexpr float kCameraMinPitch = 0.05f;
    constexpr float kCameraMaxPitch = 1.30f;
    constexpr float kOrbitSensitivity = 0.005f;   // radians per pixel

    // Magnet grab tuning. Stiffness is how hard the die is pulled toward the
    // hold point (1/s); the speed clamp keeps a far-away die from snapping.
    constexpr float kMagnetStiffness = 14.f;
    constexpr float kMagnetMaxSpeed = 22.f;
    constexpr float kMagnetHoldDistance = 3.2f;

    // Release speed, cannon-style.
    constexpr float kLaunchSpeed = 28.f;
}

DiceScene::DiceScene(SceneManager& sceneManager, GameContext& context,
                     const std::string& returnMap, sf::Vector2f returnPlayerPos)
    : m_sceneManager(sceneManager)
    , m_context(context)
    , m_returnMap(returnMap)
    , m_returnPlayerPos(returnPlayerPos)
{
    m_physics.Initialize();

    if (m_dieMesh.LoadOBJ(AssetPaths::Path("Assets/Dice/DICE_0.obj")))
    {
        // The .obj is 2 units across; scale it to whatever kDieSize asks for.
        const glm::vec3 meshSize = m_dieMesh.Max() - m_dieMesh.Min();
        const float largestSide =
            std::max(meshSize.x, std::max(meshSize.y, meshSize.z));
        if (largestSide > 0.f)
            m_meshScale = kDieSize / largestSide;
    }
    else
    {
        std::cout << "DiceScene: die mesh missing, physics still runs\n";
    }

    const float half = kDieSize * 0.5f;

    // Floor: top surface sits exactly at y = 0.
    m_floor = m_physics.CreateBox(
        { kFloorHalfExtent, kFloorHalfThickness, kFloorHalfExtent },
        { 0.f, -kFloorHalfThickness, 0.f },
        false);

    for (int i = 0; i < 4; ++i)
        m_walls[i] = m_physics.CreateBox(
            kWalls[i].halfExtent, kWalls[i].centre, false);

    m_die = m_physics.CreateBox({ half, half, half }, m_dieStart, true);

    ResetDie();

    std::cout << "DiceScene: Space/LMB throw, hold E/RMB magnet, MMB orbit, "
                 "release to fire, R reset, Esc back\n";
}

DiceScene::~DiceScene()
{
    m_physics.Shutdown();
}

void DiceScene::ResetDie()
{
    if (m_die == PhysicsWorld::InvalidBody)
        return;

    m_physics.SetPositionAndRotation(
        m_die, m_dieStart, glm::quat(1.f, 0.f, 0.f, 0.f));
    m_physics.SetLinearAndAngularVelocity(
        m_die, glm::vec3(0.f), glm::vec3(0.f));

    m_settledReported = false;
}

void DiceScene::ThrowDie()
{
    if (m_die == PhysicsWorld::InvalidBody)
        return;

    ResetDie();

    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> sideways(-1.f, 1.f);
    std::uniform_real_distribution<float> spin(-6.f, 6.f);

    // Push it toward the far side of the table with some lift, so it tumbles
    // before landing instead of just dropping.
    const glm::vec3 impulse(1.2f * sideways(rng), 3.5f, -2.5f);
    m_physics.AddImpulse(m_die, impulse);
    m_physics.AddAngularImpulse(
        m_die, glm::vec3(spin(rng), spin(rng), spin(rng)));
}

void DiceScene::ReportFaceWhenSettled()
{
    if (m_die == PhysicsWorld::InvalidBody || m_settledReported)
        return;

    // Jolt deactivates bodies that have come to rest; that is our "settled"
    // signal, so there are no velocity thresholds of our own to tune.
    if (m_physics.IsActive(m_die))
        return;

    const glm::quat rotation = m_physics.GetRotation(m_die);

    // The six local face normals, rotated into world space. Whichever points
    // most straight up is the face we are looking at.
    const glm::vec3 localAxes[6] = {
        {  0.f,  1.f,  0.f },   // +Y
        {  0.f, -1.f,  0.f },   // -Y
        {  1.f,  0.f,  0.f },   // +X
        { -1.f,  0.f,  0.f },   // -X
        {  0.f,  0.f,  1.f },   // +Z
        {  0.f,  0.f, -1.f },   // -Z
    };

    // Opposite faces of a real die sum to seven.
    const int faceValue[6] = { 1, 6, 2, 5, 3, 4 };

    int best = 0;
    float bestUp = -2.f;
    for (int i = 0; i < 6; ++i)
    {
        const float up = (rotation * localAxes[i]).y;
        if (up > bestUp)
        {
            bestUp = up;
            best = i;
        }
    }

    std::cout << "Dice settled. Face up: " << faceValue[best]
              << " (up component " << bestUp << ")\n";

    m_settledReported = true;
}

void DiceScene::HandleInput()
{
    if (m_context.input.WasEscapePressed())
    {
        SceneManager& sm = m_sceneManager;
        GameContext& ctx = m_context;
        const std::string map = m_returnMap;
        const sf::Vector2f pos = m_returnPlayerPos;

        m_sceneManager.RequestScene([&sm, &ctx, map, pos]()
        {
            return std::make_unique<GameplayScene>(sm, ctx, map, pos);
        });
        return;
    }

    if (m_context.input.IsFiring())
        ThrowDie();

    if (m_context.input.WasResetPressed())
        ResetDie();
}

glm::vec3 DiceScene::CameraEye() const
{
    const float cp = std::cos(m_cameraPitch);
    return kCameraPivot + kCameraRadius * glm::vec3(
        cp * std::sin(m_cameraYaw),
        std::sin(m_cameraPitch),
        cp * std::cos(m_cameraYaw));
}

glm::vec3 DiceScene::AimDirection() const
{
    return glm::normalize(kCameraPivot - CameraEye());
}

void DiceScene::UpdateCamera()
{
    if (!m_context.input.IsCameraOrbiting())
    {
        // Re-arm, so the next grab starts from wherever the cursor is now
        // instead of jumping by the whole distance it moved since.
        m_firstMouseSample = true;
        return;
    }

    const sf::Vector2i mouse = sf::Mouse::getPosition(m_context.window);

    if (m_firstMouseSample)
    {
        m_lastMouse = mouse;
        m_firstMouseSample = false;
        return;
    }

    const sf::Vector2i delta = mouse - m_lastMouse;

    // Keep the cursor near the middle. The window is small, so a grabbed
    // cursor would otherwise hit an edge and the orbit would stop turning.
    const sf::Vector2u size = m_context.window.getSize();
    if (size.x > 0 && size.y > 0)
    {
        const sf::Vector2i centre(
            static_cast<int>(size.x) / 2, static_cast<int>(size.y) / 2);
        sf::Mouse::setPosition(centre, m_context.window);
        m_lastMouse = centre;
    }

    // Drag right to aim right, drag down to aim down.
    m_cameraYaw -= static_cast<float>(delta.x) * kOrbitSensitivity;
    m_cameraPitch += static_cast<float>(delta.y) * kOrbitSensitivity;
    m_cameraPitch = std::clamp(m_cameraPitch, kCameraMinPitch, kCameraMaxPitch);
}

void DiceScene::ApplyMagnetHold()
{
    if (m_die == PhysicsWorld::InvalidBody)
        return;

    const bool holding = m_context.input.IsHoldingDice();

    // A new grab re-arms the settled report, so the next landing reports a
    // fresh face.
    if (holding && !m_holding)
        m_settledReported = false;

    // Letting go fires the die along the aim, cannon-style, rather than
    // dropping it.
    if (!holding && m_holding)
        FireDie();

    m_holding = holding;

    if (!holding)
        return;

    // Hold point: a fixed distance along the aim, so the die floats between
    // the camera and the arena and visibly tracks where it will be sent.
    const glm::vec3 holdPoint =
        CameraEye() + AimDirection() * kMagnetHoldDistance;

    glm::vec3 velocity =
        (holdPoint - m_physics.GetPosition(m_die)) * kMagnetStiffness;

    const float speed = glm::length(velocity);
    if (speed > kMagnetMaxSpeed)
        velocity *= kMagnetMaxSpeed / speed;

    // Setting a velocity does not wake a sleeping body, and the die is
    // usually asleep on the floor when it gets grabbed.
    m_physics.Activate(m_die);
    m_physics.SetLinearAndAngularVelocity(m_die, velocity, glm::vec3(0.f));
}

void DiceScene::FireDie()
{
    if (m_die == PhysicsWorld::InvalidBody)
        return;

    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> spin(-8.f, 8.f);

    m_physics.Activate(m_die);
    m_physics.SetLinearAndAngularVelocity(
        m_die,
        AimDirection() * kLaunchSpeed,
        glm::vec3(spin(rng), spin(rng), spin(rng)));
}

void DiceScene::Update(float dt)
{
    // Camera first, then the magnet, so the die is held along the aim the
    // player is looking down this frame.
    UpdateCamera();
    ApplyMagnetHold();

    m_physics.Update(dt);
    ReportFaceWhenSettled();
}

void DiceScene::Render(sf::RenderWindow& window)
{
    (void)window.setActive(true);
    DrawScene(window);
    window.resetGLStates();
}

void DiceScene::DrawScene(sf::RenderWindow& window)
{
    const sf::Vector2u size = window.getSize();
    if (size.x == 0 || size.y == 0)
        return;

    const float aspect =
        static_cast<float>(size.x) / static_cast<float>(size.y);

    glViewport(0, 0, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));

    // main() cleared the colour buffer; the depth buffer is ours to clear
    // because SFML does not touch it.
    glClear(GL_DEPTH_BUFFER_BIT);

    // --- projection -------------------------------------------------------
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    constexpr float fovY = 55.f * 3.14159265358979f / 180.f;
    constexpr float zNear = 0.1f;
    constexpr float zFar = 100.f;
    const float f = 1.f / std::tan(fovY * 0.5f);

    // Standard perspective matrix, column-major as glLoadMatrixf expects.
    const GLfloat projection[16] = {
        f / aspect, 0.f,  0.f,  0.f,
        0.f,        f,    0.f,  0.f,
        0.f,        0.f,  (zFar + zNear) / (zNear - zFar), -1.f,
        0.f,        0.f,  (2.f * zFar * zNear) / (zNear - zFar), 0.f,
    };
    glLoadMatrixf(projection);

    // --- camera -----------------------------------------------------------
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    const glm::mat4 view = glm::lookAt(
        CameraEye(),
        kCameraPivot,
        glm::vec3(0.f, 1.f, 0.f));     // up
    glLoadMatrixf(glm::value_ptr(view));

    // --- lighting ---------------------------------------------------------
    // GL_POSITION is transformed by the model-view matrix that is current
    // when it is set, so this must come AFTER the camera and BEFORE any model
    // transforms. w = 0 makes it directional.
    const GLfloat lightDirection[4] = { 0.45f, 0.8f, 0.5f, 0.f };
    const GLfloat lightDiffuse[4] = { 1.f, 0.97f, 0.9f, 1.f };
    const GLfloat lightSpecular[4] = { 1.f, 1.f, 1.f, 1.f };
    const GLfloat ambient[4] = { 0.22f, 0.23f, 0.28f, 1.f };

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);   // convex die: depth test hides the far side
    glDisable(GL_BLEND);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(GL_SMOOTH);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_POSITION, lightDirection);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    const GLfloat materialSpecular[4] = { 0.3f, 0.3f, 0.3f, 1.f };
    const GLfloat materialShininess[1] = { 48.f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);

    // --- floor ------------------------------------------------------------
    glColor3f(0.16f, 0.18f, 0.22f);
    glBegin(GL_QUADS);
    glNormal3f(0.f, 1.f, 0.f);
    glVertex3f(-kFloorHalfExtent, 0.f, -kFloorHalfExtent);
    glVertex3f( kFloorHalfExtent, 0.f, -kFloorHalfExtent);
    glVertex3f( kFloorHalfExtent, 0.f,  kFloorHalfExtent);
    glVertex3f(-kFloorHalfExtent, 0.f,  kFloorHalfExtent);
    glEnd();

    // A grid gives the eye something to judge the die's motion against.
    glDisable(GL_LIGHTING);
    glColor3f(0.28f, 0.31f, 0.36f);
    glBegin(GL_LINES);
    for (int i = -10; i <= 10; ++i)
    {
        const float line = static_cast<float>(i);
        glVertex3f(line, 0.002f, -kFloorHalfExtent);
        glVertex3f(line, 0.002f,  kFloorHalfExtent);
        glVertex3f(-kFloorHalfExtent, 0.002f, line);
        glVertex3f( kFloorHalfExtent, 0.002f, line);
    }
    glEnd();
    glEnable(GL_LIGHTING);

    // --- walls ------------------------------------------------------------
    // Only the inward faces are drawn; the camera always sits inside the
    // arena, so the outside is never visible.
    const float wallX = kFloorHalfExtent;
    const float wallY = kWallHeight;

    glColor3f(0.24f, 0.26f, 0.31f);
    glBegin(GL_QUADS);

    // +X wall, facing -X
    glNormal3f(-1.f, 0.f, 0.f);
    glVertex3f( wallX, 0.f,  wallX);
    glVertex3f( wallX, 0.f, -wallX);
    glVertex3f( wallX, wallY, -wallX);
    glVertex3f( wallX, wallY,  wallX);

    // -X wall, facing +X
    glNormal3f(1.f, 0.f, 0.f);
    glVertex3f(-wallX, 0.f, -wallX);
    glVertex3f(-wallX, 0.f,  wallX);
    glVertex3f(-wallX, wallY,  wallX);
    glVertex3f(-wallX, wallY, -wallX);

    // +Z wall, facing -Z
    glNormal3f(0.f, 0.f, -1.f);
    glVertex3f(-wallX, 0.f, wallX);
    glVertex3f( wallX, 0.f, wallX);
    glVertex3f( wallX, wallY, wallX);
    glVertex3f(-wallX, wallY, wallX);

    // -Z wall, facing +Z
    glNormal3f(0.f, 0.f, 1.f);
    glVertex3f( wallX, 0.f, -wallX);
    glVertex3f(-wallX, 0.f, -wallX);
    glVertex3f(-wallX, wallY, -wallX);
    glVertex3f( wallX, wallY, -wallX);

    glEnd();

    // --- die --------------------------------------------------------------
    if (m_dieMesh.IsLoaded() && m_die != PhysicsWorld::InvalidBody)
    {
        const glm::vec3 position = m_physics.GetPosition(m_die);
        const glm::quat rotation = m_physics.GetRotation(m_die);

        const glm::mat4 model =
            glm::translate(glm::mat4(1.f), position) *
            glm::mat4_cast(rotation) *
            glm::scale(glm::mat4(1.f), glm::vec3(m_meshScale));

        glPushMatrix();
        glMultMatrixf(glm::value_ptr(model));
        glColor3f(0.88f, 0.86f, 0.80f);
        m_dieMesh.Draw();
        glPopMatrix();
    }

    // --- hand the context back to SFML in a sane state --------------------
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_NORMALIZE);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
