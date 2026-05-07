#pragma once

#include "WP/Core/WallWalkTypes.h"

namespace RE
{
    class PlayerCharacter;
}

namespace WP::Camera
{
    struct CameraPolicy
    {
        bool firstPersonLockHorizon = true;
        float thirdPersonRollFollow = 0.35f;
        float surfaceBlendTime = 0.15f;
        float ceilingLookClampDeg = 35.0f;
    };

    class CameraMediator
    {
    public:
        static CameraMediator& Get();

        void SetEnabled(bool enabled) { _enabled = enabled; }
        bool IsEnabled() const { return _enabled; }

        void Update(RE::PlayerCharacter* player, const Core::AttachState& state, float deltaTime);
        void SetPolicy(const CameraPolicy& policy) { _policy = policy; }

    private:
        CameraMediator() = default;
        ~CameraMediator() = default;

        bool _enabled = false;
        CameraPolicy _policy;
        float _currentRollOffset = 0.0f;
    };
}
