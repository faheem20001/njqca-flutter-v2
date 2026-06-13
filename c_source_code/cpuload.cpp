#include <windows.h>
#include <iostream>
#include <chrono>
#include <thread>

double CalculateCPULoad(FILETIME idleTime, FILETIME kernelTime, FILETIME userTime) {
    static FILETIME prevIdleTime, prevKernelTime, prevUserTime;

    ULONGLONG idleDiff = (reinterpret_cast<ULONGLONG&>(idleTime) - reinterpret_cast<ULONGLONG&>(prevIdleTime));
    ULONGLONG kernelDiff = (reinterpret_cast<ULONGLONG&>(kernelTime) - reinterpret_cast<ULONGLONG&>(prevKernelTime));
    ULONGLONG userDiff = (reinterpret_cast<ULONGLONG&>(userTime) - reinterpret_cast<ULONGLONG&>(prevUserTime));

    prevIdleTime = idleTime;
    prevKernelTime = kernelTime;
    prevUserTime = userTime;

    return (kernelDiff + userDiff - idleDiff) * 100.0 / (kernelDiff + userDiff);
}

int main() {
    FILETIME idleTime, kernelTime, userTime;

    while (true) {
        if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
            double cpuLoad = CalculateCPULoad(idleTime, kernelTime, userTime);
            std::cout << "CPU Load: " << cpuLoad << "%" << std::endl;
        } else {
            std::cerr << "Failed to get CPU times." << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}
