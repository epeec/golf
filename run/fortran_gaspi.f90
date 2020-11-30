module fortran_gaspi
  use iso_c_binding
  implicit none
  
  interface
    subroutine setup_double_buffers (mylen) bind(C, NAME="setup_double_buffers_f")
      use iso_c_binding
      implicit none
      integer(c_int32_t) :: mylen
    end subroutine setup_double_buffers

    function gaspi_allreduce_sum (mybuf, mylen, check) result(rvalue) bind(C, NAME="gaspi_allreduce_sum_f")
      use iso_c_binding
      implicit none
      real(c_double)     :: mybuf(mylen)
      integer(c_int32_t) :: mylen
      integer(c_int32_t) :: check
      integer(c_int32_t) :: rvalue
    end function gaspi_allreduce_sum

  end interface

end module fortran_gaspi
