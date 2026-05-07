#include "WP/Physics/SurfaceScanner.h"
#include "RE/P/PlayerCharacter.h"
#include "RE/B/bhkCharacterController.h"
#include "RE/B/bhkCharProxyController.h"
#include "SKSE/SKSE.h"

namespace WP::Physics
{
    void SurfaceScanner::Initialize()
    {
        _candidates.reserve(8);
        SKSE::log::info("SurfaceScanner: Initialized (controller-based)");
    }

    void SurfaceScanner::CastProbe(const RE::NiPoint3&, const RE::NiPoint3&, float, Core::SurfaceSample&) {}

    bool SurfaceScanner::Scan(RE::PlayerCharacter* player, const Core::AttachState& state,
                               const Core::RuntimeConfig& cfg)
    {
        _candidates.clear();
        _primary.valid = false;
        if (!player) return false;

        auto* controller = player->GetCharController();
        if (!controller) return false;

        RE::NiPoint3 pos = player->GetPosition();

        RE::NiPoint3 surfaceNormal(
            controller->surfaceInfo.surfaceNormal.quad.m128_f32[0],
            controller->surfaceInfo.surfaceNormal.quad.m128_f32[1],
            controller->surfaceInfo.surfaceNormal.quad.m128_f32[2]
        );

        static int fc = 0;
        if (++fc >= 60)
        {
            fc = 0;
            auto stateVal = controller->surfaceInfo.supportedState.underlying();
            SKSE::log::info("[CTRL] pos=({:.1f},{:.1f},{:.1f}) supState={} surfNrm=({:.2f},{:.2f},{:.2f}) armed={}",
                pos.x, pos.y, pos.z, stateVal,
                surfaceNormal.x, surfaceNormal.y, surfaceNormal.z,
                state.hotkeyArmed);
        }

        bool isSupported = controller->surfaceInfo.supportedState.any(
            RE::hkpSurfaceInfo::SupportedState::kSupported);

        if (!isSupported || surfaceNormal.z > 0.55f)
            return false;

        float normalLen = surfaceNormal.Length();
        if (normalLen < 0.01f) return false;
        surfaceNormal.Unitize();

        bool isSliding = controller->surfaceInfo.supportedState.any(
            RE::hkpSurfaceInfo::SupportedState::kSliding);

        Core::SurfaceSample s;
        s.valid = true;
        s.hitPointWS = pos;
        s.normalWS = surfaceNormal;
        s.distance = 0.0f;
        s.isCeiling = (surfaceNormal.z < -0.55f);
        s.isDynamic = controller->surfaceInfo.surfaceIsDynamic;
        s.score = isSliding ? 5.0f : 15.0f;
        s.tangentForwardWS = RE::NiPoint3(
            controller->forwardVec.quad.m128_f32[0],
            controller->forwardVec.quad.m128_f32[1],
            controller->forwardVec.quad.m128_f32[2]
        );

        _candidates.push_back(s);
        _primary = s;

        if (fc == 0)
        {
            SKSE::log::info("[CTRL] WALL FOUND! normal=({:.2f},{:.2f},{:.2f}) sliding={} ceil={}",
                surfaceNormal.x, surfaceNormal.y, surfaceNormal.z, isSliding, s.isCeiling);
        }

        return true;
    }

    void SurfaceScanner::ClassifySamples() {}
    void SurfaceScanner::ScoreCandidates(const RE::NiPoint3&, const Core::AttachState&, const Core::RuntimeConfig&) {}
    void SurfaceScanner::SelectBest() {}
}
