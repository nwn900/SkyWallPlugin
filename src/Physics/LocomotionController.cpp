#include "WP/Physics/LocomotionController.h"
#include "RE/P/PlayerCharacter.h"
#include "RE/B/bhkCharacterController.h"
#include "RE/B/bhkCharProxyController.h"
#include "RE/A/Actor.h"
#include "RE/N/NiPoint3.h"
#include "RE/H/hkpWorld.h"
#include "RE/H/hkpWorldRayCastInput.h"
#include "RE/H/hkpWorldRayCastOutput.h"
#include "RE/H/hkVector4.h"
#include "RE/T/TESObjectCELL.h"
#include "RE/B/bhkWorld.h"
#include "SKSE/SKSE.h"

namespace WP::Physics
{
    static constexpr float WORLD_GRAVITY = 1000.0f;

    void LocomotionController::Initialize()
    {
        _wasVanilla = true;
        SKSE::log::info("LocomotionController: Initialized");
    }

    RE::bhkCharacterController* LocomotionController::GetController(RE::PlayerCharacter* player)
    {
        if (!player) return nullptr;
        return player->GetCharController();
    }

    void LocomotionController::BeginAttach(RE::PlayerCharacter* player, Core::AttachState& state,
                                            const Core::SurfaceSample& surface)
    {
        if (!player || !surface.valid) return;

        state.primary = surface;
        state.desiredUpWS = surface.normalWS;
        state.desiredGravityWS = surface.normalWS * -1.0f;
        state.transitionAlpha = 0.0f;
        state.noSurfaceTime = 0.0f;

        if (surface.isCeiling)
            state.mode = Core::WallWalkMode::kAttachedCeiling;
        else
            state.mode = Core::WallWalkMode::kAttachedWall;

        _wasVanilla = false;

        SKSE::log::info("BeginAttach: mode={}, normal=({:.2f},{:.2f},{:.2f})",
            static_cast<int>(state.mode),
            surface.normalWS.x, surface.normalWS.y, surface.normalWS.z);
    }

    void LocomotionController::DetachToAir(RE::PlayerCharacter* player, Core::AttachState& state)
    {
        RestoreVanillaState(player);
        state.mode = Core::WallWalkMode::kAirborne;
        state.primary.valid = false;
        _wasVanilla = true;
        SKSE::log::info("DetachToAir");
    }

    bool LocomotionController::Update(RE::PlayerCharacter* player, Core::AttachState& state,
                                       const Core::RuntimeConfig& cfg, float deltaTime)
    {
        if (!player) return false;
        if (!Core::IsAttached(state.mode)) return false;

        ApplyLocalFrame(player, state);
        ProjectVelocity(player, state, deltaTime);
        ApplyAdhesion(player, state, deltaTime);
        FeedCharacterController(player, state, cfg);

        if (player->IsInMidair())
        {
            DetachToAir(player, state);
            return false;
        }

        return true;
    }

    void LocomotionController::ApplyLocalFrame(RE::PlayerCharacter* player, Core::AttachState& state)
    {
        auto* controller = GetController(player);
        if (!controller) return;

        RE::NiPoint3 newUp = state.desiredUpWS;
        newUp.Unitize();

        controller->up = RE::hkVector4(newUp);
        controller->supportNorm = RE::hkVector4(newUp);

        RE::NiPoint3 worldUp(0, 0, 1);
        float pitchDot = newUp.Dot(worldUp);
        if (pitchDot > 1.0f) pitchDot = 1.0f;
        if (pitchDot < -1.0f) pitchDot = -1.0f;
        controller->pitchAngle = std::acos(pitchDot);
    }

    void LocomotionController::ProjectVelocity(RE::PlayerCharacter* player, const Core::AttachState& state,
                                                float deltaTime)
    {
        auto* controller = GetController(player);
        if (!controller) return;

        RE::NiPoint3 currentVel(
            controller->outVelocity.quad.m128_f32[0],
            controller->outVelocity.quad.m128_f32[1],
            controller->outVelocity.quad.m128_f32[2]
        );

        RE::NiPoint3 surfaceNormal = state.desiredUpWS;
        surfaceNormal.Unitize();

        float normalComponent = currentVel.Dot(surfaceNormal);
        RE::NiPoint3 tangent = currentVel - surfaceNormal * normalComponent;

        RE::NiPoint3 forward(
            controller->forwardVec.quad.m128_f32[0],
            controller->forwardVec.quad.m128_f32[1],
            controller->forwardVec.quad.m128_f32[2]
        );

        forward = forward - surfaceNormal * forward.Dot(surfaceNormal);
        float forwardLen = forward.Unitize();

        if (forwardLen > 0.001f)
        {
            float speed = tangent.Length();
            tangent = forward * speed;
        }

        controller->outVelocity = RE::hkVector4(tangent);
    }

    void LocomotionController::ApplyAdhesion(RE::PlayerCharacter* player, const Core::AttachState& state,
                                              float deltaTime)
    {
        auto* controller = GetController(player);
        if (!controller) return;

        RE::NiPoint3 currentVel(
            controller->outVelocity.quad.m128_f32[0],
            controller->outVelocity.quad.m128_f32[1],
            controller->outVelocity.quad.m128_f32[2]
        );

        RE::NiPoint3 adhesionDir = state.desiredUpWS * -1.0f;
        adhesionDir.Unitize();
        float adhesionForce = 600.0f;

        RE::NiPoint3 adhesionVel = currentVel + adhesionDir * adhesionForce * deltaTime;

        float normalComp = adhesionVel.Dot(state.desiredUpWS);
        if (normalComp > -300.0f && normalComp < 300.0f)
        {
            RE::NiPoint3 tangent = adhesionVel - state.desiredUpWS * adhesionVel.Dot(state.desiredUpWS);
            adhesionVel = tangent + state.desiredUpWS * -200.0f;
        }

        controller->outVelocity = RE::hkVector4(adhesionVel);
    }

    void LocomotionController::FeedCharacterController(RE::PlayerCharacter* player,
                                                        const Core::AttachState& state,
                                                        const Core::RuntimeConfig& cfg)
    {
        auto* controller = GetController(player);
        if (!controller) return;

        controller->gravity = cfg.gravityMagnitude;

        controller->surfaceInfo.supportedState.set(RE::hkpSurfaceInfo::SupportedState::kSupported);
        controller->surfaceInfo.surfaceNormal = RE::hkVector4(state.desiredUpWS);

        controller->flags.set(RE::CHARACTER_FLAGS::kSupport);
        controller->flags.set(RE::CHARACTER_FLAGS::kCheckSupport);
        controller->flags.set(RE::CHARACTER_FLAGS::kCanPitch);
        controller->flags.set(RE::CHARACTER_FLAGS::kCanRoll);
    }

    void LocomotionController::RestoreVanillaState(RE::PlayerCharacter* player)
    {
        auto* controller = GetController(player);
        if (!controller) return;

        RE::NiPoint3 worldUp(0, 0, 1);
        controller->up = RE::hkVector4(worldUp);
        controller->supportNorm = RE::hkVector4(worldUp);
        controller->gravity = WORLD_GRAVITY;
        controller->flags.reset(RE::CHARACTER_FLAGS::kNoGravityOnGround);
    }

    bool LocomotionController::TryJumpAttach(RE::PlayerCharacter* player, Core::AttachState& state,
                                              const Core::RuntimeConfig& cfg)
    {
        if (!player) return false;
        if (!player->IsInMidair()) return false;

        auto* controller = GetController(player);
        if (!controller) return false;

        RE::NiPoint3 currentPos = player->GetPosition();

        RE::NiPoint3 currentVel(
            controller->outVelocity.quad.m128_f32[0],
            controller->outVelocity.quad.m128_f32[1],
            controller->outVelocity.quad.m128_f32[2]
        );

        RE::NiPoint3 velDir = currentVel;
        float speed = velDir.Unitize();
        if (speed < cfg.minAttachSpeed) return false;

        auto* cell = player->GetParentCell();
        if (!cell) return false;
        auto* bhkWorld = cell->GetbhkWorld();
        if (!bhkWorld) return false;
        auto* hkpWorld = bhkWorld->GetWorld1();
        if (!hkpWorld) return false;

        float sweepDist = speed * 0.5f;
        if (sweepDist > cfg.maxAttachDistance) sweepDist = cfg.maxAttachDistance;
        RE::NiPoint3 predictedPos = currentPos + velDir * sweepDist;

        RE::hkpWorldRayCastInput input;
        input.from = RE::hkVector4(currentPos);
        input.to = RE::hkVector4(predictedPos);

        RE::hkpWorldRayCastOutput output;
        hkpWorld->CastRay(input, output);

        if (!output.HasHit()) return false;
        if (output.hitFraction < 0.0f || output.hitFraction > 1.0f) return false;

        RE::NiPoint3 hitNormal(
            output.normal.quad.m128_f32[0],
            output.normal.quad.m128_f32[1],
            output.normal.quad.m128_f32[2]
        );
        hitNormal.Unitize();

        if (hitNormal.z > 0.55f) return false;

        RE::hkVector4 hitVec = input.from + (input.to - input.from) * RE::hkVector4(output.hitFraction);
        RE::NiPoint3 hitPoint(
            hitVec.quad.m128_f32[0],
            hitVec.quad.m128_f32[1],
            hitVec.quad.m128_f32[2]
        );

        static constexpr float CLEARANCE = 30.0f;
        RE::NiPoint3 attachPos = hitPoint + hitNormal * CLEARANCE;
        player->SetPosition(attachPos, true);

        float normalComponent = currentVel.Dot(hitNormal);
        RE::NiPoint3 tangentVel = currentVel - hitNormal * normalComponent;
        controller->outVelocity = RE::hkVector4(tangentVel);

        Core::SurfaceSample surface;
        surface.valid = true;
        surface.normalWS = hitNormal;
        surface.hitPointWS = hitPoint;
        surface.distance = output.hitFraction * sweepDist;
        surface.isCeiling = (hitNormal.z < -0.55f);

        BeginAttach(player, state, surface);

        return true;
    }
}
