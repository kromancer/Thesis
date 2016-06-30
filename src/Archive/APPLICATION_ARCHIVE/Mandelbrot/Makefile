
##### User configurable options #####

MAKE                 = make --no-print-directory

MPI_CC               = /opt/intel/compilers_and_libraries_2016.2.181/linux/mpi/intel64/bin/mpicc
MPI_F77              = /opt/intel/compilers_and_libraries_2016.2.181/linux/mpi/intel64/bin/mpif77
MPI_CLINKER          = $(MPI_CC)
MPI_FLINKER          = $(MPI_F77)
MPE_CLINKER          = /opt/mpe/bin/mpecc -
MPE_FLINKER          = /opt/mpe/bin/mpefc -
#
DEBUG_LIBS           = 
MPI_CFLAGS           = -I/opt/intel/compilers_and_libraries_2016.2.181/linux/mpi/intel64/include 
MPI_FFLAGS           = -I/opt/intel/compilers_and_libraries_2016.2.181/linux/mpi/intel64/include 
MPI_LIBS             = -L/opt/intel/compilers_and_libraries_2016.2.181/linux/mpi/intel64/lib
CFLAGS               = 
FFLAGS               = 
MPE_INCDIR           = /opt/mpe/include
MPE_LIBDIR           = /opt/mpe/lib
MPE_SBINDIR          = /opt/mpe/sbin
ANIM_LIBS            = -L$(MPE_LIBDIR) -lampe -lmpe -lX11 $(MPI_LIBS)
C_XGRAPH_LIBS        = -L$(MPE_LIBDIR) -lmpe -lX11 $(MPI_LIBS)
F_XGRAPH_LIBS        = -L$(MPE_LIBDIR) -lmpe -lX11 $(MPI_LIBS)
#
C_INCS               = -I$(MPE_INCDIR)
F_INCS               = -I$(MPE_INCDIR)
LDFLAGS              = 
MPE_BUILD_FORTRAN2C  = yes

### End User configurable options ###

SHELL         = /bin/bash
RM            = rm 

VPATH=.:$(srcdir)

srcdir        = .

# MPE include path, x_INCS, goes in front of MPI include path, MPI_xFLAGS,
# in case MPI include path is in MPI_xFLAGS containing old MPE include path.
MPICC_FLAGS   = $(C_INCS) $(MPI_CFLAGS) $(MPE_COPTS)
MPIFC_FLAGS   = $(F_INCS) $(MPI_FFLAGS) $(MPE_FOPTS)
MPICC_LDFLAGS = $(LDFLAGS) $(MPI_CFLAGS) $(MPE_LDOPTS)
MPIFC_LDFLAGS = $(LDFLAGS) $(MPI_FFLAGS) $(MPE_LDOPTS)
EXECS         = mandelbort

.SUFFIXES: .c .o 

.c.o:
	$(MPI_CC) $(MPICC_FLAGS) -c $<

mandelbrot: main.o
	$(MPE_CLINKER)mpianim $(MPICC_LDFLAGS) -o $@ $? $(DEBUG_LIBS) -lm 
#	$(MPI_CLINKER) $(MPICC_LDFLAGS) -o $@ $? $(ANIM_LIBS) $(DEBUG_LIBS) -lm 


clean:
	@-$(RM) -f work.pc work.pcl
	@-$(RM) -f *.o *~ PI* $(EXECS)
	@-$(RM) -f cpi_anim cxgraphics fxgraphics



