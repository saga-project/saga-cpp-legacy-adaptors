#!/bin/sh

./wsdl/wsdl/generate_h.sh

# we need to keep the Makefiles!
rm -f gridsam/*.* bes/*.* besfactory/*.*

soapcpp2 -C -i -d ./gridsam    -p gridsam    -I./gsoap-2.7.11/import ./wsdl/wsdl/gridsam.h
soapcpp2 -C -i -d ./besfactory -p besfactory -I./gsoap-2.7.11/import ./wsdl/wsdl/bes-factory.h
soapcpp2 -C -i -d ./bes        -p bes        -I./gsoap-2.7.11/import ./wsdl/wsdl/basic-execution-service.h

rm -f gridsam/*.xml bes/*.xml besfactory/*.xml

