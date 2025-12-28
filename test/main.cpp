#include "fastFilter.h"
#include <fstream>
#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {


    size_t kDecim = 7;
    ifstream file1("sinus.bin", ios::binary | ios::ate);
    size_t size1 = file1.tellg();
    file1.seekg(0, ios::beg);

    float *data1 = new float[size1/4];
    file1.read((char*)data1, size1);
    file1.close();
    
    ifstream file2("filter.bin", ios::binary | ios::ate);
    size_t size2 = file2.tellg();
    file2.seekg(0, ios::beg);

    float *data2 = new float[size2/4];
    file2.read((char*)data2, size2);
    file2.close();



    float x1 = data1[0];
    float x2 = data1[1];
    
    float *result  = new float[size1/4+size2/4];
    float * rest = new float[size2/4];
    std::cout<<size1/4<<std::endl;
    std::cout<<size2/4<<std::endl;
    // myComplexFilter(data1, size1/4, data2, size2/4, result, rest);

    decimate(data1, size1/4, data2, size2/4, result, rest, kDecim);
    std::ofstream fileout("Result.txt", std::ios::trunc);// trunc - перезапишет файл

    if(fileout.is_open()){
        for(int i = 0; i<(size1/4-size2/4)/kDecim; i+=2){
            fileout<<i/2<<": "<<result[i]<<" + "<<result[i+1]<<"i"<<"\n";
        }
        fileout.close();
    }
    // for(int i = 0; i<64; ++i){
    //     std::cout<<result[i];
    // }

    //return app.exec();
}