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
    gjk_settings.cpp
    physics2d/collision_manifold2d.cpp
    physics2d/hit.cpp
    physics2d/narrow_checks.cpp
    physics2d/narrow_checks_box.cpp
    physics2d/narrow_checks_polygon.cpp
    physics2d/gjk2d.cpp
    physics3d/collision_manifold3d.cpp
    physics3d/hit3d.cpp
    physics3d/scene3d.cpp
    physics3d/physics_engine3d.cpp
    physics3d/narrow_checks.cpp
    physics3d/narrow_checks_box.cpp
    physics_components.cpp
    byte_stream.cpp
    mailbox_set8.cpp
    navigation/navmesh_region_mask.cpp
    navigation/navmesh_types.cpp
    navigation/navmesh.cpp
    navigation/navmesh_agent_config.cpp
    navigation/path_finder.cpp
    navigation/navmesh_components.cpp
    navigation/navigation.cpp
    path_utils.cpp
    physics3d/gjk3d.cpp
    triangle_ccw.cpp
    entity_ref.cpp
    component_set.cpp
    component_type_id.cpp
    component_data_buffer.cpp
    component_reference.cpp
    events.cpp
    frame_serializer.cpp
    frame_checksumer_bit_stream.cpp
    frame_context.cpp
    frame_base.cpp
    frame_meta_data.cpp
    q_list.cpp
    input_types.cpp
    assert.cpp
    asset_object.cpp
    asset_db.cpp
    resource_manager.cpp
    dynamic_asset_db.cpp
    builtin_assets.cpp
    polygon_collider.cpp
    map.cpp
    asset_resource_provider.cpp
    entity_prototype.cpp
    component_prototype.cpp
    component_prototypes.cpp
)
add_library(rr::quantum_core ALIAS rr_quantum_core)
target_include_directories(rr_quantum_core PUBLIC ..)
target_link_libraries(rr_quantum_core PUBLIC rr::photon_deterministic)
rr_set_warnings(rr_quantum_core)

# Asset value types used by the gameplay assets (FPAnimationCurve).
target_sources(rr_quantum_core PRIVATE fp_animation_curve.cpp)
# 3D/2D physics scene (agent/qcore-physics2)
target_sources(rr_quantum_core PRIVATE
    physics3d/collision_info3d.cpp
    physics3d/character_controller3d.cpp
)
