import horsting as h
import time 

# The plugin's URI
uri = "http://calf.sourceforge.net/plugins/VintageDelay"

# Let's instantiate and run the plugin
p = h.lv2(uri)
p.set_control_output_updates_enabled(True)

# Set some port values. Note how each port symbol has a "_"-suffix.
# This is done to avoid clashes with other identifiers.
p.feedback_.value = 0.9
p.amount_.value = 1
p.medium_.value = 2

# Connect it up to the system ports
h.connect(h.system, p, h.system)

# And repeatedly print out some output port values
while True:
    print(f'{p.meter_outL_.value:.5f} {p.meter_outR_.value:.5f}')
    time.sleep(0.1)
