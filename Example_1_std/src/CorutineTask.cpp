#include "CorutineTask.h"
#include <thread>
#include <future>
#include "Config.h"
#include "WorkQueue.h"


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
    
    /*std::thread testThread([](std::experimental::coroutine_handle<> coro){
        threadId = "TEST";
        print("Test thread enter");
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        coro(); // Продолжаем корутину до очередного yield/await
        print("Test thread exit");
    }, thisCoro);
    testThread.detach();*/
    
    // Неправильный вариант, так как мы дожидаемся возвращаемого значения в result
    /*auto result = std::async(std::launch::async, [](std::function<void()> function){
        threadId = "TEST";
        print("Async enter");
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        function(); // Продолжаем корутину до очередного yield/await
        print("Async exit");
    }, thisCoro);*/
    
    // Так можно восстановить работу корутины
    //auto coroCopy = thisCoro;
    //coroCopy(); // coroCopy.resume();
    
    // Чтобы просто восстановить работу корутины до очередного await/yield - нужно использовать корутину как функцию
    /*std::function<void()> function = thisCoro;
    print("Async enter");
    function();
    print("Async exit");*/
}

// Вызывается при просыпании корутины в новом потоке
void AwaitObject::await_resume() const noexcept { // constexpr
    print("Await resume called");
}

////////////////////////////////////////////////////////////////////////

// Данный метод вызывается при выходе из корутины c помощью co_return
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

// Когда из корутины вылетает исключение, то мы просто завершаем все приложение
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

// Вызывается при вызове co_yield, метод нужен для создания Awaiter объекта для конкретного параметра
AwaitObject PromiseType::yield_value(WorkQueue& wq) {
    Log(__func__);
    
    // Проверяем, нужно ли нам вообще засыпать, может быть мы уже в правильной очереди
    const bool needResume = (_currentQueue == &wq);
    
    // Сохряняем целевую очередь
    _currentQueue = &wq;
    
    // Создаем await объект, чтобы корутина поставилась в очередь уже после засыпания, а не до - иначе может быть проблема с просыпанием до засыпания
    return AwaitObject(needResume, wq);
}

// Вызывается при вызове co_await, метод нужен для создания Awaiter объекта для конкретного параметра
AwaitObject PromiseType::await_transform(WorkQueue& wq) {
    return yield_value(wq);
}
