#include "WP/Core/Settings.h"
#include "SKSE/SKSE.h"

#include <fstream>
#include <filesystem>

namespace WP::Core
{
    namespace fs = std::filesystem;

    static fs::path GetSettingsPath()
    {
        auto path = SKSE::log::log_directory();
        if (!path)
            return {};

        return path->parent_path() / "SKSE" / "Plugins" / "WallWalkSKSE.ini";
    }

    bool LoadSettings(RuntimeSettings& outSettings)
    {
        auto path = GetSettingsPath();
        if (!fs::exists(path))
        {
            SKSE::log::info("No settings file found at {}, using defaults", path.string());
            SaveSettings(outSettings);
            return false;
        }

        try
        {
            std::ifstream file(path);
            if (!file.is_open())
                return false;

            std::string line;
            while (std::getline(file, line))
            {
                if (line.empty() || line[0] == '#' || line[0] == ';')
                    continue;

                auto eq = line.find('=');
                if (eq == std::string::npos)
                    continue;

                auto key = line.substr(0, eq);
                auto value = line.substr(eq + 1);

                while (!key.empty() && (key.back() == ' ' || key.back() == '\t'))
                    key.pop_back();
                while (!value.empty() && (value.front() == ' ' || value.front() == '\t'))
                    value.erase(0, 1);

                if (key == "Hotkey")
                    outSettings.hotkey = std::stoi(value);
                else if (key == "EnableMagickaCost")
                    outSettings.enableMagickaCost = (value == "1" || value == "true");
                else if (key == "MagickaPerSecond")
                    outSettings.magickaPerSecond = std::stof(value);
                else if (key == "MaxAttachDistance")
                    outSettings.maxAttachDistance = std::stof(value);
                else if (key == "MaxCeilingAttachDistance")
                    outSettings.maxCeilingAttachDistance = std::stof(value);
                else if (key == "MinAttachSpeed")
                    outSettings.minAttachSpeed = std::stof(value);
                else if (key == "EdgeTransferBlendTime")
                    outSettings.edgeTransferBlendTime = std::stof(value);
                else if (key == "DisableInCombat")
                    outSettings.disableInCombat = (value == "1" || value == "true");
                else if (key == "DebugDraw")
                    outSettings.debugDraw = (value == "1" || value == "true");
                else if (key == "DebugLogState")
                    outSettings.debugLogState = (value == "1" || value == "true");
            }

            SKSE::log::info("Settings loaded from {}", path.string());
            return true;
        }
        catch (const std::exception& e)
        {
            SKSE::log::error("Failed to load settings: {}", e.what());
            return false;
        }
    }

    bool SaveSettings(const RuntimeSettings& settings)
    {
        auto path = GetSettingsPath();
        auto parent = path.parent_path();
        if (!fs::exists(parent))
            fs::create_directories(parent);

        try
        {
            std::ofstream file(path);
            if (!file.is_open())
                return false;

            file << "# WallWalkSKSE Settings\n";
            file << "Hotkey=" << settings.hotkey << "\n";
            file << "EnableMagickaCost=" << (settings.enableMagickaCost ? "1" : "0") << "\n";
            file << "MagickaPerSecond=" << settings.magickaPerSecond << "\n";
            file << "MaxAttachDistance=" << settings.maxAttachDistance << "\n";
            file << "MaxCeilingAttachDistance=" << settings.maxCeilingAttachDistance << "\n";
            file << "MinAttachSpeed=" << settings.minAttachSpeed << "\n";
            file << "EdgeTransferBlendTime=" << settings.edgeTransferBlendTime << "\n";
            file << "DisableInCombat=" << (settings.disableInCombat ? "1" : "0") << "\n";
            file << "DebugDraw=" << (settings.debugDraw ? "1" : "0") << "\n";
            file << "DebugLogState=" << (settings.debugLogState ? "1" : "0") << "\n";

            SKSE::log::info("Settings saved to {}", path.string());
            return true;
        }
        catch (const std::exception& e)
        {
            SKSE::log::error("Failed to save settings: {}", e.what());
            return false;
        }
    }
}
