#pragma once

#include "WP/Core/WallWalkTypes.h"

namespace RE
{
    class PlayerCharacter;
}

namespace WP::Physics
{
    class MagickaCost
    {
    public:
        MagickaCost() = default;
        ~MagickaCost() = default;

        bool CanAttach(RE::PlayerCharacter* player, const Core::RuntimeConfig& cfg);
        bool Consume(RE::PlayerCharacter* player, const Core::RuntimeConfig& cfg, float deltaTime);
        void Reset();

    private:
        float _emptyTimer = 0.0f;
    };
}
