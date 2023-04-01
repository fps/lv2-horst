.PHONY: all clean

all: src/horst.so src
# all: src/horst.so src/horst_internal.so

HORST_HEADERS = $(wildcard src/include/horst/*.h) $(wildcard src/include/horst/lart/*.h) $(wildcard src/include/horst/ladspa/*.h)
HORST_SOURCES = ${wildcard src/*.cc}
HORST_OBJECTS = ${HORST_SOURCES:.cc=.o}

# OPTIMIZATION_FLAGS = -O0 -g
OPTIMIZATION_FLAGS = -O3

PYTHON_CXXFLAGS = `python3 -m pybind11 --includes` `python3-config --cflags`  
PYTHON_LDFLAGS = `python3-config --ldflags --embed` 

CXXFLAGS += $(OPTIMIZATION_FLAGS) -fPIC -std=c++20 -Isrc/include -march=native -mcpu=native -Wall -pedantic `pkg-config lilv-0 --cflags` `pkg-config lv2 --cflags` -pthread
LDFLAGS += `pkg-config lilv-0 --libs` -ljack -latomic -pthread

src/horst.so: src/horst_python.cc $(HORST_HEADERS)
	g++ -shared -o $@ $(CXXFLAGS) $(PYTHON_CXXFLAGS) $< $(LDFLAGS) $(PYTHON_LDFLAGS)

src/horst_internal.so: src/horst_internal.cc $(HORST_HEADERS)
	g++ -shared -o $@ $(CXXFLAGS) $< $(LDFLAGS)

clean:
	rm -f src/*.o src/horst.so src/horst_internal.so
