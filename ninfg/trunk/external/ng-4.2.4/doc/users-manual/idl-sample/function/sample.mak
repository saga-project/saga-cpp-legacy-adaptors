# This file 'sample.mak' was created by ng_gen. Don't edit

CC = cc
include $(NG_DIR)/lib/template.mk

# CompileOptions:
NG_USER_CFLAGS = 

#  Define NG_COMPILER & NG_LINKER as $(CC) if it is not defined.

NG_COMPILER = $(CC)
NG_LINKER = $(CC)

# stub sources

NG_STUB_SRC = _stub_sin.c  _stub_mmul.c  _stub_mmul2.c  _stub_FFT.c 

# stub programs

NG_STUB_PROGRAM = _stub_sin _stub_mmul _stub_mmul2 _stub_FFT

# stub inf files
NG_INF_FILES = _stub_sin.inf _stub_mmul.inf _stub_mmul2.inf _stub_FFT.inf

# LDAP dif file
LDAP_DIF = root.ldif

all: $(NG_STUB_PROGRAM) $(NG_INF_FILES) $(LDAP_DIF)

_stub_sin.o: _stub_sin.c
	$(NG_COMPILER) $(NG_CPPFLAGS) $(CFLAGS) $(NG_CFLAGS) $(NG_USER_CFLAGS) -c _stub_sin.c
_stub_sin: _stub_sin.o 
	$(NG_LINKER) $(CFLAGS) -o _stub_sin _stub_sin.o $(LDFLAGS) $(NG_STUB_LDFLAGS)  $(LIBS) -lm
_stub_sin.inf: _stub_sin
	 ./_stub_sin -i _stub_sin.inf


_stub_mmul.o: _stub_mmul.c
	$(NG_COMPILER) $(NG_CPPFLAGS) $(CFLAGS) $(NG_CFLAGS) $(NG_USER_CFLAGS) -c _stub_mmul.c
_stub_mmul: _stub_mmul.o sample.o
	$(NG_LINKER) $(CFLAGS) -o _stub_mmul _stub_mmul.o $(LDFLAGS) $(NG_STUB_LDFLAGS) sample.o $(LIBS) -lm
_stub_mmul.inf: _stub_mmul
	 ./_stub_mmul -i _stub_mmul.inf


_stub_mmul2.o: _stub_mmul2.c
	$(NG_COMPILER) $(NG_CPPFLAGS) $(CFLAGS) $(NG_CFLAGS) $(NG_USER_CFLAGS) -c _stub_mmul2.c
_stub_mmul2: _stub_mmul2.o sample.o
	$(NG_LINKER) $(CFLAGS) -o _stub_mmul2 _stub_mmul2.o $(LDFLAGS) $(NG_STUB_LDFLAGS) sample.o $(LIBS) -lm
_stub_mmul2.inf: _stub_mmul2
	 ./_stub_mmul2 -i _stub_mmul2.inf


_stub_FFT.o: _stub_FFT.c
	$(NG_COMPILER) $(NG_CPPFLAGS) $(CFLAGS) $(NG_CFLAGS) $(NG_USER_CFLAGS) -c _stub_FFT.c
_stub_FFT: _stub_FFT.o sample.o
	$(NG_LINKER) $(CFLAGS) -o _stub_FFT _stub_FFT.o $(LDFLAGS) $(NG_STUB_LDFLAGS) sample.o $(LIBS) -lm
_stub_FFT.inf: _stub_FFT
	 ./_stub_FFT -i _stub_FFT.inf



$(LDAP_DIF): $(NG_INF_FILES)
	$(NG_DIR)/bin/ng_gen_dif $(NG_STUB_PROGRAM)

install: $(LDAP_DIF)
	$(INSTALL) *.ldif $(LDIF_INSTALL_DIR)

clean:
	rm -f _stub_sin.o _stub_sin.c
	rm -f _stub_mmul.o _stub_mmul.c
	rm -f _stub_mmul2.o _stub_mmul2.c
	rm -f _stub_FFT.o _stub_FFT.c

veryclean: clean
	rm -f $(NG_STUB_PROGRAM) $(NG_INF_FILES) $(LDAP_DIF) *.ngdef
	rm -f sample.o
	rm -f sample.o
	rm -f sample.o


# END OF Makefile
