#ifndef USER_CODE_1_H
#define USER_CODE_1_H

#include "functions.h"

void codeWithCallbacks() {
    // Выполняем задачу в текущем потоке
    currentThreadTask();
    
    // Закидываем задачу в очередь потока записи
    writerQueue.PushTask([=]() {
        // Выполняем запись
        writerThreadTask1();
        
        // Если нужна работа с сетью - закидываем задачу...
        if (needNetwork()) {
            // ... в очередь работы с сетью
            networkQueue.PushTask([=](){
                // Выполняем работу с сетью
                auto v = networkThreadTask();
                // Если работа произошла успешно...
                if (v) {
                    // ...закидываем задачу в очередь работы с UI
                    UIQueue.PushTask([=](){
                        // Выполняем что-то в UI потоке
                        uiThreadTask();
                        
                        // Коллбек завершения
                        const auto finallyInWriterThread = [=]() {
                            writerThreadTask2();
                            ShutdownAll();
                        };
                        
                        // Закидываем задачу после обновления в очередь записи запись
                        writerQueue.PushTask(finallyInWriterThread);
                    });
                } else {
                    // Коллбек завершения
                    const auto finallyInWriterThread = [=]() {
                        writerThreadTask2();
                        ShutdownAll();
                    };
                    // Закидываем в поток записи
                    writerQueue.PushTask(finallyInWriterThread);
                }
            });
        } else {
            // Коллбек завершения
            const auto finallyInWriterThread = [=]() {
                writerThreadTask2();
                ShutdownAll();
            };
            finallyInWriterThread();
        }
    });
}

#endif
