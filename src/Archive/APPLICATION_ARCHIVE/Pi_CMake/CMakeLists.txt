cmake_minimum_required(VERSION 3.2.2)
project(calculatePI)

#Bring the headers
include_directories(/opt/intel/compilers_and_libraries_2016.2.181/linux/mpi/intel64/include)

#For the shared library
set( PROJECT_LINK_LIBS libmpicxx.so.12 )
link_directories(/opt/intel/compilers_and_libraries_2016.2.181/linux/mpi/intel64/lib)

#Add the sources
file(GLOB SOURCES "*.cpp")

#What is needed for the executable
add_executable(calculatePI ${SOURCES})
target_link_libraries(calculatePI ${PROJECT_LINK_LIBS})