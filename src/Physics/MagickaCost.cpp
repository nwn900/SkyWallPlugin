#include "WP/Physics/MagickaCost.h"
#include "RE/P/PlayerCharacter.h"
#include "RE/A/ActorValueOwner.h"
#include "RE/A/ActorValues.h"
#include "SKSE/SKSE.h"

namespace WP::Physics
{
    bool MagickaCost::CanAttach(RE::PlayerCharacter* player, const Core::RuntimeConfig& cfg)
    {
        if (!cfg.enableMagickaCost) return true;

        auto* avo = player->AsActorValueOwner();
        if (!avo) return true;

        float current = avo->GetActorValue(RE::ActorValue::kMagicka);
        return current >= cfg.minMagickaToAttach;
    }

    bool MagickaCost::Consume(RE::PlayerCharacter* player, const Core::RuntimeConfig& cfg, float deltaTime)
    {
        if (!cfg.enableMagickaCost) return true;

        auto* avo = player->AsActorValueOwner();
        if (!avo) return true;

        float cost = cfg.magickaPerSecond * deltaTime;
        float current = avo->GetActorValue(RE::ActorValue::kMagicka);

        if (current < cost)
        {
            _emptyTimer += deltaTime;
            if (_emptyTimer > cfg.emptyMagickaGracePeriod)
            {
                SKSE::log::info("MagickaCost: grace period expired, detaching");
                return false;
            }
            return true;
        }

        avo->DamageActorValue(RE::ActorValue::kMagicka, cost);
        _emptyTimer = 0.0f;
        return true;
    }

    void MagickaCost::Reset()
    {
        _emptyTimer = 0.0f;
    }
}
