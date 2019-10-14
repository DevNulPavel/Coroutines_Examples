//////////////////////// main.cpp

#include <thread>
#include "Config.h"
#include "WorkQueue.h"
#include "CorutineTask.h"
#include "TestFunctions.h"
#include "TestCodeCallbacks.h"
#include "TestCodeCoroutines.h"

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


