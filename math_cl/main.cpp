#include "fastFilter_cl.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <CL/cl.h>
#include <vector>
#define SIZE 5

cl_program CreateProgram(cl_context context, cl_device_id device, const char* fileName)
{
    cl_int errNum;
    cl_program program;

    std::ifstream kernelFile(fileName, std::ios::in);
    if (!kernelFile.is_open())
    {
        std::cerr << "Failed to open file for reading: " << fileName << std::endl;
        return NULL;
    }

    std::ostringstream oss;
    oss << kernelFile.rdbuf();

    std::string srcStdStr = oss.str();
    const char *srcStr = srcStdStr.c_str();
    program = clCreateProgramWithSource(context, 1,
                                        (const char**)&srcStr,
                                        NULL, NULL);
    if (program == NULL)
    {
        std::cerr << "Failed to create CL program from source." << std::endl;
        return NULL;
    }

    errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (errNum != CL_SUCCESS)
    {
        // Determine the reason for the error
        char buildLog[16384];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                              sizeof(buildLog), buildLog, NULL);

        std::cerr << "Error in kernel: " << std::endl;
        std::cerr << buildLog;
        clReleaseProgram(program);
        return NULL;
    }

    return program;
}


int main(){

    int size1 = 100000;
    int size2 = 100000;
    float *a =  new float[size1];
    float *b  = new float[size2];
    float *c = new float[size1+size2];

5    for(int i = 0; i<size1; ++i){
        a[i] = 1;
    }

    for(int i = 0; i<size2; ++i){
        b[i] = 0.001;
    }
    
    // 1. Получаем платформу
    cl_platform_id platform;
    cl_uint num_platforms;
    cl_int err = 0;
    err = clGetPlatformIDs(1, &platform, &num_platforms);
    //cl_int err = 0;
    
    // 2. Получаем устройство (GPU)
    cl_device_id device;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    
    // 3. Создаем контекст
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    
    // 4. Создаем очередь команд
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, 0, NULL);
    
    // 5. Создаем буферы
    cl_mem bufferA = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                    size1* sizeof(float), a, NULL);
    cl_mem bufferB = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                    size1 * sizeof(float), b, NULL);
    cl_mem bufferC = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                    size1 * sizeof(float), NULL, NULL);
    
    // 6. Создаем программу
    //cl_program program = clCreateProgramWithSource(context, 1, &kernel_source, NULL, NULL);
    cl_program program =  CreateProgram(context, device, "add.cl");
    
    // 7. Компилируем программу
    //clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    
    // 8. Создаем ядро
    cl_kernel kernel = clCreateKernel(program, "add", NULL);
    
    // 9. Устанавливаем аргументы ядра
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufferA);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufferB);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufferC);
    // clSetKernelArg(kernel, 3, sizeof(cl_int), &bufferA);s
    // clSetKernelArg(kernel, 4, sizeof(cl_int), &bufferB);
    // clSetKernelArg(kernel, 5, sizeof(cl_int), &bufferC);


    // количество потоков в группе
    size_t max_work_group_size;
    clGetDeviceInfo(device, 
                CL_DEVICE_MAX_WORK_GROUP_SIZE,
                sizeof(max_work_group_size),
                &max_work_group_size,
                NULL);
    printf("Max work-group size: %zu\n", max_work_group_size);
     // количество потоков которые могут быть кратными 
     // этому числу для максимальной эффективности
    size_t prefered_work_group_size;
    clGetDeviceInfo(device, 
                CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                sizeof(prefered_work_group_size),
                &prefered_work_group_size,
                NULL);
    printf("Preffered work-group size: %zu\n", prefered_work_group_size);
    
    //
    size_t max_dims[3];
    clGetDeviceInfo(device, 
                CL_DEVICE_MAX_WORK_ITEM_SIZES,
                sizeof(max_dims),
                &max_dims,
                NULL);
    printf("Max work-group dims: [%zu, %zu, %zu]\n", 
       max_dims[0], max_dims[1], max_dims[2]);
    

    // 10. Запускаем ядро
    /*clEnqueueNDRangeKernel(cl_command_queue command_queue,
                       cl_kernel        kernel,
                       cl_uint          work_dim,
                       const size_t *   global_work_offset,
                       const size_t *   global_work_size,
                       const size_t *   local_work_size,
                       cl_uint          num_events_in_wait_list,
                       const cl_event * event_wait_list,
                       cl_event *       event) CL_API_SUFFIX__VERSION_1_0;*/
    size_t global_size = size1;
    //
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, NULL, 0, NULL, NULL);
    
    // 11. Читаем результат
    clEnqueueReadBuffer(queue, bufferC, CL_TRUE, 0, size1 * sizeof(float), c, 0, NULL, NULL);
    
    // 12. Выводим результат
    std::cout << "Результат сложения:\n";
    for (int i = 0; i < size1; i++) {
        std::cout << a[i]  << " = " << c[i] << std::endl;
    }
    
    // 13. Освобождаем ресурсы
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseMemObject(bufferA);
    clReleaseMemObject(bufferB);
    clReleaseMemObject(bufferC);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    
    return 0;

}

/*
// Функция для получения текста ошибки
const char* getOpenCLErrorString(cl_int error) {
    switch(error) {
        case CL_SUCCESS: return "CL_SUCCESS (0)";
        case CL_DEVICE_NOT_FOUND: return "CL_DEVICE_NOT_FOUND (-1)";
        case CL_DEVICE_NOT_AVAILABLE: return "CL_DEVICE_NOT_AVAILABLE (-2)";
        case CL_COMPILER_NOT_AVAILABLE: return "CL_COMPILER_NOT_AVAILABLE (-3)";
        case CL_INVALID_PLATFORM: return "CL_INVALID_PLATFORM (-32)";
        case CL_INVALID_DEVICE_TYPE: return "CL_INVALID_DEVICE_TYPE (-31)";
        case CL_INVALID_VALUE: return "CL_INVALID_VALUE (-30)";
        default: return "Unknown error";
    }
}

// Получить информацию о платформе
void printPlatformInfo(cl_platform_id platform) {
    char name[256], vendor[256], version[256];
    
    clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(name), name, NULL);
    clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, sizeof(vendor), vendor, NULL);
    clGetPlatformInfo(platform, CL_PLATFORM_VERSION, sizeof(version), version, NULL);
    
    std::cout << "  Platform Name: " << name << std::endl;
    std::cout << "  Platform Vendor: " << vendor << std::endl;
    std::cout << "  Platform Version: " << version << std::endl;
}

int main() {
    std::cout << "=== OpenCL Device Diagnostic ===" << std::endl;
    
    cl_int err;
    
    // 1. Получаем количество платформ
    cl_uint num_platforms = 0;
    err = clGetPlatformIDs(0, NULL, &num_platforms);
    
    if (err != CL_SUCCESS) {
        std::cout << "Error getting platform count: " << getOpenCLErrorString(err) << std::endl;
        return 1;
    }
    
    std::cout << "\nFound " << num_platforms << " OpenCL platform(s)" << std::endl;
    
    if (num_platforms == 0) {
        std::cout << "\nERROR: No OpenCL platforms found!" << std::endl;
        std::cout << "You need to install OpenCL runtime." << std::endl;
        
        #ifdef __linux__
        std::cout << "\nFor Ubuntu/Debian:" << std::endl;
        std::cout << "  sudo apt install ocl-icd-opencl-dev" << std::endl;
        std::cout << "  sudo apt install intel-opencl-icd    # For Intel CPU" << std::endl;
        std::cout << "  sudo apt install mesa-opencl-icd     # For AMD CPU" << std::endl;
        #endif
        
        return 1;
    }
    
    // 2. Получаем платформы
    std::vector<cl_platform_id> platforms(num_platforms);
    err = clGetPlatformIDs(num_platforms, platforms.data(), NULL);
    
    if (err != CL_SUCCESS) {
        std::cout << "Error getting platforms: " << getOpenCLErrorString(err) << std::endl;
        return 1;
    }
    
    // 3. Анализируем каждую платформу
    bool foundAnyDevice = false;
    
    for (cl_uint i = 0; i < num_platforms; i++) {
        std::cout << "\n=== Platform " << i << " ===" << std::endl;
        printPlatformInfo(platforms[i]);
        
        // Проверяем различные типы устройств
        const char* device_types[] = {"GPU", "CPU", "Accelerator", "ALL"};
        cl_device_type types[] = {
            CL_DEVICE_TYPE_GPU, 
            CL_DEVICE_TYPE_CPU, 
            CL_DEVICE_TYPE_ACCELERATOR, 
            CL_DEVICE_TYPE_ALL
        };
        
        for (int j = 0; j < 4; j++) {
            cl_uint num_devices = 0;
            
            // Сначала получаем количество устройств
            err = clGetDeviceIDs(platforms[i], types[j], 0, NULL, &num_devices);
            
            std::cout << "\n  " << device_types[j] << " devices:" << std::endl;
            
            if (err == CL_DEVICE_NOT_FOUND) {
                std::cout << "    No " << device_types[j] << " devices found" << std::endl;
            } else if (err != CL_SUCCESS) {
                std::cout << "    Error checking " << device_types[j] << ": " 
                          << getOpenCLErrorString(err) << std::endl;
            } else {
                std::cout << "    Found " << num_devices << " device(s)" << std::endl;
                foundAnyDevice = true;
                
                // Получаем устройства
                std::vector<cl_device_id> devices(num_devices);
                err = clGetDeviceIDs(platforms[i], types[j], num_devices, devices.data(), NULL);
                
                if (err == CL_SUCCESS) {
                    for (cl_uint k = 0; k < num_devices; k++) {
                        char device_name[256];
                        clGetDeviceInfo(devices[k], CL_DEVICE_NAME, sizeof(device_name), device_name, NULL);
                        std::cout << "    - Device " << k << ": " << device_name << std::endl;
                    }
                }
            }
        }
    }
    
    // 4. Вывод рекомендаций
    std::cout << "\n=== Summary ===" << std::endl;
    
    if (!foundAnyDevice) {
        std::cout << "\nERROR: No OpenCL devices found on any platform!" << std::endl;
        std::cout << "\nPossible solutions:" << std::endl;
        std::cout << "1. Install CPU OpenCL runtime:" << std::endl;
        
        #ifdef __linux__
        std::cout << "   Ubuntu/Debian:" << std::endl;
        std::cout << "     sudo apt install intel-opencl-icd" << std::endl;
        std::cout << "     OR" << std::endl;
        std::cout << "     sudo apt install mesa-opencl-icd" << std::endl;
        std::cout << "\n2. Check if OpenCL is properly installed:" << std::endl;
        std::cout << "   ls /etc/OpenCL/vendors/" << std::endl;
        std::cout << "   Should contain .icd files" << std::endl;
        
        #elif _WIN32
        std::cout << "   Windows:" << std::endl;
        std::cout << "   - Install Intel OpenCL SDK: https://software.intel.com/content/www/us/en/develop/tools/opencl-sdk.html" << std::endl;
        std::cout << "   - Or install GPU drivers (NVIDIA/AMD)" << std::endl;
        #endif
        
        std::cout << "\n3. After installation, reboot may be required" << std::endl;
    } else {
        std::cout << "\nSUCCESS: Found OpenCL devices!" << std::endl;
        std::cout << "You can use CL_DEVICE_TYPE_ALL to access them." << std::endl;
    }
    
    return foundAnyDevice ? 0 : 1;
}
*/
