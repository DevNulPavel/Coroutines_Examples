#ifndef CORO_TASK_H
#define CORO_TASK_H

#include <experimental/coroutine>
#include <iostream>
#include <sstream>
#include "work_queue.h"

class WorkQueue;    // forward declaration
class PromiseType;  // forward declaration

struct CoroTask {
    using promise_type = PromiseType;
};

#ifndef CORO_OPTIMIZED
    struct schedule_for_execution {
        WorkQueue& wq;
        
        constexpr bool await_ready() const noexcept { return false; }
        void await_suspend(std::experimental::coroutine_handle<> this_coro) const {
            wq.PushTask(this_coro);
        }
        constexpr void await_resume() const noexcept {}
    };
#else
    struct schedule_for_execution {
        const bool do_resume;
        WorkQueue& wq;
        
        constexpr bool await_ready() const noexcept { return do_resume; }
        void await_suspend(std::experimental::coroutine_handle<> this_coro) const {
            wq.PushTask(this_coro);
        }
        constexpr void await_resume() const noexcept {}
    };
#endif

class PromiseType {
public:
    void return_void() const {         Log(__func__);                                             }
    auto initial_suspend() const {     Log(__func__); return std::experimental::suspend_never{};  }
    auto final_suspend() const {       Log(__func__); return std::experimental::suspend_never{};  }
    void unhandled_exception() const { Log(__func__); std::terminate();                           }
    auto get_return_object() const {   Log(__func__); return CoroTask{};                         }
    schedule_for_execution yield_value(WorkQueue& wq);
    
    schedule_for_execution await_transform(WorkQueue& wq);
    
private:
    void Log(const char* msg) const {
        #ifdef DEBUG_PRINT
            std::ostringstream oss; // std::osyncstream is not implemented yet :(
            oss << this << " PromiseType:  " << msg << '\n';
            std::clog << oss.str();
        #endif
        (void)msg;
    }
    
    #ifdef CORO_OPTIMIZED
        WorkQueue* current_queue_ = nullptr;
    #endif
};

#endif

