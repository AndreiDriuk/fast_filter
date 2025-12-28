#include <cstdint>
#include <immintrin.h>
// Function for fast Filtering using intinsics

void filter(float* complex_data, uint32_t N1, float *complexFilter, uint32_t N2, float * dataOut, float *rest);

uint32_t decimate(float* complex_data, uint32_t N1, float *complexFilter, uint32_t N2, float * dataOut, float *rest, uint32_t q);

void decimate(float* complex_data, uint32_t N1, float *complexFilter, uint32_t N2, float * dataOut, uint32_t q);