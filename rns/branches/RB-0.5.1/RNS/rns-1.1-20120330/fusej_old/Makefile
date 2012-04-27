include build.conf
include ${FUSE_J_HOME}/build.conf

JAVAC := ${JDK_HOME}/bin/javac
JAR := ${JDK_HOME}/bin/jar

BUILD_CLASSPATH := ${CLASSPATH}:${FUSE_J_HOME}/build
JAVASRCS := $(shell find org -name '*.java')
JAVACLASSES = $(JAVASRCS:.java=.class)


CC = gcc
CFLAGS = -g -Wall -O2
LDFLAGS =
SRCS = rnsfs-setxml.c rnsfs-getxml.c
PROGS = $(SRCS:.c=)


.SUFFIXES:
.SUFFIXES: .o .c .java .class

all: jar $(PROGS)

jar: $(JAVACLASSES)
	${JAR} cvf rnsfs.jar -C build org

.java.class:
	mkdir -p build
	${JAVAC} -Xlint -classpath ${BUILD_CLASSPATH} -d build $<

$(PROGS): Makefile

.c:
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -o $@ $(LDLIBS)

clean:
	rm -rf build rnsfs.jar $(PROGS) *~
