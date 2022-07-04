#pragma once

constexpr int tile_size = 12;

ENUM( Tile ENUM_CLASS,
    (air)
    (wall)
    (_count)
)

struct Cell
{
    MEMBERS(
        DECL(Tile INIT{}) tile
    )
};

struct Map
{
    MEMBERS(
        DECL(Array2D<Cell>) cells
    )

    [[nodiscard]] static Map LoadFromFile(Stream::ReadOnlyData file);

    // Regenerates some internal data, and validates the map.
    // Is called automatically by `LoadFromFile()`.
    void ValidateAndFinalize();

    void Render(ivec2 camera_pos) const;
};
