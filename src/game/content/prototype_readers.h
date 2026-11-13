// prototype_readers.h - reads component prototypes from decoded content data (PC-side glue;
// new code).
//
// Prototypes appear in two places in the game's content: in maps, as flat containers (one
// list per prototype type), and on prefabs, as the `Prototype` field of each entity
// component script. Both store the prototype's fields under their original names.
#pragma once

#include "content/settings_reader.h"
#include "quantum_core/component_prototype.h"
#include "quantum_core/map_entity_id.h"

#include <memory>
#include <string>

namespace Photon::Deterministic {
struct NullableFP;
struct NullableFPVector3;
} // namespace Photon::Deterministic

namespace Quantum {
class Shape3DConfig;
struct SpawnConfig;
namespace Prototypes {
class FlatEntityPrototypeContainer;
class Transform2D_Prototype;
class Transform2DVertical_Prototype;
class Transform3D_Prototype;
class View_Prototype;
class NavMeshAvoidanceAgent_Prototype;
class NavMeshAvoidanceObstacle_Prototype;
class NavMeshPathfinder_Prototype;
class NavMeshSteeringAgent_Prototype;
class CharacterController2D_Prototype;
class CharacterController3D_Prototype;
class PhysicsBody2D_Prototype;
class PhysicsBody3D_Prototype;
class PhysicsCollider2D_Prototype;
class PhysicsCollider3D_Prototype;
class PhysicsCallbacks2D_Prototype;
class PhysicsCallbacks3D_Prototype;
class Joint2D_Prototype;
class Joint3D_Prototype;
class PhysicsJoints2D_Prototype;
class PhysicsJoints3D_Prototype;
} // namespace Prototypes
} // namespace Quantum

namespace rr::content {

// Value shapes used by prototypes (framework_prototype_readers.cpp).
void ReadFields(const Value& value, Quantum::MapEntityId& out, ReaderTag);
void ReadFields(const Value& value, Photon::Deterministic::NullableFP& out, ReaderTag);
void ReadFields(const Value& value, Photon::Deterministic::NullableFPVector3& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SpawnConfig& out, ReaderTag);

// The framework's component prototypes.
void ReadFields(const Value& value, Quantum::Prototypes::Transform2D_Prototype& out, ReaderTag);
void ReadFields(const Value& value, Quantum::Prototypes::Transform2DVertical_Prototype& out,
                ReaderTag);
void ReadFields(const Value& value, Quantum::Prototypes::Transform3D_Prototype& out, ReaderTag);
void ReadFields(const Value& value, Quantum::Prototypes::View_Prototype& out, ReaderTag);
void ReadFields(const Value& value, Quantum::Prototypes::NavMeshAvoidanceAgent_Prototype& out,
                ReaderTag);
void ReadFields(const Value& value, Quantum::Prototypes::NavMeshAvoidanceObstacle_Prototype& out,
                ReaderTag);
void ReadFields(const Value& value, Quantum::Prototypes::NavMeshPathfinder_Prototype& out,
                ReaderTag);
void ReadFields(const Value& value, Quantum::Prototypes::NavMeshSteeringAgent_Prototype& out,
                ReaderTag);
void ReadFields(const Value& value, Quantum::Prototypes::CharacterController2D_Prototype& out,
                ReaderTag);
void ReadFields(const Value& value, Quantum::Prototypes::CharacterController3D_Prototype& out,
                ReaderTag);
void ReadFields(const Value& value, Quantum::Prototypes::PhysicsBody2D_Prototype& out, ReaderTag);
void ReadFields(const Value& value, Quantum::Prototypes::PhysicsBody3D_Prototype& out, ReaderTag);
void ReadFields(const Value& value, Quantum::Prototypes::PhysicsCollider2D_Prototype& out,
                ReaderTag);
void ReadFields(const Value& value, Quantum::Prototypes::PhysicsCollider3D_Prototype& out,
                ReaderTag);
void ReadFields(const Value& value, Quantum::Prototypes::PhysicsCallbacks2D_Prototype& out,
                ReaderTag);
void ReadFields(const Value& value, Quantum::Prototypes::PhysicsCallbacks3D_Prototype& out,
                ReaderTag);
void ReadFields(const Value& value, Quantum::Prototypes::Joint2D_Prototype& out, ReaderTag);
void ReadFields(const Value& value, Quantum::Prototypes::Joint3D_Prototype& out, ReaderTag);
void ReadFields(const Value& value, Quantum::Prototypes::PhysicsJoints2D_Prototype& out, ReaderTag);
void ReadFields(const Value& value, Quantum::Prototypes::PhysicsJoints3D_Prototype& out, ReaderTag);

// A gameplay component prototype by its type name ("Health_Prototype"); null for names
// that are not gameplay prototypes (prototype_readers.cpp).
std::shared_ptr<Quantum::ComponentPrototype> ReadGameComponentPrototype(const std::string& typeName,
                                                                        const Value& value);
// A framework component prototype by its type name ("Transform3D_Prototype"); null for
// other names (framework_prototype_readers.cpp).
std::shared_ptr<Quantum::ComponentPrototype>
ReadFrameworkComponentPrototype(const std::string& typeName, const Value& value);
// Either of the above.
std::shared_ptr<Quantum::ComponentPrototype> ReadComponentPrototype(const std::string& typeName,
                                                                    const Value& value);

// Every list of a serialised FlatEntityPrototypeContainer.
void ReadFlatEntityPrototypeContainer(const Value& value,
                                      Quantum::Prototypes::FlatEntityPrototypeContainer& out);

} // namespace rr::content
