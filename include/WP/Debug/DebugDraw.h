#pragma once

#include "RE/A/Actor.h"

namespace WP::Debug
{
    class DebugDraw
    {
    public:
        static DebugDraw& Get();

        void SetEnabled(bool enabled) { _enabled = enabled; }
        bool IsEnabled() const { return _enabled; }

        void DrawFrame(RE::Actor* player, float deltaTime);

    private:
        DebugDraw() = default;
        ~DebugDraw() = default;
        bool _enabled = false;
    };
}
