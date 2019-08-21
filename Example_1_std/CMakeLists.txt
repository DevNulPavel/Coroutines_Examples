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

# Стандарт С++
#set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_REQUIRED_FLAGS -std=c++2a)
#remove_definitions(-std=c++14)
#remove_definitions(-std=gnu++14)
#add_definitions(-std=c++2a -fcoroutines-ts)

# Флаги (-std=c++2a -fcoroutines-ts)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -Wextra -std=c++2a -fcoroutines-ts")

# Список исходников
set (HEADERS 
		"src/config.h"
		"src/coro_task.h"
		"src/functions.h"
		"src/work_queue.h"
		"src/user_code_1.h"
		"src/user_code_2.h")
set (SOURCES
		"src/functions.cpp"
		"src/coro_task.cpp"
		"src/work_queue.cpp"
		"src/main.cpp")

# создаем группу, чтобы заголовочники и исходники были в одной папке
source_group("Sources" FILES ${HEADERS} ${SOURCES})

# исполняемый файл
add_executable(${PROJECT} ${HEADERS} ${SOURCES})

# Стандарт С++
#set_property(TARGET ${PROJECT} PROPERTY CXX_STANDARD 20)