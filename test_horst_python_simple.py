import horsting as h
import time 

uri = "http://calf.sourceforge.net/plugins/VintageDelay"

p = h.lv2(uri, "p", False)

p.port_feedback.value = 0.9
p.port_amount = 1
p.port_medium = 2

b = h.midi_binding(True, 0, 0)
p.set_midi_binding(p.port_feedback.index, b)

cs = h.connections()
cs.add("system:capture_1", "p:in_l")
cs.add("system:capture_2", "p:in_r")
cs.add("p:out_l", "system:playback_1")
cs.add("p:out_r", "system:playback_2")
h.connect(cs)

while True:
  print(p.port_meter_outL.value)
  time.sleep(0.1)

