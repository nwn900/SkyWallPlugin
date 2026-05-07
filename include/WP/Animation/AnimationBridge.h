#pragma once

#include "WP/Core/WallWalkTypes.h"

namespace RE
{
    class Actor;
}

namespace WP::Animation
{
    class AnimationBridge
    {
    public:
        static AnimationBridge& Get();

        void SetEnabled(bool enabled) { _enabled = enabled; }
        bool IsEnabled() const { return _enabled; }

        void PushState(RE::Actor* actor, const Core::AttachState& state);
        void SendAttachEvent(RE::Actor* actor);
        void SendDetachEvent(RE::Actor* actor);
        void SendSurfaceSwitchEvent(RE::Actor* actor);

    private:
        AnimationBridge() = default;
        ~AnimationBridge() = default;

        bool _enabled = false;
        bool _lastAttached = false;
        Core::WallWalkMode _lastMode = Core::WallWalkMode::kGrounded;

        void SetGraphBool(RE::Actor* actor, const char* name, bool value);
        void SetGraphFloat(RE::Actor* actor, const char* name, float value);
        void SendGraphEvent(RE::Actor* actor, const char* name);
    };
}
