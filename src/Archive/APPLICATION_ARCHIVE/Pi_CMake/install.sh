#! /bin/bash

rm -rf objdir
mkdir objdir
export CXX=/opt/intel/compilers_and_libraries_2016.2.181/linux/mpi/intel64/bin/mpicxx
cd objdir
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

# This file is needed by Emacs to autocomplete
mv compile_commands.json ..

make


