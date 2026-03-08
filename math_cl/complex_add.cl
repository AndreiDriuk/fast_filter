__kernel void complex_add(
__global const float* input_1,   // Входной массив
__global const float* input_2,   // Входной массив
__global float* output,        // Выходной массив
const int input_size          // Размер входного массива
) {
    // Получаем глобальный индекс потока
    int gid = get_global_id(0);
    
    // Проверяем границы массива
    if (gid >= input_size/2) {
        return;
    }

    // Записываем результат
    output[2*gid] = input_1[2*gid]+input_2[2*gid];
    output[2*gid+1] = input_1[2*gid+1]+input_2[2*gid+1];
}