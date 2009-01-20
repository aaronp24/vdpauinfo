CXXFLAGS=-O3 -g
LDFLAGS=-lvdpau -lX11

PROGRAMS=vdpinfo

all: $(PROGRAMS)

vdpinfo: vdpinfo.o
	$(CXX)   -o $@ $^  $(LDFLAGS)

clean:
	rm -f *.o $(PROGRAMS) *~
