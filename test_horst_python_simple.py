import horst
import time 

h = horst.horst()

uri = "http://calf.sourceforge.net/plugins/VintageDelay"

p = h.lv2_unit(uri, "reverb", False)

h.set_control_port_value(p, "feedback", 0.9)
h.set_control_port_value(p, "amount", 1)
h.set_control_port_value(p, "medium", 2)

cs = horst.connections()
cs.add("system:capture_1", "reverb:in_l")
cs.add("system:capture_2", "reverb:in_r")
cs.add("reverb:out_l", "system:playback_1")
cs.add("reverb:out_r", "system:playback_2")
h.connect(cs)

while True:
  print(h.get_control_port_value(p, 7))
  time.sleep(0.1)

