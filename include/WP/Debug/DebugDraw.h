#pragma once

#include <cstdint>

namespace RE
{
    class Actor;
}

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
