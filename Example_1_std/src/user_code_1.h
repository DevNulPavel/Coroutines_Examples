#ifndef USER_CODE_1_H
#define USER_CODE_1_H

#include "functions.h"

void codeWithCallbacks() {
    // Выполняем задачу в текущем потоке
    currentThreadTask();
    
    // Закидываем задачу в очередь потока записи
    getWriterQueue().PushTask([=]() {
        // Выполняем запись
        writerThreadTask1();
        
        // Если нужна работа с сетью - закидываем задачу...
        if (needNetwork()) {
            // ... в очередь работы с сетью
            getNetworkQueue().PushTask([=](){
                // Выполняем работу с сетью
                auto v = networkThreadTask();
                // Если работа произошла успешно...
                if (v) {
                    // ...закидываем задачу в очередь работы с UI
                    getUIQueue().PushTask([=](){
                        // Выполняем что-то в UI потоке
                        uiThreadTask();
                        
                        // Коллбек завершения
                        const auto finallyInWriterThread = [=]() {
                            writerThreadTask2();
                            shutdownAllThreads();
                        };
                        
                        // Закидываем задачу после обновления в очередь записи запись
                        getWriterQueue().PushTask(finallyInWriterThread);
                    });
                } else {
                    // Коллбек завершения
                    const auto finallyInWriterThread = [=]() {
                        writerThreadTask2();
                        shutdownAllThreads();
                    };
                    // Закидываем в поток записи
                    getWriterQueue().PushTask(finallyInWriterThread);
                }
            });
        } else {
            // Коллбек завершения
            const auto finallyInWriterThread = [=]() {
                writerThreadTask2();
                shutdownAllThreads();
            };
            finallyInWriterThread();
        }
    });
}

#endif
