#include "WP/Physics/SurfaceScanner.h"
#include "RE/H/hkpRigidBody.h"
#include "RE/H/hkpWorldRayCastInput.h"
#include "RE/H/hkpWorldRayCastOutput.h"
#include "RE/H/hkVector4.h"
#include "RE/P/PlayerCharacter.h"
#include "RE/T/TESObjectCELL.h"
#include "RE/B/bhkWorld.h"
#include "RE/B/bhkPickData.h"
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
        if (!player) return;

        auto* cell = player->GetParentCell();
        if (!cell) return;

        auto* bhkWorldPtr = cell->GetbhkWorld();
        if (!bhkWorldPtr) return;

        RE::NiPoint3 dirNorm = direction;
        dirNorm.Unitize();
        RE::NiPoint3 endPos = origin + dirNorm * maxDist;

        RE::bhkPickData pickData;
        pickData.rayInput.from = RE::hkVector4(origin);
        pickData.rayInput.to = RE::hkVector4(endPos);

        auto* vtable = *reinterpret_cast<void***>(bhkWorldPtr);
        auto pickFn = reinterpret_cast<bool(*)(RE::bhkWorld*, RE::bhkPickData&)>(vtable[0x33]);
        pickFn(bhkWorldPtr, pickData);

        if (pickData.pickFailed)
            return;

        RE::hkpWorldRayCastOutput& output = pickData.rayOutput;
        if (output.hitFraction >= 1.0f)
            return;

        static bool firstHit = true;
        if (firstHit)
        {
            firstHit = false;
            SKSE::log::info("[RAYCAST] PICK HIT! from=({:.1f},{:.1f},{:.1f}) frac={:.4f} coll={}",
                origin.x, origin.y, origin.z, output.hitFraction,
                static_cast<const void*>(output.rootCollidable));
        }

        outSample.valid = true;
        outSample.normalWS = HkVecToNi(output.normal);
        outSample.normalWS.Unitize();
        outSample.distance = output.hitFraction * maxDist;

        RE::hkVector4 hitVec = pickData.rayInput.from +
            (pickData.rayInput.to - pickData.rayInput.from) * RE::hkVector4(output.hitFraction);
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

        float sx = std::sin(angle.x), cx = std::cos(angle.x);
        float sz = std::sin(angle.z), cz = std::cos(angle.z);
        RE::NiPoint3 forward(-sz * cx, cz * cx, sx);
        forward.Unitize();
        RE::NiPoint3 right = forward.UnitCross(_worldUp);
        right.Unitize();

        static int fc = 0;
        if (++fc >= 60)
        {
            fc = 0;
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

        if (!_candidates.empty() && fc == 0)
        {
            SKSE::log::info("[SCAN] Found {} candidates", _candidates.size());
        }

        ClassifySamples();
        ScoreCandidates(forward, state, cfg);
        SelectBest();

        if (_primary.valid && fc == 0)
            SKSE::log::info("[SCAN] Best: norm=({:.2f},{:.2f},{:.2f}) dist={:.1f} score={:.1f}",
                _primary.normalWS.x, _primary.normalWS.y, _primary.normalWS.z,
                _primary.distance, _primary.score);

        return _primary.valid;
    }

    void SurfaceScanner::ClassifySamples()
    {
        for (auto& s : _candidates) { if (s.valid) s.isCeiling = (s.normalWS.z < -0.55f); }
    }

    void SurfaceScanner::ScoreCandidates(const RE::NiPoint3& inputDir, const Core::AttachState& state,
                                          const Core::RuntimeConfig& cfg)
    {
        for (auto& s : _candidates)
        {
            if (!s.valid || !Core::IsWallOrCeiling(s)) continue;
            float score = 0.0f;
            float maxDist = cfg.maxAttachDistance;
            score += ((maxDist - s.distance) / maxDist) * 10.0f;
            score += std::abs(s.normalWS.Dot(inputDir)) * 5.0f;
            if (state.primary.valid)
                score += state.primary.normalWS.Dot(s.normalWS) * 8.0f;
            if (std::abs(s.normalWS.z) < 0.2f)
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
            if (s.score > bestScore) { bestScore = s.score; _primary = s; }
        }
    }
}
