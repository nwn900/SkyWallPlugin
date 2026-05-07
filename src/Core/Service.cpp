#include "WP/Core/Service.h"
#include "WP/Physics/SurfaceScanner.h"
#include "WP/Physics/LocomotionController.h"
#include "WP/Hooks/InputHandler.h"
#include "WP/Debug/DebugDraw.h"
#include "SKSE/SKSE.h"

namespace WP::Core
{
    Service& Service::Get()
    {
        static Service instance;
        return instance;
    }

    void Service::OnSKSELoad()
    {
        SKSE::log::info("WallWalkService: SKSE load phase");

        LoadSettings(_settings);

        if (!_scanner)
            _scanner = std::make_unique<Physics::SurfaceScanner>();
        if (!_controller)
            _controller = std::make_unique<Physics::LocomotionController>();

        _scanner->Initialize();
        _controller->Initialize();

        SKSE::log::info("WallWalkService: Initialized successfully");
    }

    void Service::OnDataLoaded()
    {
        SKSE::log::info("WallWalkService: Data loaded");

        auto* player = GetPlayer();
        if (player)
        {
            SKSE::log::info("WallWalkService: Player detected");
        }
        else
        {
            SKSE::log::warn("WallWalkService: Player not found at data load");
        }

        WP::Debug::DebugDraw::Get().SetEnabled(_settings.debugDraw);
    }

    RE::PlayerCharacter* Service::GetPlayer()
    {
        if (auto* player = RE::PlayerCharacter::GetSingleton())
            return player;

        return nullptr;
    }

    void Service::OnFrame(float deltaTime)
    {
        if (!_enabled)
            return;

        auto* player = GetPlayer();
        if (!player)
            return;

        if (_settings.debugDraw)
            WP::Debug::DebugDraw::Get().DrawFrame(player, deltaTime);

        if (_settings.disableInCombat && player->IsInCombat())
            return;

        _scanner->Scan(player, deltaTime);
    }

    void Service::OnInputEvent(std::uint32_t keyCode, bool pressed)
    {
        if (!pressed)
            return;

        if (keyCode == static_cast<std::uint32_t>(_settings.hotkey))
        {
            ProcessHotkey();
        }
    }

    void Service::ProcessHotkey()
    {
        _armed = !_armed;
        SKSE::log::info("WallWalk mode {}", _armed ? "ARMED" : "DISARMED");
    }

    void Service::Enable(bool enabled)
    {
        _enabled = enabled;
        if (!enabled && _armed)
        {
            _armed = false;
        }
        SKSE::log::info("WallWalkService: {}", enabled ? "Enabled" : "Disabled");
    }
}
