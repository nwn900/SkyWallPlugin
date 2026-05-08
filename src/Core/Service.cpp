#include "WP/Core/Service.h"
#include "WP/Core/Settings.h"
#include "WP/Core/WallWalkTypes.h"
#include "WP/Physics/SurfaceScanner.h"
#include "WP/Physics/LocomotionController.h"
#include "WP/Physics/MagickaCost.h"
#include "WP/Hooks/InputHandler.h"
#include "WP/Debug/DebugDraw.h"
#include "WP/Animation/AnimationBridge.h"
#include "WP/Camera/CameraMediator.h"
#include "RE/P/PlayerCharacter.h"
#include "RE/A/Actor.h"
#include "RE/A/ActorState.h"
#include "RE/S/SendHUDMessage.h"
#include "RE/U/UI.h"
#include "SKSE/SKSE.h"

namespace
{
    constexpr std::uint32_t kSerializationSignature = 'WPLG';
    constexpr std::uint32_t kSerializationVersion = 1;

    void SaveAttachState(SKSE::SerializationInterface* a_intfc, const WP::Core::AttachState& state)
    {
        if (!a_intfc->OpenRecord(kSerializationSignature, kSerializationVersion)) return;
        a_intfc->WriteRecordData(&state.mode, sizeof(state.mode));
        a_intfc->WriteRecordData(&state.hotkeyArmed, sizeof(state.hotkeyArmed));
        a_intfc->WriteRecordData(&state.transitionAlpha, sizeof(state.transitionAlpha));
        a_intfc->WriteRecordData(&state.noSurfaceTime, sizeof(state.noSurfaceTime));
        a_intfc->WriteRecordData(&state.adherence, sizeof(state.adherence));
    }

    void LoadAttachState(SKSE::SerializationInterface* a_intfc, WP::Core::AttachState& state)
    {
        std::uint32_t type, version, length;
        while (a_intfc->GetNextRecordInfo(type, version, length))
        {
            if (type != kSerializationSignature) continue;
            WP::Core::WallWalkMode mode;
            bool hotkeyArmed;
            float transitionAlpha, noSurfaceTime, adherence;
            a_intfc->ReadRecordData(&mode, sizeof(mode));
            a_intfc->ReadRecordData(&hotkeyArmed, sizeof(hotkeyArmed));
            a_intfc->ReadRecordData(&transitionAlpha, sizeof(transitionAlpha));
            a_intfc->ReadRecordData(&noSurfaceTime, sizeof(noSurfaceTime));
            a_intfc->ReadRecordData(&adherence, sizeof(adherence));
            if (WP::Core::IsAttached(mode) || hotkeyArmed)
            {
                state.mode = mode;
                state.hotkeyArmed = hotkeyArmed;
                state.transitionAlpha = transitionAlpha;
                state.noSurfaceTime = noSurfaceTime;
                state.adherence = adherence;
            }
            else
            {
                state.mode = WP::Core::WallWalkMode::kGrounded;
                state.hotkeyArmed = false;
                state.transitionAlpha = state.noSurfaceTime = state.adherence = 0.0f;
            }
        }
    }
}

namespace WP::Core
{
    Service& Service::Get() { static Service instance; return instance; }

    void Service::OnSKSELoad()
    {
        SKSE::log::info("WallWalkService: SKSE load phase");
        LoadSettings(_settings);
        if (!_scanner) _scanner = std::make_unique<Physics::SurfaceScanner>();
        if (!_controller) _controller = std::make_unique<Physics::LocomotionController>();
        if (!_magicka) _magicka = std::make_unique<Physics::MagickaCost>();
        _scanner->Initialize();
        _controller->Initialize();

        auto* serialization = SKSE::GetSerializationInterface();
        if (serialization)
        {
            serialization->SetUniqueID(kSerializationSignature);
            serialization->SetSaveCallback([](SKSE::SerializationInterface* a_intfc) {
                SaveAttachState(a_intfc, Service::Get().GetAttachState());
            });
            serialization->SetLoadCallback([](SKSE::SerializationInterface* a_intfc) {
                LoadAttachState(a_intfc, Service::Get().GetAttachState());
            });
        }
        SKSE::log::info("WallWalkService: Initialized");
    }

    void Service::OnDataLoaded()
    {
        SKSE::log::info("WallWalkService: Data loaded");
        auto* player = GetPlayer();
        if (player) SKSE::log::info("WallWalkService: Player detected");
        else SKSE::log::warn("WallWalkService: Player not found");
        Debug::DebugDraw::Get().SetEnabled(_settings.debugDraw);
    }

    RE::PlayerCharacter* Service::GetPlayer() { return RE::PlayerCharacter::GetSingleton(); }

    void Service::OnFrame(float deltaTime)
    {
        if (!_enabled) return;
        auto* player = GetPlayer();
        if (!player) return;

        auto* ui = RE::UI::GetSingleton();
        if (ui && ui->GameIsPaused())
        {
            if (Core::IsAttached(_attach.mode))
                _controller->DetachToAir(player, _attach);
            return;
        }

        if (Core::ShouldSuspendForState(player))
        {
            if (Core::IsAttached(_attach.mode))
                _controller->DetachToAir(player, _attach);
            Animation::AnimationBridge::Get().PushState(player, _attach);
            return;
        }

        if (_settings.disableInCombat && player->IsInCombat())
        {
            if (Core::IsAttached(_attach.mode))
                _controller->DetachToAir(player, _attach);
            Animation::AnimationBridge::Get().PushState(player, _attach);
            return;
        }

        if (!_attach.hotkeyArmed)
        {
            if (Core::IsAttached(_attach.mode))
                _controller->DetachToAir(player, _attach);
            Animation::AnimationBridge::Get().PushState(player, _attach);
            return;
        }

        bool hasSurface = _scanner->Scan(player, _attach, _settings);

        if (!Core::IsAttached(_attach.mode))
        {
            if (hasSurface)
            {
                auto& candidate = _scanner->GetPrimary();
                if (Core::IsWallOrCeiling(candidate) && _magicka->CanAttach(player, _settings))
                    _controller->BeginAttach(player, _attach, candidate);
            }
        }
        else
        {
            if (!hasSurface)
            {
                _attach.noSurfaceTime += deltaTime;
                if (_attach.noSurfaceTime > _settings.detachGraceTime)
                    _controller->DetachToAir(player, _attach);
            }
            else
            {
                _attach.noSurfaceTime = 0.0f;
            }
            if (!_magicka->Consume(player, _settings, deltaTime))
                _controller->DetachToAir(player, _attach);
        }

        Animation::AnimationBridge::Get().PushState(player, _attach);
        Camera::CameraMediator::Get().Update(player, _attach, deltaTime);

        if (_settings.debugDraw)
            Debug::DebugDraw::Get().DrawFrame(player, _attach, _scanner->GetPrimary());
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
        _magicka->Reset();
        SKSE::log::info("WallWalk mode {}", _attach.hotkeyArmed ? "ARMED" : "DISARMED");
        RE::SendHUDMessage::ShowHUDMessage(
            _attach.hotkeyArmed ? "Wall Walk: ARMED" : "Wall Walk: DISARMED", nullptr, true);
    }

    void Service::Enable(bool enabled)
    {
        _enabled = enabled;
        if (!enabled) _attach.hotkeyArmed = false;
    }
}
