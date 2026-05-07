#pragma once

#define PLUGIN_NAME "WallWalkSKSE"
#define PLUGIN_VERSION "0.1.0"
#define PLUGIN_AUTHOR "nwn900"

namespace WP
{
    class Plugin
    {
    public:
        static constexpr const char* Name = PLUGIN_NAME;
        static constexpr const char* Version = PLUGIN_VERSION;
        static constexpr const char* Author = PLUGIN_AUTHOR;
    };
}
