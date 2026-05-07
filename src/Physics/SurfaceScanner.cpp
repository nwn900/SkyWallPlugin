#include "WP/Physics/SurfaceScanner.h"
#include "RE/H/hkpRigidBody.h"
#include "RE/H/hkpWorld.h"
#include "RE/H/hkpWorldRayCastInput.h"
#include "RE/H/hkpWorldRayCastOutput.h"
#include "RE/H/hkVector4.h"
#include "RE/P/PlayerCharacter.h"
#include "RE/T/TESObjectCELL.h"
#include "RE/B/bhkWorld.h"
#include "SKSE/SKSE.h"
#include <algorithm>

namespace WP::Physics
{
    static RE::NiPoint3 HkVecToNi(const RE::hkVector4& v)
    {
        return RE::NiPoint3(v.quad.m128_f32[0], v.quad.m128_f32[1], v.quad.m128_f32[2]);
    }

    void SurfaceScanner::Initialize()
    {
        _candidates.reserve(8);
        _wasLogged = false;
        SKSE::log::info("SurfaceScanner: Initialized");
    }

    void SurfaceScanner::CastProbe(const RE::NiPoint3& origin, const RE::NiPoint3& direction,
                                    float maxDist, Core::SurfaceSample& outSample)
    {
        outSample.valid = false;
        outSample.distance = FLT_MAX;

        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player)
        {
            if (!_wasLogged) { SKSE::log::warn("SurfaceScanner: GetPlayer() is null"); _wasLogged = true; }
            return;
        }

        auto* cell = player->GetParentCell();
        if (!cell)
        {
            if (!_wasLogged) { SKSE::log::warn("SurfaceScanner: GetParentCell() is null"); _wasLogged = true; }
            return;
        }

        auto* bhkWorld = cell->GetbhkWorld();
        if (!bhkWorld)
        {
            if (!_wasLogged) { SKSE::log::warn("SurfaceScanner: GetbhkWorld() is null"); _wasLogged = true; }
            return;
        }

        auto* hkpWorld = bhkWorld->GetWorld1();
        if (!hkpWorld)
        {
            if (!_wasLogged) { SKSE::log::warn("SurfaceScanner: GetWorld1() is null"); _wasLogged = true; }
            return;
        }

        _wasLogged = false;

        RE::NiPoint3 dirNorm = direction;
        dirNorm.Unitize();
        RE::NiPoint3 endPos = origin + dirNorm * maxDist;

        RE::hkpWorldRayCastInput input;
        input.from = RE::hkVector4(origin);
        input.to = RE::hkVector4(endPos);

        RE::hkpWorldRayCastOutput output;
        hkpWorld->CastRay(input, output);

        static bool firstRay = true;
        if (firstRay)
        {
            firstRay = false;
            SKSE::log::info("[RAYCAST] from=({:.1f},{:.1f},{:.1f}) to=({:.1f},{:.1f},{:.1f}) frac={:.4f} coll={}",
                origin.x, origin.y, origin.z, endPos.x, endPos.y, endPos.z,
                output.hitFraction, static_cast<const void*>(output.rootCollidable));
        }

        if (output.hitFraction >= 1.0f)
            return;

        outSample.valid = true;
        outSample.normalWS = HkVecToNi(output.normal);
        outSample.normalWS.Unitize();
        outSample.distance = output.hitFraction * maxDist;

        RE::hkVector4 hitVec = input.from + (input.to - input.from) * RE::hkVector4(output.hitFraction);
        outSample.hitPointWS = HkVecToNi(hitVec);
    }

    bool SurfaceScanner::Scan(RE::PlayerCharacter* player, const Core::AttachState& state,
                               const Core::RuntimeConfig& cfg)
    {
        _candidates.clear();
        _primary.valid = false;

        if (!player) return false;

        RE::NiPoint3 pos = player->GetPosition();
        RE::NiPoint3 angle = player->GetAngle();

        float sx = std::sin(angle.x);
        float cx = std::cos(angle.x);
        float sz = std::sin(angle.z);
        float cz = std::cos(angle.z);

        RE::NiPoint3 forward(-sz * cx, cz * cx, sx);
        forward.Unitize();

        RE::NiPoint3 right = forward.UnitCross(_worldUp);
        right.Unitize();

        static int frameCount = 0;
        if (++frameCount >= 60)
        {
            frameCount = 0;
            SKSE::log::info("[SCAN] pos=({:.1f},{:.1f},{:.1f}) fwd=({:.2f},{:.2f},{:.2f}) armed={}",
                pos.x, pos.y, pos.z, forward.x, forward.y, forward.z, state.hotkeyArmed);
        }

        for (int i = -1; i <= 1; ++i)
        {
            RE::NiPoint3 dir = forward;
            if (i != 0) dir = forward + right * (static_cast<float>(i) * 0.5f);
            dir.Unitize();

            Core::SurfaceSample s;
            CastProbe(pos, dir, cfg.maxAttachDistance, s);
            if (s.valid) _candidates.push_back(s);
        }

        {
            Core::SurfaceSample s;
            CastProbe(pos, RE::NiPoint3(0, 0, -1), cfg.maxAttachDistance, s);
            if (s.valid && s.normalWS.z > 0.3f) _candidates.push_back(s);
        }

        {
            Core::SurfaceSample s;
            CastProbe(pos, RE::NiPoint3(0, 0, 1), cfg.maxCeilingAttachDistance, s);
            if (s.valid) _candidates.push_back(s);
        }

        if (!_candidates.empty() && frameCount == 0)
        {
            SKSE::log::info("[SCAN] Found {} candidates", _candidates.size());
            for (size_t i = 0; i < _candidates.size() && i < 3; ++i)
            {
                auto& s = _candidates[i];
                SKSE::log::info("[SCAN]   #{}= norm=({:.2f},{:.2f},{:.2f}) dist={:.1f} ceil={}",
                    i, s.normalWS.x, s.normalWS.y, s.normalWS.z, s.distance, s.isCeiling);
            }
        }

        ClassifySamples();

        RE::NiPoint3 inputDir = forward;
        ScoreCandidates(inputDir, state, cfg);
        SelectBest();

        if (_primary.valid && frameCount == 0)
        {
            SKSE::log::info("[SCAN] Best: norm=({:.2f},{:.2f},{:.2f}) dist={:.1f} score={:.1f} ceil={}",
                _primary.normalWS.x, _primary.normalWS.y, _primary.normalWS.z,
                _primary.distance, _primary.score, _primary.isCeiling);
        }

        return _primary.valid;
    }

    void SurfaceScanner::ClassifySamples()
    {
        for (auto& s : _candidates)
        {
            if (!s.valid) continue;
            float d = s.normalWS.z;
            s.isCeiling = (d < -0.55f);
        }
    }

    void SurfaceScanner::ScoreCandidates(const RE::NiPoint3& inputDir, const Core::AttachState& state,
                                          const Core::RuntimeConfig& cfg)
    {
        for (auto& s : _candidates)
        {
            if (!s.valid) continue;
            if (!Core::IsWallOrCeiling(s)) continue;

            float score = 0.0f;

            float maxDist = cfg.maxAttachDistance;
            float distScore = (maxDist - s.distance) / maxDist;
            score += distScore * 10.0f;

            float alignment = s.normalWS.Dot(inputDir);
            score += std::abs(alignment) * 5.0f;

            if (state.primary.valid)
            {
                float continuity = state.primary.normalWS.Dot(s.normalWS);
                score += continuity * 8.0f;
            }

            float angleFromUp = std::abs(s.normalWS.z);
            if (angleFromUp < 0.2f)
                score += 15.0f;

            s.score = score;
        }
    }

    void SurfaceScanner::SelectBest()
    {
        _primary.valid = false;
        float bestScore = -FLT_MAX;

        for (const auto& s : _candidates)
        {
            if (!s.valid || !Core::IsWallOrCeiling(s)) continue;
            if (s.score > bestScore)
            {
                bestScore = s.score;
                _primary = s;
            }
        }
    }
}
