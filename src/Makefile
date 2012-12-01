
# release
CXXFLAGS = -pipe -O0 -lpthread -ltcmalloc -Wall -Wno-unused-function -DWAIT_TIME_DELETE -DDIFF_CKSUM -DNATIVE_LOCK

#CXXFLAGS = -pipe -O0 -lpthread -ltcmalloc -Wall -Wno-unused-function -DWAIT_TIME_DELETE -DOPT_HOSTID -DDIFF_CKSUM -DNATIVE_LOCK

# debug
#CXXFLAGS = -pipe -O0 -lpthread -ltcmalloc -Wall -Wno-unused-function -g -DDEBUG -DWAIT_TIME_DELETE -DOPT_HOSTID -DDIFF_CKSUM

# define options
#-DDEBUG
#-DWAIT_TIME_DELETE
#-DOPT_HOSTID
#-DIP_CHECKSUM
#-DDIFF_CKSUM
#-DNATIVE_LOCK

#CXX = g++
CXX = c++

PROGNAME = natporte

.PHONY: all clean

all: ${PROGNAME} netutils_js

clean:
		rm -f *~ *.a *.o
		rm -rf *.dSYM
		rm -f ./a.out ./utils ./log ./interface ./ring ./structure
		rm -rf ./build
		rm -rf ../bin/node_module
		rm -rf ../bin/$(PROGNAME) 
		rm -rf ../bin/$(PROGNAME).core
		node-waf distclean

netutils_js: netutils_js.cpp interface.hpp
		node-waf configure
		node-waf build

${PROGNAME}: main.cpp
		${CXX} ${CXXFLAGS} -o ../bin/$@ main.cpp


.DEFAULT: all

