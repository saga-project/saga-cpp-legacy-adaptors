# $RCSfile: Makefile,v $ $Revision: 1.3 $ $Date: 2008/03/25 03:15:43 $
# $AIST_Release: 5.0.0 $
# $AIST_Copyright:
#  Copyright 2003, 2004, 2005, 2006 Grid Technology Research Center,
#  National Institute of Advanced Industrial Science and Technology
#  Copyright 2003, 2004, 2005, 2006 National Institute of Informatics
#  
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#  
#      http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#  $

FC = mpif77

CLIENT = test_client
MODULE = mpi_test
IDL    = mpi_test_f.idl
MPI_TEST_INT_MPI=mpi_test_int_mpi
# for ifort
#RL_LDFLAGS = -nofor_main
# for pgf77
#RL_LDFLAGS = -Mnomain

NG_CC  = $(NG_DIR)/bin/ng_cc
NG_GEN = $(NG_DIR)/bin/ng_gen

all: $(CLIENT) stub
client: $(CLIENT)
server: stub
mpi_only: mpi_test_int_mpi

install:
	$(MAKE) -f $(MODULE).mak install

$(CLIENT): $(CLIENT).c
	$(NG_CC) -o $@ $(CLIENT).c

stub: $(IDL)
	$(NG_GEN) $(IDL)
	$(MAKE) FC=$(FC) LDFLAGS=$(RL_LDFLAGS) -f $(MODULE).mak

clean:
	if test -f ./$(MODULE).mak; then \
		$(MAKE) -f $(MODULE).mak veryclean; \
	fi
	rm -f $(MODULE).mak $(CLIENT) $(MODULE):*
	rm -f ${MPI_TEST_INT_MPI}
	rm -f *~ *.o core core.*

$(MPI_TEST_INT_MPI): $(MPI_TEST_INT_MPI).o mpi_test_int.o
	$(FC) -o $@ $(MPI_TEST_INT_MPI).o mpi_test_int.o
