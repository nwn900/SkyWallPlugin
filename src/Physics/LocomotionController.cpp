#include "WP/Physics/LocomotionController.h"
#include "SKSE/SKSE.h"

namespace WP::Physics
{
    void LocomotionController::Initialize()
    {
        _initialized = true;
        SKSE::log::info("LocomotionController: Initialized (stub)");
    }

    void LocomotionController::Update(RE::Actor* /*player*/, float /*deltaTime*/)
    {
    }

    void LocomotionController::Reset()
    {
    }
}
