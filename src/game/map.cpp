#include "map.h"

#include "game/main.h"
#include "gameutils/tiled_map.h"
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
    for (auto pos : vector_range(cells.size()))
    {
        Tile tile = cells.safe_nonthrowing_at(pos).tile;
        if (tile < Tile{} || tile >= Tile::_count)
            throw std::runtime_error(FMT("Invalid tile #{} at position {}.", std::to_underlying(tile), pos));
    }
}

void Map::Render(ivec2 camera_pos) const
{
    ivec2 corner_a = clamp_min(div_ex(camera_pos - screen_size/2, tile_size), 0);
    ivec2 corner_b = clamp_max(div_ex(camera_pos + screen_size/2, tile_size), cells.size() - 1);
    for (ivec2 pos : corner_a <= vector_range <= corner_b)
    {
        const Cell &cell = cells.safe_nonthrowing_at(pos);

        if (cell.tile == Tile::wall)
        {
            ivec2 pixel_pos = pos * tile_size - camera_pos;
            r.iquad(pixel_pos, ivec2(tile_size)).color(fvec3(0,0.5,1));
        }
    }
}
