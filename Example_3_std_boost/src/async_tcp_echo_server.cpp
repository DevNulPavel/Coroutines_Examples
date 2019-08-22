#include "async_tcp_echo_server.h"

struct CoroutineConfig;

/*struct CoroTask {
    using promise_type = CoroutineConfig;
};*/

struct CoroutineConfig {
public:
    // Данный метод вызывается при выходе из корутины
    void return_void() const{
    }
    void get_return_object() const {
    }
    std::experimental::suspend_never initial_suspend() {
        return std::experimental::suspend_never();
    }
    std::experimental::suspend_never final_suspend() {
        return std::experimental::suspend_never();
    }
    void return_void() {
    }
    void unhandled_exception() {
        std::terminate();
    }
};

template <typename... Args>
struct std::experimental::coroutine_traits<void, Args...> {
    using promise_type = CoroutineConfig;
};


////////////////////////////////////////////////////////////////////////////////////

template <typename SyncReadStream, typename DynamicBuffer>
auto async_read_some(SyncReadStream &s, DynamicBuffer &&buffers) {
    struct Awaiter {
        // Поток данных
        SyncReadStream &s;
        // Буффер с данными
        DynamicBuffer buffers;
        // Код ошибки
        std::error_code ec;
        // Размер данных
        size_t sz;

        // Данные к моменту запуска корутины не должны быть готовы
        bool await_ready() {
            return false;
        }
        // Вызывается после засыпания корутины
        void await_suspend(std::experimental::coroutine_handle<> coro) {
            // Коллбек чтения данных
            auto readCallback = [this, coro](auto ec, auto sz) mutable {
                this->ec = ec;
                this->sz = sz;
                std::cout << "read completed" << std::endl;
                // После прочтения - восстанавливаем корутину
                coro.resume();
            };
            // Запуск чтения данных
            s.async_read_some(std::move(buffers), readCallback);
        }
        auto await_resume() {
            return std::make_pair(ec, sz);
        }
    };
    auto awaiter = Awaiter{
        s,
        std::forward<DynamicBuffer>(buffers),
        std::error_code(),
        0
    };
    return awaiter;
}

////////////////////////////////////////////////////////////////////////////////////

template <typename SyncReadStream, typename DynamicBuffer>
auto async_write_some(SyncReadStream &s, DynamicBuffer &&buffers) {
    struct Awaiter {
        // Поток данных
        SyncReadStream &s;
        // Буффер с данными
        DynamicBuffer buffers;
        // Код ошибки
        std::error_code ec;
        // Размер данных
        size_t sz;

        // Корутина при асинхронной записи должна засыпать
        bool await_ready() {
            return false;
        }
        
        // Вызывается после засыпания
        void await_suspend(std::experimental::coroutine_handle<> coro) {
            // В коллбеке после успешной записи мы восстанавливаем корутину
            auto writeCallback = [this, coro](auto ec, auto sz) mutable {
                // Сохраняем код ошибки
                this->ec = ec;
                // Размер записанных данных
                this->sz = sz;
                
                std::cout << "write completed" << std::endl;
                
                // Возобновляем работу корутины
                coro.resume();
            };
            // Ставим в очередь запись
            boost::asio::async_write(s, std::move(buffers), writeCallback);
        }
        
        // Вызывается для получения результатов из корутины
        auto await_resume() {
            return std::make_pair(ec, sz);
        }
    };
    Awaiter obj = Awaiter{
        s,
        std::forward<DynamicBuffer>(buffers),
        std::error_code(),
        0
    };
    return obj;
}

////////////////////////////////////////////////////////////////////////////////////

Session::Session(tcp::socket socket):
    _socket(std::move(socket)) {
}

// Запускаем у сессии ожидание чтения данных
void Session::start() {
    do_read();
}

auto Session::do_write(std::size_t length) {
    struct Awaiter {
        std::shared_ptr<Session> currentSession;
        std::size_t length;
        std::error_code ec;

        // Может быть не нужно засыплять корутину, так как результат уже готов -
        // нам нужно всегда усыплять
        bool await_ready() {
            return false;
        }
        
        // Вызывается после усыпления сессии
        void await_suspend(std::experimental::coroutine_handle<> coro) {
            
            // После усыпления запускаем процесс записи в сокет данных
            const auto[ec, sz] = co_await async_write_some(currentSession->_socket, boost::asio::buffer(currentSession->_data, length));
            
            // Сохраняем код ошибки
            this->ec = ec;
            std::cout << "do_write completed" << std::endl;
            // Восстанавливаем работу нашей сессии
            coro.resume();
        }
        
        // Возвращаемое значение корутины, когда мы восстанавливаем ее состояние?
        auto await_resume() {
            return ec;
        }
    };
    
    std::shared_ptr<Session> self(shared_from_this());
    auto awaiter = Awaiter{
        self,
        length,
        std::error_code()
    };
    return awaiter;
}

void Session::do_read() {
    // Создаем временную ссылку на текущий объект
    std::shared_ptr<Session> self(shared_from_this());
    // Читать будем в цикле
    while (true) {
        std::cout << "before read" << std::endl;
        
        // Запускаем асинхронное чтение
        const auto[ec, sz] = co_await async_read_some(_socket, boost::asio::buffer(_data, max_length));
        
        std::cout << "after read" << std::endl;
        
        // Если не возникло никакой ошибки, то
        if (!ec) {
            // Инициируем процедуру асинхронной записи
            std::cout << "before write" << std::endl;
            auto ec = co_await do_write(sz);
            std::cout << "after write" << std::endl;
            
            // Если возникла ошибка - прерываем работу
            if (ec) {
                std::cout << "Error writing to socket: " << ec << std::endl;
                break;
            }
        } else {
            std::cout << "Error reading from socket: " << ec << std::endl;
            break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////

template <typename Acceptor>
auto async_accept(Acceptor &acceptor) {
    // Функция корутина должна возвращать объект, который содержит необходимые методы
    struct Awaiter {
        Acceptor& acceptor;
        tcp::socket socket;
        std::error_code ec;
        
        // Определяем, нужно ли нам вообще засыпать - засыпать надо всегда
        bool await_ready() {
            return false;
        }
        
        void await_suspend(std::experimental::coroutine_handle<> coro) {
            // После засыпания начинаем ждать нового подключения
            acceptor.async_accept([this, coro](auto ec, auto socket) mutable {
                // Данная лямбда вызывается из цикла asio, который в main
                // При поступлении нового соединения - сохраняем новый сокет и код ошибки
                this->ec = ec;
                this->socket = std::move(socket);

                // Восстанавливаем работу корутины
                coro.resume();
            });
        }
        
        // Определяем, что возвращаем когда просыпается корутина
        auto await_resume() {
            return std::make_pair(ec, std::move(socket));
        }
    };
    // acceptor.get_executor().context()
    return Awaiter{acceptor, tcp::socket(acceptor.get_executor()), std::error_code()};
}

////////////////////////////////////////////////////////////////////////////////////

Server::Server(boost::asio::io_context &io_context, short port):
    _acceptor(io_context, tcp::endpoint(tcp::v4(), port)) {
    // Стартуем прием данных
    do_accept();
}

void Server::do_accept() {
    // В цикле ожидаем поступления новых соединений
    while (true) {
        // Получаем ошибку и новый сокет в асинронном режиме с сохранением корутины
        auto[ec, socket] = co_await async_accept(_acceptor);
        
        // Если нет никакой ошибки - создаем новую сессию
        if (!ec) {
            std::shared_ptr<Session> newSession = std::make_shared<Session>(std::move(socket));
            // Запускаем обработку данных в сессии
            newSession->start();
        } else {
            std::cout << "Error accepting connection: " << ec << std::endl;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: async_tcp_echo_server <port>\n";
            return 1;
        }
        
        // Создаем контекст исполнения
        boost::asio::io_context io_context;
        
        // Создаем сервер с нужным нам портом
        Server s(io_context, std::atoi(argv[1]));
        
        // Запускаем цикл сервера, все вызовы asio будут происходить из этого цикла
        io_context.run();
        
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
