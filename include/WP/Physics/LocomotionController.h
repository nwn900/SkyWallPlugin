#pragma once

#include "RE/A/Actor.h"

namespace WP::Physics
{
    class LocomotionController
    {
    public:
        LocomotionController() = default;
        ~LocomotionController() = default;

        void Initialize();
        void Update(RE::Actor* player, float deltaTime);
        void Reset();

    private:
        bool _initialized = false;
    };
}
