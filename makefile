.PHONY: all clean

all: src/horst.so src/horst_cli src/test_horst src/test_horst2
# all: src/horst.so src/horst_internal.so

HORST_HEADERS = $(wildcard src/include/horst/*.h) $(wildcard src/include/horst/lart/*.h) $(wildcard src/include/horst/ladspa/*.h)
HORST_SOURCES = ${wildcard src/*.cc}
HORST_OBJECTS = ${HORST_SOURCES:.cc=.o}

# OPTIMIZATION_FLAGS = -O2
OPTIMIZATION_FLAGS = -O3 -mtune=native -march=native -mcpu=native 

ifeq ($(DEBUG),1)
OPTIMIZATION_FLAGS = -O1 -g -pg -DHORST_DEBUG -fstack-protector -fstack-protector-strong -fstack-protector-all -fstack-protector-explicit
endif

PYTHON_CXXFLAGS = `python3 -m pybind11 --includes` `python3-config --cflags`  
PYTHON_LDFLAGS = `python3-config --ldflags --embed` 

CXXFLAGS += -fPIC -std=c++20 -Isrc/include -Wall -pedantic `pkg-config lilv-0 --cflags` `pkg-config lv2 --cflags` -pthread $(OPTIMIZATION_FLAGS) 
LDFLAGS += `pkg-config lilv-0 --libs` -ljack -latomic -pthread

src/test_horst: src/test_horst.cc $(HORST_HEADERS) makefile
	g++ -o $@ $(CXXFLAGS) $(PYTHON_CXXFLAGS) $< $(LDFLAGS) $(PYTHON_LDFLAGS)

src/test_horst2: src/test_horst2.cc $(HORST_HEADERS) makefile
	g++ -o $@ $(CXXFLAGS) $(PYTHON_CXXFLAGS) $< $(LDFLAGS) $(PYTHON_LDFLAGS)

src/horst_cli: src/horst_cli.cc $(HORST_HEADERS) makefile
	g++ -o $@ $(CXXFLAGS) $(PYTHON_CXXFLAGS) $< $(LDFLAGS) $(PYTHON_LDFLAGS)

src/horst.so: src/horst_python.cc $(HORST_HEADERS) makefile
	g++ -shared -o $@ $(CXXFLAGS) $(PYTHON_CXXFLAGS) $< $(LDFLAGS) $(PYTHON_LDFLAGS)

src/horst_internal.so: src/horst_internal.cc $(HORST_HEADERS) makefile
	g++ -shared -o $@ $(CXXFLAGS) $< $(LDFLAGS)

clean:
	rm -f src/*.o src/horst.so src/horst_internal.so
