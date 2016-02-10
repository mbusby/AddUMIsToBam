# ==================================
# define our source and object files
# ==================================

OBJ_DIR= ./obj

SOURCES= Main.cpp Handy.cpp
OBJECTS= Main.o Handy.o
BUILT_OBJECTS= $(patsubst %,$(OBJ_DIR)/%,$(OBJECTS))

# ================
# compiler options
# ================

CXX= g++
CXXFLAGS= -Wall -O2 
PROG=  AddUMIsToBam
LIBS= -L/cil/shed/apps/internal/RNA_utilities/bamtools/lib -lbamtools -lz
LDFLAGS = -Wl,-rpath /cil/shed/apps/internal/RNA_utilities/bamtools/lib
INCLUDES = -I/cil/shed/apps/internal/RNA_utilities/bamtools/include

# ================
# build targets
# ================

.PHONY: all
all: $(OBJ_DIR) $(PROG)

$(BUILT_OBJECTS): $(SOURCES)
	@$(CXX) -c -o $@ $(*F).cpp $(LDFLAGS) $(CXXFLAGS) $(INCLUDES)
   
AddUMIsToBam: $(BUILT_OBJECTS)
	@$(CXX) $(LDFLAGS) $(CXXFLAGS) -o AddUMIsToBam $(BUILT_OBJECTS) $(LIBS)

$(OBJ_DIR):
	@mkdir -p $@

.PHONY: clean
clean:
	@rm -f $(OBJ_DIR)/* $(PROG)