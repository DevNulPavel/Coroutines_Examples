cmake_minimum_required(VERSION 3.14)

set (PROJECT CoroutineTest)
project(${PROJECT})

# Sanitizer
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

##### Компилятор #####
# Вариант с путями
# execute_process(COMMAND which clang OUTPUT_VARIABLE CLANG_PATH)
# execute_process(COMMAND which clang++ OUTPUT_VARIABLE CLANGXX_PATH)
# if(NOT ("${CLANG_PATH}" STREQUAL ""))

# Вариант поиска программы
#find_program(CLANG_FOUND clang)
#if(CLANG_FOUND)
#	message("-> Clang will used for compilation")
#	set(CMAKE_C_COMPILER "clang")
#	set(CMAKE_CXX_COMPILER "clang++")
#	# Скрываем ворнинги на clang + цвета
#	# set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer -Qunused-arguments")
#	# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fsanitize=address -fno-omit-frame-pointer -Qunused-arguments")
#endif(CLANG_FOUND)

# Ищем Boost
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF)
set(BOOST_COMPONENTS thread date_time system regex context coroutine)
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

# Sanitizer
if(CLANG_FOUND)
	find_package(Sanitizers)
endif(CLANG_FOUND)

# Правка собираемости
if (WIN32 AND __COMPILER_GNU)
    # mingw-gcc fails to link boost::thread
    add_definitions(-DBOOST_THREAD_USE_LIB)
endif (WIN32 AND __COMPILER_GNU)

# дефайны
add_definitions(-DDEBUG)

# флаги
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -Wall")

# Список исходников
set (HEADERS 
		)
set (SOURCES
		"main.cpp")

# создаем группу, чтобы заголовочники и исходники были в одной папке
source_group("Sources" FILES ${HEADERS} ${SOURCES})

# исполняемый файл
# add_executable(${PROJECT} ${APP_TYPE} ${HEADERS} ${SOURCES})
add_executable(${PROJECT} ${HEADERS} ${SOURCES})

# линкуемые библиотеки
target_link_libraries(${PROJECT} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})

# Sanitizer
if(CLANG_FOUND)
	add_sanitizers(${PROJECT})
endif(CLANG_FOUND)
