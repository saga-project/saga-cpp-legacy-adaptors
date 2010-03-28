#!/bin/sh

wsdl2h -n bes -N bes -q bes basic-execution-service.wsdl
wsdl2h -n besfactory -N besfactory -q besfactory bes-factory.wsdl
wsdl2h -n gridsam -N gridsam gridsam.wsdl

