import horst
import time 

h = horst.horst()

uri = "http://calf.sourceforge.net/plugins/VintageDelay"

uid = h.insert_lv2_plugin(0, uri, "reverb", False)

h.set_control_port_value(uid, 18, 0.9)
h.set_control_port_value(uid,  19, 1)
h.set_control_port_value(uid, 21, 2)

cs = horst.connections()
cs.add("system:capture_1", "reverb:in_l")
cs.add("system:capture_2", "reverb:in_r")
cs.add("reverb:out_l", "system:playback_1")
cs.add("reverb:out_r", "system:playback_2")
h.connect(cs)

while True:
  print(h.get_control_port_value(uid, 7))
  time.sleep(0.1)

