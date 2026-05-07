#include "SKSE/Impl/PCH.h"

using namespace std::literals;

#include "REL/Relocation.h"
#include "SKSE/SKSE.h"

SKSEPluginInfo(
    .Version = REL::Version{ 0, 1, 0, 0 },
    .Name = "WallWalkSKSE"sv,
    .Author = "nwn900"sv,
    .SupportEmail = ""sv,
    .StructCompatibility = SKSE::StructCompatibility::Independent,
    .RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary,
    .MinimumSKSEVersion = REL::Version{ 0, 0, 0, 0 }
)
