#include "WP/Core/Service.h"
#include "WP/Core/Settings.h"
#include "WP/Core/WallWalkTypes.h"
#include "WP/Physics/SurfaceScanner.h"
#include "WP/Physics/LocomotionController.h"
#include "WP/Physics/MagickaCost.h"
#include "WP/Hooks/InputHandler.h"
#include "WP/Debug/DebugDraw.h"
#include "RE/P/PlayerCharacter.h"
#include "RE/A/Actor.h"
#include "RE/A/ActorState.h"
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
        if (!_magicka)
            _magicka = std::make_unique<Physics::MagickaCost>();

        _scanner->Initialize();
        _controller->Initialize();

        SKSE::log::info("WallWalkService: Initialized");
    }

    void Service::OnDataLoaded()
    {
        SKSE::log::info("WallWalkService: Data loaded");

        auto* player = GetPlayer();
        if (player)
            SKSE::log::info("WallWalkService: Player detected");
        else
            SKSE::log::warn("WallWalkService: Player not found");

        Debug::DebugDraw::Get().SetEnabled(_settings.debugDraw);
    }

    RE::PlayerCharacter* Service::GetPlayer()
    {
        return RE::PlayerCharacter::GetSingleton();
    }

    void Service::OnFrame(float deltaTime)
    {
        if (!_enabled) return;

        auto* player = GetPlayer();
        if (!player) return;

        if (Core::ShouldSuspendForState(player))
        {
            if (Core::IsAttached(_attach.mode))
                _controller->DetachToAir(player, _attach);
            return;
        }

        if (_settings.disableInCombat && player->IsInCombat())
        {
            if (Core::IsAttached(_attach.mode))
                _controller->DetachToAir(player, _attach);
            return;
        }

        if (!_attach.hotkeyArmed)
        {
            if (Core::IsAttached(_attach.mode))
                _controller->DetachToAir(player, _attach);
            return;
        }

        bool hasSurface = _scanner->Scan(player, _attach, _settings);

        if (_settings.debugDraw)
            Debug::DebugDraw::Get().DrawFrame(player, _attach, _scanner->GetPrimary());

        if (!Core::IsAttached(_attach.mode))
        {
            if (hasSurface)
            {
                auto& candidate = _scanner->GetPrimary();
                if (Core::IsWallOrCeiling(candidate))
                {
                    if (_magicka->CanAttach(player, _settings))
                    {
                        _controller->BeginAttach(player, _attach, candidate);
                    }
                }
            }
            return;
        }

        if (Core::IsAttached(_attach.mode))
        {
            if (hasSurface)
            {
                _attach.noSurfaceTime = 0.0f;

                auto& next = _scanner->GetPrimary();
                if (next.valid && Core::IsWallOrCeiling(next))
                {
                    if (_attach.mode == Core::WallWalkMode::kAttachedWall && next.isCeiling)
                    {
                        _attach.mode = Core::WallWalkMode::kTransition;
                    }
                    else if (_attach.mode == Core::WallWalkMode::kAttachedCeiling && !next.isCeiling)
                    {
                        _attach.mode = Core::WallWalkMode::kTransition;
                    }

                    float dotNormals = _attach.primary.normalWS.Dot(next.normalWS);
                    if (dotNormals < 0.9f && _attach.mode != Core::WallWalkMode::kTransition)
                    {
                        _attach.mode = Core::WallWalkMode::kTransition;
                    }

                    float blendSpeed = 5.0f;
                    _attach.transitionAlpha += deltaTime * blendSpeed;
                    if (_attach.transitionAlpha > 1.0f) _attach.transitionAlpha = 1.0f;

                    RE::NiPoint3 blendedUp = _attach.primary.normalWS +
                        (next.normalWS - _attach.primary.normalWS) * _attach.transitionAlpha;
                    blendedUp.Unitize();
                    _attach.desiredUpWS = blendedUp;
                    _attach.desiredGravityWS = blendedUp * -1.0f;
                    _attach.primary = next;

                    if (_attach.transitionAlpha >= 1.0f)
                    {
                        if (_attach.mode == Core::WallWalkMode::kTransition)
                            _attach.mode = next.isCeiling ?
                                Core::WallWalkMode::kAttachedCeiling : Core::WallWalkMode::kAttachedWall;
                    }
                }
            }
            else
            {
                _attach.noSurfaceTime += deltaTime;
                if (_attach.noSurfaceTime > _settings.detachGraceTime)
                {
                    SKSE::log::info("WallWalkService: Lost surface for {:.2f}s, detaching", _attach.noSurfaceTime);
                    _controller->DetachToAir(player, _attach);
                    return;
                }
            }

            _controller->Update(player, _attach, _settings, deltaTime);

            if (!_magicka->Consume(player, _settings, deltaTime))
            {
                _controller->DetachToAir(player, _attach);
            }
        }
    }

    void Service::OnInputEvent(std::uint32_t keyCode, bool pressed)
    {
        if (!pressed) return;
        if (keyCode == static_cast<std::uint32_t>(_settings.hotkey))
            ProcessHotkey();
    }

    void Service::ProcessHotkey()
    {
        _attach.hotkeyArmed = !_attach.hotkeyArmed;
        SKSE::log::info("WallWalk mode {}", _attach.hotkeyArmed ? "ARMED" : "DISARMED");
        _magicka->Reset();
    }

    void Service::Enable(bool enabled)
    {
        _enabled = enabled;
        if (!enabled) _attach.hotkeyArmed = false;
        SKSE::log::info("WallWalkService: {}", enabled ? "Enabled" : "Disabled");
    }
}
