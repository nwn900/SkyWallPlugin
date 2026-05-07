#pragma once

#include "RE/A/Actor.h"

namespace WP::Physics
{
    class SurfaceScanner
    {
    public:
        SurfaceScanner() = default;
        ~SurfaceScanner() = default;

        void Initialize();
        void Scan(RE::Actor* player, float deltaTime);
        bool HasValidSurface() const { return _hasSurface; }

    private:
        bool _hasSurface = false;
    };
}
