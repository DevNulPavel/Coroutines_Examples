//////////////////////// main.cpp

#include <thread>
#include "config.h"
#include "work_queue.h"
#include "coro_task.h"
#include "functions.h"
#include "user_code_1.h"
#include "user_code_2.h"

template <class Func>
void test(Func func) {
    {
        threadId = "MAIN";
        std::thread wr([](){
            threadId = "WRITER";
            getWriterQueue().Run();
        });
        std::thread net([](){
            threadId = "NETWORK";
            getNetworkQueue().Run();
        });
        std::thread ui([](){
            threadId = "UI";
            getUIQueue().Run();
        });
        
        print("START");
        func();
        print("After function call");
        
        ui.join();
        net.join();
        wr.join();
    }
    print("AFTER SCOPE");
    std::clog << "\n\n";
}

int main() {
    std::clog << "Initial code\n";
    test(codeWithCallbacks);
    
    std::clog << "Coroutine code\n";
    test(codeWithCoro);
}


