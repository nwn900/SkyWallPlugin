#pragma once

#include "WP/Core/WallWalkTypes.h"

namespace WP::Core
{
    bool LoadSettings(RuntimeConfig& outSettings);
    bool SaveSettings(const RuntimeConfig& settings);
}
