#include "WP/Hooks/InputHandler.h"
#include "WP/Core/Service.h"
#include "WP/Core/Settings.h"
#include "SKSE/SKSE.h"

namespace WP::Hooks
{
    InputHandler& InputHandler::Get()
    {
        static InputHandler instance;
        return instance;
    }

    void InputHandler::Install()
    {
        if (_installed)
            return;

        auto& settings = Core::Service::Get().GetSettings();
        _hotkey = static_cast<std::uint32_t>(settings.hotkey);

        auto* input = RE::BSInputDeviceManager::GetSingleton();
        if (!input)
        {
            SKSE::log::error("InputHandler: Failed to get BSInputDeviceManager");
            return;
        }

        input->AddEventSink(&Get());

        _installed = true;
        SKSE::log::info("InputHandler: Installed, hotkey = {}", _hotkey);
    }

    RE::BSEventNotifyControl InputHandler::ProcessEvent(
        RE::InputEvent* const* evn,
        RE::BSTEventSource<RE::InputEvent*>*)
    {
        if (!evn)
            return RE::BSEventNotifyControl::kContinue;

        for (auto* e = *evn; e; e = e->next)
        {
            if (e->eventType != RE::INPUT_EVENT_TYPE::kButton)
                continue;

            auto* buttonEvent = e->AsButtonEvent();
            if (!buttonEvent)
                continue;

            if (buttonEvent->IsDown() &&
                buttonEvent->GetIDCode() == _hotkey &&
                _hotkey != 0)
            {
                Core::Service::Get().OnInputEvent(
                    buttonEvent->GetIDCode(), true);
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
}
