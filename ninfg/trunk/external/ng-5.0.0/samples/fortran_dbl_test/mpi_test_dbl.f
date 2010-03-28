C $RCSfile: mpi_test_dbl.f,v $ $Revision: 1.1 $ $Date: 2007/09/03 03:19:12 $
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

        subroutine mpi_test_dbl(n,scalarIn,scalarOut,arrayIn,arrayOut)
        implicit none
        include 'mpif.h'

        integer n
        real*8 scalarIn
        real*8 scalarOut
        real*8 arrayIn(n)
        real*8 arrayOut(n)

        integer my_rank
        integer tag
        integer ierr
        integer i
        real*8 buffer(n)
        integer status(MPI_STATUS_SIZE)

        tag = 0
        call MPI_COMM_RANK(MPI_COMM_WORLD, my_rank, ierr)

C for debug
        if(my_rank.EQ.0) then
          open(1,file='checkInData',form='formatted',status='replace')
          write(1,*) 'my_rank:', my_rank
          write(1,*) 'n:', n
          write(1,*) 'scalarIn:', scalarIn
          write(1,*) 'arrayIn:', (arrayIn(i), i = 1, n)
          close(1)
        end if
       
        scalarOut = scalarIn
        arrayOut(my_rank+1) = arrayIn(my_rank+1)

        if (my_rank .ne. 0) then
           call MPI_SEND(arrayOut(my_rank+1), 1, MPI_DOUBLE_PRECISION,
     &                      0, tag, MPI_COMM_WORLD, ierr)
        else
C for debug
           open(1,file='checkBufData',form='formatted',status='replace')

           do i = 1, n-1
             buffer(i) = 0.D0
             call MPI_Recv(buffer(i), 1, MPI_DOUBLE_PRECISION,
     &                      i, tag, MPI_COMM_WORLD, status, ierr)
             arrayOut(i+1) = buffer(i)

C for debug
             write(1,*) 'buf:', buffer(i), i

           end do

C for debug
           close(1)

        end if
      
C for debug
        if(my_rank.EQ.0) then
          open(1,file='checkOutData',form='formatted',status='replace')
          write(1,*) 'my_rank:', my_rank
          write(1,*) 'scalarOut:', scalarOut
          write(1,*) 'arrayOut:', (arrayOut(i), i = 1, n)
          close(1)
        end if
       
        return
        end

