call .\wdsl\wsdl\generate_h.cmd

del /q gridsam\*.cpp gridsam\*.h gridsam\*.nsmap bes\*.cpp bes\*.h bes\*.nsmap  besfactory\*.cpp besfactory\*.h besfactory\*.nsmap
soapcpp2.exe -C -i -d ./gridsam -p gridsam -Igsoap-2.7.11/import ./wsdl/wsdl/gridsam.h
soapcpp2.exe -C -i -d ./bes -p bes -Igsoap-2.7.11/import ./wsdl/wsdl/basic-execution-service.h
soapcpp2.exe -C -i -d ./besfactory -p besfactory -Igsoap-2.7.11/import ./wsdl/wsdl/bes-factory.h
del /q gridsam\*.xml bes\*.xml besfactory\*.xml
