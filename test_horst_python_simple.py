import horsting as h
import time 

uri = "http://calf.sourceforge.net/plugins/VintageDelay"

p = h.lv2(uri)

p.feedback_.value = 0.9
p.amount_.value = 1
p.medium_.value = 2

h.connect(h.system, p, h.system)

while True:
    print(f'{p.meter_outL_.value:.5f} {p.meter_outR_.value:.5f}')
    time.sleep(0.1)
