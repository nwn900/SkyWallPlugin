#pragma once

#include <cstdint>

namespace RE
{
    class Actor;
}

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
