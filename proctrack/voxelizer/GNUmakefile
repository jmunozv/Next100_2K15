# specific names for this package

GATEMAKEDIR = $(GATE_DIR)/Makefile
GATELIBDIR = $(GATE_DIR)/lib

DICT  = voxelizerCint
SHLIB = libvoxelizer.so
SOURCES = $(wildcard *.cc)
#HEADERS = $(wildcard *.h)
HEADERS = $(filter-out voxelizerCint.h,$(wildcard *.h))
#OBJECTS = $(SOURCES:.cc=.o)
OBJECTS = $(filter-out Runvoxelizer.o, $(SOURCES:.cc=.o))


# include options for this package
INCFLAGS = -I.
INCFLAGS += -I$(GATE_DIR)
INCFLAGS += -I$(PAOLINA_BASEDIR)/include/

# platform-specific options
OSNAME          = $(shell uname -s)
HOST            = $(shell uname -n)
OSNAMEMODE      = $(OSNAME)

include $(GATEMAKEDIR)/Makefile.${OSNAME}

# set compiler options for ROOT
CXXFLAGS += $(shell root-config --cflags)
CXXFLAGS += '-fPIC'


# call the common GNUmakefile
include $(GATEMAKEDIR)/GNUmakefile.GATE

#all: bin

### binary compilation ###

LIBS += $(shell root-config --libs)  -lCore -lRIO -lHist
LIBS += -L$(GATE_DIR)/lib/  -lGATE -lGATEIO -lGATEUtils -lGateModule
LIBS += -L$(PAOLINA_BASEDIR)/lib/  -lpaolina

bin: lib/$(SHLIB) Runvoxelizer.o
	@echo "<< compiling Runvoxelizer >>"
	@$(CXX) -g $(CXXFLAGS) -o ./bin/Runvoxelizer $^ $(LIBS) 

.PHONY: clean cleanapps

clean:  cleanapps

cleanapps:
	@rm -f ./bin/Runvoxelizer

