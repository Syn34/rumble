# quantum.core.dll - deterministic simulation framework (third-party).
add_library(rr_quantum_core
    asset_guid.cpp
    frame_heap.cpp
    heap.cpp
    q_buffer.cpp
    q_hash_collection.cpp
    q_string.cpp
    unsafe_buffer.cpp
)
add_library(rr::quantum_core ALIAS rr_quantum_core)
target_include_directories(rr_quantum_core PUBLIC ..)
target_link_libraries(rr_quantum_core PUBLIC rr::photon_deterministic)
rr_set_warnings(rr_quantum_core)
