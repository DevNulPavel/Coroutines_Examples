all:
	clang++ src/coro_task.cpp src/work_queue.cpp -Wall -Wextra -std=c++2a "-fcoroutines-ts"

OSX:
	mkdir -p build && \
	cd build && \
	cmake -G "Xcode" ../  && \
	open CoroutineTest.xcodeproj && \
	cd ../

clean:
	rm -rf build