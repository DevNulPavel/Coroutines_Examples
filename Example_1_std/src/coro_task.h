#ifndef CORO_TASK_H
#define CORO_TASK_H

#include <experimental/coroutine>
#include <iostream>
#include <sstream>
#include "work_queue.h"
#include "functions.h"

class WorkQueue;    // forward declaration
class PromiseType;  // forward declaration

// Определяем обработчики работы с корутинами
struct CoroTask {
    using promise_type = PromiseType;
};

struct AwaitObject {
    // Данные, с которыми мы работаем
    const bool doResume = false; // Нужно ли нам засыпать? Может быть мы уже в правильной очереди
    WorkQueue& wq; // Очередь, в которую мы планируем перейти
    
    explicit AwaitObject(bool doResumeParam, WorkQueue& wqParam);
    
    // Вызывается перед засыпанием корутины, чтобы определить - нужно ли нам вообще засыпать, или уже все готово
    bool await_ready() const noexcept; // constexpr
    
    // Вызывается после засыпания корутины
    void await_suspend(const std::experimental::coroutine_handle<>& thisCoro) const;
    
    // Вызывается при просыпании корутины в новом потоке
    void await_resume() const noexcept; // constexpr
};

class PromiseType {
public:
    // Данный метод вызывается при выходе из корутины
    void return_void() const;
    
    // С помощью данного метода происходит определение, нужно ли нам засыпать в входе в корутину
    std::experimental::suspend_never initial_suspend() const;
    
    // С помощью данного метода мы определяем, нужно ли усыплять корутину при выходе из корутины
    std::experimental::suspend_never final_suspend() const;
    
    void unhandled_exception() const;
    
    // Данный метод вызывается при входе в метод, в котором есть корутина,
    // с помощью данного метода фактически создается объект корутины
    CoroTask get_return_object() const;
    
    // Вызывается при вызове co_yield
    AwaitObject yield_value(WorkQueue& wq);
    
    // Вызывается при вызове co_await
    AwaitObject await_transform(WorkQueue& wq);
    
private:
    void Log(const char* msg) const {
        #ifdef DEBUG_PRINT
            std::ostringstream oss; // std::osyncstream is not implemented yet :(
            oss << "Thread: " << threadId << " -> " << this << " PromiseType: " << msg << '\n';
            std::clog << oss.str();
        #endif
        (void)msg;
    }
    
    WorkQueue* current_queue_ = nullptr;
};

#endif

