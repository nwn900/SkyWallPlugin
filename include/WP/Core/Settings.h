#pragma once

#include <cstdint>

namespace WP::Core
{
    struct RuntimeSettings
    {
        int hotkey = 0;
        bool enableMagickaCost = false;
        float magickaPerSecond = 5.0f;
        float maxAttachDistance = 96.0f;
        float maxCeilingAttachDistance = 96.0f;
        float minAttachSpeed = 30.0f;
        float edgeTransferBlendTime = 0.12f;
        bool disableInCombat = false;
        bool debugDraw = false;
        bool debugLogState = false;
    };

    bool LoadSettings(RuntimeSettings& outSettings);
    bool SaveSettings(const RuntimeSettings& settings);
}
