#include "WP/Camera/CameraMediator.h"
#include "RE/B/bhkRigidBody.h"
#include "RE/P/PlayerCamera.h"
#include "RE/P/PlayerCharacter.h"
#include "RE/T/ThirdPersonState.h"
#include "SKSE/SKSE.h"
#include <algorithm>

namespace WP::Camera
{
    CameraMediator& CameraMediator::Get()
    {
        static CameraMediator instance;
        return instance;
    }

    void CameraMediator::Update(RE::PlayerCharacter* player, const Core::AttachState& state, float deltaTime)
    {
        if (!_enabled || !player) return;

        auto* pc = RE::PlayerCamera::GetSingleton();
        if (!pc) return;

        if (!Core::IsAttached(state.mode))
        {
            _currentRollOffset = 0.0f;
            return;
        }

        if (!pc->IsInThirdPerson()) return;

        auto runtimeData = pc->GetRuntimeData();
        auto* thirdPerson = static_cast<RE::ThirdPersonState*>(
            runtimeData.cameraStates[RE::CameraStates::CameraState::kThirdPerson].get());
        if (!thirdPerson) return;

        RE::NiPoint3 surfaceUp = state.desiredUpWS;
        surfaceUp.Unitize();

        RE::NiPoint3 worldUp(0.0f, 0.0f, 1.0f);
        float dot = std::clamp(surfaceUp.Dot(worldUp), -1.0f, 1.0f);
        float surfaceAngleDeg = std::acos(dot) * 57.29578f;

        float targetRoll = surfaceAngleDeg * _policy.thirdPersonRollFollow;

        if (state.mode == Core::WallWalkMode::kAttachedCeiling)
            targetRoll = std::min(targetRoll, _policy.ceilingLookClampDeg);

        if (state.mode == Core::WallWalkMode::kTransition)
            targetRoll *= state.transitionAlpha;

        float blendSpeed = 1.0f / std::max(_policy.surfaceBlendTime, 0.01f);
        _currentRollOffset += (targetRoll - _currentRollOffset) *
            std::min(blendSpeed * deltaTime, 1.0f);

        RE::NiPoint3 cameraOffset = surfaceUp * _currentRollOffset * -0.5f;
        thirdPerson->posOffsetExpected = thirdPerson->posOffsetExpected + cameraOffset * deltaTime * 3.0f;
    }
}
