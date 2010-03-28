# This file 'sample_object.mak' was created by ng_gen. Don't edit

CC = cc
include $(NG_DIR)/lib/template.mk

# CompileOptions:
NG_USER_CFLAGS = 

#  Define NG_COMPILER & NG_LINKER as $(CC) if it is not defined.

NG_COMPILER = $(CC)
NG_LINKER = $(CC)

# stub sources

NG_STUB_SRC = _stub_sample_object.c 

# stub programs

NG_STUB_PROGRAM = _stub_sample_object

# stub inf files
NG_INF_FILES = _stub_sample_object.inf

# LDAP dif file
LDAP_DIF = root.ldif

all: $(NG_STUB_PROGRAM) $(NG_INF_FILES) $(LDAP_DIF)

_stub_sample_object.o: _stub_sample_object.c
	$(NG_COMPILER) $(NG_CPPFLAGS) $(CFLAGS) $(NG_CFLAGS) $(NG_USER_CFLAGS) -c _stub_sample_object.c
_stub_sample_object: _stub_sample_object.o sample.o
	$(NG_LINKER) $(CFLAGS) -o _stub_sample_object _stub_sample_object.o $(LDFLAGS) $(NG_STUB_LDFLAGS) sample.o $(LIBS) -lm
_stub_sample_object.inf: _stub_sample_object
	 ./_stub_sample_object -i _stub_sample_object.inf



$(LDAP_DIF): $(NG_INF_FILES)
	$(NG_DIR)/bin/ng_gen_dif $(NG_STUB_PROGRAM)

install: $(LDAP_DIF)
	$(INSTALL) *.ldif $(LDIF_INSTALL_DIR)

clean:
	rm -f _stub_sample_object.o _stub_sample_object.c

veryclean: clean
	rm -f $(NG_STUB_PROGRAM) $(NG_INF_FILES) $(LDAP_DIF) *.ngdef
	rm -f sample.o


# END OF Makefile
