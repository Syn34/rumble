// globals.cpp - Quantum._globals_ (quantum.code.dll).
#include "quantum_code/globals.h"

#include "quantum_core/frame_serializer.h"

namespace Quantum {

void _globals_::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* g = static_cast<_globals_*>(ptr);
    auto& stream = serializer->Stream();
    stream.Serialize(&g->nPlayerDataReceived);
    QBoolean::Serialize(&g->AI_BotsInactive, serializer);
    QBoolean::Serialize(&g->cheat_BotsInactive, serializer);
    QBoolean::Serialize(&g->followPlayerCheatActive, serializer);
    AssetRefMap::Serialize(&g->Map, serializer);
    BitSet32::Serialize(&g->PlayerLastConnectionState, serializer);
    FP::Serialize(&g->DeltaTime, *serializer);
    FP::Serialize(&g->UnscaledDeltaTime, *serializer);
    NavMeshRegionMask::Serialize(&g->NavMeshRegions, serializer);
    // (PhysicsState2D and PhysicsState3D: see the TODO in the header.)
    RNGSession::Serialize(&g->RngSession, *serializer);
    BotSDKData::Serialize(&g->BotSDKData, serializer);
    FPVector3::Serialize(&g->followPlayerPos, serializer);
    FrameMetaData::Serialize(&g->FrameMetaData, serializer);
    for (Input& input : g->get_input()) {
        Input::Serialize(&input, serializer);
    }
    BitSet1024::Serialize(&g->Systems, serializer);
    // (PhysicsSettings: see the TODO in the header.)
}

} // namespace Quantum
