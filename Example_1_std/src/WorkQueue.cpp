#include "Config.h"
#include "WorkQueue.h"
#include "TestFunctions.h"

static WorkQueue writerQueue;
static WorkQueue networkQueue;
static WorkQueue UIQueue;

////////////////////////////////////////////////////////////

// Цикл обработки задач
void WorkQueue::Run() {
    _stop = false;
    while (1) {
        // При завершении потока вернется пустая функция
        TaskType t = PopTask();
        if (!t){
            return;
        }
        print("Queue task begin");
        t();
        print("Queue task end");
    }
}

void WorkQueue::Shutdown() {
    // Оповещаем об завершении
    _stop = true;
    _tasksCond.notify_all();
}

WorkQueue::TaskType WorkQueue::PopTask() {
    TaskType ret;
    
    // Ждем новых задач, если у нас их нету
    std::unique_lock<std::mutex> lock(_tasksMutex);
    _tasksCond.wait(lock, [this]{
        // Блокируется mutex
        // затем просыпаемся, если список задач не пуст, либо завершение потока
        return !_tasks.empty() || _stop;
        // Снова блокируем mutex
    });
    
    // Если нету задач - выходим
    if (_tasks.empty()) {
        return ret;
    }
    
    // Получаем новую задачу
    ret = std::move(_tasks.front());
    _tasks.pop_front();
    
    return ret;
}

void WorkQueue::PushTask(const TaskType& task) {
    // Закидываем задачу
    std::unique_lock<std::mutex> lock(_tasksMutex);
    _tasks.push_back(task);
    lock.unlock();
    
    // Оповещаем об обновлении
    _tasksCond.notify_one();
}

////////////////////////////////////////////////////////////

WorkQueue& getWriterQueue(){
    return writerQueue;
}

WorkQueue& getNetworkQueue(){
    return networkQueue;
}

WorkQueue& getUIQueue(){
    return UIQueue;
}

void shutdownAllThreads() {
    writerQueue.Shutdown();
    networkQueue.Shutdown();
    UIQueue.Shutdown();
}
