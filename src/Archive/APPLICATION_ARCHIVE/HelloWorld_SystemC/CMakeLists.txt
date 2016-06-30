cmake_minimum_required(VERSION 3.2.2)
project(HelloWorld_SystemC)

#Bring the headers
include_directories(/opt/systemc-2.3.1/include)

#For the shared library
set( PROJECT_LINK_LIBS libsystemc.so )
link_directories(/opt/systemc-2.3.1/lib-linux64)

#Add the sources
file(GLOB SOURCES "*.cpp")

#What is needed for the executable
add_executable(helloWorld ${SOURCES})
target_link_libraries(helloWorld ${PROJECT_LINK_LIBS})