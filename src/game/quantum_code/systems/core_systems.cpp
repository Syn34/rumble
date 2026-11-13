// core_systems.cpp - the Quantum.Core systems of quantum.code.dll.
#include "quantum_code/systems/core_systems.h"

#include "quantum_core/component_iterators.h"
#include "quantum_core/map.h"
#include "quantum_core/map_entity_id.h"

#include <stdexcept>

namespace Quantum::Core {

namespace {
// The physics scene only ever reports to the game's frame type.
Frame* AsGameFrame(FrameBase* f) {
    auto* frame = dynamic_cast<Frame*>(f);
    if (frame == nullptr) {
        throw std::invalid_argument("PhysicsSystem3D: callback for a frame that is not a Quantum.Frame");
    }
    return frame;
}
} // namespace

void PhysicsSystem3D::OnCollision3D(FrameBase* f, const CollisionInfo3D& info) {
    AsGameFrame(f)->Signals.OnCollision3D(info);
}

void PhysicsSystem3D::OnCollisionEnter3D(FrameBase* f, const CollisionInfo3D& info) {
    AsGameFrame(f)->Signals.OnCollisionEnter3D(info);
}

void PhysicsSystem3D::OnCollisionExit3D(FrameBase* f, const ExitInfo3D& info) {
    AsGameFrame(f)->Signals.OnCollisionExit3D(info);
}

void PhysicsSystem3D::OnTrigger3D(FrameBase* f, const TriggerInfo3D& info) {
    AsGameFrame(f)->Signals.OnTrigger3D(info);
}

void PhysicsSystem3D::OnTriggerEnter3D(FrameBase* f, const TriggerInfo3D& info) {
    AsGameFrame(f)->Signals.OnTriggerEnter3D(info);
}

void PhysicsSystem3D::OnTriggerExit3D(FrameBase* f, const ExitInfo3D& info) {
    AsGameFrame(f)->Signals.OnTriggerExit3D(info);
}

void NavigationSystem::OnWaypointReached(EntityRef entity, FPVector3 waypoint, WaypointFlag waypointFlags,
                                         bool& resetAgent) {
    _currentFrame->Signals.OnNavMeshWaypointReached(entity, waypoint, waypointFlags, resetAgent);
}

void NavigationSystem::OnSearchFailed(EntityRef entity, bool& resetAgent) {
    _currentFrame->Signals.OnNavMeshSearchFailed(entity, resetAgent);
}

void NavigationSystem::OnMoveAgent(EntityRef entity, FPVector2 desiredDirection) {
    _currentFrame->Signals.OnNavMeshMoveAgent(entity, desiredDirection);
}

void EntityPrototypeSystem::OnMapChanged(Frame* f, AssetRefMap previousMap) {
    if (previousMap.Id.IsValid()) {
        for (const auto& pair : f->GetComponentIterator<MapEntityLink>().Range()) {
            f->Destroy(pair.Entity);
        }
    }
    if (Map* map = f->get_Map()) {
        f->Create(map->MapEntities, map);
    }
}

void PlayerConnectedSystem::Update(Frame* f) {
    if (!f->IsVerified()) {
        return;
    }
    BitSet32& lastState = f->get_Global()->PlayerLastConnectionState;
    for (int player = 0; player < f->get_PlayerCount(); ++player) {
        const auto flags = f->GetPlayerInputFlags(player);
        const bool connected =
            (static_cast<int>(flags) & static_cast<int>(Photon::Deterministic::DeterministicInputFlags::PlayerNotPresent)) == 0;
        if (lastState.IsSet(player) == connected) {
            continue;
        }
        if (connected) {
            f->Signals.OnPlayerConnected(PlayerRef::FromIndex(player));
            lastState.Set(player);
        } else {
            f->Signals.OnPlayerDisconnected(PlayerRef::FromIndex(player));
            lastState.Clear(player);
        }
    }
}

} // namespace Quantum::Core
