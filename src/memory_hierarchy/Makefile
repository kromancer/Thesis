




traces: false_sharing parse_trace.awk
	pin -t /home/konstantinos/Documents/build_directories/atomic-memory-trace/trace/obj-intel64/trace.so -i 1 -r 1 -- ./false_sharing
	./parse_trace.awk memory_trace.out

false_sharing: false_sharing.cpp annotation.cpp
	g++ -std=c++11 -O2 -o false_sharing false_sharing.cpp annotation.cpp -pthread -lboost_system -lboost_thread

clean:
	rm -f false_sharing memory_trace.out thread_*


