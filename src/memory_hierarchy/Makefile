



model1:
	g++ -w -std=c++11 top.cpp cpu.cpp cache.cpp directory.cpp esu_fsm.cpp msi_fsm.cpp memory_hierarchy_configuration.cpp -lsystemc	

test1:
	g++ -g -w -std=c++11 cpu.cpp cache.cpp memory_hierarchy_configuration.cpp msi_fsm.cpp -lsystemc

traces: false_sharing parse_trace.awk
	pin -t /home/konstantinos/Documents/build_directories/atomic-memory-trace/trace/obj-intel64/trace.so -i 1 -r 1 -- ./false_sharing
	./parse_trace.awk memory_trace.out

false_sharing: false_sharing.cpp annotation.cpp
	g++ -std=c++11 -O2 -o false_sharing false_sharing.cpp annotation.cpp -pthread -lboost_system -lboost_thread

clean:
	rm -f false_sharing memory_trace.out thread_*


