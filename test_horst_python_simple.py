import horsting as h
import time 

uri = "http://calf.sourceforge.net/plugins/VintageDelay"

p = h.lv2(uri)

p.port_feedback.value = 0.9
p.port_amount.value = 1
p.port_medium.value = 2

p.port_feedback.bind_midi(0, 0)

h.connect([
  ('system:capture_1', p.port_in_l),
  ('system:capture_2', p.port_in_r),
  (p.port_out_l, 'system:playback_1'),
  (p.port_out_r, 'system:playback_2')])

while True:
  print(p.port_meter_outL.value)
  time.sleep(0.1)

