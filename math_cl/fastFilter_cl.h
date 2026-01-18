#pragma once

#include <cstdint>

void filter(float* complex_data, uint32_t N1, float *complexFilter, uint32_t N2, float * dataOut, float *rest);
