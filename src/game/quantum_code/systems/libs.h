// libs.h - the static helper classes (Quantum.XxxLib) the systems share. Only the
// methods reimplemented so far are declared; pending ones are listed as TODOs.
#pragma once

#include "quantum_code/frame.h"

namespace Quantum {

class GameModeSpec;
class BoltKingGameModeSpec;
class UltimateAbilitySpec;

class GameModeHandlerLib {
  public:
    // 0x7ba2d24
    static bool TryGetGameModeHandler(Frame* frame, GameModeHandler*& gameModeHandler) {
        return frame->TryGetPointerSingleton<GameModeHandler>(gameModeHandler);
    }
    // 0x7ba18a4 - The game mode spec named by the handler (FindAsset: a warning when
    // missing). False without a handler.
    static bool TryGetGameModeSpec(Frame* f, GameModeSpec*& gameModeSpec);
    // Generic TryGetGameModeSpec<T> (shared instance 0x4f150ec) - Silent lookup of the
    // handler's spec as T.
    template <class T> static bool TryGetGameModeSpec(Frame* f, T*& gameModeSpec) {
        gameModeSpec = nullptr;
        GameModeHandler handler{};
        if (!f->TryGetSingleton<GameModeHandler>(handler)) {
            return false;
        }
        return f->TryFindAssetInternal<T>(handler.gameModeSpecRef.Id, true, gameModeSpec);
    }
    // 0x7ba4004 - None (0) without a handler.
    static GAME_MODE_HANDLER_STATE GetGameModeState(Frame* f);
    // 0x7ba4094 - True in the main loop, and when there is no handler.
    static bool IsInActionPhase(Frame* f);
    // 0x7ba3cd0 - Zero without a handler.
    static FP GetCurrentStateElapsedTime(Frame* frame);
    // 0x7ba361c - A finish frame has been set.
    static bool IsGameModeFinished(Frame* frame);
    // 0x7ba36b4 - Sets the finish frame `delay` from now (whole frames, truncated) unless
    // one is already set.
    static void FinishGameMode(Frame* frame, FP delay);
};

class BoltKingGameModeLib {
  public:
    // 0x7ba1cb8
    static bool TryGetGameModeSpec(Frame* f, BoltKingGameModeSpec*& gameModeSpec) {
        return GameModeHandlerLib::TryGetGameModeSpec<BoltKingGameModeSpec>(f, gameModeSpec);
    }
};

class BoltLib {
  public:
    // 0x7bb6c70 - A gatherer's bolts, or its team's total when it has a team counter
    // (and a team); 0 without a BoltGathererActor.
    static std::int32_t GetCollectedBolts(Frame* frame, EntityRef entity);
    // 0x7bb71b4 - Sum over every gatherer of the team.
    static std::int32_t GetTeamCollectedBolts(Frame* frame, std::int32_t teamId);
    // 0x7bb6f48
    static void TryGiveBolts(Frame* f, EntityRef gathererEntity, std::int32_t boltsToGive) {
        TryChangeBolts(f, gathererEntity, boltsToGive);
    }
    // 0x7bb6ff0
    static void TryRemoveBolts(Frame* f, EntityRef gathererEntity, BoltGathererActor* boltGathererActor,
                               std::int32_t boltsToRemove) {
        TryChangeBolts(f, gathererEntity, boltGathererActor, -boltsToRemove);
    }
    // 0x7bb6f4c - For entities with a BoltGathererActor.
    static void TryChangeBolts(Frame* f, EntityRef gathererEntity, std::int32_t changeAmount);
    // 0x7bb6ff8 - Adds (or removes) bolts within 0..the spec's maximum (unlimited when it
    // is 0 or there is no spec) and reports what was actually collected or lost. Players
    // whose game mode has finished keep their bolts.
    static void TryChangeBolts(Frame* frame, EntityRef entity, BoltGathererActor* boltGathererActor,
                               std::int32_t changeAmount);
};

class HeroSpec;
class CallbackAction;

class BalancingLib {
  public:
    // 0x7cda4e0 (forwards to 0x7cda4e4) - The spec of a hero in the session's heroes
    // table. A missing table is logged and fails. Note: when the table has no entry for
    // the hero the original still reports success (with no spec).
    static bool TryGetHeroSpec(Frame* f, GENERATED_CHARACTER_ID heroID, HeroSpec*& heroSpec);
    // 0x7cda428 - The human players' average weapon level as a 0..1 ratio (levels 0..9);
    // false without a game mode handler.
    static bool TryGetHumanWeaponsLevelRatio(Frame* f, FP& ratio);
};

class WeaponLib {
  public:
    // 0x7c10e24 - The level of `weapon` in its owner's loadout; 0 when either entity is
    // gone, the owner has no loadout or the weapon is not in it.
    static std::int32_t GetWeaponLevel(Frame* frame, EntityRef owner, EntityRef weapon);
};

class CallbackActionLib {
  public:
    // 0x7cb0ebc - Runs the callback action asset on the entity (if the asset exists).
    static void Execute(AssetRefCallbackAction callbackActionAssetRef, Frame* frame, EntityRef entity);
};

class PlayerActorInputActionLib {
  public:
    // 0x7c0c678
    static bool FilterContains(PlayerActorInputActionFilter filter, PlayerActorInputAction action) {
        return ((static_cast<std::uint32_t>(filter) >> static_cast<std::uint32_t>(action)) & 1U) != 0;
    }
    // 0x7c04b7c - Neither a running ultimate nor a gadget in use forbids the action.
    static bool IsInputActionAllowed(Frame* frame, EntityRef entity, PlayerActorInputAction inputAction) {
        const bool byUltimate = IsAllowedByUltimate(frame, entity, inputAction);
        const bool byGadget = IsAllowedByGadget(frame, entity, inputAction);
        return byUltimate && byGadget;
    }
    // 0x7c0c684 - While an ultimate is active its spec's allowed actions decide (allowed
    // when the spec is missing).
    static bool IsAllowedByUltimate(Frame* frame, EntityRef entity, PlayerActorInputAction inputAction);
    // 0x7c0c7a8 - Same for a gadget in use.
    static bool IsAllowedByGadget(Frame* frame, EntityRef entity, PlayerActorInputAction inputAction);
};

class UnlockedFeaturesLib {
  public:
    // 0x7be70dc - The entity's unlocked features; entities without the manager get
    // `isUnlockedByDefault`.
    static bool IsFeatureUnlocked(Frame* f, EntityRef entity, UnlockedFeature feature, bool isUnlockedByDefault);
    // 0x7be7190 - False when the entity has no UnlockedFeaturesManager.
    static bool TryUnlockFeature(Frame* f, EntityRef entity, UnlockedFeature feature);
};

class UltimateAbilityLib {
  public:
    // TODO(0x7c0c114, Quantum.UltimateAbilityLib.TryGiveCharge): adds charge to the
    // ultimate (charge state machine, stacks, events); not reimplemented yet - the stub
    // grants nothing.
    static bool TryGiveCharge(Frame* frame, EntityRef entity, UltimateAbilityUser* user, UltimateAbilitySpec* spec,
                              FP charge) {
        (void)frame;
        (void)entity;
        (void)user;
        (void)spec;
        (void)charge;
        return false;
    }
};

} // namespace Quantum
