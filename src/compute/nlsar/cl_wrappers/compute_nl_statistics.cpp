#include "compute_nl_statistics.h"

#include <stdio.h>
#include <cstdlib>
#include <sstream>

nlsar::compute_nl_statistics::compute_nl_statistics(const size_t block_size,
                                                    cl::Context context,
                                                    const int search_window_size,
                                                    const int dimension) : kernel_env<compute_nl_statistics>(block_size,
                                                                                                             context,
                                                                                                             return_build_options(search_window_size, block_size, dimension)),
                                                                           search_window_size(search_window_size),
                                                                           dimension(dimension) {}

nlsar::compute_nl_statistics::compute_nl_statistics(const compute_nl_statistics& other) : kernel_env<compute_nl_statistics>(other),
                                                                                          search_window_size(other.search_window_size),
                                                                                          dimension(other.dimension) {}

std::string nlsar::compute_nl_statistics::return_build_options(const int search_window_size, const int block_size, const int dimension)
{
    std::ostringstream out;
    out << " -D SEARCH_WINDOW_SIZE=" << search_window_size << " -D BLOCK_SIZE=" << block_size << " -D DIMENSION=" << dimension;
    return return_default_build_opts() + out.str();
}

void nlsar::compute_nl_statistics::run(cl::CommandQueue cmd_queue,
                                       cl::Buffer covmat_ori,
                                       cl::Buffer weights,
                                       cl::Buffer intensities_nl,
                                       cl::Buffer weighted_variances,
                                       cl::Buffer weight_sums,
                                       cl::Buffer eq_nols,
                                       const int height_ori,
                                       const int width_ori,
                                       const int search_window_size,
                                       const int patch_size_max,
                                       const int scale_size_max)
{
    kernel.setArg(0,  covmat_ori);
    kernel.setArg(1,  weights);
    kernel.setArg(2,  intensities_nl);
    kernel.setArg(3,  weighted_variances);
    kernel.setArg(4,  weight_sums);
    kernel.setArg(5,  eq_nols);
    kernel.setArg(6,  height_ori);
    kernel.setArg(7,  width_ori);
    kernel.setArg(8,  search_window_size);
    kernel.setArg(9,  patch_size_max);
    kernel.setArg(10, scale_size_max);

    cl::NDRange global_size {(size_t) block_size*( (height_ori - 1)/block_size + 1), \
                             (size_t) block_size*( (width_ori  - 1)/block_size + 1)};

    cl::NDRange local_size  {block_size, block_size};

    cmd_queue.enqueueNDRangeKernel(kernel, cl::NullRange, global_size, local_size, NULL, NULL);
}
