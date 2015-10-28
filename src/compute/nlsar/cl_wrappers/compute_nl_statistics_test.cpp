#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP

#include <iostream>

#include "compute_nl_statistics.h"

using namespace nlsar;

TEST_CASE( "compute_nl_statistics", "[cl_kernels]" ) {

        // data setup
        const int height_ori = 40;
        const int width_ori = 50;

        const int search_window_size = 21;
        const int patch_size = 7;
        const int scale_width = 1;
        const int overlap_avg = (patch_size-1)/2 + (search_window_size-1)/2 + (scale_width-1)/2;
        const int dimension = 2;

        const int covmat_in_nelem    = (height_ori + 2*overlap_avg) * (width_ori + 2*overlap_avg) * dimension * dimension * 2;
        const int weights_nelem      = height_ori * width_ori * search_window_size * search_window_size;
        const int weights_sums_nelem = height_ori * width_ori;
        const int stats_nelem        = height_ori * width_ori * dimension;

        //input
        std::vector<float> covmat_in            (covmat_in_nelem, 1.0);
        std::vector<float> weights              (weights_nelem,   1.0);

        //output
        std::vector<float> weights_sums           (weights_sums_nelem, -1.0);
        std::vector<float> eq_nol                 (weights_sums_nelem, -1.0);
        std::vector<float> desired_weights_sums   (weights_sums_nelem, search_window_size*search_window_size);
        std::vector<float> intensities_nl             (stats_nelem, -1.0);
        std::vector<float> desired_intensities_nl     (stats_nelem,  1.0);
        std::vector<float> weighted_variances         (stats_nelem, -1.0);
        std::vector<float> desired_weighted_variances (stats_nelem,  0.0);

        // opencl setup
        cl::Context context = opencl_setup();

        std::vector<cl::Device> devices;
        context.getInfo(CL_CONTEXT_DEVICES, &devices);

        cl::CommandQueue cmd_queue{context, devices[0]};

        // kernel setup
        const int block_size = 16;
        compute_nl_statistics KUT{block_size, context, search_window_size, dimension};

        // allocate memory
        cl::Buffer device_covmat_in  {context, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, covmat_in_nelem * sizeof(float), covmat_in.data(), NULL};
        cl::Buffer device_weights    {context, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, weights_nelem   * sizeof(float), weights.data(),   NULL};

        cl::Buffer device_weights_sums       {context, CL_MEM_READ_WRITE, weights_sums_nelem * sizeof(float), NULL, NULL};
        cl::Buffer device_eq_nol             {context, CL_MEM_READ_WRITE, weights_sums_nelem * sizeof(float), NULL, NULL};
        cl::Buffer device_intensities_nl     {context, CL_MEM_READ_WRITE,        stats_nelem * sizeof(float), NULL, NULL};
        cl::Buffer device_weighted_variances {context, CL_MEM_READ_WRITE,        stats_nelem * sizeof(float), NULL, NULL};

        KUT.run(cmd_queue, 
                device_covmat_in,
                device_weights,
                device_intensities_nl,
                device_weighted_variances,
                device_weights_sums,
                device_eq_nol,
                height_ori,
                width_ori,
                search_window_size,
                patch_size,
                scale_width);

        cmd_queue.enqueueReadBuffer(device_weights_sums,       CL_TRUE, 0, weights_sums_nelem * sizeof(float), weights_sums.data(),       NULL, NULL);
        cmd_queue.enqueueReadBuffer(device_eq_nol,             CL_TRUE, 0, weights_sums_nelem * sizeof(float), eq_nol.data(),             NULL, NULL);
        cmd_queue.enqueueReadBuffer(device_intensities_nl,     CL_TRUE, 0,        stats_nelem * sizeof(float), intensities_nl.data(),     NULL, NULL);
        cmd_queue.enqueueReadBuffer(device_weighted_variances, CL_TRUE, 0,        stats_nelem * sizeof(float), weighted_variances.data(), NULL, NULL);

        bool weights_sums_flag = true;
        for(int i = 0; i < (int) weights_sums.size(); i++) {
            weights_sums_flag = weights_sums_flag && Approx(weights_sums[i]) == desired_weights_sums[i];

        }

        bool eq_nol_flag = true;
        for(int i = 0; i < (int) eq_nol.size(); i++) {
            eq_nol_flag = eq_nol_flag && Approx(eq_nol[i]) == search_window_size*search_window_size;
        }

        bool intensities_nl_flag = true;
        for(int i = 0; i < (int) intensities_nl.size(); i++) {
            intensities_nl_flag = intensities_nl_flag && Approx(intensities_nl[i]) == desired_intensities_nl[i];
        }

        bool weighted_variances_flag = true;
        for(int i = 0; i < (int) weighted_variances.size(); i++) {
            weighted_variances_flag = weighted_variances_flag && Approx(weighted_variances[i]) == desired_weighted_variances[i];
        }

        REQUIRE( (weights_sums_flag) );
        REQUIRE( (eq_nol_flag) );
        REQUIRE( (intensities_nl_flag) );
        REQUIRE( (weighted_variances_flag) );
}
