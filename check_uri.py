import horsting as h
import sys
import time

print (sys.argv[1])

try:
  p = h.lv2(sys.argv[1], sys.argv[1][-30:])
  # time.sleep(1)
  p = None
except Exception as e:
  print(e)

print ("done")
