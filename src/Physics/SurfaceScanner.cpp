#include "WP/Physics/SurfaceScanner.h"
#include "SKSE/SKSE.h"

namespace WP::Physics
{
    void SurfaceScanner::Initialize()
    {
        SKSE::log::info("SurfaceScanner: Initialized (stub)");
    }

    void SurfaceScanner::Scan(RE::Actor* /*player*/, float /*deltaTime*/)
    {
        _hasSurface = false;
    }
}
