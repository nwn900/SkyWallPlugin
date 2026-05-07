#include "WP/Physics/LocomotionController.h"
#include "RE/P/PlayerCharacter.h"
#include "RE/B/bhkCharacterController.h"
#include "SKSE/SKSE.h"

namespace WP::Physics
{
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
        state.mode = surface.isCeiling ? Core::WallWalkMode::kAttachedCeiling : Core::WallWalkMode::kAttachedWall;
        _wasVanilla = false;

        RE::NiPoint3 n = surface.normalWS;
        SKSE::log::info("BeginAttach: mode={} normal=({:.2f},{:.2f},{:.2f})",
            static_cast<int>(state.mode), n.x, n.y, n.z);
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

        auto* controller = GetController(player);
        if (!controller) return false;

        RE::NiPoint3 up = state.desiredUpWS;
        up.Unitize();

        controller->up = RE::hkVector4(up);
        controller->supportNorm = RE::hkVector4(up);
        controller->gravity = cfg.gravityMagnitude;

        controller->flags.set(RE::CHARACTER_FLAGS::kSupport);
        controller->flags.set(RE::CHARACTER_FLAGS::kCheckSupport);
        controller->flags.set(RE::CHARACTER_FLAGS::kCanPitch);
        controller->flags.set(RE::CHARACTER_FLAGS::kCanRoll);

        RE::NiPoint3 currentVel(
            controller->outVelocity.quad.m128_f32[0],
            controller->outVelocity.quad.m128_f32[1],
            controller->outVelocity.quad.m128_f32[2]
        );

        float normalSpeed = currentVel.Dot(up);
        RE::NiPoint3 tangent = currentVel - up * normalSpeed;

        float adhesion = cfg.adhesionStrength * deltaTime;
        tangent = tangent - up * adhesion;

        controller->outVelocity = RE::hkVector4(tangent);

        return true;
    }

    void LocomotionController::RestoreVanillaState(RE::PlayerCharacter* player)
    {
        auto* controller = GetController(player);
        if (!controller) return;

        RE::NiPoint3 worldUp(0, 0, 1);
        controller->up = RE::hkVector4(worldUp);
        controller->supportNorm = RE::hkVector4(worldUp);
        controller->gravity = 1000.0f;
        controller->flags.reset(RE::CHARACTER_FLAGS::kNoGravityOnGround);
    }

    bool LocomotionController::TryJumpAttach(RE::PlayerCharacter*, Core::AttachState&, const Core::RuntimeConfig&)
    {
        return false;
    }
}
