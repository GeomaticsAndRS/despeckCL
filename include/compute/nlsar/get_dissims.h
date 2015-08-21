#ifndef GET_DISSIMS_H
#define GET_DISSIMS_H

#include <vector>

#include "insar_data.h"

std::vector<float> get_dissims(const insar_data& sub_insar_data,
                               const int patch_size,
                               const int window_width = 3);

#endif