// triangle_mesh.h - Quantum.TriangleMesh and Quantum.TriangleReference (quantum.core.dll).
//
// The static triangle mesh of the 3D scene: every triangle of the map's mesh colliders,
// bucketed into a sparse grid of cubic cells (TriangleMeshCellSize world units). A cell is
// found through a hash of its integer coordinates (chained buckets over a prime-sized
// table, as the original); each cell lists the triangles whose bounds touch it, in
// insertion order. Triangle slot 0 and cell slot 0 are reserved, so 0 means "none" in the
// chains.
//
// The mesh belongs to the physics engine (shared by all frames). Immutable colliders are
// inserted once per map and locked; toggleable colliders that start enabled are inserted
// on top. Inserting is skipped while the engine runs a predicted frame.
#pragma once

#include "quantum_core/asset_guid.h"
#include "quantum_core/triangle_ccw.h"

#include <cstdint>
#include <vector>

namespace Quantum {

class Map;
namespace Core {
class FrameBase;
}
namespace Physics3D {
struct IntAABB;
}

// Quantum.TriangleReference: where a static collider's triangles start in the mesh (the
// last inserted one; the rest follow through Tri.Next). The index is stored plus one so
// the default value means "not inserted".
struct TriangleReference {
    static constexpr int SIZE = 8;
    static constexpr int ALIGNMENT = 4;

    std::int32_t _index = 0;
    std::int32_t _many = 0;

    TriangleReference() = default;
    // 0x7d361cc
    TriangleReference(std::int32_t index, bool isMany) : _index(index + 1), _many(isMany ? 1 : 0) {}

    // 0x7d361dc / 0x7d36204 (the object overload compares boxed references the same way)
    bool Equals(const TriangleReference& other) const {
        return _index == other._index && _many == other._many;
    }
    // 0x7d3628c
    std::int32_t GetHashCode() const {
        return static_cast<std::int32_t>(static_cast<std::uint32_t>(_index) * 397u) ^ _many;
    }
    // 0x7d36490
    friend bool operator==(const TriangleReference& a, const TriangleReference& b) { return a.Equals(b); }
    // TODO(0x7d362a0, Quantum.TriangleReference.Serialize): both ints through the frame
    // serializer's stream (needed with TriangleMesh.Serialize).
};

class TriangleMesh {
  public:
    static constexpr std::int32_t COORDINATE_OFFSET = 32768;
    static constexpr std::int32_t CELL_TRIANGLE_MAX_BITS = 16;
    static constexpr std::int32_t CELL_TRIANGLE_MAX = 65536;

    // TriangleMesh.CellBounds: an inclusive box of cell coordinates.
    struct CellBounds {
        std::int32_t MinX = 0;
        std::int32_t MaxX = 0;
        std::int32_t MinY = 0;
        std::int32_t MaxY = 0;
        std::int32_t MinZ = 0;
        std::int32_t MaxZ = 0;
    };

    // TriangleMesh.Tri: one triangle slot.
    struct Tri {
        std::int32_t Used = 0;
        std::int32_t Next = 0;  // previous triangle of the same collider (0 ends the chain)
        std::int32_t Index = 0; // this slot's index
        TriangleCCW CCWTri;
    };

    // TriangleMesh.Cell: one grid cell with the indices of the triangles touching it.
    struct Cell {
        std::uint64_t Hash = 0;
        std::uint32_t Next = 0; // next cell in the same hash bucket
        std::vector<std::int32_t> Triangles;
        std::int32_t TrianglesImmutableCount = 0;
    };

    // 0x7d364cc - an empty mesh with 4096-unit cells; starts as predicted (no inserts until
    // the engine marks a verified frame).
    TriangleMesh();

    std::int32_t CellSize() const { return _cellSize; }                  // 0x7d3649c
    std::int32_t CellExtent() const { return _cellExtent; }              // 0x7d364a4
    FP CellBroadRadius() const { return _cellBroadRadius; }              // 0x7d364ac
    std::int32_t TriangleCount() const { return _trisUsedCount; }        // 0x7d364b4
    std::int32_t TriangleCapacity() const { return _trisCapacity; }      // 0x7d364bc
    void SetPredicted(bool value) { _predicted = value; }                // 0x7d364c4
    bool Predicted() const { return _predicted; }
    AssetGuid MapGuid() const { return _map; }

    // 0x7d3651c - drops everything and sets up an empty mesh: half-cell extent, the broad
    // radius of a cell (sqrt(3) / 2 * size), a prime number of hash buckets.
    void Initialize(std::int32_t cellSize, std::uint32_t cellsCapacity, std::int32_t trianglesCapacity,
                    std::int32_t staticColliderCapacity);
    // 0x7d36690
    void Dispose();

    // 0x7d367a8
    Tri* GetTriangle(std::int32_t index) { return &_tris[static_cast<std::size_t>(index)]; }
    const Tri* GetTriangle(std::int32_t index) const { return &_tris[static_cast<std::size_t>(index)]; }

    // 0x7d367cc - the cell holding a point: each raw coordinate divided by the cell size
    // (truncating), then floored to an integer.
    void ToCell(FPVector3 pos, std::int32_t& x, std::int32_t& y, std::int32_t& z) const;
    // 0x7d36810 - coordinates offset by COORDINATE_OFFSET and packed 16 bits apiece (sign
    // extension of out-of-range values included), top bit set.
    static std::uint64_t CellHash(std::int32_t x, std::int32_t y, std::int32_t z);
    // 0x7d36834 - the cells an entry's box covers, clipped to the cells in use.
    CellBounds GetBounds(const Physics3D::IntAABB& aabb) const;
    // 0x7d368c0 - the triangle indices of a cell; false (and empty) when the cell has none.
    bool TryGetTriangles(std::int32_t x, std::int32_t y, std::int32_t z, const std::int32_t*& triangles,
                         std::int32_t& count) const;

    // 0x7d36954 - inserts the triangles of a toggleable static collider (once; nothing on
    // predicted frames). An immutable collider throws std::logic_error (the original's
    // InvalidOperationException); its triangles are always in the mesh.
    void EnableStaticColliderInternal(Core::FrameBase* f, std::int32_t index);
    // 0x7d36acc - inserts a collider's triangles, chaining each to the previous one; the
    // reference names the last. Nothing (a default reference) on predicted frames.
    TriangleReference InsertMany(const std::vector<TriangleCCW>& tris);

    // TODO(0x7d372fc, Quantum.TriangleMesh.Serialize): writes or reads the whole mesh (map
    // guid, triangle and cell tables, immutable counts, static collider references) through
    // the frame serializer for snapshots; a read for another map rebuilds the mesh from it
    // (InitializeFromMap). Needed for late joins only.

    // 0x7d39c6c - triangles of the colliders present from the start (immutable or
    // toggleable-on), at least 2.
    static std::int32_t CalculateDefaultTriangleCapacityForMap(const Map& map);
    // 0x7d39698 - inserts every immutable collider's triangles.
    void InjectImmutableCollidersFromMap(const Map& map);
    // 0x7d39888 / 0x7d398ac - the number of significant bits of n (0 for 0).
    static std::int32_t BitsRequiredForNumber(std::uint32_t n);
    static std::int32_t BitsRequiredForNumber(std::int32_t n) {
        return BitsRequiredForNumber(static_cast<std::uint32_t>(n));
    }
    // 0x7d39438 - builds the mesh for a map once: sized for its colliders, immutable ones
    // inserted and locked, then the toggleable ones that start enabled.
    void InitializeFromMap(Core::FrameBase* f, const Map& map);
    // 0x7d39838 - everything inserted so far becomes the immutable base.
    void LockImmutable();

  private:
    // 0x7d36b6c - stores one triangle (reusing a freed slot first) and adds it to every
    // cell its bounds touch; the mesh's cell bounds grow to include them.
    Tri* InsertInternal(const TriangleCCW& tri);
    // 0x7d37170 - a bigger prime-sized table; the cells are rehashed in index order.
    void ExpandCellsCapacity();
    std::uint32_t BucketOf(std::uint64_t hash) const {
        return static_cast<std::uint32_t>(hash % _cellsCapacity);
    }
    Cell* FindCell(std::uint64_t hash);
    const Cell* FindCell(std::uint64_t hash) const;

    std::vector<std::uint32_t> _buckets;
    std::vector<Cell> _cells;
    std::int32_t _cellSize = 0;
    std::int32_t _cellExtent = 0;
    FP _cellBroadRadius;
    std::uint32_t _cellsFreeHead = 0;
    std::uint32_t _cellsFreeCount = 0;
    std::uint32_t _cellsUsedCount = 0;
    std::uint32_t _cellsCapacity = 0;
    std::vector<Tri> _tris;
    std::int32_t _trisFreeHead = 0;
    std::int32_t _trisUsedCount = 0;
    std::int32_t _trisCapacity = 0;
    std::int32_t _trisImmutableCount = 0;
    CellBounds _bounds;
    AssetGuid _map;
    bool _predicted = true;
    std::vector<TriangleReference> _staticColliderReferences;
};

} // namespace Quantum
