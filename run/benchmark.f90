program benchmark
  use iso_c_binding
  use fortran_gaspi
  implicit none
  
  integer(c_int32_t) :: mylen, check, ret
  real(c_double), allocatable :: mybuf(:)
  
  allocate(mybuf(mylen))
  check = 1
  mybuf = 1.0d0

  call setup_double_buffers(mylen)
  ret = gaspi_allreduce_sum(mybuf, mylen, check)
  
end program benchmark

