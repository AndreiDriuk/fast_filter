#include "fastFilter.h"
#include <immintrin.h>
#include <iostream>
#include <cstring>

/*

*/
void myComplexFilter(float* complex_data, uint32_t N1, float *complexFilter, uint32_t N2, float * dataOut, float* rest){
    // sum -1
    // temp - 2
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

void complexFilterAVX2(const float* data, int N1, const float* filter, int N2, float* output) {
    for (int i = 0; i <= N1 - N2; ++i) {
        __m256 sum_real = _mm256_setzero_ps();
        __m256 sum_imag = _mm256_setzero_ps();

        int j = 0;
        for (; j <= N2 - 4; j += 4) {
            // Загружаем данные и фильтр
            __m256 data_vec = _mm256_loadu_ps(&data[(i + j) * 2]);
            __m256 filter_vec = _mm256_loadu_ps(&filter[j * 2]);

            // Создаем маски для разделения реальных и мнимых частей
            __m256 real_mask = _mm256_set_ps(0.0f, -0.0f, 0.0f, -0.0f, 0.0f, -0.0f, 0.0f, -0.0f);
            __m256 imag_mask = _mm256_set_ps(-0.0f, 0.0f, -0.0f, 0.0f, -0.0f, 0.0f, -0.0f, 0.0f);
            
            // Извлекаем реальные и мнимые части с помощью масок
            __m256 data_real = _mm256_andnot_ps(real_mask, data_vec);
            __m256 data_imag = _mm256_andnot_ps(imag_mask, data_vec);
            __m256 filter_real = _mm256_andnot_ps(real_mask, filter_vec);
            __m256 filter_imag = _mm256_andnot_ps(imag_mask, filter_vec);

            // Вычисляем комплексное умножение
            __m256 real_real = _mm256_mul_ps(data_real, filter_real);
            __m256 imag_imag = _mm256_mul_ps(data_imag, filter_imag);
            __m256 real_imag = _mm256_mul_ps(data_real, filter_imag);
            __m256 imag_real = _mm256_mul_ps(data_imag, filter_real);

            __m256 result_real = _mm256_sub_ps(real_real, imag_imag);
            __m256 result_imag = _mm256_add_ps(real_imag, imag_real);

            sum_real = _mm256_add_ps(sum_real, result_real);
            sum_imag = _mm256_add_ps(sum_imag, result_imag);
        }

        // Редукция суммы
        __m128 sum_low_real = _mm256_extractf128_ps(sum_real, 0);
        __m128 sum_high_real = _mm256_extractf128_ps(sum_real, 1);
        __m128 sum_low_imag = _mm256_extractf128_ps(sum_imag, 0);
        __m128 sum_high_imag = _mm256_extractf128_ps(sum_imag, 1);

        __m128 total_real = _mm_add_ps(sum_low_real, sum_high_real);
        __m128 total_imag = _mm_add_ps(sum_low_imag, sum_high_imag);

        // Горизонтальное сложение
        total_real = _mm_hadd_ps(total_real, total_real);
        total_real = _mm_hadd_ps(total_real, total_real);
        total_imag = _mm_hadd_ps(total_imag, total_imag);
        total_imag = _mm_hadd_ps(total_imag, total_imag);

        float final_real, final_imag;
        _mm_store_ss(&final_real, total_real);
        _mm_store_ss(&final_imag, total_imag);

        // Добавляем оставшиеся элементы
        for (; j < N2; ++j) {
            int data_idx = (i + j) * 2;
            int filter_idx = j * 2;
            
            float data_real = data[data_idx];
            float data_imag = data[data_idx + 1];
            float filter_real = filter[filter_idx];
            float filter_imag = filter[filter_idx + 1];
            
            final_real += data_real * filter_real - data_imag * filter_imag;
            final_imag += data_real * filter_imag + data_imag * filter_real;
        }

        output[i * 2] = final_real;
        output[i * 2 + 1] = final_imag;
    }
}

void complexFilterAVXDecimate2Optimized(const float* data, int N1, const float* filter, int N2, float* output) {
    // Проверяем корректность размеров
    if (N1 < N2 || N2 <= 0) return;
    
    // Размер выходного массива с учетом децимации
    int output_size = (N1 - N2 + 1 + 1) / 2; // +1 для округления вверх
    
    // Обрабатываем каждую вторую позицию
    for (int output_idx = 0; output_idx < output_size; ++output_idx) {
        int i = output_idx * 2; // Позиция во входном массиве
        
        __m256 sum_real = _mm256_setzero_ps();
        __m256 sum_imag = _mm256_setzero_ps();

        int j = 0;
        // Основной цикл с векторизацией
        for (; j <= N2 - 8; j += 8) {
            // Обрабатываем два блока по 4 комплексных числа
            for (int block = 0; block < 2; ++block) {
                int current_j = j + block * 4;
                __m256 data_vec = _mm256_loadu_ps(&data[(i + current_j) * 2]);
                __m256 filter_vec = _mm256_loadu_ps(&filter[current_j * 2]);

                // Эффективное разделение реальных и мнимых частей
                __m256 data_real = _mm256_mul_ps(data_vec, _mm256_set1_ps(1.0f));
                __m256 data_imag = _mm256_mul_ps(data_vec, _mm256_set1_ps(1.0f));
                __m256 filter_real = _mm256_mul_ps(filter_vec, _mm256_set1_ps(1.0f));
                __m256 filter_imag = _mm256_mul_ps(filter_vec, _mm256_set1_ps(1.0f));
                
                // Применяем маски для разделения
                __m256 real_mask = _mm256_set_ps(0.0f, -0.0f, 0.0f, -0.0f, 0.0f, -0.0f, 0.0f, -0.0f);
                __m256 imag_mask = _mm256_set_ps(-0.0f, 0.0f, -0.0f, 0.0f, -0.0f, 0.0f, -0.0f, 0.0f);
                
                data_real = _mm256_andnot_ps(real_mask, data_real);
                data_imag = _mm256_andnot_ps(imag_mask, data_imag);
                filter_real = _mm256_andnot_ps(real_mask, filter_real);
                filter_imag = _mm256_andnot_ps(imag_mask, filter_imag);

                // Комплексное умножение и аккумуляция
                __m256 temp_real = _mm256_sub_ps(_mm256_mul_ps(data_real, filter_real), 
                                               _mm256_mul_ps(data_imag, filter_imag));
                __m256 temp_imag = _mm256_add_ps(_mm256_mul_ps(data_real, filter_imag), 
                                               _mm256_mul_ps(data_imag, filter_real));
                
                sum_real = _mm256_add_ps(sum_real, temp_real);
                sum_imag = _mm256_add_ps(sum_imag, temp_imag);
            }
        }

        // Обработка оставшихся элементов блоками по 4
        for (; j <= N2 - 4; j += 4) {
            __m256 data_vec = _mm256_loadu_ps(&data[(i + j) * 2]);
            __m256 filter_vec = _mm256_loadu_ps(&filter[j * 2]);

            // Упрощенное комплексное умножение
            __m256 real_real = _mm256_mul_ps(_mm256_shuffle_ps(data_vec, data_vec, _MM_SHUFFLE(2, 0, 2, 0)),
                                           _mm256_shuffle_ps(filter_vec, filter_vec, _MM_SHUFFLE(2, 0, 2, 0)));
            __m256 imag_imag = _mm256_mul_ps(_mm256_shuffle_ps(data_vec, data_vec, _MM_SHUFFLE(3, 1, 3, 1)),
                                           _mm256_shuffle_ps(filter_vec, filter_vec, _MM_SHUFFLE(3, 1, 3, 1)));
            __m256 real_imag = _mm256_mul_ps(_mm256_shuffle_ps(data_vec, data_vec, _MM_SHUFFLE(2, 0, 2, 0)),
                                           _mm256_shuffle_ps(filter_vec, filter_vec, _MM_SHUFFLE(3, 1, 3, 1)));
            __m256 imag_real = _mm256_mul_ps(_mm256_shuffle_ps(data_vec, data_vec, _MM_SHUFFLE(3, 1, 3, 1)),
                                           _mm256_shuffle_ps(filter_vec, filter_vec, _MM_SHUFFLE(2, 0, 2, 0)));

            sum_real = _mm256_add_ps(sum_real, _mm256_sub_ps(real_real, imag_imag));
            sum_imag = _mm256_add_ps(sum_imag, _mm256_add_ps(real_imag, imag_real));
        }

        // Редукция и сохранение результата
        float final_real = horizontalSumAVX(sum_real);
        float final_imag = horizontalSumAVX(sum_imag);

        // Скалярная обработка хвостов
        for (; j < N2; ++j) {
            int data_idx = (i + j) * 2;
            int filter_idx = j * 2;
            
            final_real += data[data_idx] * filter[filter_idx] - data[data_idx + 1] * filter[filter_idx + 1];
            final_imag += data[data_idx] * filter[filter_idx + 1] + data[data_idx + 1] * filter[filter_idx];
        }

        output[output_idx * 2] = final_real;
        output[output_idx * 2 + 1] = final_imag;
    }
}

// Вспомогательная функция для горизонтального суммирования
float horizontalSumAVX(__m256 vec) {
    __m128 vlow = _mm256_extractf128_ps(vec, 0);
    __m128 vhigh = _mm256_extractf128_ps(vec, 1);
    __m128 sum128 = _mm_add_ps(vlow, vhigh);
    sum128 = _mm_hadd_ps(sum128, sum128);
    sum128 = _mm_hadd_ps(sum128, sum128);
    float result;
    _mm_store_ss(&result, sum128);
    return result;
}