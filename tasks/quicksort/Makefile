PROG_NAME = quicksort

GTEST_HOME = /home/ramntry/libraries/gtest/gtest-1.6.0
BOOST_HOME = /home/ramntry/libraries/boost/boost_1_53_0

TBB_INIT_COMMAND = /home/ramntry/libraries/tbb/tbb41_20130314oss/bin/tbbvars.sh intel64

COMPILER = g++ -std=c++11
OPTS = -Wall -O2 -pthread

INCLUDE_PATHS = \
	-I ${GTEST_HOME}/include \
	-I ${BOOST_HOME}

LIBRARIES = \
	${GTEST_HOME}/lib/.libs/libgtest.a \
	-ltbb \
	-lrt

SOURCES = \
	${PROG_NAME}.cpp

HEADERS = \
	SortTest.h

${PROG_NAME}: ${SOURCES} ${HEADERS}
	${TBB_INIT_COMMAND}
	${COMPILER} ${OPTS} ${INCLUDE_PATHS} ${SOURCES} -o $@ ${LIBRARIES}

