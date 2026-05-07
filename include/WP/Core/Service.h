#pragma once

#include <memory>
#include "WP/Core/Settings.h"

namespace RE
{
    class PlayerCharacter;
}

namespace WP
{
    namespace Physics
    {
        class SurfaceScanner;
        class LocomotionController;
    }

    namespace Core
    {
        class Service
        {
        public:
            static Service& Get();

            void OnSKSELoad();
            void OnDataLoaded();
            void OnFrame(float deltaTime);
            void OnInputEvent(std::uint32_t keyCode, bool pressed);

            void Enable(bool enabled);
            bool IsEnabled() const noexcept { return _enabled; }
            bool IsArmed() const noexcept { return _armed; }

            const RuntimeSettings& GetSettings() const { return _settings; }
            void SetSettings(const RuntimeSettings& cfg) { _settings = cfg; }

        private:
            Service() = default;
            ~Service() = default;

            bool _enabled = true;
            bool _armed = false;
            RuntimeSettings _settings;

            std::unique_ptr<Physics::SurfaceScanner> _scanner;
            std::unique_ptr<Physics::LocomotionController> _controller;

            RE::PlayerCharacter* GetPlayer();
            void ProcessHotkey();
        };
    }
}
