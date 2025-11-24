#include <cstdint>
#include <immintrin.h>
// Function for fast Filtering using intinsics

void complexFilterAVX2(const float* data, int N1, const float* filter, int N2, float* output);
void complexFilterAVXDecimate2Optimized(const float* data, int N1, const float* filter, int N2, float* output);

void myComplexFilter(float* complex_data, uint32_t N1, float *complexFilter, uint32_t N2, float * dataOut, float *rest);
float horizontalSumAVX( __m256 vec);

