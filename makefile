.PHONY: all clean

all: src/horst.so src/horst_internal.so

HORST_HEADERS = $(wildcard src/include/horst/*.h) $(wildcard src/include/horst/lart/*.h) $(wildcard src/include/horst/ladspa/*.h)
HORST_SOURCES = ${wildcard src/*.cc}
HORST_OBJECTS = ${HORST_SOURCES:.cc=.o}

# OPTIMIZATION_FLAGS = -O0 -g
OPTIMIZATION_FLAGS = -O3

PYTHON_CXXFLAGS = `python3-config --cflags`  
PYTHON_LDFLAGS = `python3-config --ldflags --embed` -lboost_python310 

CLI_CXXFLAGS = 
CLI_LDFLAGS = -lboost_program_options -lreadline 

CXXFLAGS += $(OPTIMIZATION_FLAGS) -fPIC -std=c++11 -Isrc/include -march=native -mcpu=native -Wall -pedantic `pkg-config lilv-0 --cflags`
LDFLAGS += `pkg-config lilv-0 --libs` -ljack

# src/horst: $(HORST_SOURCES) $(HORS_HEADERS)
# We need this static rule to add the dependency on all headers
# $(HORST_OBJECTS): %.o: %.cc $(HORST_HEADERS)

src/horst_cli: src/horst_cli.cc $(HORST_HEADERS)
	g++ -o $@ $(CXXFLAGS) $(CLI_CXXFLAGS) $< $(LDFLAGS) $(CLI_LDFLAGS) 

src/horst.so: src/horst_python.cc $(HORST_HEADERS)
	g++ -shared -o $@ $(CXXFLAGS) $(PYTHON_CXXFLAGS) $< $(LDFLAGS) $(PYTHON_LDFLAGS)

src/horst_internal.so: src/horst_internal.cc $(HORST_HEADERS)
	g++ -shared -o $@ $(CXXFLAGS) $< $(LDFLAGS)

clean:
	rm -f src/*.o src/horst_cli src/horst.so src/horst_internal.so
