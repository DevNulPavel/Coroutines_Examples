#include "config.h"
#include "work_queue.h"

WorkQueue writerQueue;
WorkQueue networkQueue;
WorkQueue UIQueue;


WorkQueue::TaskType WorkQueue::PopTask() {
    TaskType ret{};
    
    std::unique_lock<std::mutex> lock(_tasks_mutex);
    _tasks_cond.wait(lock, [this]{ return !_tasks.empty() || _stop; });
    
    if (_tasks.empty()) {
        return ret;
    }
    
    ret = std::move(_tasks.front());
    _tasks.pop_front();
    
    return ret;
}

void WorkQueue::PushTask(TaskType task) {
    std::unique_lock<std::mutex> lock(_tasks_mutex);
    _tasks.push_back(std::move(task));
    lock.unlock();
    
    _tasks_cond.notify_one();
}

void WorkQueue::Run() {
    _stop = false;
    while (1) {
        TaskType t = PopTask();
        if (!t)
            return;
        t();
    }
}

void WorkQueue::Shutdown() {
    _stop = true;
    _tasks_cond.notify_all();
}

void ShutdownAll() {
    writerQueue.Shutdown();
    networkQueue.Shutdown();
    UIQueue.Shutdown();
}
