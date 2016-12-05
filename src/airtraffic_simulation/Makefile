compile:
	mpicxx -g --std=c++11 process.cpp testbench.cpp

run: a.out
	mpirun -n 3 a.out

debug: a.out
	mpirun -np 3 xterm -e gdb a.out

clean_all:
	rm -f airport_log* flight_schedule* a.out

clean_csv:
	rm -f airport_log* flight_schedule*

clean_exe:
	rm a.out
