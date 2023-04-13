# lv2-horst

A simple plugin host exposing plugins as jack clients. Features python bindings.

# Examples

You can find examples of using lv2-horst in the examples directory.

```
import horsting as h                                                                                                                   │jack DSP load 0.150037
import time                                                                                                                            │jack DSP load 0.150037
                                                                                                                                       │jack DSP load 0.150037
# The plugin's URI                                                                                                                     │jack DSP load 0.150037
uri = "http://calf.sourceforge.net/plugins/VintageDelay"                                                                               │jack DSP load 0.150037
                                                                                                                                       │jack DSP load 0.150037
# Let's instantiate and run the plugin                                                                                                 │jack DSP load 0.150037
p = h.lv2(uri)                                                                                                                         │jack DSP load 0.150037
                                                                                                                                       │jack DSP load 0.150037
# Set some port values. Note how each port symbol has a "_"-suffix.                                                                    │jack DSP load 0.150037
# This is done to avoid clashes with other identifiers.                                                                                │jack DSP load 0.150037
p.feedback_.value = 0.9                                                                                                                │jack DSP load 0.150037
p.amount_.value = 1                                                                                                                    │jack DSP load 0.150037
p.medium_.value = 2                                                                                                                    │jack DSP load 0.150037
                                                                                                                                       │jack DSP load 0.150037
# Connect it up to the system ports                                                                                                    │jack DSP load 0.150037
h.connect(h.system, p, h.system)                                                                                                       │jack DSP load 0.150037
                                                                                                                                       │jack DSP load 0.150037
# And repeatedly print out some output port values                                                                                     │jack DSP load 0.159414
while True:                                                                                                                            │jack DSP load 0.150037
    print(f'{p.meter_outL_.value:.5f} {p.meter_outR_.value:.5f}')                                                                      │jack DSP load 0.150037
    time.sleep(0.1)                                                
```
