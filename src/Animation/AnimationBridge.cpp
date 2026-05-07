#include "WP/Animation/AnimationBridge.h"
#include "RE/A/Actor.h"
#include "SKSE/SKSE.h"

namespace WP::Animation
{
    AnimationBridge& AnimationBridge::Get()
    {
        static AnimationBridge instance;
        return instance;
    }

    void AnimationBridge::PushState(RE::Actor* actor, const Core::AttachState& state)
    {
        if (!actor || !_enabled) return;

        bool isCurrentlyAttached = Core::IsAttached(state.mode);

        SetGraphBool(actor, "WP_IsAdhered", isCurrentlyAttached);
        SetGraphBool(actor, "WP_IsCeiling", state.mode == Core::WallWalkMode::kAttachedCeiling);
        SetGraphFloat(actor, "WP_AttachBlend", state.transitionAlpha);
        SetGraphFloat(actor, "WP_SurfacePitch", 0.0f);
        SetGraphFloat(actor, "WP_SurfaceAngle", std::abs(state.desiredUpWS.z) * 90.0f);
        SetGraphBool(actor, "WP_IsTransitioning", state.mode == Core::WallWalkMode::kTransition);

        if (_lastAttached != isCurrentlyAttached)
        {
            if (isCurrentlyAttached)
                SendAttachEvent(actor);
            else
                SendDetachEvent(actor);
        }

        if (_lastMode != state.mode && Core::IsAttached(state.mode) && Core::IsAttached(_lastMode))
        {
            SendSurfaceSwitchEvent(actor);
        }

        _lastAttached = isCurrentlyAttached;
        _lastMode = state.mode;
    }

    void AnimationBridge::SendAttachEvent(RE::Actor* actor)
    {
        SendGraphEvent(actor, "WP_Attach");
        SKSE::log::info("AnimationBridge: Attach event sent");
    }

    void AnimationBridge::SendDetachEvent(RE::Actor* actor)
    {
        SendGraphEvent(actor, "WP_Detach");
        SKSE::log::info("AnimationBridge: Detach event sent");
    }

    void AnimationBridge::SendSurfaceSwitchEvent(RE::Actor* actor)
    {
        SendGraphEvent(actor, "WP_SurfaceSwitch");
        SKSE::log::info("AnimationBridge: SurfaceSwitch event sent");
    }

    void AnimationBridge::SetGraphBool(RE::Actor*, const char* name, bool value)
    {
        SKSE::log::trace("AnimationBridge::SetGraphBool({}) = {}", name, value);
    }

    void AnimationBridge::SetGraphFloat(RE::Actor*, const char* name, float value)
    {
        SKSE::log::trace("AnimationBridge::SetGraphFloat({}) = {}", name, value);
    }

    void AnimationBridge::SendGraphEvent(RE::Actor*, const char* name)
    {
        SKSE::log::trace("AnimationBridge::SendGraphEvent({})", name);
    }
}
