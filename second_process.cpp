#if defined(WIN32)
#define _CRT_SECURE_NO_DEPRECATE
#endif
#include <iostream>
#include <fstream>
#include <mutex>
#include <thread>
#include "Counter.hpp"

#if defined (WIN32)
#include <Windows.h>
#include<process.h>
#include<conio.h>
#else
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <cstring>
#endif

#if defined(WIN32)
HANDLE mut = OpenMutex(NULL, FALSE, "FILE_MUTEX");
#else
std::mutex _mutex;
#endif

int main(int argc, char* argv[])
{
    auto now = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(now);
    auto c_end_time = std::ctime(&end_time);

    std::ofstream log_file(
        "log_file.txt", std::ios_base::out | std::ios_base::app);

    
#if defined(WIN32)
    WaitForSingleObject(mut, INFINITE);
    log_file << "PID: " << _getpid() << ", Creating second proccess time: " << c_end_time;
    ReleaseMutex(mut);
#else
    _mutex.lock();
    log_file << "PID: " << getpid() << ", Creating second proccess time: " << c_end_time;
    _mutex.unlock();
#endif
    
#if defined(WIN32)
    HANDLE hMapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,      // read/write access
        FALSE,                    // do not inherit the name
        "Local//test-shared-memory");

    if (hMapFile == FALSE) {
        std::cout << "OpenFileMapping error";
    }

    auto data = (Counter*)::MapViewOfFile(hMapFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, sizeof(Counter));
    if (data == NULL) {
        std::cout << "MapViewOfFile error";
    }
#else
    int shm;
    if ((shm = shm_open("my_shared_memory", O_CREAT | O_RDWR, 0777)) == -1) {
        perror("SHM_OPEN");
        return 1;
    }

    Counter* data = (Counter*)mmap(0, sizeof(Counter), PROT_WRITE | PROT_READ, MAP_SHARED, shm, 0);
#endif

    data->Multiply();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    data->Divide();

    now = std::chrono::system_clock::now();
    end_time = std::chrono::system_clock::to_time_t(now);
#if defined(WIN32)
    WaitForSingleObject(mut, INFINITE);
    log_file << "Exit second proccess time: " << std::ctime(&end_time);
    ReleaseMutex(mut);
#else
    _mutex.lock();
    log_file << "Exit second proccess time: " << std::ctime(&end_time);
    _mutex.unlock();
#endif

    return 0;
}
