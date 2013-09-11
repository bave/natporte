
CXXFLAGS = -pipe -O3 -lpthread -ltcmalloc -Wall -Wno-deprecated-declarations -Wno-unused-function -DWAIT_TIME_DELETE -DDIFF_CKSUM -DNATIVE_LOCK

DEBUG_CXXFLAGS = -g -pipe -O0 -lpthread -ltcmalloc -Wall -Wno-deprecated-declarations -Wno-unused-function -DWAIT_TIME_DELETE -DDIFF_CKSUM -DNATIVE_LOCK -DDEBUG

# use for hostid option
#CXXFLAGS = -pipe -O3 -lpthread -ltcmalloc -Wall -Wno-deprecated-declarations -Wno-unused-function -DWAIT_TIME_DELETE -DOPT_HOSTID -DDIFF_CKSUM -DNATIVE_LOCK


# define options
#-DDEBUG
#-DWAIT_TIME_DELETE
#-DOPT_HOSTID
#-DIP_CHECKSUM
#-DDIFF_CKSUM
#-DNATIVE_LOCK

CXX = g++
#CXX = c++

DEP = divert.h interface.hpp main.cpp utils.hpp addrpool.h hostid.h structure.h alias.h log.hpp ring.h
PROGNAME = natporte

.PHONY: all clean

all: ${PROGNAME}

clean:
	rm -f *~ *.a *.o ../bin/$(PROGNAME) ../bin/$(PROGNAME)_debug ../bin/$(PROGNAME).core
	rm -rf *.dSYM ../bin/node_module

${PROGNAME}: ${DEP}
	${CXX} ${CXXFLAGS} -o ../bin/${PROGNAME} main.cpp

debug: ${DEP}
	${CXX} ${DEBUG_CXXFLAGS} -o ../bin/${PROGNAME}_debug main.cpp


.DEFAULT: all

