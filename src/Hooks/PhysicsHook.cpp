#include "WP/Hooks/PhysicsHook.h"
#include "WP/Core/Service.h"
#include "WP/Core/WallWalkTypes.h"
#include "RE/P/PlayerCharacter.h"
#include "RE/H/hkVector4.h"
#include "RE/B/bhkCharacterController.h"
#include "REL/Relocation.h"
#include "SKSE/SKSE.h"

namespace WP::Hooks
{
    using CheckSupportFn = void(*)(RE::bhkCharacterController*);

    static CheckSupportFn g_originalCheckSupport = nullptr;
    static RE::bhkCharacterController* g_playerController = nullptr;

    static void CheckSupportHook(RE::bhkCharacterController* self)
    {
        g_originalCheckSupport(self);

        if (!g_playerController || self != g_playerController)
            return;

        auto& state = WP::Core::Service::Get().GetAttachState();
        if (!WP::Core::IsAttached(state.mode))
            return;

        RE::NiPoint3 wallNormal(
            self->surfaceInfo.surfaceNormal.quad.m128_f32[0],
            self->surfaceInfo.surfaceNormal.quad.m128_f32[1],
            self->surfaceInfo.surfaceNormal.quad.m128_f32[2]
        );
        wallNormal.Unitize();

        auto& cfg = WP::Core::Service::Get().GetSettings();

        state.desiredUpWS = state.desiredUpWS + (wallNormal - state.desiredUpWS) * 0.3f;
        state.desiredUpWS.Unitize();
        state.desiredGravityWS = state.desiredUpWS * -1.0f;

        RE::NiPoint3 up = state.desiredUpWS;

        self->up = RE::hkVector4(up);
        self->supportNorm = RE::hkVector4(up);

        self->surfaceInfo.supportedState.reset();
        self->surfaceInfo.supportedState.set(RE::hkpSurfaceInfo::SupportedState::kSupported);
        self->surfaceInfo.surfaceNormal = RE::hkVector4(up);

        self->gravity = cfg.gravityMagnitude;

        self->flags.set(RE::CHARACTER_FLAGS::kSupport);
        self->flags.set(RE::CHARACTER_FLAGS::kCheckSupport);
        self->flags.set(RE::CHARACTER_FLAGS::kCanPitch);
        self->flags.set(RE::CHARACTER_FLAGS::kCanRoll);

        RE::NiPoint3 vel(
            self->outVelocity.quad.m128_f32[0],
            self->outVelocity.quad.m128_f32[1],
            self->outVelocity.quad.m128_f32[2]
        );

        float intoWall = vel.Dot(up);
        RE::NiPoint3 tangent = vel - up * intoWall;
        RE::NiPoint3 adhesion = up * (-300.0f * 0.016f);

        self->outVelocity = RE::hkVector4(tangent + adhesion);

        static int fc = 0;
        if (++fc >= 300)
        {
            fc = 0;
            SKSE::log::info("[PHYSX] up=({:.2f},{:.2f},{:.2f}) vel=({:.1f},{:.1f},{:.1f})",
                up.x, up.y, up.z, tangent.x, tangent.y, tangent.z);
        }
    }

    PhysicsHook& PhysicsHook::Get()
    {
        static PhysicsHook instance;
        return instance;
    }

    void PhysicsHook::Install()
    {
        if (_installed) return;

        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player) return;

        auto* controller = player->GetCharController();
        if (!controller) return;

        _playerController = controller;
        g_playerController = controller;

        auto* vtable = *reinterpret_cast<uintptr_t**>(controller);

        g_originalCheckSupport = reinterpret_cast<CheckSupportFn>(vtable[0x0D]);

        REL::safe_write(reinterpret_cast<std::uintptr_t>(&vtable[0x0D]),
            reinterpret_cast<uintptr_t>(CheckSupportHook));

        _installed = true;
        SKSE::log::info("PhysicsHook: CheckSupportImpl vtable hook installed");
    }
}
