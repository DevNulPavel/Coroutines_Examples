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

struct PromiseType;

struct CoroTask {
    using promise_type = PromiseType;
};

////////////////////////////////////////////////////////////////////////////////////

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket);
    
    void start();
    
private:
    CoroTask doRunLoop();
    
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
    CoroTask doAccept();
    
private:
    tcp::acceptor _acceptor;
};

