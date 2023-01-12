#define _CRT_SECURE_NO_DEPRECATE
#include <iostream>
#include <fstream>
#include <mutex>

#if defined (WIN32)
#include<process.h>
#include <Windows.h>
#include<conio.h>
#endif


class Counter {
public:
    void Increase() {
        countert_mutex.lock();
        k++;
        countert_mutex.unlock();
    }

    void IncreaseValue(size_t value) {
        countert_mutex.lock();
        k += value;
        countert_mutex.unlock();
    }

    void SetValue(size_t _k) {
        countert_mutex.lock();
        k = _k;
        countert_mutex.unlock();
    }

    void Multiply() {
        countert_mutex.lock();
        k *= 2;
        countert_mutex.unlock();
    }

    void Divide() {
        countert_mutex.lock();
        k /= 2;
        countert_mutex.unlock();
    }

    size_t GetValue() {
        size_t result;
        countert_mutex.lock();
        result = k;
        countert_mutex.unlock();

        return result;
    }
public:
    size_t k{ 0 };
    std::mutex countert_mutex;
};

std::mutex _mutex;

int main(int argc, char* argv[])
{
    auto now = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(now);
    auto c_end_time = std::ctime(&end_time);
    
    std::ofstream log_file(
        "log_file.txt", std::ios_base::out | std::ios_base::app);
    
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

    data->IncreaseValue(10);
    _mutex.lock();
#if defined(WIN32)
    log_file << "PID: " << _getpid() << " ,Creating proccess time: " << c_end_time << ", counter: " << data->GetValue() << std::endl;
#else
    log_file << "PID: " << getpid() << " ,Creating proccess time: " << c_end_time << ", counter: " << data->GetValue() << std::endl;
#endif
    _mutex.unlock();
}
