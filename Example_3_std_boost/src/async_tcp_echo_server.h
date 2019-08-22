#include <boost/asio.hpp>
#include <cstdlib>
#include <experimental/coroutine>
#include <iostream>
#include <memory>
#include <utility>

using boost::asio::ip::tcp;

template <typename SyncReadStream, typename DynamicBuffer>
auto async_read_some(SyncReadStream &s, DynamicBuffer &&buffers);

template <typename SyncReadStream, typename DynamicBuffer>
auto async_write(SyncReadStream &s, DynamicBuffer &&buffers);

template <typename Acceptor>
auto async_accept(Acceptor &acceptor);


class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket);
    
    void start();
    
private:
    auto do_write(std::size_t length);
    void do_read();
    
private:
    enum {
        max_length = 1024
    };
    tcp::socket _socket;
    char _data[max_length];
};



class Server {
public:
    Server(boost::asio::io_context &io_context, short port);
    
private:
    void do_accept();
    
private:
    tcp::acceptor _acceptor;
};

