import horsting as h
import sys
import time

print (sys.argv[1])

try:
  p = h.lv2(sys.argv[1], "test-plugin")
  time.sleep(0.1)
except Exception as e:
  print(e)