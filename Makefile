CXXFLAGS=-O3 -ffast-math -g -march=opteron 
LDFLAGS=-lvdpau -lX11

PROGRAMS=vdpinfo

all: $(PROGRAMS)

vdpinfo: vdpinfo.o
	$(CXX)   -o $@ $^  $(LDFLAGS)

clean:
	rm -f *.o $(PROGRAMS)
