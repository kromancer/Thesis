tlm_lt_example: tlm_lt_example.cpp
	g++ tlm_lt_example.cpp -l systemc -o tlm_lt_example

run_tlm_lt: tlm_lt_example
	./tlm_lt_example

tlm_quantum: tlm_lt_with_quantum_keeper_example.cpp
	g++ tlm_lt_with_quantum_keeper_example.cpp -l systemc -o tlm_lt_with_quantum_keeper_example

run_tlm_quantum: tlm_quantum
	./tlm_lt_with_quantum_keeper_example

clean:
	rm -f tlm_lt_example tlm_lt_with_quantum_keeper_example
