#pragma once

#include <cstdint>
#include "RE/N/NiPoint3.h"
#include "RE/A/Actor.h"
#include "RE/A/ActorState.h"
#include "RE/A/ActorValues.h"

namespace WP::Core
{
    enum class WallWalkMode : std::uint8_t
    {
        kDisabled = 0,
        kGrounded,
        kProbeAttach,
        kAttachedWall,
        kAttachedCeiling,
        kTransition,
        kAirborne,
        kDetaching
    };

    struct SurfaceSample
    {
        RE::NiPoint3 hitPointWS{};
        RE::NiPoint3 normalWS{};
        RE::NiPoint3 tangentForwardWS{};
        RE::NiPoint3 tangentRightWS{};
        float distance = FLT_MAX;
        float score = -FLT_MAX;
        bool isDynamic = false;
        bool isCeiling = false;
        bool valid = false;
    };

    struct AttachState
    {
        WallWalkMode mode = WallWalkMode::kGrounded;
        SurfaceSample primary;
        RE::NiPoint3 desiredUpWS{ 0.0f, 0.0f, 1.0f };
        RE::NiPoint3 desiredGravityWS{ 0.0f, 0.0f, -1.0f };
        float adherence = 0.0f;
        float transitionAlpha = 0.0f;
        float noSurfaceTime = 0.0f;
        bool hotkeyArmed = false;
    };

    struct RuntimeConfig
    {
        int hotkey = 21;
        bool enableMagickaCost = false;
        float magickaPerSecond = 5.0f;
        float maxAttachDistance = 3000.0f;
        float maxCeilingAttachDistance = 3000.0f;
        float minAttachSpeed = 30.0f;
        float edgeTransferBlendTime = 0.12f;
        float adhesionStrength = 600.0f;
        float gravityMagnitude = 1000.0f;
        float detachGraceTime = 0.3f;
        float emptyMagickaGracePeriod = 1.5f;
        float minMagickaToAttach = 10.0f;
        bool disableInCombat = false;
        bool debugDraw = false;
        bool debugLogState = false;
    };

    inline bool IsAttached(WallWalkMode mode)
    {
        return mode == WallWalkMode::kAttachedWall ||
               mode == WallWalkMode::kAttachedCeiling ||
               mode == WallWalkMode::kTransition;
    }

    inline bool IsWallOrCeiling(const SurfaceSample& sample, float floorThreshold = 0.55f)
    {
        if (!sample.valid) return false;
        float d = sample.normalWS.z;
        if (d > floorThreshold) return false;
        return true;
    }

    inline void ClassifySurface(const SurfaceSample& sample, bool& outIsFloor, bool& outIsWall, bool& outIsCeiling)
    {
        outIsFloor = outIsWall = outIsCeiling = false;
        if (!sample.valid) return;
        float d = sample.normalWS.z;
        if (d > 0.55f) outIsFloor = true;
        else if (std::abs(d) < 0.35f) outIsWall = true;
        else if (d < -0.55f) outIsCeiling = true;
    }

    inline bool ShouldSuspendForState(RE::Actor* player)
    {
        if (!player) return true;
        if (player->GetLifeState() != RE::ACTOR_LIFE_STATE::kAlive) return true;
        if (player->IsInKillMove()) return true;
        if (player->IsStaggered()) return true;
        auto sitState = player->GetSitSleepState();
        if (sitState != RE::SIT_SLEEP_STATE::kNormal) return true;
        return false;
    }
}
