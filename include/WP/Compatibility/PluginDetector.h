#pragma once

namespace WP::Compatibility
{
    struct Plugins
    {
        bool tdm = false;
        bool precision = false;
        bool smoothCam = false;
        bool oar = false;
        bool bdi = false;
        bool dca = false;
        bool improvedCamera = false;
        bool valhalla = false;
        bool tkDodge = false;
        bool mco = false;
        bool scar = false;
        bool raySense = false;
    };

    class PluginDetector
    {
    public:
        static PluginDetector& Get();

        void DetectAll();
        const Plugins& GetPlugins() const { return _plugins; }

    private:
        PluginDetector() = default;
        ~PluginDetector() = default;

        Plugins _plugins;
        bool HasPlugin(const char* name);
    };
}
