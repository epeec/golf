program benchmark
  use iso_c_binding
  use fortran_gaspi
  implicit none
  integer(c_int32_t) :: mylen, check
  real(c_double), allocatable :: mybuf(:)

  mylen = 4
  check = 1
  allocate(mybuf(mylen))
  mybuf = 1.0d0

  call init_gaspi ()
  call setup_double_buffers(mylen)
  call gaspi_allreduce_sum( mybuf(:), mylen, check)
  call gaspi_barrier ()
  write(*, *) mybuf(:)
  call close_gaspi ()
end program benchmark
