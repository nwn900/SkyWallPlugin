#pragma once

#include "WP/Core/WallWalkTypes.h"

namespace RE
{
    class PlayerCharacter;
    struct bhkCharacterController;
}

namespace WP::Physics
{
    class LocomotionController
    {
    public:
        LocomotionController() = default;
        ~LocomotionController() = default;

        void Initialize();
        bool Update(RE::PlayerCharacter* player, Core::AttachState& state, const Core::RuntimeConfig& cfg, float deltaTime);

        void BeginAttach(RE::PlayerCharacter* player, Core::AttachState& state, const Core::SurfaceSample& surface);
        void DetachToAir(RE::PlayerCharacter* player, Core::AttachState& state);
        bool TryJumpAttach(RE::PlayerCharacter* player, Core::AttachState& state, const Core::RuntimeConfig& cfg);

    private:
        void ApplyLocalFrame(RE::PlayerCharacter* player, Core::AttachState& state);
        void ProjectVelocity(RE::PlayerCharacter* player, const Core::AttachState& state, float deltaTime);
        void ApplyAdhesion(RE::PlayerCharacter* player, const Core::AttachState& state, float deltaTime);
        void FeedCharacterController(RE::PlayerCharacter* player, const Core::AttachState& state, const Core::RuntimeConfig& cfg);
        void RestoreVanillaState(RE::PlayerCharacter* player);

        RE::bhkCharacterController* GetController(RE::PlayerCharacter* player);

        bool _wasVanilla = true;
    };
}
