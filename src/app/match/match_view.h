// match_view.h - presentation of a running match (PC glue in place of the original's Unity
// view layer: EntityViewUpdater, EntityView and the game's camera rigs).
//
// Every entity with a View component gets an instance of its EntityView asset's prefab in
// an engine scene; each frame the instances follow the entities' transforms, interpolated
// between the previous and the current predicted frame. The camera orbits the local
// player's actor (PlayerActor of slot 0), or the middle of the viewed entities when there
// is none.
#pragma once

#include "game_content.h"
#include "match_input.h"

#include "quantum_code/frame.h"
#include "rr/assets/prefab.h"
#include "rr/assets/streamed_data.h"
#include "rr/scene/content_resolver.h"
#include "rr/scene/scene.h"
#include "rr/scene/scene_renderer.h"

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <vector>

namespace rr::app {

struct ViewStats {
    std::size_t viewedEntities = 0;    // entities with a View component (last sync)
    std::size_t instances = 0;         // prefab instances alive
    std::size_t instancesCreated = 0;
    std::size_t viewAssets = 0;        // distinct EntityView assets seen
    std::size_t viewsWithoutPrefab = 0;// EntityView assets whose prefab couldn't be read
    std::size_t entitiesWithoutPrefab = 0;
};

class MatchView {
public:
    explicit MatchView(GameContent& content);

    // Brings the scene up to date with the frame (interpolated toward `current` from
    // `previous` by `alpha` when both have the entity).
    void Sync(Quantum::Frame& current, Quantum::Frame* previous, float alpha);
    // The camera for the given angles around the followed point.
    scene::Camera Camera(const CameraAngles& angles) const;

    // Instantiates the prefab of every given EntityView asset once (numeric check of the
    // EntityView -> prefab path; the instances stay hidden). Returns how many resolved.
    std::size_t CheckViews(const std::vector<Quantum::AssetObjectIdentifier>& views);

    scene::Scene& Scene() { return scene_; }
    scene::ResourceLibrary& Resources() { return resources_; }
    const ViewStats& Stats() const { return stats_; }
    const scene::ContentStats& ContentStats() const { return resolver_.Stats(); }

private:
    struct Instance {
        int node = -1;
        std::int64_t viewGuid = 0;
        bool seen = false;
    };
    const assets::Prefab* PrefabOf(Quantum::AssetGuid view);
    int Spawn(Quantum::AssetGuid view);

    GameContent& content_;
    scene::Scene scene_;
    scene::ResourceLibrary resources_;
    assets::ResourceStore store_;
    scene::ContentResolver resolver_;
    assets::PrefabReader reader_;
    std::map<std::int64_t, std::optional<assets::Prefab>> prefabs_;
    std::map<std::int64_t, Instance> instances_;            // by EntityRef (index | version << 32)
    std::map<std::int64_t, std::vector<int>> pooled_;       // hidden instances by view GUID
    scene::Vec3 focus_{0, 0, 0};
    bool haveFocus_ = false;
    ViewStats stats_;
};

} // namespace rr::app
