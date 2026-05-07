#include "SKSE/SKSE.h"
#include "WP/Plugin.h"
#include "WP/Core/Service.h"
#include "WP/Core/Settings.h"
#include "WP/Hooks/InputHandler.h"
#include "WP/Debug/DebugDraw.h"

#include <chrono>
#include <Windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

namespace
{
    UINT g_timerId = 0;

    void FrameHandler()
    {
        static int callCount = 0;
        static auto lastTime = std::chrono::high_resolution_clock::now();
        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;

        if (dt > 0.0f && dt < 0.5f)
        {
            WP::Core::Service::Get().OnFrame(dt);
        }

        callCount++;
        if (callCount == 1 || callCount % 300 == 0)
        {
            SKSE::log::info("FrameHandler: call {} dt={:.4f}s armed={} enabled={}",
                callCount, dt,
                WP::Core::Service::Get().IsArmed(),
                WP::Core::Service::Get().IsEnabled());
        }
    }

    void CALLBACK FrameTimer(UINT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR)
    {
        SKSE::GetTaskInterface()->AddTask(FrameHandler);
    }

    void OnSKSEMessage(SKSE::MessagingInterface::Message* msg)
    {
        if (!msg)
            return;

        switch (msg->type)
        {
        case SKSE::MessagingInterface::kPostLoad:
            {
                SKSE::log::info("{} v{} loaded", WP::Plugin::Name, WP::Plugin::Version);
                WP::Core::Service::Get().OnSKSELoad();
                break;
            }
        case SKSE::MessagingInterface::kDataLoaded:
            {
                SKSE::log::info("Data loaded - initializing services");
                WP::Core::Service::Get().OnDataLoaded();
                WP::Hooks::InputHandler::Get().Install();

                if (!g_timerId)
                {
                    g_timerId = timeSetEvent(16, 1, FrameTimer, 0, TIME_PERIODIC);
                    SKSE::log::info("Frame timer started (id={})", g_timerId);
                }
                break;
            }
        case SKSE::MessagingInterface::kPostPostLoad:
            {
                SKSE::log::info("Post-post load - all plugins should be ready");
                break;
            }
        default:
            break;
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SKSE::Init(skse);

    SKSE::log::info("{} v{} initializing", WP::Plugin::Name, WP::Plugin::Version);
    SKSE::log::info("Author: {}", WP::Plugin::Author);

    auto* messaging = SKSE::GetMessagingInterface();
    if (!messaging)
    {
        SKSE::log::error("Failed to get SKSE messaging interface");
        return false;
    }

    messaging->RegisterListener(OnSKSEMessage);

    SKSE::log::info("{} v{} loaded successfully", WP::Plugin::Name, WP::Plugin::Version);
    return true;
}
