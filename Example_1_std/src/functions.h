#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <iostream>

thread_local const char* threadId = nullptr;

void print(const char* func) {
    std::clog << "Thread: " << threadId << " -> " << func << '\n';
}

void currentThreadTask() {
    //print(__func__);
    print("Code in current thread");
}

void writerThreadTask1() {
    //print(__func__);
    print("Code in writer thread 1");
}

void writerThreadTask2() {
    //print(__func__);
    print("Code in writer thread 2");
}

bool networkThreadTask() {
    print(__func__);
    return true;
}

void uiThreadTask() {
    print(__func__);
}

bool needNetwork() {
    return true;
}

#endif
