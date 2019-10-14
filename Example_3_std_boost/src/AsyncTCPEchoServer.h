#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <experimental/coroutine>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

////////////////////////////////////////////////////////////////////////////////////

template <typename SyncReadStream, typename DynamicBuffer>
auto async_read_some(SyncReadStream &s, DynamicBuffer &&buffers);

template <typename SyncReadStream, typename DynamicBuffer>
auto async_write(SyncReadStream &s, DynamicBuffer &&buffers);

template <typename Acceptor>
auto async_accept(Acceptor &acceptor);

////////////////////////////////////////////////////////////////////////////////////

struct CoroutineConfig {
public:
    // Данный метод вызывается при выходе из корутины
    void return_void() const{
    }
    // Данный метод вызывается при выходе из корутины
    void return_void() {
    }
    // Данный метод вызывается при выходе из корутины
    void get_return_object() const {
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
};

template <typename... Args>
struct std::experimental::coroutine_traits<void, Args...> {
    using promise_type = CoroutineConfig;
};

////////////////////////////////////////////////////////////////////////////////////

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket);
    
    void start();
    
private:
    auto doWrite(std::size_t length);
    void doRead();
    
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

