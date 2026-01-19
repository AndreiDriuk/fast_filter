__kernel void c_conv(
__global const float* input,   // Входной массив
__global const float* kernel_f,  // Массив ядра свертки
__global float* output,        // Выходной массив
const int input_size,          // Размер входного массива
const int kernel_size,         // Размер ядра свертки
const int output_size          // Размер выходного массиваx
) {
    // Получаем глобальный индекс потока
    int gid = get_global_id(0);
    if(mod(gid),2)==0){
        // Проверяем границы массива
        if (gid >= output_size) {
            return;
        }

        float real1 = 0.0f;
        float imag2 = 0.0f;

        // Вычисляем свертку
        for (int k = 0; k < kernel_size; k++) {
            int input_index = gid + k;

            // Проверяем границы входного массива
            if (input_index >= 0 && input_index < input_size) {
                sum += input[input_index] * kernel_f[k];
            }
        }
        // Записываем результат
        output[gid] = sum;
        output[gid+1] = sum;
    }
}