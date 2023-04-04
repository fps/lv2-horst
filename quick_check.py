import horsting as h
import sys
import time

p = h.lv2(sys.argv[1], sys.argv[1][-30:])

h.connect(h.system, p, h.system)
