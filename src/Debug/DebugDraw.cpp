#include "WP/Debug/DebugDraw.h"
#include "SKSE/SKSE.h"

namespace WP::Debug
{
    DebugDraw& DebugDraw::Get()
    {
        static DebugDraw instance;
        return instance;
    }

    void DebugDraw::DrawFrame(RE::Actor* /*player*/, float /*deltaTime*/)
    {
    }
}
