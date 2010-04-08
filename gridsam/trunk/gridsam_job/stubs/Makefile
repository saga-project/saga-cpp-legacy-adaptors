#  Copyright (c) 2005-2007 Andre Merzky (andre@merzky.net)
# 
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

SAGA_SUBDIRS      = bes gridsam besfactory gsoap-2.7.11

STUBS            += gridsam/JobSubmissionSOAPBinding.nsmap
STUBS            += gridsam/gridsamJobControlSOAPBindingProxy.cpp
STUBS            += gridsam/gridsamJobSubmissionSOAPBindingProxy.h
STUBS            += gridsam/gridsamH.h
STUBS            += gridsam/JobControlSOAPBinding.nsmap
STUBS            += gridsam/gridsamJobControlSOAPBindingProxy.h
STUBS            += gridsam/gridsamJobMonitoringSOAPBindingProxy.cpp
STUBS            += gridsam/gridsamC.cpp
STUBS            += gridsam/gridsamStub.h
STUBS            += gridsam/JobMonitoringSOAPBinding.nsmap
STUBS            += gridsam/gridsamJobMonitoringSOAPBindingProxy.h
STUBS            += gridsam/gridsamJobSubmissionSOAPBindingProxy.cpp
STUBS            += bes/BESSOAPBinding.nsmap
STUBS            += bes/besBESSOAPBindingProxy.cpp
STUBS            += bes/besH.h
STUBS            += bes/besBESSOAPBindingProxy.h
STUBS            += bes/besC.cpp
STUBS            += bes/besStub.h

# FIXME: this should be moved into SAGA_ROOT/config!
SOAP_CXX          = soapcpp2
SOAP_CXXFLAGS     = -C -i -I./gsoap-2.7.11/import

-include ../../config/make.cfg
-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.mk

SAGA_CPPDEFS      += -DPTHREADS -DWITH_NONAMESPACES -DWITH_DOM -DWITH_IPV6

ifeq "$(SAGA_HAVE_OPENSSL)" "yes"
  SAGA_CPPDEFS    += -DWITH_OPENSSL -DWITH_COOKIES
endif

# make sure stubs are generated before dependency check!
subdirs_before: $(STUBS)

$(STUBS):
	@$(ECHO) "     running       generate.sh"
	@$(RM)       gridsam/*.* bes/*.* besfactory/*.*

	@$(ECHO) "     generating    stubs"
	@$(SOAP_CXX) $(SOAP_CXXFLAGS) -d ./gridsam    -p gridsam    ./wsdl/wsdl/gridsam.h
	@$(SOAP_CXX) $(SOAP_CXXFLAGS) -d ./bes        -p bes        ./wsdl/wsdl/basic-execution-service.h
	@$(SOAP_CXX) $(SOAP_CXXFLAGS) -d ./besfactory -p besfactory ./wsdl/wsdl/bes-factory.h

	@$(RM)       gridsam/*.xml bes/*.xml besfactory/*.xml

