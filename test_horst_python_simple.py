import horst
import time 
import subprocess


h = horst.horst()

cs = horst.connections()

uri = "http://calf.sourceforge.net/plugins/VintageDelay"

h.insert_lv2_plugin(0, uri, "reverb", False)

h.set_control_port_value(0, 18, 0.9)
h.set_control_port_value(0, 19, 1)
h.set_control_port_value(0, 21, 2)

cs.add("system:capture_1", "reverb:in_l")
cs.add("system:capture_2", "reverb:in_r")
cs.add("reverb:out_l", "system:playback_1")
cs.add("reverb:out_r", "system:playback_2")

h.connect(cs)
