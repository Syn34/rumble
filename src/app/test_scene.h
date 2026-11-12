// test_scene.h - placeholder scene used until real game rendering exists.
//
// Draws a spinning, vertex-coloured cube over a ground grid. It exercises the
// renderer end to end (buffers, pipelines, depth, culling, per-draw constants)
// and gives both backends something identical to draw for comparison.
#pragma once

#include "rr/render/render_device.h"

namespace rr::app {

class TestScene {
public:
    bool Create(render::RenderDevice& device);
    // `time` drives the animation; pass a frame-derived value for reproducible output.
    void Draw(render::RenderDevice& device, float time);

private:
    render::PipelineHandle solid_;
    render::PipelineHandle lines_;
    render::BufferHandle cubeVertices_;
    render::BufferHandle cubeIndices_;
    render::BufferHandle gridVertices_;
    std::uint32_t cubeIndexCount_ = 0;
    std::uint32_t gridVertexCount_ = 0;
};

} // namespace rr::app
