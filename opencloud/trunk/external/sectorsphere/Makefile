
-include ../../config/make.cfg

SAGA_SUBDIRS  += auth sector sphere

SAGA_LIB       = saga_sectsp
SAGA_HDR       = $(wildcard *.hpp)
SAGA_SRC       = $(wildcard *.cpp)
SAGA_LIB_OBJ   = $(SAGA_SRC:%.cpp=%.o)

SAGA_LIB_OBJ  += $(wildcard auth/*.o)
SAGA_LIB_OBJ  += $(wildcard sector/*.o)
SAGA_LIB_OBJ  += $(wildcard sphere/*.o)

include $(SAGA_MAKE_INCLUDE_ROOT)/saga.mk

SAGA_CXXFLAGS += $(CODEBLUE2_CXXFLAGS)
SAGA_LDFLAGS  += $(CODEBLUE2_LDFLAGS)

# SAGA_CXXFLAGS += -I./ 
#                  -I../../codeblue2/client/ 
#                  -I../../codeblue2/udt
#                  -I../../codeblue2/gmp 
#                  -I../../codeblue2/common 
#                  -I../../codeblue2/security

# SAGA_LDFLAGS  += -L../../codeblue2/client/ 
#                  -L../../codeblue2/lib/ 
#                  -lsecurity
#                  -lrpc 
#                  -ludt 
#                  -lcommon 
#                  -lclient

