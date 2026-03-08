__kernel void complex_convolution(
    __global const float2* input_signal,  // Входной комплексный сигнал
    __global const float2* conv_kernel,   // Комплексное ядро свёртки
    __global float2* output_signal,       // Выходной комплексный сигнал
    const int signal_length,              // Длина входного сигнала
    const int kernel_length,              // Длина ядра свёртки
    const int pad_left                    // Количество отсчётов слева для дополнения нулями (padding)
)
{
    int gid = get_global_id(0); // Глобальный индекс выходного элемента
    if (gid >= signal_length) return; // Проверка границ

    float2 sum = (float2)(0.0f, 0.0f); // Аккумулятор для результата (real, imag)
    int kernel_radius = kernel_length / 2;

    // Проходим по всем элементам ядра
    for (int k = 0; k < kernel_length; ++k) {
        // Рассчитываем индекс во входном сигнале с учётом смещения ядра и padding
        int input_index = gid - kernel_radius + k + pad_left;

        float2 input_val, kernel_val;   
        // Обработка граничных случаев (boundary conditions): дополнение нулями (zero-padding)
        if (input_index < 0 || input_index >= signal_length) {
            input_val = (float2)(0.0f, 0.0f);
        } else {
            input_val = input_signal[input_index];
        }
        kernel_val = conv_kernel[kernel_length - 1 - k]; // Ядро может потребовать переворота (flip)

        // Комплексное умножение: (a + ib) * (c + id) = (ac - bd) + i(ad + bc)
        float2 prod;
        prod.x = input_val.x * kernel_val.x - input_val.y * kernel_val.y; // Действительная часть
        prod.y = input_val.x * kernel_val.y + input_val.y * kernel_val.x; // Мнимая часть

        // Суммируем результат
        sum.x += prod.x;
        sum.y += prod.y;
    }
    output_signal[gid] = sum;
}