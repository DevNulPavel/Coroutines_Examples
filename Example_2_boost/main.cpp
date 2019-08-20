#define BOOST_EXCEPTION_DISABLE 1
// std
#include <thread>
#include <mutex>
#include <iostream>
#include <string>
#include <unordered_map>
#include <condition_variable>
// Boost
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/coroutine/all.hpp>

// сокращения для пространств имен
namespace bc = boost::coroutines;
namespace bs = boost::system;

using namespace std;
using namespace bc;
using namespace bs;

void asymmetricCoroutineTestPullType(){
    // метод-сопрограмма по обработке
    auto coroutineMethod = [&](bc::asymmetric_coroutine<int>::push_type& sink){
        int first = 1;
        int second = 1;

        // выдываем возврат значения + продолжение далее кода вызова
        sink(second);
        
        // в цикле обрабатываем значения
        for(int i = 0; i < 8; ++i){
            int third = first + second;
            first = second;
            second = third;
            
            // выдываем возврат значения + продолжение далее кода вызова
            sink(third);
        }
    };
    // создание сопрограммы
    bc::asymmetric_coroutine<int>::pull_type source(coroutineMethod);
    
    std::cout << "Assymmetric coroutine, pull type (Fibonaci):" << endl;
    for(auto i: source){
        std::cout << i <<  " ";
    }
    std::cout << std::endl << std::endl;
}

void asymmetricCoroutineTestPushType(){
    struct FinalEOL{
        ~FinalEOL(){
            std::cout << std::endl;
        }
    };
    
    const int num = 3;
    const int width = 15;
    
    auto coroutineMethod = [&](bc::asymmetric_coroutine<std::string>::pull_type& in){
        // специальный объект, который показывает существование сопрограммы
        FinalEOL eol;
        // читаем данные из входного потока и обрабатываем их
        for (;;){
            for(int i = 0; i < num; ++i){
                // если нету входных данных, то выходим из сопрограммы
                if(!in){
                    return;
                }
                
                // выводим входные данные
                std::cout << std::setw(width) << in.get();
                
                // обработали входной итем - переходим к следующему
                in();
            }
            // перенос строки после окончания строки
            std::cout << std::endl;
        }
    };
    
    bc::asymmetric_coroutine<std::string>::push_type writer(coroutineMethod);
    
    std::vector<std::string> words{
        "peas", "porridge", "hot", "peas",
        "porridge", "cold", "peas", "porridge",
        "in", "the", "pot", "nine",
        "days", "old", "." };
    
    
    // выполняем поэлементное копирование элементов в сопрограмму
    std::cout << "Assymmetric coroutine, push:" << endl;
    std::copy(boost::begin(words), boost::end(words), boost::begin(writer));
    std::cout << std::endl;
}

void asymetricGetFromCoroutine(){
    auto coroutineMethod = [&](bc::asymmetric_coroutine<int>::push_type& sink){
        sink(1); // возвращаем значение и переходим к главному контексту
        sink(2);
        sink(3);
        sink(5);
        sink(8);
    };
    bc::asymmetric_coroutine<int>::pull_type source(coroutineMethod);
    
    // есть ли у нас сопрограмма?
    std::cout << "Assymmetric coroutine, get val:" << endl;
    while(source){
        int ret = source.get(); // получаем данные
        std::cout << ret <<  " ";
        
        source();             // идем продолжать выполнение сопрограммы
    }
    std::cout << std::endl << std::endl;
}

void asymetricSetToCoroutine(){
    auto coroutineMethod = [&](bc::asymmetric_coroutine<int>::pull_type& source){
        for (int i:source) {
            std::cout << i <<  " ";
        }
    };
    bc::asymmetric_coroutine<int>::push_type sink(coroutineMethod);
    
    std::cout << "Assymmetric coroutine, push val:" << endl;
    std::vector<int> v{1,2,3,5,8,13,21,34,55};
    for( int i: v){
        sink(i); // push {i} to coroutine-function
    }
    std::cout << std::endl << std::endl;
}

int main(int argc, char* argv[]) {
    asymmetricCoroutineTestPullType();

    asymmetricCoroutineTestPushType();
    
    asymetricGetFromCoroutine();

    asymetricSetToCoroutine();

    return 0;
}
