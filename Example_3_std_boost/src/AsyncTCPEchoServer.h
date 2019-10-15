#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <experimental/coroutine>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

////////////////////////////////////////////////////////////////////////////////////

struct PromiseType;

struct CoroTask {
    using promise_type = PromiseType;
};

// Универсальный тип вместо CoroTask
template <typename... Args>
struct std::experimental::coroutine_traits<void, Args...> {
    using promise_type = PromiseType;
};

struct PromiseType {
public:
    // Данный метод вызывается при выходе из корутины c помощью co_return
    void return_void() const{
    }
    // Данный метод вызывается при выходе из корутины c помощью co_return
    void return_void() {
    }
    // Данный метод вызывается при выходе из корутины
    // с помощью данного метода фактически создается объект корутины
    void get_return_object() const {
        //return CoroTask(); // Может возвращать CoroTask
    }
    // Не засыпаем при входе в корутину
    std::experimental::suspend_never initial_suspend() {
        return std::experimental::suspend_never();
    }
    // Не засыпаем во время выхода корутины
    std::experimental::suspend_never final_suspend() {
        return std::experimental::suspend_never();
    }
    // При исключении будем вырубать приложение
    void unhandled_exception() {
        std::terminate();
    }
    // Вызывается при вызове co_yield, метод нужен для создания Awaiter объекта для конкретного типа параметра
    //auto yield_value(){
    //}
    
    // Вызывается при вызове co_await, метод нужен для создания Awaiter объекта для конкретного типа параметра
    //auto await_transform(){
    //}
};

////////////////////////////////////////////////////////////////////////////////////

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket);
    
    void start();
    
private:
    void doRunLoop();
    
private:
    enum {
        BUFFER_SIZE = 1024
    };
    tcp::socket _socket;
    uint8_t _data[BUFFER_SIZE];
};

////////////////////////////////////////////////////////////////////////////////////

class Server {
public:
    Server(boost::asio::io_context &io_context, short port);
    
private:
    void doAccept();
    
private:
    tcp::acceptor _acceptor;
};

