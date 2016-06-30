CC=icpc
CFLAGS=-c -I../Test_SysC/include -mmic
LFLAGS=-mmic -L../Test_SysC/lib-linux64/ -L/opt/intel/compilers_and_libraries_2016.2.181/linux/compiler/lib/intel64_lin_mic/ -lsystemc
SOURCES=main.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=hello_world

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
