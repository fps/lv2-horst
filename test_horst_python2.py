import horst
import time 
import subprocess


h = horst.horst()

cs = horst.connections()

uri = "http://calf.sourceforge.net/plugins/Gate"

number_of_units = 5
index = 0
for plugin_index in range(0,number_of_units):
  try:
    h.insert_lv2_plugin(0, uri, str(index)+"_"+uri, False)
    cs.add(str(index)+"_"+uri+":"+"out_l", str(index-1)+"_"+uri+":"+"in_l")
  except Exception as e:
    print ("Failed to insert plugin " + uri + ": " + str(e))
  index += 1

for plugin_index in range(0,number_of_units):
  try:
    h.insert_lv2_plugin(0, uri, str(index)+"_"+uri, True)
    cs.add(str(index)+"_"+uri+":"+"out_l", str(index-1)+"_"+uri+":"+"in_l")
  except Exception as e:
    print ("Failed to insert plugin " + uri + ": " + str(e))
  index += 1

for plugin_index in range(0,number_of_units):
  try:
    h.insert_lv2_plugin_internal(0, uri, str(index)+"_"+uri, False)
    cs.add(str(index)+"_"+uri+":"+"out_l", str(index-1)+"_"+uri+":"+"in_l")
  except Exception as e:
    print ("Failed to insert plugin " + uri + ": " + str(e))
  index += 1

for plugin_index in range(0,number_of_units):
  try:
    h.insert_lv2_plugin_internal(0, uri, str(index)+"_"+uri, True)
    cs.add(str(index)+"_"+uri+":"+"out_l", str(index-1)+"_"+uri+":"+"in_l")
  except Exception as e:
    print ("Failed to insert plugin " + uri + ": " + str(e))
  index += 1

h.connect(cs)
