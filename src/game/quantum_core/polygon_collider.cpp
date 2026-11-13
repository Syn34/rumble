// Original: Quantum.PolygonCollider (quantum.core.dll).
#include "polygon_collider.h"

#include "photon_deterministic/native.h"

#include <cstring>

namespace Quantum {

using Photon::Deterministic::FPVector2;

namespace {

// UnmanagedUtils' array copy: the elements in a fresh native block (null when empty).
FPVector2* CopyToNative(const std::vector<FPVector2>& items, AssetObject::Allocator& allocator) {
    if (items.empty()) {
        return nullptr;
    }
    const int bytes = static_cast<int>(items.size() * sizeof(FPVector2));
    auto* block = static_cast<FPVector2*>(allocator.Alloc(bytes));
    std::memcpy(block, items.data(), static_cast<std::size_t>(bytes));
    return block;
}

} // namespace

PolygonCollider::PolygonCollider(const PolygonCollider& other)
    : AssetObject(other), IBuiltInAssetObject(other), Vertices(other.Vertices),
      Normals(other.Normals), Radius(other.Radius), Centroid(other.Centroid) {}

PolygonCollider& PolygonCollider::operator=(const PolygonCollider& other) {
    if (this != &other) {
        AssetObject::operator=(other);
        Vertices = other.Vertices;
        Normals = other.Normals;
        Radius = other.Radius;
        Centroid = other.Centroid;
    }
    return *this;
}

void PolygonCollider::Loaded(IResourceManager* resourceManager, Allocator* allocator) {
    FPVector2::MakeCounterClockWise(Vertices);
    Radius = FPVector2::CalculatePolygonRadius(Vertices);
    Normals = FPVector2::CalculatePolygonNormals(Vertices);
    Centroid = FPVector2::CalculatePolygonCentroid(Vertices);
    if (resourceManager != nullptr) {
        InitializeNativeBuffers(allocator);
    }
}

void PolygonCollider::Disposed(IResourceManager* resourceManager, Allocator* allocator) {
    if (resourceManager != nullptr) {
        FreeNativeBuffers(allocator);
    }
}

void PolygonCollider::InitializeNativeBuffers(Allocator* allocator) {
    if (allocator == nullptr) {
        return;
    }
    if (VerticesBuffer == nullptr) {
        VerticesBuffer = CopyToNative(Vertices, *allocator);
    }
    if (NormalsBuffer == nullptr) {
        NormalsBuffer = CopyToNative(Normals, *allocator);
    }
}

void PolygonCollider::FreeNativeBuffers(Allocator* allocator) {
    if (VerticesBuffer != nullptr) {
        FPVector2* block = VerticesBuffer;
        VerticesBuffer = nullptr;
        allocator->Free(block);
    }
    if (NormalsBuffer != nullptr) {
        FPVector2* block = NormalsBuffer;
        NormalsBuffer = nullptr;
        allocator->Free(block);
    }
}

} // namespace Quantum
