import horsting as h
import sys
import time

units = [h.lv2(uri) for uri in sys.argv[1:]]

h.connect(h.system, *units, h.system)
