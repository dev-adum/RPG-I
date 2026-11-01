#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <cstdint>

// Thin wrapper around a Jolt physics world.
//
// Jolt's types stay inside the .cpp: the rest of the game only sees opaque
// body handles and glm maths. That keeps Jolt's headers and macros out of
// every translation unit that happens to need physics.
//
// Responsibilities are deliberately narrow: simulate bodies and hand back
// transforms. It knows nothing about dice, rules or rendering.
class PhysicsWorld
{
public:
    using BodyHandle = std::uint32_t;

    // Matches Jolt's own invalid ID. Zero is a *valid* handle (the first body
    // created has index 0 and sequence 0), so it must not be used as "none".
    static constexpr BodyHandle InvalidBody = 0xFFFFFFFFu;

    PhysicsWorld() = default;
    ~PhysicsWorld();

    PhysicsWorld(const PhysicsWorld&) = delete;
    PhysicsWorld& operator=(const PhysicsWorld&) = delete;

    // Registers Jolt and creates the world. Safe to call more than once.
    bool Initialize();
    void Shutdown();

    // Advances the simulation. Internally uses a fixed 1/60 s step with a
    // small substep budget, so a frame hitch cannot tunnel a body through the
    // floor.
    void Update(float dt);

    // halfExtent is half the box size; position is its centre.
    BodyHandle CreateBox(const glm::vec3& halfExtent, const glm::vec3& position,
                         bool dynamic);

    glm::vec3 GetPosition(BodyHandle body) const;
    glm::quat GetRotation(BodyHandle body) const;
    glm::vec3 GetLinearVelocity(BodyHandle body) const;

    void AddImpulse(BodyHandle body, const glm::vec3& impulse);
    void AddAngularImpulse(BodyHandle body, const glm::vec3& impulse);

    void SetPositionAndRotation(BodyHandle body, const glm::vec3& position,
                                const glm::quat& rotation);
    void SetLinearAndAngularVelocity(BodyHandle body, const glm::vec3& linear,
                                     const glm::vec3& angular);

    // Wakes a body that Jolt has put to sleep. Setting a velocity does not
    // wake it on its own, so anything that drives a resting body (the dice
    // magnet) has to call this first.
    void Activate(BodyHandle body);

    // False once Jolt has put the body to sleep, which is how we know a die
    // has come to rest.
    bool IsActive(BodyHandle body) const;

private:
    struct Impl;
    Impl* m_impl = nullptr;
};
