# Copyright (C) 2008-2012 High Energy Accelerator Research Organization (KEK)
# Copyright (C) 2008-2012 National Institute of Informatics in Japan.
# All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

JFLAGS =
JC = javac
SRA_CLASSDIR=$(SAGA_LOCATION)/share/sra/classes
.SUFFIXES: .java .class
.java.class:
	$(JC) $(JFLAGS) $*.java

#CLASSES = \
#	RNS_list.java \
#	RNS_mkdir.java \
#	RNS_rmdir.java 

CLASSES = $(wildcard *.java)

default: classes

classes: $(CLASSES:.java=.class)


install : copy_files

copy_files :
	if [ ! -d $(SRA_CLASSDIR) ]; then \
		mkdir -p $(SRA_CLASSDIR); \
	fi
	cp *.class $(SRA_CLASSDIR)


clean:
	$(RM) *.class
