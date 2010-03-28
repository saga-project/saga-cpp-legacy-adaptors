# $RCSfile: Makefile,v $ $Revision: 1.3 $ $Date: 2006/11/08 08:35:45 $
# $AIST_Release: 4.2.4 $
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

JAVAC = ${JAVA_HOME}/bin/javac
JAVA  = ${JAVA_HOME}/bin/java
JAR   = ${JAVA_HOME}/bin/jar

INVOKE_SERVER = ng_invoke_server.Condor
CONDOR_IS_JAR = condorIS.jar

INVOKE_SERVER_JAR_INSTALLED = ${NG_DIR}/lib/ng_invoke_server.jar
INVOKE_SERVER_JAR_BUILD     = ../javacommon/ng_invoke_server.jar
CLASSPATH=.:${INVOKE_SERVER_JAR_BUILD}:./lib/classad.jar:./lib/condorAPI.jar

all: jar

is: ${INVOKE_SERVER_JAR_BUILD}
	${JAVAC} -classpath ${CLASSPATH} org/apgrid/grpc/tools/invokeServer/condor/*.java

jar: is
	${JAR} cf ${CONDOR_IS_JAR} org

install:${INVOKE_SERVER_JAR_INSTALLED} jar
	test -d ${NG_DIR}
	cp ${INVOKE_SERVER} ${NG_DIR}/bin
	cp ${CONDOR_IS_JAR} ${NG_DIR}/lib
	cp ./lib/classad.jar ./lib/condorAPI.jar ${NG_DIR}/lib

clean:
	rm -f org/apgrid/grpc/tools/invokeServer/condor/*.class
	rm -f ${CONDOR_IS_JAR}

${INVOKE_SERVER_JAR_BUILD}:
	cd ../javacommon && ant jar

${INVOKE_SERVER_JAR_INSTALLED}:
	test -n "${NG_DIR}"
	cd ../javacommon && ant install
