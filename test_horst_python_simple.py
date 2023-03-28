import horsting as h
import time 

uri = "http://calf.sourceforge.net/plugins/VintageDelay"

p = h.lv2(uri)

p.feedback_.value = 0.9
p.amount_.value = 1
p.medium_.value = 2

p.feedback_.bind_midi(0, 0)

# h.connect([
#   ('system:capture_1', p._in_l),
#   ('system:capture_2', p._in_r),
#   (p._out_l, 'system:playback_1'),
#   (p._out_r, 'system:playback_2')])

# h.connect([(h.system, p), (p, h.system)])
h.connect(h.system, p, h.system)

while True:
    print(f'{p.meter_outL_.value:.5f} {p.meter_outR_.value:.5f}')
    time.sleep(0.1)
  
# while True:
#   p.set_enabled(True)
#   for n in range(30):
#     print(f'{p._meter_outL.value:.5f} {p._meter_outR.value:.5f}')
#     time.sleep(0.1)
#   p.set_enabled(False)
#   for n in range(30):
#     print(f'{p._meter_outL.value:.5f} {p._meter_outR.value:.5f}')
#     time.sleep(0.1)

