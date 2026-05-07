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
        static int frameCounter = 0;
        if (!_enabled || !player) return;

        if (++frameCounter % 60 != 0) return;

        if (sample.valid)
        {
            SKSE::log::info("[DEBUG] Surface: hit=({:.1f},{:.1f},{:.1f}) norm=({:.2f},{:.2f},{:.2f}) dist={:.1f} ceil={} score={:.1f}",
                sample.hitPointWS.x, sample.hitPointWS.y, sample.hitPointWS.z,
                sample.normalWS.x, sample.normalWS.y, sample.normalWS.z,
                sample.distance, sample.isCeiling, sample.score);
        }

        if (Core::IsAttached(state.mode))
        {
            const char* modeStr = "Unknown";
            switch (state.mode)
            {
            case Core::WallWalkMode::kAttachedWall: modeStr = "Wall"; break;
            case Core::WallWalkMode::kAttachedCeiling: modeStr = "Ceiling"; break;
            case Core::WallWalkMode::kTransition: modeStr = "Transition"; break;
            case Core::WallWalkMode::kAirborne: modeStr = "Airborne"; break;
            case Core::WallWalkMode::kDetaching: modeStr = "Detaching"; break;
            default: break;
            }

            SKSE::log::info("[DEBUG] State: mode={} up=({:.2f},{:.2f},{:.2f}) grav=({:.2f},{:.2f},{:.2f}) blend={:.2f} nosurf={:.2f}",
                modeStr,
                state.desiredUpWS.x, state.desiredUpWS.y, state.desiredUpWS.z,
                state.desiredGravityWS.x, state.desiredGravityWS.y, state.desiredGravityWS.z,
                state.transitionAlpha, state.noSurfaceTime);
        }

        RE::NiPoint3 pos = player->GetPosition();
        SKSE::log::info("[DEBUG] Player pos=({:.1f},{:.1f},{:.1f}) armed={}",
            pos.x, pos.y, pos.z, state.hotkeyArmed);
    }

    void DebugDraw::DrawLine(const RE::NiPoint3&, const RE::NiPoint3&, std::uint32_t) {}
    void DebugDraw::DrawCross(const RE::NiPoint3&, float, std::uint32_t) {}
}
