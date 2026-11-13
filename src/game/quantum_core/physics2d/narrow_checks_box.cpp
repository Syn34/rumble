// narrow_checks_box.cpp - Quantum.Physics2D.NarrowChecks.OOBB and the oriented box-box
// check (NarrowChecks.BoxBox).
#include "narrow_checks.h"

#include "photon_deterministic/fp_internal.h"
#include "photon_deterministic/fp_math.h"

#include <algorithm>
#include <cassert>

namespace Quantum::Physics2D::NarrowChecks {

namespace fpdetail = Photon::Deterministic::detail;
namespace FPMath = Photon::Deterministic::FPMath;

// 0x7d16434
void OOBB::Set(FPVector2 extents) {
    UL = {-extents.X, extents.Y};
    LL = {-extents.X, -extents.Y};
    UR = {extents.X, extents.Y};
    LR = {extents.X, -extents.Y};
}

// 0x7d16aa4
bool OOBB::InsideLocal(FPVector2 point) const {
    return LL.X <= point.X && point.X <= UR.X && LL.Y <= point.Y && point.Y <= UR.Y;
}

// 0x7d16ae0
FPVector2 OOBB::NormalFromClosestFace(FPVector2 point) const {
    FPVector2 normal = FPVector2::Up();
    FP best = FPMath::Abs(point.Y - UR.Y);
    if (const FP d = FPMath::Abs(point.Y - LL.Y); d < best) {
        normal = FPVector2::Down();
        best = d;
    }
    if (const FP d = FPMath::Abs(point.X - LL.X); d < best) {
        normal = FPVector2::Left();
        best = d;
    }
    if (FPMath::Abs(point.X - UR.X) < best) {
        normal = FPVector2::Right();
    }
    return normal;
}

// 0x7d13d10
bool BoxBox::Check(CollisionManifoldInfo2D* mInfo, bool detailedManifold) {
    LocalsContact contacts[4] = {};
    Locals locals;
    locals.penetration = FP::MaxValue();
    locals.contacts = contacts;
    const bool overlap = Collision(&locals, *mInfo->EntryA, *mInfo->EntryB);
    if (overlap && detailedManifold) {
        CollisionManifold2D* manifold = mInfo->Manifold;
        manifold->Penetration = locals.penetration;
        manifold->Normal = locals.contactNormal;
        CollisionManifold2D::AddContactPoint(manifold, contacts[0].point);
        if (locals.contactSize >= 2) {
            CollisionManifold2D::AddContactPoint(manifold, contacts[1].point);
        }
    }
    return overlap;
}

// 0x7d16188
bool BoxBox::Collision(Locals* locals, const NarrowEntry2D& a, const NarrowEntry2D& b) {
    if (!CollideOrdered(locals, a, b, false) || !CollideOrdered(locals, b, a, true)) {
        return false;
    }
    if (locals->contactSize == 0) {
        // No corner inside the other box (crossing boxes): one contact (left at the
        // origin of A's frame) and the normal along the axis found, towards B.
        locals->contactSize = 1;
        const FPVector2 offset = b.Narrow.Position - a.Narrow.Position;
        locals->contactNormal = FPVector2::Normalize(
            {locals->contactNormal.X == 0 ? FP::Zero() : offset.X,
             locals->contactNormal.Y == 0 ? FP::Zero() : offset.Y});
    }
    for (std::int32_t i = 0; i < locals->contactSize; ++i) {
        LocalsContact& contact = locals->contacts[i];
        const NarrowEntry2D& frame = contact.flipped == 1 ? b : a;
        contact.point = FPVector2::Rotate(contact.point, frame.Narrow.RotSin, frame.Narrow.RotCos) +
                        frame.Narrow.Position;
    }
    if (locals->flipped) {
        locals->contactNormal = -FPVector2::Normalize(
            FPVector2::Rotate(locals->contactNormal, b.Narrow.RotSin, b.Narrow.RotCos));
    } else {
        locals->contactNormal = FPVector2::Normalize(
            FPVector2::Rotate(locals->contactNormal, a.Narrow.RotSin, a.Narrow.RotCos));
    }
    return true;
}

// 0x7d16338
bool BoxBox::CollideOrdered(Locals* locals, const NarrowEntry2D& a, const NarrowEntry2D& b,
                            bool flipped) {
    const FPVector2 extents = a.Shape->Box._extents;
    OOBB reference;
    reference.Set(extents);
    OOBB other;
    ChangeBase(&other, a, b);
    if (!Project(locals, {fpdetail::MulInt(extents.X, 2), FP::Zero()}, other, flipped)) {
        return false;
    }
    if (!Project(locals, {FP::Zero(), fpdetail::MulInt(extents.Y, 2)}, other, flipped)) {
        return false;
    }
    FindContactPoint(locals, reference, other, flipped ? 1 : 0);
    return true;
}

// 0x7d16450
void BoxBox::ChangeBase(OOBB* box, const NarrowEntry2D& reference, const NarrowEntry2D& target) {
    box->Set(target.Shape->Box._extents);
    const FP sin = target.Narrow.RotSin;
    const FP cos = target.Narrow.RotCos;
    box->UL = FPVector2::Rotate(box->UL, sin, cos);
    box->UR = FPVector2::Rotate(box->UR, sin, cos);
    box->LL = FPVector2::Rotate(box->LL, sin, cos);
    box->LR = FPVector2::Rotate(box->LR, sin, cos);

    // Into the reference frame: translate, then rotate by the inverse angle.
    const FPVector2 offset = target.Narrow.Position - reference.Narrow.Position;
    const FP inverseSin = -reference.Narrow.RotSin;
    const FP refCos = reference.Narrow.RotCos;
    box->UL = FPVector2::Rotate(box->UL + offset, inverseSin, refCos);
    box->UR = FPVector2::Rotate(box->UR + offset, inverseSin, refCos);
    box->LL = FPVector2::Rotate(box->LL + offset, inverseSin, refCos);
    box->LR = FPVector2::Rotate(box->LR + offset, inverseSin, refCos);
}

// 0x7d165dc
bool BoxBox::Project(Locals* locals, FPVector2 axis, const OOBB& other, bool flipped) {
    // The reference box spans [-axis/2, axis/2] along the axis (only one component of the
    // axis is non-zero). Halving uses integer division, truncating towards zero.
    const FP::Raw ax = axis.X.RawValue();
    const FP::Raw ay = axis.Y.RawValue();
    const FP::Raw referenceMin = std::min(-ax / 2, -ay / 2);
    const FP::Raw referenceMax = std::max(ax / 2, ay / 2);

    auto component = [&](FPVector2 v) { return (ax != 0 ? v.X : v.Y).RawValue(); };
    const FP::Raw otherMin = std::min(std::min(component(other.LL), component(other.UL)),
                                      std::min(component(other.UR), component(other.LR)));
    const FP::Raw otherMax = std::max(std::max(component(other.LL), component(other.UL)),
                                      std::max(component(other.UR), component(other.LR)));

    const bool overlap = referenceMax >= otherMin && otherMax >= referenceMin;
    if (overlap) {
        const FP depth = FP::FromRaw(std::min(referenceMax - otherMin, otherMax - referenceMin));
        if (depth < locals->penetration) {
            locals->contactNormal = otherMax < referenceMax ? -axis : axis;
            locals->flipped = flipped;
            locals->penetration = depth;
        }
    }
    return overlap;
}

// 0x7d16844
void BoxBox::FindContactPoint(Locals* locals, const OOBB& a, const OOBB& b, std::int32_t flipped) {
    if (locals->contactSize > 1) {
        return;
    }
    for (const FPVector2 corner : {b.LL, b.UR, b.UL, b.LR}) {
        if (!Inside(corner, a)) {
            continue;
        }
        LocalsContact& contact = locals->contacts[locals->contactSize++];
        contact.point = corner;
        contact.flipped = flipped;
        if (locals->contactSize > 1) {
            return;
        }
    }
}

// 0x7d16a5c
bool BoxBox::Inside(FPVector2 point, const OOBB& shape) {
    return shape.LL.X <= point.X && point.X <= shape.UR.X && shape.LL.Y <= point.Y &&
           point.Y <= shape.UR.Y;
}

} // namespace Quantum::Physics2D::NarrowChecks
