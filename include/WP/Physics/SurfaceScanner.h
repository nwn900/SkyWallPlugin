#pragma once

#include <vector>
#include "WP/Core/WallWalkTypes.h"

namespace RE
{
    class Actor;
    class PlayerCharacter;
}

namespace WP::Physics
{
    class SurfaceScanner
    {
    public:
        SurfaceScanner() = default;
        ~SurfaceScanner() = default;

        void Initialize();
        bool Scan(RE::PlayerCharacter* player, const Core::AttachState& state, const Core::RuntimeConfig& cfg);

        const std::vector<Core::SurfaceSample>& GetCandidates() const { return _candidates; }
        const Core::SurfaceSample& GetPrimary() const { return _primary; }
        bool HasValidSurface() const { return _primary.valid; }

    private:
        void CastProbe(const RE::NiPoint3& origin, const RE::NiPoint3& direction, float maxDist, Core::SurfaceSample& outSample);
        void ClassifySamples();
        void ScoreCandidates(const RE::NiPoint3& inputDir, const Core::AttachState& state, const Core::RuntimeConfig& cfg);
        void SelectBest();

        std::vector<Core::SurfaceSample> _candidates;
        Core::SurfaceSample _primary;
        RE::NiPoint3 _worldUp{ 0.0f, 0.0f, 1.0f };
    };
}
