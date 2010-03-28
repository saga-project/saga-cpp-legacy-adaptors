C $RCSfile: mpi_test_int_mpi.f,v $ $Revision: 1.1 $ $Date: 2007/09/03 03:19:12 $
C $AIST_Release: 5.0.0 $
C $AIST_Copyright:
C  Copyright 2003, 2004, 2005, 2006 Grid Technology Research Center,
C  National Institute of Advanced Industrial Science and Technology
C  Copyright 2003, 2004, 2005, 2006 National Institute of Informatics
C  
C  Licensed under the Apache License, Version 2.0 (the "License");
C  you may not use this file except in compliance with the License.
C  You may obtain a copy of the License at
C  
C      http://www.apache.org/licenses/LICENSE-2.0
C  
C  Unless required by applicable law or agreed to in writing, software
C  distributed under the License is distributed on an "AS IS" BASIS,
C  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
C  See the License for the specific language governing permissions and
C  limitations under the License.
C  $

        program main
        implicit none
        include 'mpif.h'

        integer n
        integer scalarIn
        integer scalarOut
        integer arrayIn(100)
        integer arrayOut(100)

        integer my_rank
        integer ierr
        integer i
        integer mismatchCount

        call MPI_INIT(ierr)
        call MPI_COMM_RANK(MPI_COMM_WORLD, my_rank, ierr)      
        call MPI_COMM_SIZE(MPI_COMM_WORLD, n, ierr)

        scalarIn = 1
        do i = 1, n
          arrayIn(i) = i-1
        end do
        
        call mpi_test_int(n, scalarIn, scalarOut, arrayIn, arrayOut)

C Check Result
        if(my_rank .eq.0) then
          if(scalarIn .ne. scalarOut) then
             write(6,*)'Result is NOT correct: scalarIn', scalarIn, 
     &                          '!= scalarOut(%d)', scalarOut
          else
             write(6,*) 'Result is correct: scalarIn', scalarIn,
     &                          ' == scalarOut', scalarOut
          end if

          mismatchCount = 0
          do i = 1, n
             if(arrayIn(i) .ne. arrayOut(i)) then
                write(6,*)'Result is NOT correct: arrayIn', arrayIn(i), 
     &                               '!= arrayOut', arrayOut(i), i
                mismatchCount = mismatchCount + 1
             end if
          end do
          if(mismatchCount .eq. 0) then
             write(6,*) 'Result is correct for all array elements'
          end if
        end if

        call MPI_FINALIZE(ierr)

        stop
        end
        
 
