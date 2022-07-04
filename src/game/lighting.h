#pragma once

namespace Lighting
{
    SIMPLE_STRUCT( Attribs
        DECL(fvec3) pos // z is either 0 (attached to the tile) or 1 (extended to infinity).
    )

    class EdgeLoops
    {
        Graphics::VertexBuffer<Attribs> vertex_buffer;

      public:
        EdgeLoops() {}

        // `pos` is the position.
        // `last = true` ends a loop, then `pos` should be the same as the position of the first vertex.
        using VertexInputCallback = std::function<void(ivec2 pos, bool last)>;

        // `func` will be called once with the right callback. Pass all your vertices to it.
        EdgeLoops(std::function<void(VertexInputCallback callback)> func);

        // Changes the shader.
        void Render(ivec2 camera_pos, ivec2 light_pos) const;
    };
}
