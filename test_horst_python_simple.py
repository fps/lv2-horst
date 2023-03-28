import horsting as h
import time 

uri = "http://calf.sourceforge.net/plugins/VintageDelay"

p = h.lv2(uri)

p._feedback.value = 0.9
p._amount.value = 1
p._medium.value = 2

p._feedback.bind_midi(0, 0)

# h.connect([
#   ('system:capture_1', p._in_l),
#   ('system:capture_2', p._in_r),
#   (p._out_l, 'system:playback_1'),
#   (p._out_r, 'system:playback_2')])

h.connect([(h.system, p), (p, h.system)])

while True:
    print(f'{p._meter_outL.value:.5f} {p._meter_outR.value:.5f}')
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

