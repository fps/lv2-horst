import horst
import time 

h = horst.horst()

uri = "http://calf.sourceforge.net/plugins/VintageDelay"

p = h.lv2_unit(uri, "delay", False)

def idx(h, p, name):
  return h.get_control_port_index(p, name)

h.set_control_port_value(p, idx(h, p, "feedback"), 0.9)
h.set_control_port_value(p, idx(h, p, "amount"), 1)
h.set_control_port_value(p, idx(h, p, "medium"), 2)

b = horst.midi_binding(True, 0, 0)
h.set_midi_binding(p, idx(h, p, "feedback"), b)

cs = horst.connections()
cs.add("system:capture_1", "delay:in_l")
cs.add("system:capture_2", "delay:in_r")
cs.add("delay:out_l", "system:playback_1")
cs.add("delay:out_r", "system:playback_2")
h.connect(cs)

while True:
  print(h.get_control_port_value(p, 7))
  time.sleep(0.1)

