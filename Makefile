CWD= $(shell pwd)

ARCH= $(shell uname -m)
ifeq ($(ARCH),x86_64)
MEXEXT= mexa64
else
MEXEXT= mexglx
endif

SRCDIR= ./src/
INCDIR= ./inc/
LIBDIR= ./lib/
BINDIR= ./bin/
DRIVERDIR= ./driver/

CC= gcc
CXX= g++
MEX= mex
AR= ar

CFLAGS= -O3
CXXFLAGS= -O3 
MEXFLAGS= -O 
INCFLAGS= -I$(INCDIR)
LIBFLAGS= -L$(LIBDIR) -ls626 -lpthread

$(BINDIR)%.o: $(SRCDIR)%.c
	$(CC) $(CFLAGS) $(INCFLAGS) -o $@ -c $<
$(BINDIR)%.o: $(SRCDIR)%.cc
	$(CXX) $(CXXFLAGS) $(INCFLAGS) -o $@ -c $<
$(BINDIR)%.o: $(SRCDIR)%.cpp
	$(CXX) $(CXXFLAGS) $(INCFLAGS) -o $@ -c $<

all: s626lib thread time

# test function for quadrature encoders
test: $(BINDIR)haptics_util.o \
				$(BINDIR)s626_util.o \
				$(BINDIR)s626_test.o
	$(CXX) -o $(BINDIR)s626_test $^ $(LIBFLAGS) 


# mex function for accessing encoder data
thread: $(SRCDIR)s626.cc \
					$(SRCDIR)s626Thread.cc \
					$(SRCDIR)s626Functions.cc \
					$(SRCDIR)haptics_util.cc \
					$(SRCDIR)s626_util.cc \
					$(SRCDIR)timeScalar.cc 
	$(MEX) $(MEXFLAGS) $(INCFLAGS) $(LIBFLAGS) $^
	mkdir -p ./mex
	mv *.$(MEXEXT) mex/

time: $(SRCDIR)time.cc \
				$(SRCDIR)timeScalar.cc
	$(MEX) $(MEXFLAGS) $(INCFLAGS) $(LIBFLAGS) $^
	mkdir -p ./mex
	mv *.$(MEXEXT) mex/



# make the library for the s626
s626lib:
	cd ./driver && make lib && cd $(CWD)	
	mkdir -p ./lib
	cp ./driver/*.a ./lib/

# make the demo that came with the s626 driver
s626demo: s626lib
	cd ./driver && make lib && make demo && cd $(CWD)	
	mkdir -p ./bin
	cp ./driver/s626demo ./driver/s626dm2b ./bin/


clean:
	rm -vf ./lib/* ./mex/*.$(MEXEXT)
	cd ./driver && make clnall && cd $(CWD)

