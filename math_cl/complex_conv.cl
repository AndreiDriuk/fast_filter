__kernel void complex_conv(
    __global const float* input,   // Входной массив комплексных чисел
    __global const float* filter,   // Фильтр (вещественный)
    __global float* output,        // Выходной массив комплексных чисел
    const int input_size,          // Размер входного массива (количество комплексных чисел * 2)
    const int filter_size          // Размер фильтра (количество вещественных коэффициентов)
) {
    // Получаем глобальный индекс потока (индекс комплексного числа)
    int gid = get_global_id(0);
    
    // Проверяем границы массива (количество комплексных чисел)
    if (gid >= input_size/2) {
        return;
    }
    
    // Инициализируем выходные значения нулями
    float sum_real = 0.0f;
    float sum_imag = 0.0f;
    
    // Выполняем свертку
    for(int i = 0; i < filter_size; ++i) {
        // Проверяем, не выходим ли за границы входного массива
        if (gid + i < input_size/2) {
            // Индексы в массивах комплексных чисел
            int input_idx = 2 * (gid + i);
            
            // Умножение комплексного числа на вещественный коэффициент
            sum_real += input[input_idx] * filter[i];
            sum_imag += input[input_idx + 1] * filter[i];
        }
    }
    
    // Записываем результат
    output[2 * gid] = sum_real;
    output[2 * gid + 1] = sum_imag;
}