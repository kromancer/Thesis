CXX=icpc


FLAGS=-I/home/kisp/TLM_skeleton/headers -I/opt/systemc-2.3.1/include -L/opt/systemc-2.3.1/lib-linux64 -lsystemc


SOURCES=main.cpp top.cpp lt_initiator.cpp lt_target.cpp SimpleBus.cpp

all: ${SOURCES}
	${CXX} ${FLAGS} ${SOURCES}
