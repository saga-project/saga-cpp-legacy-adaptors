# This file 'sample.mak' was created by /home/tashiro/ngdvl/ng2/p/bin/ng_gen. Don't edit

CC = gcc
include $(NG_DIR)/lib/template.mk

# CompileOptions:
NG_USER_CFLAGS = 

#  Define NG_COMPILER & NG_LINKER as $(CC) if it is not defined.

NG_COMPILER ?= $(CC)
NG_LINKER ?= $(CC)

# stub sources

NG_STUB_SRC = _stub_sin.c  _stub_mmul.c  _stub_mmul2.c  _stub_FFT.c 

# stub programs

NG_STUB_PROGRAM = _stub_sin _stub_mmul _stub_mmul2 _stub_FFT

# stub inf files
NG_INF_FILES = _stub_sin.inf _stub_mmul.inf _stub_mmul2.inf _stub_FFT.inf
# temporary NRF dummy target.
NRF_DUMMY = nrf_dummy

all: $(NG_STUB_PROGRAM) $(NG_INF_FILES) $(NRF_DUMMY)

_stub_sin.o: _stub_sin.c
	$(NG_COMPILER) $(CFLAGS) $(NG_CFLAGS) $(NG_USER_CFLAGS) $(NG_CPPFLAGS) -c _stub_sin.c

_stub_sin: _stub_sin.o 
	$(NG_LINKER) $(NG_CFLAGS) $(CFLAGS) -o _stub_sin _stub_sin.o $(LDFLAGS) $(NG_STUB_LDFLAGS)  $(LIBS) -lm

_stub_sin.inf: _stub_sin
	 ./_stub_sin -i _stub_sin.inf



_stub_mmul.o: _stub_mmul.c
	$(NG_COMPILER) $(CFLAGS) $(NG_CFLAGS) $(NG_USER_CFLAGS) $(NG_CPPFLAGS) -c _stub_mmul.c

_stub_mmul: _stub_mmul.o sample.o
	$(NG_LINKER) $(NG_CFLAGS) $(CFLAGS) -o _stub_mmul _stub_mmul.o $(LDFLAGS) $(NG_STUB_LDFLAGS) sample.o $(LIBS) -lm

_stub_mmul.inf: _stub_mmul
	 ./_stub_mmul -i _stub_mmul.inf



_stub_mmul2.o: _stub_mmul2.c
	$(NG_COMPILER) $(CFLAGS) $(NG_CFLAGS) $(NG_USER_CFLAGS) $(NG_CPPFLAGS) -c _stub_mmul2.c

_stub_mmul2: _stub_mmul2.o sample.o
	$(NG_LINKER) $(NG_CFLAGS) $(CFLAGS) -o _stub_mmul2 _stub_mmul2.o $(LDFLAGS) $(NG_STUB_LDFLAGS) sample.o $(LIBS) -lm

_stub_mmul2.inf: _stub_mmul2
	 ./_stub_mmul2 -i _stub_mmul2.inf



_stub_FFT.o: _stub_FFT.c
	$(NG_COMPILER) $(CFLAGS) $(NG_CFLAGS) $(NG_USER_CFLAGS) $(NG_CPPFLAGS) -c _stub_FFT.c

_stub_FFT: _stub_FFT.o sample.o
	$(NG_LINKER) $(NG_CFLAGS) $(CFLAGS) -o _stub_FFT _stub_FFT.o $(LDFLAGS) $(NG_STUB_LDFLAGS) sample.o $(LIBS) -lm

_stub_FFT.inf: _stub_FFT
	 ./_stub_FFT -i _stub_FFT.inf




$(NRF_DUMMY): $(NG_INF_FILES)
	$(NG_DIR)/bin/ng_gen_nrf $(NG_STUB_PROGRAM)

clean:
	rm -f _stub_sin.o _stub_sin.c
	rm -f _stub_mmul.o _stub_mmul.c
	rm -f _stub_mmul2.o _stub_mmul2.c
	rm -f _stub_FFT.o _stub_FFT.c

veryclean: clean
	rm -f $(NG_STUB_PROGRAM) $(NG_INF_FILES) *.nrf
	rm -f sample.o
	rm -f sample.o
	rm -f sample.o
	rm -f _stub_*-info


# END OF Makefile
