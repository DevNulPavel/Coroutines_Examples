#include "config.h"
#include "coro_task.h"
#include "work_queue.h"

#ifndef CORO_OPTIMIZED
schedule_for_execution PromiseType::yield_value(WorkQueue& wq) {
    Log(__func__);
    return schedule_for_execution{wq};
}
#else
schedule_for_execution PromiseType::yield_value(WorkQueue& wq) {
    Log(__func__);
    const bool do_resume = (current_queue_ == &wq);
    current_queue_ = &wq;
    return schedule_for_execution{do_resume, wq};
}
#endif

schedule_for_execution PromiseType::await_transform(WorkQueue& wq) {
    return yield_value(wq);
}
