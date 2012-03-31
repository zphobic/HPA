#------------------------------------------------------------------------------
# $Id: Makefile,v 1.12 2002/11/08 18:56:25 emarkus Exp $
# $Source: /usr/cvsroot/project_pathfind/Makefile,v $
#------------------------------------------------------------------------------

CXX ?= g++
#CXX = g++3
#CXX = /usr/scratch/emarkus/software/gcc-3.1/bin/g++

CXXFLAGS = -Wall -g -O3 -DNDEBUG

#CXXFLAGS_G = -Wall -gstabs+
CXXFLAGS_G = -Wall -g

PATHFIND = pathfind

PATHFIND_G = pathfind_g

LIBPATHFIND = lib$(PATHFIND).a

LIBPATHFIND_G = lib$(PATHFIND_G).a

LIBPATHFIND_SRC = \
  astar.cpp \
  environment.cpp \
  error.cpp \
  idastar.cpp \
  search.cpp \
  searchutils.cpp \
  statistics.cpp \
  tiling.cpp \
  util.cpp \

LIBPATHFIND_OBJ = $(LIBPATHFIND_SRC:.cpp=.o)

LIBPATHFIND_OBJ_G = $(LIBPATHFIND_SRC:.cpp=_g.o)

EXAMPLE = pf

EXAMPLE_G = pf_g

EXAMPLE_SRC = \
  main.cpp \
  cluster.cpp \
  abstiling.cpp \
  absnode.cpp \
  abswizard.cpp \
  localentrance.cpp \
  smoothwizard.cpp \
  experiment.cpp \
  htiling.cpp


EXAMPLE_OBJ = $(EXAMPLE_SRC:.cpp=.o)

EXAMPLE_OBJ_G = $(EXAMPLE_SRC:.cpp=_g.o)

all: $(LIBPATHFIND) $(EXAMPLE) $(LIBPATHFIND_G) $(EXAMPLE_G)
gall: $(LIBPATHFIND_G) $(EXAMPLE_G)
rall: $(LIBPATHFIND) $(EXAMPLE)

$(LIBPATHFIND): $(LIBPATHFIND_OBJ)
	ar cr $@ $(LIBPATHFIND_OBJ)

$(LIBPATHFIND_G): $(LIBPATHFIND_OBJ_G)
	ar cr $@ $(LIBPATHFIND_OBJ_G)

$(EXAMPLE): $(EXAMPLE_OBJ) $(LIBPATHFIND)
	$(CXX) -o $@ $(EXAMPLE_OBJ) -L. -l$(PATHFIND)

$(EXAMPLE_G): $(EXAMPLE_OBJ_G) $(LIBPATHFIND_G)
	$(CXX) -o $@ $(EXAMPLE_OBJ_G) -L. -l$(PATHFIND_G)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

%_g.o: %.cpp
	$(CXX) -c $(CXXFLAGS_G) -o $@ $<

# Rules for dependency file generation
%.d: %.cpp
	set -e; $(CC) -MM $(CXXFLAGS) $< | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; [ -s $@ ] || rm -f $@

%_g.d: %.cpp
	set -e; $(CC) -MM $(CXXFLAGS_G) $< | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; [ -s $@ ] || rm -f $@

clean:
	-rm *.o *.d $(LIBPATHFIND) $(EXAMPLE) $(LIBPATHFIND_G) $(EXAMPLE_G)

gclean:
	-rm *_g.o *_g.d $(LIBPATHFIND_G) $(EXAMPLE_G)

rclean:
	-rm *.o *.d $(LIBPATHFIND) $(EXAMPLE)

.PHONY: clean

.SUFFIXES: .cpp

# Include dependency files
include $(LIBPATHFIND_SRC:.cpp=.d)
include $(EXAMPLE_SRC:.cpp=.d)
include $(LIBPATHFIND_SRC:.cpp=_g.d)
include $(EXAMPLE_SRC:.cpp=_g.d)
