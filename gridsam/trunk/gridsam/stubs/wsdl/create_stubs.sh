#!/bin/sh

# generate JSDL stubs
xsd cxx-tree --generate-inline --generate-serialization --generate-forward --output-dir .. --suppress-parsing  --hxx-suffix .hpp --fwd-suffix -fwd.hpp --ixx-suffix .ipp --cxx-suffix .cpp --show-anonymous --root-element JobDefinition jsdl.xsd
#xsd cxx-tree --generate-inline --generate-serialization --generate-forward --output-dir .. --suppress-parsing  --hxx-suffix .hpp --fwd-suffix -fwd.hpp --ixx-suffix .ipp --cxx-suffix .cpp --show-anonymous --root-element ... jsdl-mpi.xsd
#xsd cxx-tree --generate-inline --generate-serialization --generate-forward --output-dir .. --suppress-parsing  --hxx-suffix .hpp --fwd-suffix -fwd.hpp --ixx-suffix .ipp --cxx-suffix .cpp --show-anonymous --root-element ... jsdl-posix.xsd
#xsd cxx-tree --generate-inline --generate-serialization --generate-forward --output-dir .. --suppress-parsing  --hxx-suffix .hpp --fwd-suffix -fwd.hpp --ixx-suffix .ipp --cxx-suffix .cpp --show-anonymous --root-element ... jsdl-myproxy.xsd

