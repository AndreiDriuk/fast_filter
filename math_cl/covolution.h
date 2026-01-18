const char* data = 
    "__kernel void convolution_1d(\n"
    "__global const float* input,   // Входной массив\n"
    "__global const float* kernel,  // Массив ядра свертки\n"
    "__global float* output,        // Выходной массив\n"
    "const int input_size,          // Размер входного массива\n"
    "const int kernel_size,         // Размер ядра свертки\n"
    "const int output_size          // Размер выходного массива\n"
") {\n"
"    // Получаем глобальный индекс потока\n"
"    int gid = get_global_id(0);\n"
    
    // Проверяем границы массива\n"
"    if (gid >= output_size) {\n"
"        return;\n"
"    }\n"
    
"    float sum = 0.0f;\n"
    
"    // Вычисляем свертку\n"
"    for (int k = 0; k < kernel_size; k++) {\n"
"        int input_index = gid + k;\n"
        
"        // Проверяем границы входного массива\n"
"        if (input_index >= 0 && input_index < input_size) {\n"
"            sum += input[input_index] * kernel[k];\n"
"        }\n"
"        // Для режима 'same' или 'valid' может потребоваться другая обработка границ\n"
"    }\n"
"    // Записываем результат\n"
"    output[gid] = sum;\n"
"}\n";