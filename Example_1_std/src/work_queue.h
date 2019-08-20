#ifndef WORK_QUEUE_H
#define WORK_QUEUE_H

#include <mutex>
#include <condition_variable>
#include <deque>

class WorkQueue {
public:
    using TaskType = std::function<void()>;
    
    void PushTask(TaskType task);
    void Run();
    void Shutdown();
    
private:
    std::mutex _tasks_mutex;
    std::condition_variable _tasks_cond;
    std::deque<TaskType> _tasks;
    std::atomic<bool> _stop = false;
    
    TaskType PopTask();
};

extern WorkQueue writerQueue;
extern WorkQueue networkQueue;
extern WorkQueue UIQueue;

void ShutdownAll();

#endif

