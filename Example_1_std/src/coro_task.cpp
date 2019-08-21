#include "config.h"
#include "coro_task.h"
#include "work_queue.h"


AwaitObject::AwaitObject(bool doResumeParam, WorkQueue& wqParam):
    doResume(doResumeParam),
    wq(wqParam){
}

// Вызывается перед засыпанием корутины, чтобы определить - нужно ли нам вообще засыпать, или уже все готово
bool AwaitObject::await_ready() const noexcept { // constexpr
    print("Await ready called");
    return doResume;
}

// Вызывается после засыпания корутины
void AwaitObject::await_suspend(const std::experimental::coroutine_handle<>& thisCoro) const {
    print("Await suspend called");
    // Здесь мы закидываем текущую корутину в другую очередь задач (после засыпания корутины),
    // у корутины есть operator(), поэтому вполне подходит в качестве задачи
    wq.PushTask(thisCoro);
    
    // Так можно восстановить работу корутины
    //auto coroCopy = thisCoro;
    //coroCopy.operator()();
    // или
    //coroCopy.resume();
}

// Вызывается при просыпании корутины в новом потоке
void AwaitObject::await_resume() const noexcept { // constexpr
    print("Await resume called");
}

////////////////////////////////////////////////////////////////////////

// Данный метод вызывается при выходе из корутины
void PromiseType::return_void() const {
    Log(__func__);
}

// С помощью данного метода происходит определение, нужно ли нам засыпать в входе в корутину
std::experimental::suspend_never PromiseType::initial_suspend() const {
    Log(__func__);
    return std::experimental::suspend_never();
}

// С помощью данного метода мы определяем, нужно ли усыплять корутину при выходе из корутины
std::experimental::suspend_never PromiseType::final_suspend() const {
    Log(__func__);
    return std::experimental::suspend_never();
}

void PromiseType::unhandled_exception() const {
    Log(__func__);
    std::terminate();
}

// Данный метод вызывается при входе в метод, в котором есть корутина,
// с помощью данного метода фактически создается объект корутины
CoroTask PromiseType::get_return_object() const {
    Log(__func__);
    return CoroTask();
}

// Вызывается при вызове co_yield
AwaitObject PromiseType::yield_value(WorkQueue& wq) {
    Log(__func__);
    
    // Проверяем, нужно ли нам вообще засыпать, может быть мы уже в правильной очереди
    const bool do_resume = (current_queue_ == &wq);
    
    // Сохряняем целевую очередь
    current_queue_ = &wq;
    
    // Создаем await объект
    return AwaitObject(do_resume, wq);
}

// Вызывается при вызове co_await
AwaitObject PromiseType::await_transform(WorkQueue& wq) {
    return yield_value(wq);
}
