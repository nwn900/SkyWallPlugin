#include "WP/Compatibility/PluginDetector.h"
#include "SKSE/SKSE.h"

#include <Windows.h>

namespace WP::Compatibility
{
    PluginDetector& PluginDetector::Get()
    {
        static PluginDetector instance;
        return instance;
    }

    bool PluginDetector::HasPlugin(const char* name)
    {
        return GetModuleHandleA(name) != nullptr;
    }

    void PluginDetector::DetectAll()
    {
        _plugins.tdm            = HasPlugin("TrueDirectionalMovement.dll");
        _plugins.precision      = HasPlugin("Precision.dll");
        _plugins.smoothCam      = HasPlugin("SmoothCam.dll");
        _plugins.oar            = HasPlugin("OpenAnimationReplacer.dll");
        _plugins.bdi            = HasPlugin("BehaviorDataInjector.dll");
        _plugins.dca            = HasPlugin("DynamicCollisionAdjustment.dll");
        _plugins.improvedCamera = HasPlugin("ImprovedCameraSE.dll");
        _plugins.valhalla       = HasPlugin("ValhallaCombat.dll");
        _plugins.tkDodge        = HasPlugin("TKDodgeRE.dll");
        _plugins.mco            = HasPlugin("AttackDXP.dll");
        _plugins.scar           = HasPlugin("SCAR.dll");
        _plugins.raySense       = HasPlugin("RaySense.dll");

        SKSE::log::info("PluginDetector: TDM={} Precision={} SmoothCam={} OAR={} BDI={} DCA={} Valhalla={} TKDodge={} MCO={} SCAR={} RaySense={}",
            _plugins.tdm, _plugins.precision, _plugins.smoothCam, _plugins.oar,
            _plugins.bdi, _plugins.dca, _plugins.valhalla, _plugins.tkDodge,
            _plugins.mco, _plugins.scar, _plugins.raySense);
    }
}
