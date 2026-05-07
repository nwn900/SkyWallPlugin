#pragma once

#include "WP/Core/WallWalkTypes.h"

namespace RE
{
    class PlayerCharacter;
}

namespace WP::Debug
{
    class DebugDraw
    {
    public:
        static DebugDraw& Get();

        void SetEnabled(bool enabled) { _enabled = enabled; }
        bool IsEnabled() const { return _enabled; }

        void DrawFrame(RE::PlayerCharacter* player, const Core::AttachState& state,
                       const Core::SurfaceSample& sample);

    private:
        DebugDraw() = default;
        ~DebugDraw() = default;
        bool _enabled = false;

        void DrawLine(const RE::NiPoint3& start, const RE::NiPoint3& end, std::uint32_t color = 0xFF00FF00);
        void DrawCross(const RE::NiPoint3& point, float size = 4.0f, std::uint32_t color = 0xFFFF0000);
    };
}
