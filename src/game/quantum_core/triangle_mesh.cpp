// triangle_mesh.cpp - Quantum.TriangleMesh: the 3D scene's static triangle grid.
#include "triangle_mesh.h"

#include "photon_deterministic/fp_math.h"
#include "quantum_core/frame_base.h"
#include "quantum_core/map.h"
#include "quantum_core/physics3d/collision_manifold3d.h"
#include "quantum_core/q_buffer.h"

#include <algorithm>
#include <stdexcept>

namespace Quantum {

namespace {
// A raw coordinate's cell: divided by the cell size (truncating, as C#'s long division),
// then floored by the shift.
std::int32_t CellOf(std::int64_t raw, std::int32_t cellSize) {
    return static_cast<std::int32_t>((raw / cellSize) >> 16);
}
} // namespace

TriangleMesh::TriangleMesh() {
    Initialize(0x1000, 1, 1, 1);
    _predicted = true;
}

void TriangleMesh::Initialize(std::int32_t cellSize, std::uint32_t cellsCapacity, std::int32_t trianglesCapacity,
                              std::int32_t staticColliderCapacity) {
    Dispose();
    _cellSize = cellSize;
    _cellExtent = cellSize / 2;
    // sqrt(3) / 2 per unit of cell size: the distance from a cell's centre to a corner.
    const FP::Raw halfSqrt3 = Photon::Deterministic::FPMath::SqrtRaw(FP::Raw{3} << 16) >> 1;
    _cellBroadRadius = FP::FromRaw(halfSqrt3 * cellSize);
    _bounds = CellBounds{};
    _cellsUsedCount = 1;
    _cellsFreeHead = 0;
    _cellsFreeCount = 0;
    _cellsCapacity = Collections::QCollectionsUtils::GetNextPrimeGreaterThanOrEqual(cellsCapacity);
    _trisFreeHead = 0;
    _trisUsedCount = 1;
    _trisCapacity = trianglesCapacity + 1;
    _trisImmutableCount = 0;
    _buckets.assign(_cellsCapacity, 0u);
    _cells.assign(_cellsCapacity, Cell{});
    _tris.assign(static_cast<std::size_t>(_trisCapacity), Tri{});
    _staticColliderReferences.assign(static_cast<std::size_t>(std::max(0, staticColliderCapacity)),
                                     TriangleReference{});
}

void TriangleMesh::Dispose() {
    _buckets.clear();
    _cells.clear();
    _tris.clear();
    _staticColliderReferences.clear();
}

void TriangleMesh::ToCell(FPVector3 pos, std::int32_t& x, std::int32_t& y, std::int32_t& z) const {
    x = CellOf(pos.X.RawValue(), _cellSize);
    y = CellOf(pos.Y.RawValue(), _cellSize);
    z = CellOf(pos.Z.RawValue(), _cellSize);
}

std::uint64_t TriangleMesh::CellHash(std::int32_t x, std::int32_t y, std::int32_t z) {
    // Each offset coordinate is sign-extended before it is shifted into place.
    const auto part = [](std::int32_t c) {
        return static_cast<std::uint64_t>(static_cast<std::int64_t>(c + COORDINATE_OFFSET));
    };
    return part(x) | (part(y) << 16) | (part(z) << 32) | 0x8000000000000000ull;
}

TriangleMesh::CellBounds TriangleMesh::GetBounds(const Physics3D::IntAABB& aabb) const {
    CellBounds cells;
    cells.MinX = std::max(_bounds.MinX, CellOf(aabb.MinX, _cellSize));
    cells.MaxX = std::min(_bounds.MaxX, CellOf(aabb.MaxX, _cellSize));
    cells.MinY = std::max(_bounds.MinY, CellOf(aabb.MinY, _cellSize));
    cells.MaxY = std::min(_bounds.MaxY, CellOf(aabb.MaxY, _cellSize));
    cells.MinZ = std::max(_bounds.MinZ, CellOf(aabb.MinZ, _cellSize));
    cells.MaxZ = std::min(_bounds.MaxZ, CellOf(aabb.MaxZ, _cellSize));
    return cells;
}

TriangleMesh::Cell* TriangleMesh::FindCell(std::uint64_t hash) {
    for (std::uint32_t index = _buckets[BucketOf(hash)]; index != 0; index = _cells[index].Next) {
        if (_cells[index].Hash == hash) {
            return &_cells[index];
        }
    }
    return nullptr;
}

const TriangleMesh::Cell* TriangleMesh::FindCell(std::uint64_t hash) const {
    return const_cast<TriangleMesh*>(this)->FindCell(hash);
}

bool TriangleMesh::TryGetTriangles(std::int32_t x, std::int32_t y, std::int32_t z, const std::int32_t*& triangles,
                                   std::int32_t& count) const {
    const Cell* cell = _cellsCapacity == 0 ? nullptr : FindCell(CellHash(x, y, z));
    if (cell == nullptr || cell->Triangles.empty()) {
        triangles = nullptr;
        count = 0;
        return false;
    }
    triangles = cell->Triangles.data();
    count = static_cast<std::int32_t>(cell->Triangles.size());
    return true;
}

void TriangleMesh::EnableStaticColliderInternal(Core::FrameBase* f, std::int32_t index) {
    const Map* map = f->get_Map();
    if (map == nullptr) {
        throw std::invalid_argument("TriangleMesh: no map");
    }
    const MapStaticCollider3D& collider = map->StaticColliders3D.at(static_cast<std::size_t>(index));
    if (collider.MutableMode == StaticColliderMutableMode::Immutable) {
        throw std::logic_error("TriangleMesh: cannot toggle an immutable static collider");
    }
    if (_predicted) {
        return;
    }
    const auto slot = static_cast<std::size_t>(index);
    if (slot < _staticColliderReferences.size() && !(_staticColliderReferences[slot] == TriangleReference{})) {
        return;
    }
    if (slot >= _staticColliderReferences.size()) {
        // The original grows the buffer to twice the index (without recording it).
        _staticColliderReferences.resize(std::max<std::size_t>(slot * 2, slot + 1));
    }
    // Dictionary indexer: a collider without triangles throws (KeyNotFoundException).
    const std::vector<TriangleCCW>& tris = map->StaticColliders3DTriangles.at(index);
    _staticColliderReferences[slot] = InsertMany(tris);
}

TriangleReference TriangleMesh::InsertMany(const std::vector<TriangleCCW>& tris) {
    if (_predicted) {
        return {};
    }
    std::int32_t last = 0;
    for (const TriangleCCW& tri : tris) {
        Tri* slot = InsertInternal(tri);
        slot->Next = last;
        last = slot->Index;
    }
    return TriangleReference(last, true);
}

TriangleMesh::Tri* TriangleMesh::InsertInternal(const TriangleCCW& tri) {
    Tri* slot = nullptr;
    if (_trisFreeHead != 0) {
        slot = &_tris[static_cast<std::size_t>(_trisFreeHead)];
        _trisFreeHead = slot->Next;
        slot->Used = 1;
        slot->Next = 0;
    } else {
        if (_trisUsedCount == _trisCapacity) {
            _trisCapacity *= 2;
            _tris.resize(static_cast<std::size_t>(_trisCapacity));
        }
        slot = &_tris[static_cast<std::size_t>(_trisUsedCount)];
        slot->Index = _trisUsedCount;
        slot->Used = 1;
        ++_trisUsedCount;
    }
    slot->CCWTri = tri;

    const FPVector3 low = slot->CCWTri.Min3();
    const FPVector3 high = slot->CCWTri.Max3();
    const std::int32_t minX = CellOf(low.X.RawValue(), _cellSize);
    const std::int32_t minY = CellOf(low.Y.RawValue(), _cellSize);
    const std::int32_t minZ = CellOf(low.Z.RawValue(), _cellSize);
    const std::int32_t maxX = CellOf(high.X.RawValue(), _cellSize);
    const std::int32_t maxY = CellOf(high.Y.RawValue(), _cellSize);
    const std::int32_t maxZ = CellOf(high.Z.RawValue(), _cellSize);
    _bounds.MinX = std::min(_bounds.MinX, minX);
    _bounds.MinY = std::min(_bounds.MinY, minY);
    _bounds.MinZ = std::min(_bounds.MinZ, minZ);
    _bounds.MaxX = std::max(_bounds.MaxX, maxX);
    _bounds.MaxY = std::max(_bounds.MaxY, maxY);
    _bounds.MaxZ = std::max(_bounds.MaxZ, maxZ);

    for (std::int32_t z = minZ; z <= maxZ; ++z) {
        for (std::int32_t y = minY; y <= maxY; ++y) {
            for (std::int32_t x = minX; x <= maxX; ++x) {
                const std::uint64_t hash = CellHash(x, y, z);
                Cell* cell = FindCell(hash);
                if (cell == nullptr) {
                    std::uint32_t index = 0;
                    if (_cellsFreeHead != 0) {
                        index = _cellsFreeHead;
                        _cellsFreeHead = _cells[index].Next;
                        --_cellsFreeCount;
                    } else {
                        if (_cellsUsedCount == _cellsCapacity) {
                            ExpandCellsCapacity();
                        }
                        index = _cellsUsedCount++;
                    }
                    cell = &_cells[index];
                    const std::uint32_t bucket = BucketOf(hash);
                    cell->Hash = hash;
                    cell->Next = _buckets[bucket];
                    _buckets[bucket] = index;
                }
                cell->Triangles.push_back(slot->Index);
            }
        }
    }
    return slot;
}

void TriangleMesh::ExpandCellsCapacity() {
    const std::uint32_t oldCapacity = _cellsCapacity;
    _cellsCapacity = Collections::QCollectionsUtils::GetNextPrime(oldCapacity);
    _cells.resize(_cellsCapacity);
    _buckets.assign(_cellsCapacity, 0u);
    for (std::uint32_t index = 1; index < oldCapacity; ++index) {
        const std::uint32_t bucket = BucketOf(_cells[index].Hash);
        _cells[index].Next = _buckets[bucket];
        _buckets[bucket] = index;
    }
}

std::int32_t TriangleMesh::CalculateDefaultTriangleCapacityForMap(const Map& map) {
    std::int32_t count = 0;
    for (const auto& [index, tris] : map.StaticColliders3DTriangles) {
        const auto mode = map.StaticColliders3D.at(static_cast<std::size_t>(index)).MutableMode;
        if (static_cast<std::uint32_t>(mode) < 2) {
            count += static_cast<std::int32_t>(tris.size());
        }
    }
    return std::max(2, count);
}

void TriangleMesh::InjectImmutableCollidersFromMap(const Map& map) {
    for (const auto& [index, tris] : map.StaticColliders3DTriangles) {
        if (map.StaticColliders3D.at(static_cast<std::size_t>(index)).MutableMode ==
            StaticColliderMutableMode::Immutable) {
            InsertMany(tris);
        }
    }
}

std::int32_t TriangleMesh::BitsRequiredForNumber(std::uint32_t n) {
    for (std::int32_t bits = 32; bits > 0; --bits) {
        if (((n >> (bits - 1)) & 1u) != 0) {
            return bits;
        }
    }
    return 0;
}

void TriangleMesh::InitializeFromMap(Core::FrameBase* f, const Map& map) {
    if (_map == map.Guid) {
        return;
    }
    _map = map.Guid;
    Initialize(map.TriangleMeshCellSize, Collections::QCollectionsUtils::GetNextPrime(std::uint32_t{0x1000}),
               CalculateDefaultTriangleCapacityForMap(map), static_cast<std::int32_t>(map.StaticColliders3D.size()));
    InjectImmutableCollidersFromMap(map);
    LockImmutable();
    for (const auto& [index, tris] : map.StaticColliders3DTriangles) {
        if (map.StaticColliders3D.at(static_cast<std::size_t>(index)).MutableMode ==
            StaticColliderMutableMode::ToggleableStartOn) {
            EnableStaticColliderInternal(f, index);
        }
    }
}

void TriangleMesh::LockImmutable() {
    _trisImmutableCount = _trisUsedCount;
    for (Cell& cell : _cells) {
        cell.TrianglesImmutableCount = static_cast<std::int32_t>(cell.Triangles.size());
    }
}

} // namespace Quantum
