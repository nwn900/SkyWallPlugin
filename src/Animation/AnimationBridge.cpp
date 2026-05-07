#include "WP/Animation/AnimationBridge.h"
#include "RE/A/Actor.h"
#include "RE/B/BSFixedString.h"
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
        SetGraphFloat(actor, "WP_SurfacePitch", std::abs(1.0f - std::abs(state.desiredUpWS.z)) * 90.0f);
        SetGraphFloat(actor, "WP_SurfaceAngle", std::acos(std::clamp(std::abs(state.desiredUpWS.z), 0.0f, 1.0f)) * 57.29578f);
        SetGraphBool(actor, "WP_IsTransitioning", state.mode == Core::WallWalkMode::kTransition);
        SetGraphBool(actor, "WP_IsAirAttachPending", state.mode == Core::WallWalkMode::kAirborne && state.hotkeyArmed);

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
        if (!actor || !_enabled) return;
        SendGraphEvent(actor, "WP_Attach");
    }

    void AnimationBridge::SendDetachEvent(RE::Actor* actor)
    {
        if (!actor || !_enabled) return;
        SendGraphEvent(actor, "WP_Detach");
    }

    void AnimationBridge::SendSurfaceSwitchEvent(RE::Actor* actor)
    {
        if (!actor || !_enabled) return;
        SendGraphEvent(actor, "WP_SurfaceSwitch");
    }

    void AnimationBridge::SetGraphBool(RE::Actor* actor, const char* name, bool value)
    {
        if (!actor || !_enabled) return;
        actor->SetGraphVariableBool(RE::BSFixedString(name), value);
    }

    void AnimationBridge::SetGraphFloat(RE::Actor* actor, const char* name, float value)
    {
        if (!actor || !_enabled) return;
        actor->SetGraphVariableFloat(RE::BSFixedString(name), value);
    }

    void AnimationBridge::SendGraphEvent(RE::Actor* actor, const char* name)
    {
        if (!actor || !_enabled) return;
        actor->NotifyAnimationGraph(RE::BSFixedString(name));
    }
}
