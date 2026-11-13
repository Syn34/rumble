// demo_scene.h - a synthetic scene built entirely in code.
//
// Exercises every path of the scene renderer without any game files: lit,
// normal-mapped, unlit (vertex coloured), alpha-tested, alpha-blended and
// additive materials, a GPU-skinned mesh with animated bones, a LOD group, and
// enough objects around the camera for frustum culling to reject some. Meshes are
// authored in the game's (left-handed) convention and go through the same
// conversion as the game's assets. Used by tools/sceneview --demo, the app's
// placeholder scene and the tests; safe to screenshot and compare.
#pragma once

#include "rr/scene/resources.h"
#include "rr/scene/scene.h"
#include "rr/scene/scene_renderer.h"

#include <memory>
#include <vector>

namespace rr::scene {

struct DemoScene {
    ResourceLibrary resources;
    Scene scene;
    Camera camera;
    std::vector<int> bendBones; // nodes rotated by Animate
    int lodGroup = -1;

    // Poses the skinned mesh for time `t` (seconds) and updates transforms/bounds.
    void Animate(float t);
};

std::unique_ptr<DemoScene> BuildDemoScene();

} // namespace rr::scene
