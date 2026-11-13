// frame.h - Quantum.Frame (quantum.code.dll): the game's simulation frame.
//
// Adds to the framework frame (FrameBase: entities and components) everything the game
// keeps per frame: the global state block (_globals_, in native memory, copied with the
// frame), the configs the simulation runs with, the systems and their signal receivers,
// per-player data, and the FrameEvents / FrameSignals front ends.
#pragma once

#include "quantum_code/components.h"
#include "quantum_code/frame_events.h"
#include "quantum_code/frame_signals.h"
#include "quantum_code/globals.h"
#include "quantum_code/runtime_config.h"
#include "quantum_code/simulation_config.h"
#include "quantum_code/system_base.h"
#include "quantum_core/frame_base.h"

#include "photon_deterministic/deterministic_session_config.h"

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Quantum {

// A system together with its view as one signal interface (the original stores the
// interface array and casts back to SystemBase for the enabled check).
template <class TSignal> struct SignalReceiver {
    SystemBase* System = nullptr;
    TSignal* Signal = nullptr;
};
template <class TSignal> using SignalReceivers = std::vector<SignalReceiver<TSignal>>;

// Quantum.ISignalOnComponentAdded<T> / ISignalOnComponentRemoved<T>: per component type.
template <class T> class ISignalOnComponentAdded : public virtual ISignal {
  public:
    virtual void OnAdded(Frame* f, EntityRef entity, T* component) = 0;
};
template <class T> class ISignalOnComponentRemoved : public virtual ISignal {
  public:
    virtual void OnRemoved(Frame* f, EntityRef entity, T* component) = 0;
};

class Frame : public Core::FrameBase {
  public:
    static constexpr int DumpFlag_NoSimulationConfig = 1;
    static constexpr int DumpFlag_NoRuntimeConfig = 2;
    static constexpr int DumpFlag_NoDeterministicSessionConfig = 4;
    static constexpr int DumpFlag_NoRuntimePlayers = 8;
    static constexpr int DumpFlag_NoDynamicDB = 16;
    static constexpr int DumpFlag_ReadableDynamicDB = 32;
    static constexpr int DumpFlag_PrintRawValues = 64;
    static constexpr int DumpFlag_ComponentChecksums = 128;
    static constexpr int DumpFlag_AssetDBCheckums = 256;
    static constexpr int DumpFlag_NoIsVerified = 512;
    static constexpr int DumpFlag_PrintComponentChecksums = 1024;
    static constexpr int DumpFlag_PrintReadableDynamicDB = 2048;

    // 0x7c6ad28 - Registers the component types (once per process), allocates the global
    // block, collects the systems' signal receivers and creates the entity store sized by
    // the simulation config; takes the map and the RNG seed from the runtime config and
    // sets the enable bit of every system that starts enabled. The frame keeps the configs and systems by pointer; they must
    // outlive it. `systemsAll` lists every system in runtime-index order.
    Frame(Core::FrameContext* context, std::vector<SystemBase*> systemsAll,
          std::vector<SystemBase*> systemsRoots, Photon::Deterministic::DeterministicSessionConfig* sessionConfig,
          RuntimeConfig* runtimeConfig, SimulationConfig* simulationConfig, FP deltaTime);
    ~Frame() override;

    // --- input ---------------------------------------------------------------------------

    // 0x7c6a870 - Stores a player's input for this frame: buttons are updated against
    // their previous state (pressed/released detection), directions copied. Throws
    // std::out_of_range for a slot past Input::MAX_COUNT.
    void SetPlayerInput(int player, const ::Quantum::Input& input);
    // 0x7c5adfc - The stored input of a player slot; throws std::out_of_range past
    // Input::MAX_COUNT.
    ::Quantum::Input* GetPlayerInput(int player);

    // --- state ---------------------------------------------------------------------------

    // 0x7c6aaec
    _globals_* get_Global() const { return _globals; }
    // 0x7c6aaf4
    RNGSession* get_RNG() const { return &_globals->RngSession; }
    // 0x7c6ab0c
    int get_PlayerCount() const { return _sessionConfig->PlayerCount; }
    // 0x7c6ab24
    NavMeshRegionMask* get_NavMeshRegionMask() const { return &_globals->NavMeshRegions; }
    // 0x7c6ab3c
    FrameMetaData* get_FrameMetaData() override { return &_globals->FrameMetaData; }
    // 0x7c6ab54 - Bytes only until quantum.core's PhysicsEngineState exists.
    PhysicsEngineStateStorage* get__physicsState2D() const { return &_globals->PhysicsState2D; }
    // 0x7c6ab6c
    PhysicsEngineStateStorage* get__physicsState3D() const { return &_globals->PhysicsState3D; }
    // 0x7c6ab84 - From the simulation config's entity settings.
    CommitCommandsModes get_CommitCommandsMode() const override {
        return _simulationConfig->Entities.CommitCommandsMode;
    }
    // 0x7c6aba8 - The context (FrameContextUser in the original: the game adds per-thread
    // hit collections to it, not reimplemented yet).
    Core::FrameContext* get_Context() const { return Core::FrameBase::get_Context(); }
    // 0x7c6ac20, 0x7c6ac28
    RuntimeConfig* get_RuntimeConfig() const { return _runtimeConfig; }
    void set_RuntimeConfig(RuntimeConfig* value) { _runtimeConfig = value; }
    // 0x7c6ac38, 0x7c6ac40
    SimulationConfig* get_SimulationConfig() const { return _simulationConfig; }
    void set_SimulationConfig(SimulationConfig* value) { _simulationConfig = value; }
    // 0x7c6ac50, 0x7c6ac58
    Photon::Deterministic::DeterministicSessionConfig* get_SessionConfig() const { return _sessionConfig; }
    void set_SessionConfig(Photon::Deterministic::DeterministicSessionConfig* value) { _sessionConfig = value; }
    // 0x7c6ac68
    const PhysicsSceneSettings* get_PhysicsSceneSettings() const override { return &_globals->PhysicsSettings; }
    PhysicsSceneSettings* MutablePhysicsSceneSettings() { return &_globals->PhysicsSettings; }
    // 0x7c6ac80, 0x7c6ac98
    FP get_DeltaTime() const override { return _globals->DeltaTime; }
    void set_DeltaTime(FP value) override { _globals->DeltaTime = value; }
    // 0x7c6acb0 - The map asset named by the globals (logged if missing).
    Quantum::Map* get_Map() const override;
    // 0x7c73c00, 0x7c73c08
    FP get_UnscaledDeltaTime() const { return _unscaledDeltaTime; }
    void set_UnscaledDeltaTime(FP value) { _unscaledDeltaTime = value; }
    // 0x7c73cb4 - DeltaTime / UnscaledDeltaTime (1 while the unscaled time is not positive).
    FP get_TimeScale() const;

    // --- frame lifetime ------------------------------------------------------------------

    // 0x7c72e64 - Copies another frame: player data (shared), heap, entity store, dynamic
    // assets and the global block.
    void Copy(const Photon::Deterministic::DeterministicFrame& frame) override;
    // 0x7c73068 - The global block, then the framework frame.
    void Free() override;

    // TODO(0x7c70380, Quantum.Frame.Serialize(mode)) and the other serialisation members:
    // TODO(0x7c7043c, Quantum.Frame.Serialize), TODO(0x7c70458, Quantum.Frame.Serialize),
    // TODO(0x7c710d4, Quantum.Frame.Deserialize), TODO(0x7c710a4, Quantum.Frame.SerializeState),
    // TODO(0x7c70ad0, Quantum.Frame.SerializeRuntimePlayers),
    // TODO(0x7c712b8, Quantum.Frame.DeserializeRuntimePlayers),
    // TODO(0x7c714f0, Quantum.Frame.DeserializeDynamicAssetDB): all write or read the
    // global block, whose serialiser waits on quantum.core's physics state types.
    std::vector<std::uint8_t> Serialize(Photon::Deterministic::DeterministicFrameSerializeMode mode) override;
    void Deserialize(std::span<const std::uint8_t> data) override;
    // TODO(0x7c71964, Quantum.Frame.DumpFrame): needs FramePrinter.
    std::string DumpFrame(int dumpFlags) override;
    // 0x7c72b60 - With the context's shared checksum serializer if it has one.
    std::uint64_t CalculateChecksum() override;
    // 0x7c70a20 - The shared serializer, or a new one over a FrameChecksumerBitStream.
    std::uint64_t CalculateChecksum(bool useSharedSerializer);
    // 0x7c72b84 - CRC64 of the global block and the entity store, seeded with the frame
    // number. The global block leaves out the physics parts (see _globals_::Serialize).
    std::uint64_t CalculateChecksumInternal(FrameSerializer* serializer);

    // --- systems -------------------------------------------------------------------------

    // 0x7c730b4 - The system's own enable bit (false for null).
    bool SystemIsEnabledSelf(const SystemBase* s) const;
    // 0x7c73104 - Enabled itself and every parent up to the root (false for null).
    bool SystemIsEnabledInHierarchy(const SystemBase* system) const;
    // 0x7c73170 (SystemEnable(Type)) - Sets T's enable bit; if it was off and its parents
    // are enabled, the system gets OnEnabled. 0x7c73280 (FindSystem) logs an unknown type.
    template <class T> void SystemEnable() { SystemEnable(typeid(T)); }
    void SystemEnable(const std::type_info& type);
    // 0x7c733a4 (SystemDisable(Type)) - Clears the bit; OnDisabled as for SystemEnable.
    template <class T> void SystemDisable() { SystemDisable(typeid(T)); }
    void SystemDisable(const std::type_info& type);
    // 0x7c73280 - The system of `type` and its index, or (null, -1) with an error logged.
    std::pair<SystemBase*, int> FindSystem(const std::type_info& type) const;

    // 0x7c734b4 - Stamps the event with the frame number and appends it to the context's
    // event list.
    void AddEvent(EventBase* evnt);

    // --- generated set-up ----------------------------------------------------------------

    // 0x7c6b710 - Static set-up of the game (component registration).
    static void InitStatic() { InitStaticGen(); }
    // 0x7c734ec
    static void InitStaticGen();
    // 0x7c6b724 - Entity store, signal receivers of every interface and component type.
    void InitGen();
    // 0x7c6b6dc - The zeroed global block.
    void AllocGen();
    // 0x7c73084
    void FreeGen();
    // 0x7c72ff8 - Byte copy of the other frame's global block.
    void CopyFromGen(const Frame* frame);
    // 0x7c70364 - Unscaled delta time starts at the delta time.
    void InitUser() { _unscaledDeltaTime = _globals->DeltaTime; }
    // 0x7c73cf4 - A reusable hit collection for the thread, emptied. The original keeps one
    // per worker thread in the context (FrameContextUser, handed out by AllocUser
    // 0x7c70308, capacity 64, persistent); the PC build simulates on one thread, so the
    // frame owns its collection and creates it on first use.
    Physics3D::HitCollection3D* GetPersistentHitCollection(int threadIndex = 0);
    // 0x7c73d5c - The same for area-of-effect queries.
    Physics3D::HitCollection3D* GetAreaOfEffectHitCollection(int threadIndex = 0);
    // TODO(0x7c735c0, Quantum.Frame.GetPlayerData), TODO(0x7c717fc,
    // Quantum.Frame.TryGetPlayerData), TODO(0x7c735f0, Quantum.Frame.UpdatePlayerData):
    // need RuntimePlayer and the session's player data (PersistentMap).
    // TODO(0x7c73c10, Quantum.Frame.get_MapCustomData): needs the MapCustomData asset class.
    // TODO(0x7c73dc4, Quantum.Frame.<.ctor>b__173_0): prototype-materialized signal
    // forwarder; needs EntityPrototypeRef.

    // Receiver lists of one signal interface / component signal (Frame.BuildSignalsArray<T>,
    // generic; instance e.g. 0x4e4f148).
    template <class TSignal> SignalReceivers<TSignal> BuildSignalsArray() const;
    template <class T> void BuildSignalsArrayOnComponentAdded();
    template <class T> void BuildSignalsArrayOnComponentRemoved();

    const std::vector<SystemBase*>& SystemsAll() const { return _systemsAll; }
    const std::vector<SystemBase*>& SystemsRoots() const { return _systemsRoots; }

    // --- signal receivers (Frame._ISignalXxxSystems) -------------------------------------
    SignalReceivers<ISignalAIMoveForward_OnBounce> _ISignalAIMoveForward_OnBounceSystems;
    SignalReceivers<ISignalAISpawner_OnSpawned> _ISignalAISpawner_OnSpawnedSystems;
    SignalReceivers<ISignalAISpawner_OnSpawnDone> _ISignalAISpawner_OnSpawnDoneSystems;
    SignalReceivers<ISignalPlayerActor_OnInputLocked> _ISignalPlayerActor_OnInputLockedSystems;
    SignalReceivers<ISignalActorStatus_OnChanged> _ISignalActorStatus_OnChangedSystems;
    SignalReceivers<ISignalAreaOfEffect_OnCalculateDynamicShape> _ISignalAreaOfEffect_OnCalculateDynamicShapeSystems;
    SignalReceivers<ISignalAreaOfEffect_OnHit> _ISignalAreaOfEffect_OnHitSystems;
    SignalReceivers<ISignalAreaOfEffect_OnMiss> _ISignalAreaOfEffect_OnMissSystems;
    SignalReceivers<ISignalAutoJumperActor_OnStartJump> _ISignalAutoJumperActor_OnStartJumpSystems;
    SignalReceivers<ISignalAutoJumperActor_OnEndJump> _ISignalAutoJumperActor_OnEndJumpSystems;
    SignalReceivers<ISignalBlastBallGame_StateChanged> _ISignalBlastBallGame_StateChangedSystems;
    SignalReceivers<ISignalBlastBallGame_LeadChanged> _ISignalBlastBallGame_LeadChangedSystems;
    SignalReceivers<ISignalBolt_OnCollected> _ISignalBolt_OnCollectedSystems;
    SignalReceivers<ISignalBolt_OnLost> _ISignalBolt_OnLostSystems;
    SignalReceivers<ISignalDamageable_OnBeforeHit> _ISignalDamageable_OnBeforeHitSystems;
    SignalReceivers<ISignalDamageable_HitReceived> _ISignalDamageable_HitReceivedSystems;
    SignalReceivers<ISignalDamageToImpulseConverter_ComputeImpulse> _ISignalDamageToImpulseConverter_ComputeImpulseSystems;
    SignalReceivers<ISignalDamageToImpulseConverter_OnBeforeImpulse> _ISignalDamageToImpulseConverter_OnBeforeImpulseSystems;
    SignalReceivers<ISignalDamageToImpulseConverter_OnAfterImpulse> _ISignalDamageToImpulseConverter_OnAfterImpulseSystems;
    SignalReceivers<ISignalDashActor_OnBeforeDash> _ISignalDashActor_OnBeforeDashSystems;
    SignalReceivers<ISignalDashActor_OnDashed> _ISignalDashActor_OnDashedSystems;
    SignalReceivers<ISignalDashActor_OnDashIsReady> _ISignalDashActor_OnDashIsReadySystems;
    SignalReceivers<ISignalDashActor_OnAfterDash> _ISignalDashActor_OnAfterDashSystems;
    SignalReceivers<ISignalDrillDash_OnStateChanged> _ISignalDrillDash_OnStateChangedSystems;
    SignalReceivers<ISignalDroppable_OnDestroyed> _ISignalDroppable_OnDestroyedSystems;
    SignalReceivers<ISignalGadgetUser_OnGadgetActivated> _ISignalGadgetUser_OnGadgetActivatedSystems;
    SignalReceivers<ISignalGadgetUser_OnGadgetDeactivated> _ISignalGadgetUser_OnGadgetDeactivatedSystems;
    SignalReceivers<ISignalGameMode_OnMainLoopStart> _ISignalGameMode_OnMainLoopStartSystems;
    SignalReceivers<ISignalGameMode_OnStateChanged> _ISignalGameMode_OnStateChangedSystems;
    SignalReceivers<ISignalGameMode_OnPlayerEliminated> _ISignalGameMode_OnPlayerEliminatedSystems;
    SignalReceivers<ISignalGameMode_OnPlayerFinished> _ISignalGameMode_OnPlayerFinishedSystems;
    SignalReceivers<ISignalBolt_OnGoldenBoltCollected> _ISignalBolt_OnGoldenBoltCollectedSystems;
    SignalReceivers<ISignalGrinderActor_OnExitGrind> _ISignalGrinderActor_OnExitGrindSystems;
    SignalReceivers<ISignalGroundDashActor_OnDashStarting> _ISignalGroundDashActor_OnDashStartingSystems;
    SignalReceivers<ISignalGroundDashActor_OnDashEnded> _ISignalGroundDashActor_OnDashEndedSystems;
    SignalReceivers<ISignalHealth_OnHealthDecreased> _ISignalHealth_OnHealthDecreasedSystems;
    SignalReceivers<ISignalKccExtension_OnFilterCollision3D> _ISignalKccExtension_OnFilterCollision3DSystems;
    SignalReceivers<ISignalKccExtension_OnStaticWallCollision3D> _ISignalKccExtension_OnStaticWallCollision3DSystems;
    SignalReceivers<ISignalKccExtension_OnCharacterCollision3D> _ISignalKccExtension_OnCharacterCollision3DSystems;
    SignalReceivers<ISignalKccExtension_OnMovementPackComputed> _ISignalKccExtension_OnMovementPackComputedSystems;
    SignalReceivers<ISignalKccExtension_OnValidLedgeDetected> _ISignalKccExtension_OnValidLedgeDetectedSystems;
    SignalReceivers<ISignalKccExtension_OnConfigChanged> _ISignalKccExtension_OnConfigChangedSystems;
    SignalReceivers<ISignalKillable_OnKill> _ISignalKillable_OnKillSystems;
    SignalReceivers<ISignalKillable_OnRevive> _ISignalKillable_OnReviveSystems;
    SignalReceivers<ISignalLedgeGrabbingActor_OnLedgeGrabStart> _ISignalLedgeGrabbingActor_OnLedgeGrabStartSystems;
    SignalReceivers<ISignalLifetime_OnSpawnEntityOnDeath> _ISignalLifetime_OnSpawnEntityOnDeathSystems;
    SignalReceivers<ISignalMobGroup_OnAllMembersDied> _ISignalMobGroup_OnAllMembersDiedSystems;
    SignalReceivers<ISignalMoveTo_OnMoveStarted> _ISignalMoveTo_OnMoveStartedSystems;
    SignalReceivers<ISignalMoveTo_OnMoveFinished> _ISignalMoveTo_OnMoveFinishedSystems;
    SignalReceivers<ISignalParabolaFollower_OnRemove> _ISignalParabolaFollower_OnRemoveSystems;
    SignalReceivers<ISignalParabolaFollower_OnReachedEnd> _ISignalParabolaFollower_OnReachedEndSystems;
    SignalReceivers<ISignalPlayerActor_OnInitialized> _ISignalPlayerActor_OnInitializedSystems;
    SignalReceivers<ISignalPlayerActor_OnAfterInitialized> _ISignalPlayerActor_OnAfterInitializedSystems;
    SignalReceivers<ISignalPowerUp_OnCollected> _ISignalPowerUp_OnCollectedSystems;
    SignalReceivers<ISignalProjectile_OnSpawnEntityOnDeath> _ISignalProjectile_OnSpawnEntityOnDeathSystems;
    SignalReceivers<ISignalPushable_OnAddPush> _ISignalPushable_OnAddPushSystems;
    SignalReceivers<ISignalPushable_OnBeforePush> _ISignalPushable_OnBeforePushSystems;
    SignalReceivers<ISignalPushable_OnPushed> _ISignalPushable_OnPushedSystems;
    SignalReceivers<ISignalPushable_OnPushStopEnd> _ISignalPushable_OnPushStopEndSystems;
    SignalReceivers<ISignalRanking_OnLeaderChanged> _ISignalRanking_OnLeaderChangedSystems;
    SignalReceivers<ISignalReward_OnCollected> _ISignalReward_OnCollectedSystems;
    SignalReceivers<ISignalShield_OnBreaks> _ISignalShield_OnBreaksSystems;
    SignalReceivers<ISignalShield_OnStartRegen> _ISignalShield_OnStartRegenSystems;
    SignalReceivers<ISignalShield_OnFull> _ISignalShield_OnFullSystems;
    SignalReceivers<ISignalTargetedEffect_OnTick> _ISignalTargetedEffect_OnTickSystems;
    SignalReceivers<ISignalTargetedEffect_OnTargetSet> _ISignalTargetedEffect_OnTargetSetSystems;
    SignalReceivers<ISignalUltimateAbilityUser_OnStateChanged> _ISignalUltimateAbilityUser_OnStateChangedSystems;
    SignalReceivers<ISignalUltimateAbilityUser_OnRecast> _ISignalUltimateAbilityUser_OnRecastSystems;
    SignalReceivers<ISignalOnFeatureUnlocked> _ISignalOnFeatureUnlockedSystems;
    SignalReceivers<ISignalOnFeatureLocked> _ISignalOnFeatureLockedSystems;
    SignalReceivers<ISignalOnVoidRepulserShieldStateChanged> _ISignalOnVoidRepulserShieldStateChangedSystems;
    SignalReceivers<ISignalWallRunningActor_OnWallRunningStart> _ISignalWallRunningActor_OnWallRunningStartSystems;
    SignalReceivers<ISignalWaveSpawner_OnSpawned> _ISignalWaveSpawner_OnSpawnedSystems;
    SignalReceivers<ISignalWeapon_OnCreated> _ISignalWeapon_OnCreatedSystems;
    SignalReceivers<ISignalWeapon_OnReload> _ISignalWeapon_OnReloadSystems;
    SignalReceivers<ISignalWeapon_OnShoot> _ISignalWeapon_OnShootSystems;
    SignalReceivers<ISignalWeaponLoadoutActor_OnManualSwap> _ISignalWeaponLoadoutActor_OnManualSwapSystems;
    SignalReceivers<ISignalWeapon_OnBeforeEquip> _ISignalWeapon_OnBeforeEquipSystems;
    SignalReceivers<ISignalWeapon_OnEquip> _ISignalWeapon_OnEquipSystems;
    SignalReceivers<ISignalWeapon_OnUnEquip> _ISignalWeapon_OnUnEquipSystems;
    SignalReceivers<ISignalWeapon_OnBeforeUse> _ISignalWeapon_OnBeforeUseSystems;
    SignalReceivers<ISignalWeapon_OnUse> _ISignalWeapon_OnUseSystems;
    SignalReceivers<ISignalOnPlayerDataSet> _ISignalOnPlayerDataSet;
    SignalReceivers<ISignalOnEntityPrototypeMaterialized> _ISignalOnEntityPrototypeMaterializedSystems;
    SignalReceivers<ISignalOnPlayerConnected> _ISignalOnPlayerConnectedSystems;
    SignalReceivers<ISignalOnPlayerDisconnected> _ISignalOnPlayerDisconnectedSystems;
    SignalReceivers<ISignalOnNavMeshWaypointReached> _ISignalOnNavMeshWaypointReachedSystems;
    SignalReceivers<ISignalOnNavMeshSearchFailed> _ISignalOnNavMeshSearchFailedSystems;
    SignalReceivers<ISignalOnNavMeshMoveAgent> _ISignalOnNavMeshMoveAgentSystems;
    SignalReceivers<ISignalOnCollision3D> _ISignalOnCollision3DSystems;
    SignalReceivers<ISignalOnCollisionEnter3D> _ISignalOnCollisionEnter3DSystems;
    SignalReceivers<ISignalOnCollisionExit3D> _ISignalOnCollisionExit3DSystems;
    SignalReceivers<ISignalOnTrigger3D> _ISignalOnTrigger3DSystems;
    SignalReceivers<ISignalOnTriggerEnter3D> _ISignalOnTriggerEnter3DSystems;
    SignalReceivers<ISignalOnTriggerExit3D> _ISignalOnTriggerExit3DSystems;

    FrameSignals Signals;
    FrameEvents Events;

  private:
    void BuildComponentSignals();
    void BuildSignalArrays();

    _globals_* _globals = nullptr;
    RuntimeConfig* _runtimeConfig = nullptr;
    SimulationConfig* _simulationConfig = nullptr;
    Photon::Deterministic::DeterministicSessionConfig* _sessionConfig = nullptr;
    std::vector<SystemBase*> _systemsAll;
    std::vector<SystemBase*> _systemsRoots;
    std::unordered_map<std::type_index, int> _systemIndexByType;
    FP _unscaledDeltaTime;
    std::unique_ptr<Physics3D::HitCollection3D> _persistentHitCollection;
    std::unique_ptr<Physics3D::HitCollection3D> _areaOfEffectHitCollection;
};

// ---------------------------------------------------------------------------------------
// Template members

// Every system implementing TSignal, in system order (`_systemsAll.OfType<T>()`).
template <class TSignal> SignalReceivers<TSignal> Frame::BuildSignalsArray() const {
    SignalReceivers<TSignal> receivers;
    for (SystemBase* system : _systemsAll) {
        if (auto* signal = dynamic_cast<TSignal*>(system)) {
            receivers.push_back({system, signal});
        }
    }
    return receivers;
}

// The component-added signal of T, installed only when some system implements it.
// Systems not enabled in the hierarchy are skipped, as for every other signal.
template <class T> void Frame::BuildSignalsArrayOnComponentAdded() {
    SignalReceivers<ISignalOnComponentAdded<T>> receivers = BuildSignalsArray<ISignalOnComponentAdded<T>>();
    if (receivers.empty()) {
        return;
    }
    _ComponentSignalsOnAdded.at(static_cast<std::size_t>(ComponentTypeId<T>::Id)) =
        [this, receivers](EntityRef entity, void* component) {
            for (const auto& receiver : receivers) {
                if (SystemIsEnabledInHierarchy(receiver.System)) {
                    receiver.Signal->OnAdded(this, entity, static_cast<T*>(component));
                }
            }
        };
}

template <class T> void Frame::BuildSignalsArrayOnComponentRemoved() {
    SignalReceivers<ISignalOnComponentRemoved<T>> receivers = BuildSignalsArray<ISignalOnComponentRemoved<T>>();
    if (receivers.empty()) {
        return;
    }
    _ComponentSignalsOnRemoved.at(static_cast<std::size_t>(ComponentTypeId<T>::Id)) =
        [this, receivers](EntityRef entity, void* component) {
            for (const auto& receiver : receivers) {
                if (SystemIsEnabledInHierarchy(receiver.System)) {
                    receiver.Signal->OnRemoved(this, entity, static_cast<T*>(component));
                }
            }
        };
}

} // namespace Quantum
