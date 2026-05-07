#pragma once

#include <memory>
#include "WP/Core/Settings.h"
#include "WP/Core/WallWalkTypes.h"

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
        class MagickaCost;
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
            bool IsArmed() const noexcept { return _attach.hotkeyArmed; }

            RuntimeConfig& GetSettings() { return _settings; }
            void SetSettings(const RuntimeConfig& cfg) { _settings = cfg; }

        private:
            Service() = default;
            ~Service() = default;

            bool _enabled = true;
            RuntimeConfig _settings;

            std::unique_ptr<Physics::SurfaceScanner> _scanner;
            std::unique_ptr<Physics::LocomotionController> _controller;
            std::unique_ptr<Physics::MagickaCost> _magicka;

            RE::PlayerCharacter* GetPlayer();
            void ProcessHotkey();

            AttachState _attach;
        };
    }
}
