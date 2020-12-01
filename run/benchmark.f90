program benchmark
  use iso_c_binding
  use fortran_gaspi
  use mpi
  implicit none
  integer(c_int32_t) :: mylen, check
  real(c_double), allocatable :: mybuf(:)
  integer :: ierr

  mylen = 4
  check = 1
  allocate(mybuf(mylen))
  mybuf = 1.0d0

  call mpi_init (ierr)
  call init_gaspi ()
  call setup_double_buffers(mylen)
  call gaspi_allreduce_sum( mybuf(:), mylen, check)
  call gaspi_barrier ()
  write(*, *) mybuf(:)
  call close_gaspi ()
  call mpi_finalize (ierr)
end program benchmark
