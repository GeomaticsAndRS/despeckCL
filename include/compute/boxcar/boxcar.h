#ifndef BOXCAR_H
#define BOXCAR_H

#include "utils/easylogging++.h"

void boxcar(float* master_amplitude,
            float* slave_amplitude,
            float* dphase,
            float* ampl_filt,
            float* dphase_filt,
            float* coh_filt,
            const int height,
            const int width,
            const int window_width,
            std::vector<el::Level> enabled_log_levels);
#endif