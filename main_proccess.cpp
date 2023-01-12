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
#include <sys/mman.h>
#include <fcntl.h>
#include <cstring>
#include <sys/file.h>
#include <errno.h>
#endif

#include "Counter.hpp"

#if defined (WIN32)
HANDLE mut = CreateMutex(NULL, FALSE, "FILE_MUTEX");
#endif


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
        //_mutex.lock();
        auto val = count.GetValue();

#if defined(WIN32)
        WaitForSingleObject(mut, INFINITE);
        log << "PID: " << _getpid() << ", counter: " << val << ", Current time: " << std::ctime(&end_time);
        ReleaseMutex(mut);
#else
        _mutex.lock();
        log << "PID: " << getpid() << ", counter: " << val << ", Current time: " << std::ctime(&end_time);
        _mutex.unlock();
#endif
        //_mutex.unlock();
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
                    //_mutex.lock();
                    WaitForSingleObject(mut, INFINITE);
                    log << "First proccess still active" << std::endl;
                    ReleaseMutex(mut);
                    first_write = true;
                    //_mutex.unlock();
                }

            } while (exitCode == STILL_ACTIVE);
        }

        sti = { 0 };
        pi = { 0 };

        if (!CreateProcess("first_process.exe ",   // No module name (use command line)
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
                pid_t result = waitpid(pid, &status, WNOHANG);
                if (result == 0 && !first_write) {
                    _mutex.lock();
                    log << "First proccess still active" << std::endl;
                    _mutex.unlock();
                    first_write = true;
                }
            } while (result == 0);
        }

        switch (pid = fork()) {
        case-1: {
            perror("fork");
            exit(1);
        }
        case 0: {
            execve("./FirstProcess", NULL, NULL);
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
                    //_mutex.lock();
                    WaitForSingleObject(mut, INFINITE);
                    log << "Second Proccess still active" << std::endl;
                    ReleaseMutex(mut);
                    //_mutex.unlock();
                    first_write = true;
                }


            } while (exitCode == STILL_ACTIVE);
        }

        sti = { 0 };
        pi = { 0 };

        if (!CreateProcess("second_process.exe",   // No module name (use command line)
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
                pid_t result = waitpid(pid, &status, WNOHANG);
                if (result == 0 && !first_write) {
                    _mutex.lock();
                    log << "Second proccess still active" << std::endl;
                    _mutex.unlock();
                    first_write = true;
                }
            } while (result == 0);
        }

        switch (pid = fork()) {
        case-1: {
            perror("fork");
            exit(1);
        }
        case 0: {
            execve("./SecondProcess", NULL, NULL);
        }
        }
    }
#endif
}


int main()
{
    Counter counter;
    auto now = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(now);
    auto c_end_time = std::ctime(&end_time);

    std::ofstream log_file(
        "log_file.txt", std::ios_base::out | std::ios_base::app);

#if defined(WIN32)
    HANDLE mut;
    mut = CreateMutex(NULL, FALSE, "FirstStep");
    DWORD result;
    result = WaitForSingleObject(mut, 0);
    // this is the first instance
    if (GetLastError() != ERROR_ALREADY_EXISTS) {

        WaitForSingleObject(mut, INFINITE);
        log_file << "PID: " << _getpid() << " ,Creating proccess time: " << c_end_time;
        ReleaseMutex(mut);


        auto mapping = ::CreateFileMappingW(
            nullptr, // без файла на диске
            nullptr, // не наследуется процессами-потомками
            PAGE_READWRITE,
            0, sizeof(Counter), // размер
            L"Local//test-shared-memory");

        auto data = (Counter*)::MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Counter));
        CopyMemory(data, &counter, sizeof(Counter));

        std::thread timer_thr(timer, std::ref(*data));
        std::thread record_thr(record, std::ref(log_file), std::ref(*data));
        std::thread count_change_thr(count_change, std::ref(*data));
        std::thread create_second_proccess_thr(CreteSecondProccesses, std::ref(log_file));

        CreteFirstProccesses(log_file);

        CloseHandle(mut);
    }
    // this is another instance
    else {
        std::cout << "not first proccess";
        HANDLE mut = OpenMutex(NULL, FALSE, "FILE_MUTEX");
        WaitForSingleObject(mut, INFINITE);
        log_file << "PID: " << _getpid() << " ,Creating copy proccess time: " << c_end_time;
        ReleaseMutex(mut);
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
        std::thread timer_thr(timer, std::ref(*data));
        std::thread count_change_thr(count_change, std::ref(*data));

        timer_thr.join();
        count_change_thr.join();
    }
#else
    int pid_file = open("whatever.pid", O_CREAT | O_RDWR, 0666);
    int rc = flock(pid_file, LOCK_EX | LOCK_NB);
    if (rc) {
        if (EWOULDBLOCK == errno) {
            // another instance is running
            int shm;
            if ((shm = shm_open("my_shared_memory", O_CREAT | O_RDWR, 0777)) == -1) {
                perror("SHM_OPEN");
                return 1;
            }

            Counter* data = (Counter*)mmap(0, sizeof(Counter), PROT_WRITE | PROT_READ, MAP_SHARED, shm, 0);

            std::thread timer_thr(timer, std::ref(*data));
            std::thread count_change_thr(count_change, std::ref(*data));

            timer_thr.join();
            count_change_thr.join();

        }
    }
    else {
        // this is the first instance

        _mutex.lock();
        log_file << "PID: " << getpid() << " ,Creating proccess time: " << c_end_time;
        _mutex.unlock();

        int shm;
        if ((shm = shm_open("my_shared_memory", O_CREAT | O_RDWR, 0777)) == -1) {
            perror("SHM_OPEN");
            return 1;
        }

        if (ftruncate(shm, sizeof(Counter)) == -1) {
            perror("FTRUNCATE");
            return 1;
        }

        Counter* data = (Counter*)mmap(0, sizeof(Counter), PROT_WRITE | PROT_READ, MAP_SHARED, shm, 0);
        memcpy(data, &counter, sizeof(Counter));

        std::thread timer_thr(timer, std::ref(*data));
        std::thread record_thr(record, std::ref(log_file), std::ref(*data));
        std::thread count_change_thr(count_change, std::ref(*data));
        std::thread create_second_proccess_thr(CreteSecondProccesses, std::ref(log_file));

        CreteFirstProccesses(log_file);

    }
#endif
    return 0;
}

