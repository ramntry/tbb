PROG_NAME = partialsum

GTEST_HOME = /home/ramntry/gtest/gtest-1.6.0
BOOST_HOME = /home/ramntry/boost/boost_1_52_0

TBB_INIT_COMMAND = /home/ramntry/projects/tbb/tbb/bin/tbbvars.sh intel64

COMPILER = g++-4.7 -std=c++11
OPTS = -Wall -O2 -pthread

INCLUDE_PATHS = \
	-I ${GTEST_HOME}/include \
	-I ${BOOST_HOME}

LIBRARIES = \
	${GTEST_HOME}/build/libgtest.a \
	-ltbb \
	-lrt

SOURCES = \
	${PROG_NAME}.cpp

HEADERS =

${PROG_NAME}: ${SOURCES} ${HEADERS}
	${TBB_INIT_COMMAND}
	${COMPILER} ${OPTS} ${INCLUDE_PATHS} ${SOURCES} -o $@ ${LIBRARIES}
