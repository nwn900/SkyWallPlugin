#include "SKSE/SKSE.h"
#include "WP/Plugin.h"
#include "WP/Core/Service.h"
#include "WP/Core/Settings.h"
#include "WP/Hooks/InputHandler.h"
#include "WP/Debug/DebugDraw.h"

namespace
{
    void InitializeLogging()
    {
        auto path = SKSE::log::log_directory();
        if (!path)
            return;

        *path /= std::string(WP::Plugin::Name) + ".log";
        auto file = *path;

        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(file.string(), true);
        auto log = std::make_shared<spdlog::logger>("WallWalk", std::move(sink));

        log->set_level(spdlog::level::info);
        log->flush_on(spdlog::level::info);

        spdlog::set_default_logger(std::move(log));
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
    InitializeLogging();

    SKSE::log::info("{} v{} initializing", WP::Plugin::Name, WP::Plugin::Version);
    SKSE::log::info("Author: {}", WP::Plugin::Author);

    SKSE::Init(skse);

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
