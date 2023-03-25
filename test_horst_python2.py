import horst
import time 
import subprocess


h = horst.horst()

cs = horst.connections()

uri = "http://calf.sourceforge.net/plugins/Reverb"

units = []

number_of_units = 5
index = 0
cs.add("system:capture_1", str(index)+"_"+uri+":"+"in_l")
for plugin_index in range(0,number_of_units):
  try:
    units.append(h.lv2(uri, str(index)+"_"+uri, False))
    if plugin_index > 0:
      cs.add(str(index)+"_"+uri+":"+"out_l", str(index-1)+"_"+uri+":"+"in_l")
  except Exception as e:
    print ("Failed to insert plugin " + uri + ": " + str(e))
  index += 1
cs.add(str(index-1)+"_"+uri+":"+"out_l", "system:playback_1")

for plugin_index in range(0,number_of_units):
  try:
    units.append(h.lv2(uri, str(index)+"_"+uri, True))
    if plugin_index > 0:
      cs.add(str(index)+"_"+uri+":"+"out_l", str(index-1)+"_"+uri+":"+"in_l")
  except Exception as e:
    print ("Failed to insert plugin " + uri + ": " + str(e))
  index += 1

for plugin_index in range(0,number_of_units):
  try:
    units.append(h.lv2_internal(uri, str(index)+"_"+uri, False))
    if plugin_index > 0:
      cs.add(str(index)+"_"+uri+":"+"out_l", str(index-1)+"_"+uri+":"+"in_l")
  except Exception as e:
    print ("Failed to insert plugin " + uri + ": " + str(e))
  index += 1

for plugin_index in range(0,number_of_units):
  try:
    units.append(h.lv2_internal(uri, str(index)+"_"+uri, True))
    if plugin_index > 0:
      cs.add(str(index)+"_"+uri+":"+"out_l", str(index-1)+"_"+uri+":"+"in_l")
  except Exception as e:
    print ("Failed to insert plugin " + uri + ": " + str(e))
  index += 1

h.connect(cs)
