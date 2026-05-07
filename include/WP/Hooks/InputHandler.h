#pragma once

#include <cstdint>

#include "RE/B/BSInputDeviceManager.h"
#include "RE/B/ButtonEvent.h"
#include "RE/B/BSTEvent.h"
#include "RE/I/InputEvent.h"

namespace WP::Hooks
{
    class InputHandler final : public RE::BSTEventSink<RE::InputEvent*>
    {
    public:
        static InputHandler& Get();

        void Install();
        std::uint32_t GetCurrentHotkey() const { return _hotkey; }
        void SetHotkey(std::uint32_t key) { _hotkey = key; }

        RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* evn, RE::BSTEventSource<RE::InputEvent*>*) override;

    private:
        InputHandler() = default;
        ~InputHandler() = default;
        InputHandler(const InputHandler&) = delete;
        InputHandler& operator=(const InputHandler&) = delete;

        std::uint32_t _hotkey = 0;
        bool _installed = false;
    };
}
