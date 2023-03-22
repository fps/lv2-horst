import horst
import time 
import subprocess


h = horst.horst()

uri = "http://calf.sourceforge.net/plugins/Gate"

number_of_units = 20

for index in range(0,number_of_units):
  try:
    h.insert_lv2_plugin_internal(0, uri, str(index)+"_"+uri)
  except Exception as e:
    print ("Failed to insert plugin " + uri + ": " + str(e))

cs = horst.connections()
for index in range(1,number_of_units):
    cs.add(horst.connection(str(index)+"_"+uri+":"+"out_l", str(index-1)+"_"+uri+":"+"in_l"))

h.connect(cs)
