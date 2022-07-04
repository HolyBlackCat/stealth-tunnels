#include "map.h"

#include "game/main.h"
#include "gameutils/tiled_map.h"
#include "gameutils/tiles_to_edges.h"
#include "utils/json.h"

Map Map::LoadFromFile(Stream::ReadOnlyData file)
{
    Map ret;

    Json json(file.string(), 32);
    auto layer = Tiled::LoadTileLayer(Tiled::FindLayer(json.GetView(), "mid"));

    ret.cells.resize(layer.size());
    for (auto pos : vector_range(layer.size()))
        ret.cells.safe_nonthrowing_at(pos).tile = Tile(layer.safe_nonthrowing_at(pos));

    ret.ValidateAndFinalize();
    return ret;
}

void Map::ValidateAndFinalize()
{
    // Check tile IDs.
    for (auto pos : vector_range(cells.size()))
    {
        Tile tile = cells.safe_nonthrowing_at(pos).tile;
        if (tile < Tile{} || tile >= Tile::_count)
            throw std::runtime_error(FMT("Invalid tile #{} at position {}.", std::to_underlying(tile), pos));
    }

    // Generate edge loops.
    static GameUtils::TilesToEdges::TileSet edge_tileset(GameUtils::TilesToEdges::TileSet::Params{
        .tile_size = ivec2(tile_size),
        .vertices = {ivec2(0,0),ivec2(tile_size,0),ivec2(tile_size),ivec2(0,tile_size)},
        .tiles = {
            {},
            {{{0,1,2,3}}},
        },
    });

    lighting_edge_loops = Lighting::EdgeLoops([&](Lighting::EdgeLoops::VertexInputCallback callback)
    {
        GameUtils::TilesToEdges::Params params;
        params.tiles.resize(cells.size());
        for (auto pos : vector_range(cells.size()))
        {
            params.tiles.safe_nonthrowing_at(pos) = cells.safe_nonthrowing_at(pos).tile == Tile::air ? 0 : 1;
        }
        params.tileset = &edge_tileset;
        params.output_vertex = std::move(callback);
        GameUtils::TilesToEdges::Convert(params);
    });
}

void Map::Render(ivec2 camera_pos) const
{
    // Shadows.
    r.Finish();
    lighting_edge_loops.Render(camera_pos, -camera_pos);
    r.BindShader();

    // Tiles.
    ivec2 corner_a = clamp_min(div_ex(camera_pos - screen_size/2, tile_size), 0);
    ivec2 corner_b = clamp_max(div_ex(camera_pos + screen_size/2, tile_size), cells.size() - 1);
    for (ivec2 pos : corner_a <= vector_range <= corner_b)
    {
        const Cell &cell = cells.safe_nonthrowing_at(pos);

        if (cell.tile == Tile::wall)
        {
            ivec2 pixel_pos = pos * tile_size - camera_pos;
            r.iquad(pixel_pos, ivec2(tile_size)).color(fvec3(1, 1, 1));
        }
    }
}
