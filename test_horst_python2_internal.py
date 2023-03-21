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

for index in range(1,number_of_units):
    subprocess.check_call (["jack_connect", str(index)+"_"+uri+":"+"out_l", str(index-1)+"_"+uri+":"+"in_l"])

subprocess.check_call (["jack_connect", str(number_of_units-1)+"_"+uri+":"+"out_l", "system:playback_1"])
subprocess.check_call (["jack_connect", str(0)+"_"+uri+":"+"in_l", "system:capture_1"])
