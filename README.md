# GASPI collective benchmarking

The folder external/ constains the gpi-collectives
and the gpi2 sources. gpi2 need to be installed and
gpi-collectives are automatically included while
compiling the benchmark code.


# Fortran bindings

Fortran bindingd for AVBP is provided in sources. Users must
compile agaist the `fortan_gaspi` module provided in `external`
folder (file:`fortran_gaspi.f90`). Works on MPI+GASPI mode as 
well. 
