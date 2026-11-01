#include "PhysicsWorld.h"

// Jolt's CMake exports these as PUBLIC defines because client code has to be
// compiled with exactly the same feature set as the library. RegisterTypes()
// builds a version ID out of them and aborts on a mismatch, and several of
// them change class layout (PhysicsSystem) or calling convention (Vec4), so a
// silent difference would corrupt memory. Keep in sync with build_jolt.ps1.
#define JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
#define JPH_DEBUG_RENDERER
#define JPH_PROFILE_ENABLED
#define JPH_OBJECT_STREAM
#define JPH_USE_DX12
#define JPH_USE_CPU_COMPUTE
#define JPH_USE_AVX2
#define JPH_USE_AVX
#define JPH_USE_SSE4_1
#define JPH_USE_SSE4_2
#define JPH_USE_LZCNT
#define JPH_USE_TZCNT
#define JPH_USE_F16C
#define JPH_USE_FMADD
#ifdef _DEBUG
#define JPH_DEBUG
#endif

#include <Jolt/Jolt.h>

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>

#include <algorithm>
#include <iostream>

namespace
{
    // --- collision layers -------------------------------------------------
    // Two is all this needs: things that never move (the floor) and things
    // that do (the die). Keeping them separate lets the broadphase skip
    // static/static pairs.

    namespace Layers
    {
        constexpr JPH::ObjectLayer NON_MOVING = 0;
        constexpr JPH::ObjectLayer MOVING = 1;
        constexpr JPH::ObjectLayer COUNT = 2;
    }

    namespace BroadPhaseLayers
    {
        constexpr JPH::BroadPhaseLayer NON_MOVING(0);
        constexpr JPH::BroadPhaseLayer MOVING(1);
        constexpr JPH::uint COUNT = 2;
    }

    class BroadPhaseLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
    {
    public:
        JPH::uint GetNumBroadPhaseLayers() const override
        {
            return BroadPhaseLayers::COUNT;
        }

        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
        {
            JPH_ASSERT(inLayer < Layers::COUNT);
            return inLayer == Layers::NON_MOVING
                ? BroadPhaseLayers::NON_MOVING
                : BroadPhaseLayers::MOVING;
        }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
        {
            switch (inLayer.GetValue())
            {
            case 0:  return "NON_MOVING";
            case 1:  return "MOVING";
            default: return "INVALID";
            }
        }
#endif
    };

    class ObjectVsBroadPhaseLayerFilterImpl final
        : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer inLayer1,
                           JPH::BroadPhaseLayer inLayer2) const override
        {
            switch (inLayer1)
            {
            case Layers::NON_MOVING:
                return inLayer2 == BroadPhaseLayers::MOVING;
            case Layers::MOVING:
                return true;
            default:
                return false;
            }
        }
    };

    class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer inObject1,
                           JPH::ObjectLayer inObject2) const override
        {
            switch (inObject1)
            {
            case Layers::NON_MOVING:
                return inObject2 == Layers::MOVING;
            case Layers::MOVING:
                return inObject2 == Layers::MOVING ||
                       inObject2 == Layers::NON_MOVING;
            default:
                return false;
            }
        }
    };

    inline JPH::Vec3 ToJolt(const glm::vec3& v)
    {
        return JPH::Vec3(v.x, v.y, v.z);
    }

    inline glm::vec3 FromJolt(const JPH::RVec3& v)
    {
        return glm::vec3(static_cast<float>(v.GetX()),
                         static_cast<float>(v.GetY()),
                         static_cast<float>(v.GetZ()));
    }
}

// Everything Jolt lives here so the header stays free of it.
struct PhysicsWorld::Impl
{
    JPH::PhysicsSystem physicsSystem;

    BroadPhaseLayerInterfaceImpl broadPhaseLayerInterface;
    ObjectVsBroadPhaseLayerFilterImpl objectVsBroadPhaseLayerFilter;
    ObjectLayerPairFilterImpl objectLayerPairFilter;

    JPH::TempAllocatorImpl* tempAllocator = nullptr;
    JPH::JobSystemThreadPool* jobSystem = nullptr;

    float accumulator = 0.f;
};

PhysicsWorld::~PhysicsWorld()
{
    Shutdown();
}

bool PhysicsWorld::Initialize()
{
    if (m_impl)
        return true;

    Impl* impl = new Impl();

    // Global Jolt setup. RegisterTypes()/UnregisterTypes() are refcounted, so
    // repeated scene entry is fine as long as they stay paired.
    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();

    impl->tempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
    impl->jobSystem = new JPH::JobSystemThreadPool(
        JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, -1);

    constexpr JPH::uint maxBodies = 1024;
    constexpr JPH::uint numBodyMutexes = 0;   // 0 = pick automatically
    constexpr JPH::uint maxBodyPairs = 1024;
    constexpr JPH::uint maxContactConstraints = 1024;

    impl->physicsSystem.Init(
        maxBodies, numBodyMutexes, maxBodyPairs, maxContactConstraints,
        impl->broadPhaseLayerInterface,
        impl->objectVsBroadPhaseLayerFilter,
        impl->objectLayerPairFilter);

    m_impl = impl;
    return true;
}

void PhysicsWorld::Shutdown()
{
    if (!m_impl)
        return;

    delete m_impl->jobSystem;
    delete m_impl->tempAllocator;
    m_impl->jobSystem = nullptr;
    m_impl->tempAllocator = nullptr;

    // Destroy the world (and every body in it) before unregistering the Jolt
    // types those objects were built from.
    delete m_impl;
    m_impl = nullptr;

    JPH::UnregisterTypes();
    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}

void PhysicsWorld::Update(float dt)
{
    if (!m_impl)
        return;

    constexpr float fixedStep = 1.f / 60.f;
    constexpr int maxSubSteps = 4;

    // Drop absurd frame times instead of trying to catch up forever.
    m_impl->accumulator += std::clamp(dt, 0.f, 0.25f);

    int steps = 0;
    while (m_impl->accumulator >= fixedStep && steps < maxSubSteps)
    {
        m_impl->physicsSystem.Update(
            fixedStep, 1, m_impl->tempAllocator, m_impl->jobSystem);
        m_impl->accumulator -= fixedStep;
        ++steps;
    }

    // Still behind after the budget: throw the backlog away rather than
    // spiralling.
    if (steps == maxSubSteps)
        m_impl->accumulator = 0.f;
}

PhysicsWorld::BodyHandle PhysicsWorld::CreateBox(const glm::vec3& halfExtent,
                                                const glm::vec3& position,
                                                bool dynamic)
{
    if (!m_impl)
        return InvalidBody;

    JPH::BoxShapeSettings shapeSettings(ToJolt(halfExtent));
    JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
    if (shapeResult.HasError())
    {
        std::cerr << "PhysicsWorld: box shape failed: "
                  << shapeResult.GetError() << '\n';
        return InvalidBody;
    }

    JPH::BodyCreationSettings bodySettings(
        shapeResult.Get().GetPtr(),
        JPH::RVec3(position.x, position.y, position.z),
        JPH::Quat::sIdentity(),
        dynamic ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static,
        dynamic ? Layers::MOVING : Layers::NON_MOVING);

    if (dynamic)
    {
        // A die should land with a thud and settle, not bounce around the
        // room forever.
        bodySettings.mFriction = 0.45f;
        bodySettings.mRestitution = 0.15f;
        bodySettings.mLinearDamping = 0.05f;
        bodySettings.mAngularDamping = 0.1f;
    }

    JPH::BodyInterface& bodies = m_impl->physicsSystem.GetBodyInterface();
    const JPH::BodyID id =
        bodies.CreateAndAddBody(bodySettings, JPH::EActivation::Activate);

    return id.GetIndexAndSequenceNumber();
}

glm::vec3 PhysicsWorld::GetPosition(BodyHandle body) const
{
    if (!m_impl || body == InvalidBody)
        return glm::vec3(0.f);

    JPH::RVec3 position;
    JPH::Quat rotation;
    m_impl->physicsSystem.GetBodyInterface().GetPositionAndRotation(
        JPH::BodyID(body), position, rotation);

    return FromJolt(position);
}

glm::quat PhysicsWorld::GetRotation(BodyHandle body) const
{
    if (!m_impl || body == InvalidBody)
        return glm::quat(1.f, 0.f, 0.f, 0.f);

    JPH::RVec3 position;
    JPH::Quat rotation;
    m_impl->physicsSystem.GetBodyInterface().GetPositionAndRotation(
        JPH::BodyID(body), position, rotation);

    const JPH::Vec4 xyzw = rotation.GetXYZW();
    return glm::quat(xyzw.GetW(), xyzw.GetX(), xyzw.GetY(), xyzw.GetZ());
}

glm::vec3 PhysicsWorld::GetLinearVelocity(BodyHandle body) const
{
    if (!m_impl || body == InvalidBody)
        return glm::vec3(0.f);

    const JPH::Vec3 v =
        m_impl->physicsSystem.GetBodyInterface().GetLinearVelocity(JPH::BodyID(body));
    return glm::vec3(v.GetX(), v.GetY(), v.GetZ());
}

void PhysicsWorld::AddImpulse(BodyHandle body, const glm::vec3& impulse)
{
    if (!m_impl || body == InvalidBody)
        return;

    m_impl->physicsSystem.GetBodyInterface().AddImpulse(
        JPH::BodyID(body), ToJolt(impulse));
}

void PhysicsWorld::AddAngularImpulse(BodyHandle body, const glm::vec3& impulse)
{
    if (!m_impl || body == InvalidBody)
        return;

    m_impl->physicsSystem.GetBodyInterface().AddAngularImpulse(
        JPH::BodyID(body), ToJolt(impulse));
}

void PhysicsWorld::SetPositionAndRotation(BodyHandle body,
                                          const glm::vec3& position,
                                          const glm::quat& rotation)
{
    if (!m_impl || body == InvalidBody)
        return;

    m_impl->physicsSystem.GetBodyInterface().SetPositionAndRotation(
        JPH::BodyID(body),
        JPH::RVec3(position.x, position.y, position.z),
        JPH::Quat(rotation.x, rotation.y, rotation.z, rotation.w),
        JPH::EActivation::Activate);
}

void PhysicsWorld::SetLinearAndAngularVelocity(BodyHandle body,
                                               const glm::vec3& linear,
                                               const glm::vec3& angular)
{
    if (!m_impl || body == InvalidBody)
        return;

    m_impl->physicsSystem.GetBodyInterface().SetLinearAndAngularVelocity(
        JPH::BodyID(body), ToJolt(linear), ToJolt(angular));
}

void PhysicsWorld::Activate(BodyHandle body)
{
    if (!m_impl || body == InvalidBody)
        return;

    m_impl->physicsSystem.GetBodyInterface().ActivateBody(JPH::BodyID(body));
}

bool PhysicsWorld::IsActive(BodyHandle body) const
{
    if (!m_impl || body == InvalidBody)
        return false;

    return m_impl->physicsSystem.GetBodyInterface().IsActive(JPH::BodyID(body));
}
