#include "WP/UI/MenuIntegration.h"
#include "WP/Core/Service.h"
#include "SKSE/SKSE.h"

#include <Windows.h>

namespace WP::UI
{
    using RenderFn = void(__stdcall*)();
    using AddSectionItemFn = void(__cdecl*)(const char*, RenderFn);

    using IgCheckboxFn = bool(__cdecl*)(const char*, bool*);
    using IgSliderFloatFn = bool(__cdecl*)(const char*, float*, float, float, const char*, int);
    using IgTextFn = void(__cdecl*)(const char*, ...);

    static HMODULE g_frameworkModule = nullptr;
    static IgCheckboxFn g_igCheckbox = nullptr;
    static IgSliderFloatFn g_igSliderFloat = nullptr;
    static IgTextFn g_igText = nullptr;

    static bool InitImGuiFunctions()
    {
        if (g_igCheckbox) return true;

        g_frameworkModule = GetModuleHandleA("SKSEMenuFramework.dll");
        if (!g_frameworkModule) return false;

        g_igCheckbox = reinterpret_cast<IgCheckboxFn>(
            GetProcAddress(g_frameworkModule, "igCheckbox"));
        g_igSliderFloat = reinterpret_cast<IgSliderFloatFn>(
            GetProcAddress(g_frameworkModule, "igSliderFloat"));
        g_igText = reinterpret_cast<IgTextFn>(
            GetProcAddress(g_frameworkModule, "igText"));

        return g_igCheckbox && g_igSliderFloat && g_igText;
    }

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
        if (!InitImGuiFunctions()) return;
        auto& cfg = Core::Service::Get().GetSettings();

        bool enabled = Core::Service::Get().IsEnabled();
        if (g_igCheckbox("Enable Plugin", &enabled))
            Core::Service::Get().Enable(enabled);

        const char* keyName = cfg.hotkey == 21 ? "Y" : cfg.hotkey == 0 ? "None" : "?";
        g_igText("Hotkey: %s (set in WallWalkSKSE.ini)", keyName);
        g_igCheckbox("Disable In Combat", &cfg.disableInCombat);
        g_igCheckbox("Log State Changes", &cfg.debugLogState);
    }

    static void __stdcall RenderMovement()
    {
        if (!InitImGuiFunctions()) return;
        auto& cfg = Core::Service::Get().GetSettings();

        g_igSliderFloat("Attach Distance", &cfg.maxAttachDistance, 100.0f, 10000.0f, "%.0f", 0);
        g_igSliderFloat("Ceiling Attach Dist", &cfg.maxCeilingAttachDistance, 100.0f, 10000.0f, "%.0f", 0);
        g_igSliderFloat("Min Attach Speed", &cfg.minAttachSpeed, 0.0f, 500.0f, "%.0f", 0);
        g_igSliderFloat("Adhesion Strength", &cfg.adhesionStrength, 100.0f, 3000.0f, "%.0f", 0);
        g_igSliderFloat("Gravity Magnitude", &cfg.gravityMagnitude, 100.0f, 5000.0f, "%.0f", 0);
        g_igSliderFloat("Edge Blend Time", &cfg.edgeTransferBlendTime, 0.02f, 1.0f, "%.2f", 0);
        g_igSliderFloat("Detach Grace Time", &cfg.detachGraceTime, 0.05f, 2.0f, "%.2f", 0);
    }

    static void __stdcall RenderMagicka()
    {
        if (!InitImGuiFunctions()) return;
        auto& cfg = Core::Service::Get().GetSettings();

        g_igCheckbox("Use Magicka Cost", &cfg.enableMagickaCost);
        g_igSliderFloat("Magicka Per Second", &cfg.magickaPerSecond, 0.0f, 50.0f, "%.1f", 0);
        g_igSliderFloat("Min Magicka To Attach", &cfg.minMagickaToAttach, 0.0f, 100.0f, "%.0f", 0);
        g_igSliderFloat("Grace On Empty (s)", &cfg.emptyMagickaGracePeriod, 0.0f, 5.0f, "%.1f", 0);
    }

    static void __stdcall RenderDebug()
    {
        if (!InitImGuiFunctions()) return;
        auto& cfg = Core::Service::Get().GetSettings();
        auto& service = Core::Service::Get();

        g_igCheckbox("Debug Draw", &cfg.debugDraw);
        g_igText("Armed: %s", service.IsArmed() ? "YES" : "NO");
        g_igText("Enabled: %s", service.IsEnabled() ? "YES" : "NO");
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
