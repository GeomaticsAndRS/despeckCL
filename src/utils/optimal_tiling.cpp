#include "optimal_tiling.h"

#include <algorithm>

std::vector<std::pair<int, int>> all_pairs(std::vector<int> range) {
  std::vector<std::pair<int, int>> pairs;
  for(auto iter1 = std::begin(range); iter1 != std::end(range); ++iter1) {
    for(auto iter2 = std::begin(range); iter2 != std::end(range); ++iter2) {
      pairs.push_back(std::pair<int, int>(*iter1, *iter2));
    }
  }
  return pairs;
}

size_t tiled_img_npixels(std::pair<int, int> tile_dim, std::pair<int, int> nm_tiles) {
  const size_t n_tiles = nm_tiles.first*nm_tiles.second;
  const size_t n_pixels_per_tile = tile_dim.first*tile_dim.second;
  return n_tiles*n_pixels_per_tile;
}

std::vector<std::pair<int, int>>
retain_small_offcut_tiles(std::vector<std::pair<int, int>> tiles,
                          size_t img_height, size_t img_width, float offcut) {
  get_nm_tiles gnmt (img_height, img_width);
  tiles.erase(std::remove_if(tiles.begin(), tiles.end(),
                             [&](auto t) {
                               return tiled_img_npixels(t, gnmt(t)) >
                                      offcut * img_height * img_width;
                             }),
              tiles.end());
  return tiles;
}

std::pair<int, int> biggest_tile(std::vector<std::pair<int, int>> tiles) {
  return *std::max_element(tiles.begin(), tiles.end(), [] (auto t1, auto t2) {return t1.first*t1.second < t2.first*t2.second;});
}
