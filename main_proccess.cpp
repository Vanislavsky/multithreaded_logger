#define _CRT_SECURE_NO_DEPRECATE
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <mutex>
#include <string>

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
#endif

#include<conio.h>


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

void timer(Counter& count) {
    std::mutex m;
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        count.Increase();
    }
}

void count_change(Counter& count) {
    size_t value;
    while (true) {
        std::cin >> value;
        count.SetValue(value);
    }
}

void record(std::ofstream& log, Counter& count) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        auto now = std::chrono::system_clock::now();
        std::time_t end_time = std::chrono::system_clock::to_time_t(now);
        _mutex.lock();
        auto val = count.GetValue();
#if defined(WIN32)
        log << "PID: " << _getpid() << ", Current time: " << std::ctime(&end_time) << ", counter: " << val << std::endl;
#else
        log << "PID: " << getpid() << ", Current time: " << std::ctime(&end_time) << ", counter: " << val << std::endl;
#endif
        _mutex.unlock();
    }
}

void CreteFirstProccesses(std::ofstream& log) {
#if defined (WIN32)
    bool first_occurrence = true;
    PROCESS_INFORMATION pi;
    STARTUPINFO sti;
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        if (!first_occurrence) {
            bool first_write = false;
            DWORD exitCode = 0;
            do {
                GetExitCodeProcess(pi.hProcess, &exitCode);
                if (exitCode == STILL_ACTIVE && !first_write) {
                    _mutex.lock();
                    log << "First proccess still active" << std::endl;
                    _mutex.unlock();
                }

            } while (exitCode != STILL_ACTIVE);
        }

        sti = { 0 };
        pi = { 0 };

        std::wstring CommandLine(L"ConsoleApplication2.exe ");
        LPWSTR lpwCmdLine = &CommandLine[0];

        if (!CreateProcess("ConsoleApplication2.exe",   // No module name (use command line)
            NULL,        // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            TRUE,          // Set handle inheritance to FALSE
            NULL,              // No creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory 
            &sti,            // Pointer to STARTUPINFO structure
            &pi)           // Pointer to PROCESS_INFORMATION structure
            )
        {
            printf("CreateProcess failed (%d).\n", GetLastError());
            break;
        }

        first_occurrence = false;

    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#else
    pid_t pid;
    int status;
    bool first_occurrence = true;
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        if (!first_occurrence) {
            bool first_write = false;
            pid_t result;
            do {
                pid_t result = waitpid(, &status, WNOHANG);
                if (result == 0 && !first_write) {
                    _mutex.lock();
                    log << "First proccess still active" << std::endl;
                    _mutex.unlock();
                    first_write = true;
                }
            } while (result != 0)
        }

        switch (pid = fork()) {
        case-1: {
            perror("fork");
            exit(1);
        }
        case 0: {
            execve("/FirstProcess", NULL, NULL);
        }
        }
    }
#endif

}

void CreteSecondProccesses(std::ofstream& log) {
#if defined(WIN32)
    bool first_occurrence = true;
    PROCESS_INFORMATION pi;
    STARTUPINFO sti;
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        if (!first_occurrence) {
            bool first_write = false;
            DWORD exitCode = 0;
            do {
                GetExitCodeProcess(pi.hProcess, &exitCode);
                if (exitCode == STILL_ACTIVE && !first_write) {
                    _mutex.lock();
                    log << "Second Proccess still active" << std::endl;
                    _mutex.unlock();
                    first_write = true;
                }
                    

            } while (exitCode != STILL_ACTIVE);
        }

        sti = { 0 };
        pi = { 0 };

        std::wstring CommandLine(L"ConsoleApplication3.exe ");
        LPWSTR lpwCmdLine = &CommandLine[0];
        if (!CreateProcess("ConsoleApplication3.exe ",   // No module name (use command line)
            NULL,        // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            TRUE,          // Set handle inheritance to FALSE
            NULL,              // No creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory 
            &sti,            // Pointer to STARTUPINFO structure
            &pi)           // Pointer to PROCESS_INFORMATION structure
            )
        {
            printf("CreateProcess failed (%d).\n", GetLastError());
            break;
        }

        first_occurrence = false;
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#else
    pid_t pid;
    int status;
    bool first_occurrence = true;
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        if (!first_occurrence) {
            bool first_write = false;
            pid_t result;
            do {
                pid_t result = waitpid(, &status, WNOHANG);
                if (result == 0 && !first_write) {
                    _mutex.lock();
                    log << "Second proccess still active" << std::endl;
                    _mutex.unlock();
                    first_write = true;
                }
            } while (result != 0)
        }

        switch (pid = fork()) {
        case-1: {
            perror("fork");
            exit(1);
        }
        case 0: {
            execve("/SecondProcess", NULL, NULL);
        }
        }
    }
#endif
}


int main()
{
    auto now = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(now);
    auto c_end_time = std::ctime(&end_time);

    std::mutex _mutex;
    Counter counter;
    counter.k = 10;

    std::ofstream log_file(
        "log_file.txt", std::ios_base::out | std::ios_base::app);

    _mutex.lock();
#if defined(WIN32)
    log_file << "PID: " << _getpid() << " ,Creating proccess time: " << c_end_time;
#else
    log_file << "PID: " << getpid() << " ,Creating proccess time: " << c_end_time;
#endif
    _mutex.unlock();


#if defined(WIN32)
    auto mapping = ::CreateFileMappingW(
        nullptr, // без файла на диске
        nullptr, // не наследуется процессами-потомками
        PAGE_READWRITE,
        0, sizeof(Counter), // размер
        L"Local//test-shared-memory");

    auto data = (Counter*)::MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Counter));
    CopyMemory(data, &counter, sizeof(Counter));
#else
    int shm;
    if ((shm = shm_open("my_shared_memory", O_CREAT | O_RDWR, 0777)) == -1) {
        perror("SHM_OPEN");
        return 1;
    }

    if (ftruncate(shm, sizeof(Counter)) == -1) {
        perror("FTRUNCATE");
        return 1;
    }

    Counter* addr = (Counter*)mmap(0, sizeof(Counter), PROT_WRITE | PROT_READ, MAP_SHARED, shm, 0);
    memcpy(addr, &counter, sizeof(Counter));
#endif

    std::thread create_first_proccess_thr(CreteFirstProccesses, std::ref(log_file));
    std::thread create_second_proccess_thr(CreteSecondProccesses, std::ref(log_file));
    std::thread timer_thr(timer, std::ref(*data));
    std::thread record_thr(record, std::ref(log_file), std::ref(*data));
    std::thread count_change_thr(count_change, std::ref(*data));



    create_first_proccess_thr.join();
    create_second_proccess_thr.join();
    timer_thr.join();
    record_thr.join();
    count_change_thr.join();
    return 0;
}

