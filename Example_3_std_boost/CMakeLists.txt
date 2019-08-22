cmake_minimum_required(VERSION 3.14)

set (PROJECT CoroutineTest)
project(${PROJECT})

##### Компилятор #####
# Вариант с путями
# execute_process(COMMAND which clang OUTPUT_VARIABLE CLANG_PATH)
# execute_process(COMMAND which clang++ OUTPUT_VARIABLE CLANGXX_PATH)
# if(NOT ("${CLANG_PATH}" STREQUAL ""))
# Вариант поиска программы
find_program(CLANG_FOUND clang)
if(CLANG_FOUND)
	message("Clang will be used for compilation")
	set(CMAKE_C_COMPILER "clang")
	set(CMAKE_CXX_COMPILER "clang++")
	# Скрываем ворнинги на clang + цвета
	# set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer -Qunused-arguments")
	# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fsanitize=address -fno-omit-frame-pointer -Qunused-arguments")
endif(CLANG_FOUND)

# Ищем Boost
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF)
set(BOOST_COMPONENTS system)
find_package(Boost 1.42.0 REQUIRED COMPONENTS  ${BOOST_COMPONENTS})
if(Boost_FOUND)
	include_directories("${Boost_INCLUDE_DIR}/./") # Специально так, иначе не прописывается в XCode
	link_directories("${Boost_LIBRARY_DIRS}")
	message("Boost FOUND: ${Boost_INCLUDE_DIR}, ${Boost_LIBRARY_DIRS}, ${Boost_LIBRARIES}")
else(Boost_FOUND)
	message(FATAL_ERROR "No BOOST")
endif(Boost_FOUND)

# Поиск библиотеки потоков
find_package(Threads REQUIRED)
message("Threads found: ${Threads_FOUND} ${Threads_INCLUDE_DIR} ${Threads_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}")
set(Threads_LIBRARIES pthread)

# Стандарт С++
#set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_REQUIRED_FLAGS -std=c++2a)
#remove_definitions(-std=c++14)
#remove_definitions(-std=gnu++14)
#add_definitions(-std=c++2a -fcoroutines-ts)

# Флаги (-std=c++2a -fcoroutines-ts)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -Wextra -std=c++2a -fcoroutines-ts")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nodefaultlibs -lc++ -lc++abi -lm -lc") #  -lgcc_s

# Список исходников
set (HEADERS 
		"")
set (SOURCES
		"")

# создаем группу, чтобы заголовочники и исходники были в одной папке
source_group("Sources" FILES ${HEADERS} ${SOURCES})

# Исполняемый таргет
add_executable(AsyncServer "src/async_tcp_echo_server.h" "src/async_tcp_echo_server.cpp")
target_link_libraries(AsyncServer ${Threads_LIBRARIES} ${Boost_LIBRARIES})

# Исполняемый таргет
add_executable(AsyncServerCoro "src/async_tcp_echo_server_cppcoro.cpp")
target_link_libraries(AsyncServerCoro ${Threads_LIBRARIES} ${Boost_LIBRARIES})

# Стандарт С++
#set_property(TARGET ${PROJECT} PROPERTY CXX_STANDARD 20)