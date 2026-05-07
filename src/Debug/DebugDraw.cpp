#include "WP/Debug/DebugDraw.h"
#include "RE/P/PlayerCharacter.h"
#include "SKSE/SKSE.h"

namespace WP::Debug
{
    DebugDraw& DebugDraw::Get()
    {
        static DebugDraw instance;
        return instance;
    }

    void DebugDraw::DrawFrame(RE::PlayerCharacter* player, const Core::AttachState& state,
                               const Core::SurfaceSample& sample)
    {
        if (!_enabled || !player) return;

        if (sample.valid)
        {
            RE::NiPoint3 normalEnd = sample.hitPointWS + sample.normalWS * 30.0f;
            DrawLine(sample.hitPointWS, normalEnd, 0xFF0000FF);
            DrawCross(sample.hitPointWS, 4.0f, 0xFFFF0000);
        }

        if (Core::IsAttached(state.mode))
        {
            RE::NiPoint3 pos = player->GetPosition();
            RE::NiPoint3 upEnd = pos + state.desiredUpWS * 50.0f;
            DrawLine(pos, upEnd, 0xFF00FF00);

            RE::NiPoint3 gravEnd = pos + state.desiredGravityWS * 50.0f;
            DrawLine(pos, gravEnd, 0xFFFF0000);
        }
    }

    void DebugDraw::DrawLine(const RE::NiPoint3& /*start*/, const RE::NiPoint3& /*end*/,
                              std::uint32_t /*color*/)
    {
    }

    void DebugDraw::DrawCross(const RE::NiPoint3& /*point*/, float /*size*/,
                               std::uint32_t /*color*/)
    {
    }
}
