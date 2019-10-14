#include "AsyncTCPEchoServer.h"

////////////////////////////////////////////////////////////////////////////////////

template <typename SyncReadStream, typename DynamicBuffer>
auto asyncReadSome(SyncReadStream& s, DynamicBuffer&& buffers) {
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
        std::pair<std::error_code, size_t> await_resume() {
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
auto asyncWriteSome(SyncReadStream &s, DynamicBuffer &&buffers) {
    struct WriteAwaiter {
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
        std::pair<std::error_code, size_t> await_resume() {
            return std::make_pair(ec, sz);
        }
    };
    WriteAwaiter obj = WriteAwaiter{
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
    doRead();
}

auto Session::doWrite(std::size_t length) {
    struct WriteAwaiter {
        std::shared_ptr<Session> currentSession;
        std::size_t length;
        std::error_code ec;

        // Может быть не нужно засыплять корутину, так как результат уже готов - нам нужно всегда усыплять
        bool await_ready() {
            return false;
        }
        
        // Вызывается после усыпления сессии
        void await_suspend(std::experimental::coroutine_handle<> coro) {
            
            // После усыпления запускаем процесс записи в сокет данных
            const std::pair<std::error_code, std::size_t> res = co_await asyncWriteSome(currentSession->_socket, boost::asio::buffer(currentSession->_data, length));
            
            // Сохраняем код ошибки
            this->ec = res.first;
            std::cout << "do_write completed" << std::endl;
            
            // Восстанавливаем работу нашей сессии
            coro.resume();
        }
        
        // Возвращаемое значение корутины, когда мы восстанавливаем ее состояние?
        std::error_code await_resume() {
            return ec;
        }
    };
    
    std::shared_ptr<Session> self(shared_from_this());
    WriteAwaiter awaiter = WriteAwaiter{
        self,
        length,
        std::error_code()
    };
    return awaiter;
}

void Session::doRead() {
    // Создаем временную ссылку на текущий объект
    std::shared_ptr<Session> self(shared_from_this());
    // Читать будем в цикле
    while (true) {
        std::cout << "Before read" << std::endl;
        
        // Запускаем асинхронное чтение в локальный буффер данных
        const std::pair<std::error_code, size_t> readResult = co_await asyncReadSome(_socket, boost::asio::buffer(_data, BUFFER_SIZE));
        
        std::cout << "after read" << std::endl;
        
        // Если не возникло никакой ошибки, то
        if (!readResult.first) {
            // Инициируем процедуру асинхронной записи из локального буффера сессии
            std::cout << "before write" << std::endl;
            auto ec = co_await doWrite(readResult.second);
            std::cout << "after write" << std::endl;
            
            // Если возникла ошибка - прерываем работу
            if (ec) {
                std::cout << "Error writing to socket: " << ec << std::endl;
                break;
            }
        } else {
            std::cout << "Error reading from socket: " << readResult.first << std::endl;
            break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////

auto asyncAccept(tcp::acceptor& acceptor) {
    // Функция корутина должна возвращать объект, который содержит необходимые методы
    struct AcceptAwaiter {
        tcp::acceptor& acceptor;
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
        std::pair<std::error_code, tcp::socket> await_resume() {
            return std::make_pair(ec, std::move(socket));
        }
    };
    
    // acceptor.get_executor().context()
    return AcceptAwaiter{
        acceptor,
        tcp::socket(acceptor.get_executor()),
        std::error_code()
    };
}

////////////////////////////////////////////////////////////////////////////////////

Server::Server(boost::asio::io_context &io_context, short port):
    _acceptor(io_context, tcp::endpoint(tcp::v4(), port)) {
    // Стартуем прием данных
    doAccept();
}

void Server::doAccept() {
    // В цикле ожидаем поступления новых соединений
    while (true) {
        // Получаем ошибку и новый сокет в асинронном режиме с сохранением корутины
        std::pair<std::error_code, tcp::socket> accepted = co_await asyncAccept(_acceptor);
        
        // Если нет никакой ошибки - создаем новую сессию
        if (!accepted.first) {
            std::shared_ptr<Session> newSession = std::make_shared<Session>(std::move(accepted.second));
            // Запускаем обработку данных в сессии и выходим
            newSession->start();
        } else {
            std::cout << "Error accepting connection: " << accepted.first << std::endl;
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
        
        std::cerr << "Trying to start server at " << std::atoi(argv[1]) << " port..." << std::endl;
        
        // Создаем контекст исполнения
        boost::asio::io_context ioContext;
        
        // Создаем сервер с нужным нам портом
        Server s(ioContext, std::atoi(argv[1]));
        
        std::cerr << "Running..." << std::endl;
        
        // Запускаем цикл сервера, все вызовы asio будут происходить из этого цикла
        ioContext.run();
        
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
