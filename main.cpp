#include <iostream>
#include <bitset>
#include <intrin.h>
#include <Windows.h>
#include <functional>
#include <cstring>
#include <cstdint>
#include <stdio.h>

/*
 * Данные методы реализованы на основе обнаружение гипервизора VMWare
 * Операционная система windows
 */

struct cpuid_buffer_t
{
    int EAX;
    int EBX;
    int ECX;
    int EDX;
};

int bios_method(){
    std::cout<<"Serial number verification method"<<std::endl;

    FILE* fp = popen("wmic bios get serialnumber", "r");
    if (fp == NULL) {
        std::cout << "Failed to execute command\n";
        return -1;
    }

    char buffer[128];
    size_t n = fread(buffer, 1, sizeof(buffer), fp);

    pclose(fp);

    if(strstr(buffer, "VMware-")||strstr(buffer, "VMW"))
    {
        std::cout<<"The program is on a virtual machine"<<std::endl;
        std::cout<<"---------------------\n"<<std::endl;
        return 1;
    }
    else{
        std::cout<<"The program is on a physical machine"<<std::endl;
        std::cout<<"---------------------\n"<<std::endl;
        return 0;
    }

}

int cpuid_method(){
    std::cout<<"Method to check 31 bits of CPUID instruction"<<std::endl;

    cpuid_buffer_t cpuidBuffer;
    char hyper_vendor_id[13];

    __cpuid(reinterpret_cast<int*>(&cpuidBuffer),0x1);

    if (cpuidBuffer.ECX & 0x80000000) {
        std::cout<<"Hypervisor detected."<<std::endl;

        __cpuid(reinterpret_cast<int*>(&cpuidBuffer),0x40000000);

        memcpy(hyper_vendor_id + 0, &cpuidBuffer.EBX, 4);
        memcpy(hyper_vendor_id + 4, &cpuidBuffer.ECX, 4);
        memcpy(hyper_vendor_id + 8, &cpuidBuffer.EDX, 4);
        hyper_vendor_id[12] = '\0';

        std::cout<<"Hypervisor: "<< hyper_vendor_id<<std::endl;
        std::cout<<"---------------------\n"<<std::endl;
        return 1;
    }
    else{
        std::cout<<"Hypervisor not found."<<std::endl;
        std::cout<<"---------------------\n"<<std::endl;
        return 0;
    }
}

int time_method(){

    std::cout<<"Instruction execution time check method"<<std::endl;

    long long valid_time   = 5000;
    cpuid_buffer_t cpuidBuffer;
    LARGE_INTEGER frequency = {};
    LARGE_INTEGER start     = {};
    LARGE_INTEGER end       = {};
    long long cpuid_time   = 0;
    QueryPerformanceFrequency(&frequency);

    int num_runs = 100000;

    for (int i = 0; i < num_runs; i++) {
        QueryPerformanceCounter(&start);

        __cpuid(reinterpret_cast<int*>(&cpuidBuffer),0);

        QueryPerformanceCounter(&end);

        auto delta = end.QuadPart - start.QuadPart;
        delta*=1000000000;
        delta /= frequency.QuadPart;
        cpuid_time+=delta;
    }
    cpuid_time/=num_runs;

    if (cpuid_time>valid_time){
        std::cout<<"The program is on a virtual machine"<<std::endl;
        std::cout<<"---------------------\n"<<std::endl;
        return 1;
    }
    else{
        std::cout<<"The program is on a physical machine"<<std::endl;
        std::cout<<"---------------------\n"<<std::endl;
        return 0;
    }
}
int check_invalid_leaf_method(){

    std::cout<<"Check_invalid_leaf_method"<<std::endl;

    unsigned int invalid_leaf = 0x13371337;
    unsigned int valid_leaf = 0x40000000;

    cpuid_buffer_t invalidLeafResponse;
    cpuid_buffer_t validLeafResponse;

    __cpuid( reinterpret_cast<int*>(&invalidLeafResponse), invalid_leaf );
    __cpuid( reinterpret_cast<int*>(&validLeafResponse),  valid_leaf );

    if( ( invalidLeafResponse.EAX != validLeafResponse.EAX ) ||
        ( invalidLeafResponse.EBX != validLeafResponse.EDX ) ||
        ( invalidLeafResponse.ECX != validLeafResponse.ECX ) ||
        ( invalidLeafResponse.EDX != validLeafResponse.EDX ) ){
        std::cout<<"Hypervisor detected."<<std::endl;
        std::cout<<"---------------------\n"<<std::endl;
        return 1;
    }
    else{
        std::cout<<"Hypervisor not found."<<std::endl;
        std::cout<<"---------------------\n"<<std::endl;
        return 0;
    }
}

int main() {
    bios_method();
    cpuid_method();
    time_method();
    check_invalid_leaf_method();

    return 0;
}