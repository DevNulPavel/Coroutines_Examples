#include <iostream>
#include <thread>
#include "TestFunctions.h"


thread_local const char* threadId = nullptr;

void print(const char* func) {
    if (threadId != nullptr) {
        std::clog << "Thread: " << threadId << " -> " << func << '\n';
    }else{
        std::clog << "Thread: " << "unknown" << " -> " << func << '\n';
    }
}

void currentThreadTask() {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    //print(__func__);
    print("Code in current thread");
}

void writerThreadTask1() {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    //print(__func__);
    print("Code in writer thread 1");
}

void writerThreadTask2() {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    //print(__func__);
    print("Code in writer thread 2");
}

bool networkThreadTask() {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    //print(__func__);
    print("Code in network thread");
    return true;
}

void uiThreadTask() {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    //print(__func__);
    print("Code in UI thread");
}

bool needNetwork() {
    return true;
}

