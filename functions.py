#!/usr/bin/python
# Build VDPDeviceImpl.h
# (C) Wladimir J. van der Laan 2008
from sys import stderr

def process_file(fname):
    """Process include file"""
    f = open(fname, "r")
    while True:
        line = f.readline()
        if not line:
            break
        line = line.rstrip()
        if line.startswith("#define VDP_FUNC_ID_"):
            begin = line.find("VDP_")
            end = line.find(" ", begin)
            funcname = line[begin:end]
            functions.append(funcname)
        if line.startswith("typedef") and line.endswith("("):
            begin = line.rfind("Vdp")
            end = line.find("(")
            typename = line[begin:end]
            types[typename[3:].lower()] = typename
    f.close()

def lower_case(s):
    "BLA_BLA to blabla"
    parts = s.split("_")
    parts = [x.lower() for x in parts]
    return "".join(parts)

functions = []
types = {}

process_file("/usr/include/vdpau/vdpau.h")
process_file("/usr/include/vdpau/vdpau_x11.h")

# Prototype
proto = []
for f in functions:
    if f == "VDP_FUNC_ID_BASE_WINSYS": # skip this one
        continue
    base = f[12:]
    # to camelcase
    base = lower_case(base)
    try:
        proto.append((f, types[base], types[base][3:]))
    except KeyError:
        stderr.write("No prototype found for %s (%s)\n" %(f, base))

# We now have an array of prototypes and VDP_FUNC_ID...
# Build our impl class
print "#ifndef H_VDPDeviceImpl"
print "#define H_VDPDeviceImpl"
print "#include <assert.h>"
print "struct VDPDeviceImpl {"
print "    VDPDeviceImpl(VdpDevice device, VdpGetProcAddress *get_proc_address);"
print "    VdpDevice device;"
print
for (id, type, name) in proto:
    print "    %s *%s;" % (type, name)
print "};";

print "#define GETADDR(device, function_id, function_pointer) \\"
print "    assert(get_proc_address(device, function_id, function_pointer) == VDP_STATUS_OK)"

print "VDPDeviceImpl::VDPDeviceImpl(VdpDevice device, VdpGetProcAddress *get_proc_address):"
print "    device(device)"
print "{"
# look'em up
for (id, type, name) in proto:
    print "    GETADDR(device, %s, (void**)&%s);" % (id, name)
print "}"

print "#undef GETADDR"

print "#endif"
#print proto
