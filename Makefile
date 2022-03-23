# replace it to your HDF5 install directory here
HDF5_DIR?=/home/wuty/ncinstall

CXX=mpic++
CXXFLAGS=-Wall -O2 `pkg-config fuse3 --cflags --libs` -I$(HDF5_DIR)/include -L$(HDF5_DIR)/lib
LDFLAGS=-lhdf5

all:h5filewrapper.o h5fuse.o h5fuse

h5filewrapper.o: h5filewrapper.h h5filewrapper.cpp
	$(CXX) $(CXXFLAGS) -c h5filewrapper.cpp

h5fuse.o: h5fuse.cpp
	$(CXX) $(CXXFLAGS) -c h5fuse.cpp

h5fuse: h5filewrapper.o
	$(CXX) $(CXXFLAGS) h5filewrapper.o h5fuse.o -o h5fuse $(LDFLAGS)

.PHONY: clean
clean:
	rm -f ./*.o ./h5fuse