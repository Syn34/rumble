// quantum_code_factories.h - registers the C++ asset classes with the content provider
// (PC-side glue; new code).
#pragma once

namespace rr::content {

// Every asset class of the gameplay simulation (quantum.code.dll) and of quantum.core,
// registered under the engine-side script class names ("WeaponSpecDefaultAsset", ...).
// Safe to call more than once.
void RegisterQuantumCodeAssetFactories();

// quantum.core's built-in assets (maps, navmeshes, binary data, physics materials,
// character controller configs, entity views and prototypes...) and SimulationConfig.
void RegisterBuiltInAssetFactories();

} // namespace rr::content
