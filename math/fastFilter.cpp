#include "fastFilter.h"
#include <immintrin.h>
#include <iostream>
#include <cstring>

/*

*/

void decimate(float* complex_data, uint32_t N1, float *complexFilter, uint32_t N2, float * dataOut, float *rest, uint32_t q){
    float data[4];
    int k = 0;
    size_t N = (N1-N2)/q;

    __m256 temp = _mm256_setzero_ps();
    __m256 sum = _mm256_setzero_ps();
    __m256 data_vec  = _mm256_setzero_ps();
    __m256 filter_vec  = _mm256_setzero_ps();

    for(int i = 0; i<(N1-N2); i+=2*q){
        sum = _mm256_setzero_ps();
        for(int j = 0; j<N2/8; ++j){
            data_vec = _mm256_loadu_ps(&complex_data[i+8*j]);
            filter_vec = _mm256_loadu_ps(&complexFilter[j*8]);
            temp =_mm256_mul_ps(data_vec, filter_vec);
            sum = _mm256_add_ps(sum, temp);
        }
        sum = _mm256_shuffle_ps(sum, sum,_MM_SHUFFLE(3, 1, 2, 0)); //[s1,s2, i1,i2, s3,s4, i3,i4]
        sum = _mm256_hadd_ps(sum, sum);                            //[s12, i12, s34, i34, s12, i12, s34, i34]
        temp = _mm256_permute_ps(sum,_MM_SHUFFLE(3, 1, 4, 0)); //[s12, s34, i12, i34, s12, s34, i12, i34]
        
        __m256i perm_mask = _mm256_setr_epi32(0,4,1,5,4,5,6,7);
        sum = _mm256_permutevar8x32_ps(sum, perm_mask);

        sum = _mm256_hadd_ps(sum, sum);        
         __m128 low = _mm256_castps256_ps128(sum);
        _mm_storeu_ps(data, low);
        dataOut[k] = data[0];
        dataOut[k+1] = data[1];
        k+=2;      
    }
    memcpy(rest, &complex_data[N1-(N2-2*q)], (N2-2*q)*4);
}

void filter(float* complex_data, uint32_t N1, float *complexFilter, uint32_t N2, float * dataOut, float* rest){
    float data[4];
    __m256 temp = _mm256_setzero_ps();
    __m256 sum = _mm256_setzero_ps();
    __m256 data_vec  = _mm256_setzero_ps();
    __m256 filter_vec  = _mm256_setzero_ps();
    for(int i = 0; i<(N1-N2); i+=2){
        sum = _mm256_setzero_ps();
        for(int j = 0; j<N2/8; ++j){
            data_vec = _mm256_loadu_ps(&complex_data[i+8*j]);
            filter_vec = _mm256_loadu_ps(&complexFilter[j*8]);
            temp =_mm256_mul_ps(data_vec, filter_vec);
            sum = _mm256_add_ps(sum, temp);
                    //[s1-4, i1-4, s1-4, i1-4, s1-4, i1-4, s1-4, i1-4]
        }
        sum = _mm256_shuffle_ps(sum, sum,_MM_SHUFFLE(3, 1, 2, 0)); //[s1,s2, i1,i2, s3,s4, i3,i4]
        sum = _mm256_hadd_ps(sum, sum);                            //[s12, i12, s34, i34, s12, i12, s34, i34]
        temp = _mm256_permute_ps(sum,_MM_SHUFFLE(3, 1, 4, 0)); //[s12, s34, i12, i34, s12, s34, i12, i34]
        
        __m256i perm_mask = _mm256_setr_epi32(0,4,1,5,4,5,6,7);
        sum = _mm256_permutevar8x32_ps(sum, perm_mask);

        sum = _mm256_hadd_ps(sum, sum);        
         __m128 low = _mm256_castps256_ps128(sum);
        _mm_storeu_ps(data, low);
        dataOut[i] = data[0];
        dataOut[i+1] = data[1];      
    }
    memcpy(rest, &complex_data[N1-N2], N2*4);
}