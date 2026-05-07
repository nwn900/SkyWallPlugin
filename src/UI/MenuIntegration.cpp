#include "WP/UI/MenuIntegration.h"
#include "WP/Core/Service.h"
#include "SKSE/SKSE.h"

#include <Windows.h>
#include <imgui/imgui.h>

namespace WP::UI
{
    using RenderFn = void(__stdcall*)();
    using AddSectionItemFn = void(__cdecl*)(const char*, RenderFn);

    static bool IsFrameworkInstalled()
    {
        return GetModuleHandleA("SKSEMenuFramework.dll") != nullptr;
    }

    static AddSectionItemFn GetAddSectionItem()
    {
        auto* mod = GetModuleHandleA("SKSEMenuFramework.dll");
        if (!mod) return nullptr;
        return reinterpret_cast<AddSectionItemFn>(
            GetProcAddress(mod, "AddSectionItem"));
    }

    static void __stdcall RenderGeneral()
    {
        auto& cfg = Core::Service::Get().GetSettings();

        bool enabled = Core::Service::Get().IsEnabled();
        if (ImGui::Checkbox("Enable Plugin", &enabled))
            Core::Service::Get().Enable(enabled);

        const char* keyName = "Unknown";
        if (cfg.hotkey == 21) keyName = "Y";
        else if (cfg.hotkey == 0) keyName = "None";
        ImGui::Text("Hotkey: %s (set in WallWalkSKSE.ini)", keyName);

        ImGui::Checkbox("Disable In Combat", &cfg.disableInCombat);
        ImGui::Checkbox("Log State Changes", &cfg.debugLogState);
    }

    static void __stdcall RenderMovement()
    {
        auto& cfg = Core::Service::Get().GetSettings();

        ImGui::SliderFloat("Attach Distance", &cfg.maxAttachDistance, 100.0f, 10000.0f, "%.0f");
        ImGui::SliderFloat("Ceiling Attach Distance", &cfg.maxCeilingAttachDistance, 100.0f, 10000.0f, "%.0f");
        ImGui::SliderFloat("Min Attach Speed", &cfg.minAttachSpeed, 0.0f, 500.0f, "%.0f");
        ImGui::SliderFloat("Adhesion Strength", &cfg.adhesionStrength, 100.0f, 3000.0f, "%.0f");
        ImGui::SliderFloat("Gravity Magnitude", &cfg.gravityMagnitude, 100.0f, 5000.0f, "%.0f");
        ImGui::SliderFloat("Edge Blend Time", &cfg.edgeTransferBlendTime, 0.02f, 1.0f, "%.2f");
        ImGui::SliderFloat("Detach Grace Time", &cfg.detachGraceTime, 0.05f, 2.0f, "%.2f");
    }

    static void __stdcall RenderMagicka()
    {
        auto& cfg = Core::Service::Get().GetSettings();

        ImGui::Checkbox("Use Magicka Cost", &cfg.enableMagickaCost);
        ImGui::SliderFloat("Magicka Per Second", &cfg.magickaPerSecond, 0.0f, 50.0f, "%.1f");
        ImGui::SliderFloat("Min Magicka To Attach", &cfg.minMagickaToAttach, 0.0f, 100.0f, "%.0f");
        ImGui::SliderFloat("Grace Period On Empty", &cfg.emptyMagickaGracePeriod, 0.0f, 5.0f, "%.1f");
    }

    static void __stdcall RenderDebug()
    {
        auto& cfg = Core::Service::Get().GetSettings();
        auto& service = Core::Service::Get();

        ImGui::Checkbox("Debug Draw", &cfg.debugDraw);
        ImGui::Text("Armed: %s", service.IsArmed() ? "YES" : "NO");
        ImGui::Text("Enabled: %s", service.IsEnabled() ? "YES" : "NO");
    }

    void RegisterSKSEFrameworkMenu()
    {
        if (!IsFrameworkInstalled())
        {
            SKSE::log::info("SKSE Menu Framework not installed, skipping menu registration");
            return;
        }

        auto* addFn = GetAddSectionItem();
        if (!addFn)
        {
            SKSE::log::error("SKSE Menu Framework loaded but AddSectionItem export not found");
            return;
        }

        addFn("Wall Walk/General", RenderGeneral);
        addFn("Wall Walk/Movement", RenderMovement);
        addFn("Wall Walk/Magicka", RenderMagicka);
        addFn("Wall Walk/Debug", RenderDebug);

        SKSE::log::info("SKSE Menu Framework: registered 4 menu pages");
    }
}
