module fortran_gaspi
  use iso_c_binding
  implicit none
  
  interface
  
    subroutine setup_double_buffers (mylen) bind(C, NAME="setup_double_buffers_f")
      use iso_c_binding
      implicit none
      integer(c_int32_t) :: mylen
    end subroutine setup_double_buffers

    subroutine gaspi_allreduce_sum (mybuf, mylen, check) bind(C, NAME="gaspi_allreduce_sum_f")
      use iso_c_binding
      implicit none
      real(c_double)     :: mybuf(mylen)
      integer(c_int32_t) :: mylen
      integer(c_int32_t) :: check    
    end subroutine gaspi_allreduce_sum

    subroutine init_gaspi () bind(C, NAME="init_gaspi_f")
      use iso_c_binding
      implicit none
    end subroutine init_gaspi

    subroutine close_gaspi () bind(C, NAME="close_gaspi_f")
      use iso_c_binding
      implicit none
    end subroutine close_gaspi

    subroutine gaspi_barrier () bind(C, NAME="gaspi_barrier_f")
      use iso_c_binding
      implicit none
    end subroutine gaspi_barrier

  end interface

end module fortran_gaspi
