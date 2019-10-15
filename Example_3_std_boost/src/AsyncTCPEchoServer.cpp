#include "AsyncTCPEchoServer.h"

////////////////////////////////////////////////////////////////////////////////////

struct ReadAwaiter {
    // Поток данных
    tcp::socket &s;
    // Буффер с данными
    boost::asio::mutable_buffer buffers;
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

ReadAwaiter asyncReadSome(tcp::socket&s, boost::asio::mutable_buffer&& buffers) {
    auto awaiter = ReadAwaiter{
        s,
        std::forward<boost::asio::mutable_buffer>(buffers), // Перемещаем buffers в Awaiter
        std::error_code(),
        0
    };
    return awaiter;
}

////////////////////////////////////////////////////////////////////////////////////

struct WriteAwaiter {
    // Поток данных
    tcp::socket &s;
    // Буффер с данными
    boost::asio::mutable_buffer buffers;
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

WriteAwaiter asyncWriteSome(tcp::socket& s, boost::asio::mutable_buffer&& buffers) {
    WriteAwaiter obj = WriteAwaiter{
        s,
        std::forward<boost::asio::mutable_buffer>(buffers), // Перемещаем buffers в Awaiter
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
    doRunLoop();
}

void Session::doRunLoop() { // Может возвращать CoroTask
    // Создаем временную ссылку на текущий объект, чтобы не уничтожился текущий объект, пока мы ждем в корутине
    std::shared_ptr<Session> self(shared_from_this());
    // Читать будем в цикле
    while (true) {
        // Запускаем асинхронное чтение в локальный буффер данных
        std::cout << "Before read" << std::endl;
        ReadAwaiter readAwaiter = asyncReadSome(_socket, boost::asio::buffer(_data, BUFFER_SIZE)); // Иницируем задачу
        const std::pair<std::error_code, size_t> readResult = co_await readAwaiter; // Дожидаемся результата с возобновлением работы корутины, либо не ждем, если результат готов
        std::cout << "after read" << std::endl;
        
        // Если возникла ошибка - прерываем работу сессии
        if (readResult.first) {
            std::cout << "Error reading from socket: " << readResult.first << std::endl;
            break;
        }

        // Инициируем процедуру асинхронной записи из локального буффера сессии
        std::cout << "before write" << std::endl;
        WriteAwaiter writeAwaiter = asyncWriteSome(_socket, boost::asio::buffer(_data, readResult.second)); // Иницируем задачу
        const std::pair<std::error_code, std::size_t> res = co_await writeAwaiter; // Дожидаемся результата с возобновлением работы корутины, либо не ждем, если результат готов
        std::cout << "after write" << std::endl;
        
        // Если возникла ошибка - прерываем работу сессии
        if (res.first) {
            std::cout << "Error writing to socket: " << res.first << std::endl;
            break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////

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

AcceptAwaiter asyncAccept(tcp::acceptor& acceptor) {
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

void Server::doAccept() { // Может возвращать CoroTask
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
