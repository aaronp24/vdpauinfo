CXXFLAGS=-O3 -g
LDFLAGS=-lvdpau -lX11

PROGRAMS=vdpauinfo

all: $(PROGRAMS)

vdpauinfo: vdpauinfo.o
	$(CXX)   -o $@ $^  $(LDFLAGS)

clean:
	rm -f *.o $(PROGRAMS) *~
