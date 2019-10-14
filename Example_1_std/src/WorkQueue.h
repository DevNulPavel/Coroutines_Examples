#ifndef WORK_QUEUE_H
#define WORK_QUEUE_H

#include <mutex>
#include <condition_variable>
#include <deque>

class WorkQueue {
public:
    using TaskType = std::function<void()>;
    
    void Run();
    void Shutdown();
    void PushTask(const TaskType& task);
    
private:
    std::mutex _tasksMutex;
    std::condition_variable _tasksCond;
    std::deque<TaskType> _tasks;
    std::atomic<bool> _stop = false;

private:
    TaskType PopTask();
};

WorkQueue& getWriterQueue();
WorkQueue& getNetworkQueue();
WorkQueue& getUIQueue();
void shutdownAllThreads();

#endif

