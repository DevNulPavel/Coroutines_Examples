#ifndef USER_CODE_2_H
#define USER_CODE_2_H

#include "coro_task.h"
#include "functions.h"

CoroTask codeWithCoro() {
    // Выполняем в текущем потоке задачу
    currentThreadTask();
    
    // Перейти в writerQueue, co_await/co_yield в данной реализации совпадают
    co_await getWriterQueue();
    
    // Выполняем что-то в потоке записи
    writerThreadTask1();
    
    // В потоке записи проверяем нужно ли нам работать с сетью
    if (needNetwork()) {
        
        // Перейти в networkQueue
        co_await getNetworkQueue();
        
        // Выполняем что-то в сетевом потоке
        auto networkSuccess = networkThreadTask();
        
        // Если что-то успешно выполнилось в сетевом потоке
        if (networkSuccess) {
            
            // Перейти в UIQueue
            co_await getUIQueue();
            
            // Выполняем что-то в главном потоке
            uiThreadTask();
        }
    }
    
    // Перейти в writerQueue
    co_await getWriterQueue();
    
    // Выполняем что-то в потоке запись
    writerThreadTask2();
    
    shutdownAllThreads();
}

#endif
