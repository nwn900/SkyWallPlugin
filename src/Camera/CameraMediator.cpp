#include "WP/Camera/CameraMediator.h"
#include "RE/P/PlayerCharacter.h"
#include "SKSE/SKSE.h"

namespace WP::Camera
{
    CameraMediator& CameraMediator::Get()
    {
        static CameraMediator instance;
        return instance;
    }

    void CameraMediator::Update(RE::PlayerCharacter*, const Core::AttachState& state, float deltaTime)
    {
        if (!_enabled) return;

        if (!Core::IsAttached(state.mode))
        {
            _currentRollOffset = 0.0f;
            return;
        }

        RE::NiPoint3 up = state.desiredUpWS;
        up.Unitize();

        RE::NiPoint3 worldUp(0.0f, 0.0f, 1.0f);
        float dot = up.Dot(worldUp);
        float targetRoll = std::acos(std::clamp(dot, -1.0f, 1.0f)) * _policy.thirdPersonRollFollow;

        if (state.mode == Core::WallWalkMode::kAttachedCeiling && targetRoll > _policy.ceilingLookClampDeg)
            targetRoll = _policy.ceilingLookClampDeg;

        float blendSpeed = 1.0f / _policy.surfaceBlendTime;
        _currentRollOffset += (targetRoll - _currentRollOffset) * std::min(blendSpeed * deltaTime, 1.0f);
    }
}
