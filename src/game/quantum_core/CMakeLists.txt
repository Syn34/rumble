# quantum.core.dll - deterministic simulation framework (third-party).
add_library(rr_quantum_core
    asset_guid.cpp
    frame_heap.cpp
    heap.cpp
    q_buffer.cpp
    q_hash_collection.cpp
    q_string.cpp
    unsafe_buffer.cpp
    transform.cpp
    shape2d.cpp
    shape3d.cpp
    physics2d/collision_manifold2d.cpp
    physics2d/hit.cpp
    physics2d/narrow_checks.cpp
    physics3d/collision_manifold3d.cpp
    physics3d/hit3d.cpp
    physics3d/narrow_checks.cpp
    byte_stream.cpp
    mailbox_set8.cpp
    navigation/navmesh_region_mask.cpp
    navigation/navmesh_types.cpp
    navigation/navmesh.cpp
    navigation/navmesh_agent_config.cpp
    navigation/path_finder.cpp
)
add_library(rr::quantum_core ALIAS rr_quantum_core)
target_include_directories(rr_quantum_core PUBLIC ..)
target_link_libraries(rr_quantum_core PUBLIC rr::photon_deterministic)
rr_set_warnings(rr_quantum_core)
