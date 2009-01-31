bin_PROGRAMS = vdpauinfo

vdpauinfo_CXXFLAGS = $(VDPAUINFO_CXXFLAGS)
vdpauinfo_LDADD = $(VDPAUINFO_LIBS)
vdpauinfo_SOURCES = vdpauinfo.cpp
EXTRA_DIST = VDPDeviceImpl.h


EXTRA_DIST += ChangeLog
MAINTAINERCLEANFILES = ChangeLog

.PHONY: ChangeLog

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog
